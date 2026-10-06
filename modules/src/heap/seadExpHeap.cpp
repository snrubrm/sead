#include <heap/seadExpHeap.h>
#include <heap/seadHeapMgr.h>
#include <math/seadMathCalcCommon.h>
#include <prim/seadPtrUtil.h>
#include <prim/seadScopedLock.h>

namespace sead
{
namespace
{
// sizeof(ExpHeap): the heap object is placed at the start (forward) or at the end (reverse) of its memory.
constexpr size_t cExpHeapObjectSize = 0x110;
}  // namespace

static_assert(sizeof(ExpHeap) == cExpHeapObjectSize, "sead::ExpHeap size mismatch");

ExpHeap* ExpHeap::create(size_t size, const SafeString& name, Heap* parent, s32 alignment,
                         HeapDirection direction, bool enable_lock)
{
    return tryCreate(size, name, parent, alignment, direction, enable_lock);
}

ExpHeap* ExpHeap::tryCreate(size_t size, const SafeString& name, Heap* parent, s32 alignment,
                            HeapDirection direction, bool enable_lock)
{
    if (!parent)
    {
        parent = HeapMgr::sInstancePtr->getCurrentHeap();
        if (!parent)
            return nullptr;
    }

    size = (size ? size + 7 : parent->getMaxAllocatableSize(alignment)) & ~size_t(7);
    if (size < cExpHeapObjectSize + sizeof(MemBlock) + 1)
        return nullptr;

    const u32 abs_alignment = Mathi::abs(alignment);
    if ((abs_alignment & (abs_alignment - 1)) != 0)
        return nullptr;

    void* memory = parent->tryAlloc(size, direction * alignment);
    if (!memory)
        return nullptr;

    const HeapDirection heap_direction =
        parent->mDirection == cHeapDirection_Reverse ? HeapDirection(-direction) : direction;
    ExpHeap* heap;
    if (heap_direction == cHeapDirection_Forward)
        heap = new (memory) ExpHeap(name, parent, memory, size, heap_direction, enable_lock);
    else
        heap = new (PtrUtil::addOffset(memory, size - cExpHeapObjectSize))
            ExpHeap(name, parent, memory, size, heap_direction, enable_lock);

    createMaxSizeFreeMemBlock_(heap);
    parent->pushBackChild_(heap);
    return heap;
}

ExpHeap* ExpHeap::tryCreate(void* address, size_t size, const SafeString& name, bool enable_lock)
{
    size &= ~size_t(7);
    if (size < cExpHeapObjectSize + sizeof(MemBlock) + 1)
        return nullptr;

    ExpHeap* heap = new (address) ExpHeap(name, nullptr, address, size, cHeapDirection_Forward, enable_lock);
    createMaxSizeFreeMemBlock_(heap);
    return heap;
}

void ExpHeap::createMaxSizeFreeMemBlock_(ExpHeap* heap)
{
    ConditionalScopedLock<CriticalSection> lock(&heap->mCS, heap->isLockEnabled());
    MemBlock* block;
    if (heap->mDirection == cHeapDirection_Forward)
        block = new (static_cast<void*>(heap + 1)) MemBlock();
    else
        block = new (heap->mStart) MemBlock();
    block->mSize = heap->mSize - (cExpHeapObjectSize + sizeof(MemBlock));
    block->mHeapCheckTag = 0xffff;
    heap->mFreeList.pushBack(block);
}

size_t ExpHeap::getFreeSize() const
{
    ConditionalScopedLock<CriticalSection> lock(const_cast<CriticalSection*>(&mCS), isLockEnabled());
    size_t total = 0;
    for (auto it = mFreeList.begin(); it != mFreeList.end(); ++it)
        total += it->mSize;
    return total;
}

size_t ExpHeap::getMaxAllocatableSize(int alignment) const
{
    const s32 abs_alignment = Mathi::abs(alignment);
    if ((abs_alignment & (abs_alignment - 1)) != 0)
        return 0;

    ConditionalScopedLock<CriticalSection> lock(const_cast<CriticalSection*>(&mCS), isLockEnabled());
    const MemBlock* best = nullptr;
    size_t padding = 0;
    auto it = mFreeList.begin();
    const auto end = mFreeList.end();
    if (abs_alignment <= 8)
    {
        for (; it != end; ++it)
        {
            if (it->mSize >= 8 && (!best || best->mSize < it->mSize))
                best = &*it;
        }
    }
    else
    {
        const uintptr_t mask = u32(abs_alignment - 1);
        for (; it != end; ++it)
        {
            if (it->mSize < 8)
                continue;
            const uintptr_t data = reinterpret_cast<uintptr_t>(&*it) + it->mOffset + sizeof(MemBlock);
            const size_t block_padding = ((data + mask) & ~mask) - data;
            if (it->mSize >= block_padding + 8 && (!best || best->mSize < it->mSize))
                best = &*it;
        }
        if (best)
        {
            const uintptr_t data = reinterpret_cast<uintptr_t>(best) + best->mOffset + sizeof(MemBlock);
            padding = ((data + mask) & ~mask) - data;
        }
    }
    return best ? best->mSize - padding : 0;
}

void ExpHeap::freeAll()
{
    ConditionalScopedLock<CriticalSection> lock(&mCS, isLockEnabled());
    dispose_(nullptr, nullptr);
    mUseList.clear();
    mFreeList.clear();
    createMaxSizeFreeMemBlock_(this);
}

ExpHeap::ExpHeap(const SafeString& name, Heap* parent, void* address, size_t size,
                 HeapDirection direction, bool enable_lock)
    : Heap(name, parent, address, size, direction, enable_lock), mAllocMode(AllocMode::FirstFit),
      mFindFreeBlockMode(FindFreeBlockMode::Auto)
{
    ConditionalScopedLock<CriticalSection> lock(&mCS, isLockEnabled());
    mFreeList.initOffset(MemBlock::getOffset());
    mUseList.initOffset(MemBlock::getOffset());
}

ExpHeap::~ExpHeap()
{
    destruct_();
}

size_t ExpHeap::getManagementAreaSize(s32 alignment)
{
    return alignment + cExpHeapObjectSize + sizeof(MemBlock);
}

void ExpHeap::destroy()
{
    destroyAndGetAllocatableSize(8);
}

uintptr_t ExpHeap::getStartAddress() const
{
    return reinterpret_cast<uintptr_t>(mStart);
}

uintptr_t ExpHeap::getEndAddress() const
{
    return reinterpret_cast<uintptr_t>(mStart) + mSize;
}

size_t ExpHeap::getSize() const
{
    return mSize;
}

bool ExpHeap::isInclude(const void* ptr) const
{
    const bool forward = mDirection == cHeapDirection_Forward;
    const void* end_base = forward ? mStart : PtrUtil::addOffset(mStart, -intptr_t(cExpHeapObjectSize));
    const void* start = forward ? PtrUtil::addOffset(mStart, cExpHeapObjectSize) : mStart;
    const void* end = PtrUtil::addOffset(end_base, mSize);
    return start <= ptr && ptr < end;
}

s32 ExpHeap::compareMemBlockAddr_(const MemBlock* a, const MemBlock* b)
{
    return (s32(u32(uintptr_t(a)) - u32(uintptr_t(b))) >> 31) | 1;
}

void ExpHeap::setFindFreeBlockMode(FindFreeBlockMode mode)
{
    mFindFreeBlockMode = mode;
}

void ExpHeap::genInformation_(hostio::Context* context)
{
    Heap::genInformation_(context);
}

bool ExpHeap::isEmpty() const
{
    return this->mUseList.size() == 0;
}

bool ExpHeap::isFreeable() const
{
    return true;
}

bool ExpHeap::isResizable() const
{
    return true;
}

bool ExpHeap::isAdjustable() const
{
    return true;
}
}  // namespace sead