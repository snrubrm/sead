#include <heap/seadHeapMgr.h>
#include <heap/seadUnitHeap.h>
#include <math/seadMathCalcCommon.h>
#include <prim/seadScopedLock.h>

namespace sead
{
namespace
{
constexpr size_t cUnitHeapObjectSize = 0x108;

void callAllocFailedCallback(HeapMgr* mgr, Heap* heap, size_t size, s32 alignment,
                             size_t alloc_size, s32 alloc_alignment)
{
    if (!mgr)
        return;
    if (auto* callback = mgr->getAllocFailedCallback())
    {
        HeapMgr::AllocFailedCallbackArg arg{heap, size, alignment, alloc_size, alloc_alignment};
        callback->invoke(&arg);
    }
}
}  // namespace

UnitHeap* UnitHeap::create(size_t size, const SafeString& name, u32 unit_size, s32 alignment,
                           Heap* parent, bool enable_lock)
{
    return tryCreate(size, name, unit_size, alignment, parent, enable_lock);
}

UnitHeap::UnitHeap(const SafeString& name, Heap* parent, void* address, size_t size, u32 unit_size,
                   bool enable_lock)
    : Heap(name, parent, address, size, cHeapDirection_Forward, enable_lock),
      mUnitSize(unit_size), mUnitAreaStart(nullptr), mUnitAreaSize(0), mFreeSize(0),
      mFreeList(nullptr), _100(nullptr)
{
}

UnitHeap::~UnitHeap()
{
    destruct_();
}

UnitHeap* UnitHeap::tryCreate(size_t size, const SafeString& name, u32 unit_size, s32 alignment,
                              Heap* parent, bool enable_lock)
{
    const s32 abs_alignment = alignment < 0 ? -alignment : alignment;
    if (unit_size == 0)
        return nullptr;

    const s32 unit_alignment = abs_alignment > 8 ? abs_alignment : 8;
    if ((unit_alignment & (unit_alignment - 1)) != 0)
        return nullptr;

    if (!parent)
    {
        parent = HeapMgr::sInstancePtr->getCurrentHeap();
        if (!parent)
            return nullptr;
    }

    size = (size ? size + 7 : parent->getMaxAllocatableSize(unit_alignment)) & ~size_t(7);
    if (size < ((unit_alignment - 1 + size_t(unit_size)) & s64(-unit_alignment)) + cUnitHeapObjectSize)
        return nullptr;

    void* memory = parent->tryAlloc(size, 8);
    if (!memory)
        return nullptr;

    UnitHeap* heap = new (memory) UnitHeap(name, parent, memory, size, unit_size, enable_lock);
    heap->initialize_(unit_alignment, false, parent);
    return heap;
}

void UnitHeap::initFreeList_()
{
    void** start = static_cast<void**>(mUnitAreaStart);
    const s32 count = mUnitAreaSize / mUnitSize;
    const s32 words = s32(mUnitSize) / 8;
    mFreeSize = u32(count) * mUnitSize;
    mFreeList = start;
    const s32 last = count - 1;
    for (s64 i = 0; i < last; ++i)
        start[i * words] = &start[(i + 1) * words];
    start[last * words] = nullptr;
    _100 = start;
}

// NON_MATCHING: the free list loop is strength-reduced here while the original keeps the index multiplications (unrolled by two).
void UnitHeap::initialize_(s32 alignment, bool unk, Heap* parent)
{
    ConditionalScopedLock<CriticalSection> lock(&mCS, isLockEnabled());
    const u32 mask = alignment - 1;
    const uintptr_t base = reinterpret_cast<uintptr_t>(this) + cUnitHeapObjectSize;
    uintptr_t start = (base + mask) & ~uintptr_t(mask);
    if (start == base && unk)
        start += alignment;
    mUnitSize = (mask + mUnitSize) & u32(-alignment);
    mUnitAreaStart = reinterpret_cast<void*>(start);
    mUnitAreaSize = reinterpret_cast<uintptr_t>(this) - start + mSize;
    initFreeList_();
    parent->pushBackChild_(this);
}

void UnitHeap::destroy()
{
    Heap* parent = mParent;
    void* start = mStart;
    this->~UnitHeap();
    if (parent && parent->isFreeable())
        parent->free(start);
}

size_t UnitHeap::adjust()
{
    return mSize;
}

// NON_MATCHING: register numbering of the heap and the failed-callback argument setup differ.
void* UnitHeap::tryAlloc(size_t size, s32 alignment)
{
    HeapMgr* heap_mgr = HeapMgr::sInstancePtr;
    if (alignment < 0 || mUnitSize < size || (mUnitSize & (size_t(u32(alignment)) - 1)) != 0)
    {
        callAllocFailedCallback(heap_mgr, this, size, alignment, size, alignment);
        return nullptr;
    }

    ConditionalScopedLock<CriticalSection> lock(&mCS, isLockEnabled());
    void** unit = mFreeList;
    if (!unit)
    {
        callAllocFailedCallback(heap_mgr, this, size, alignment, mUnitSize, alignment);
        return nullptr;
    }
    mFreeList = static_cast<void**>(*unit);
    mFreeSize -= mUnitSize;
    return unit;
}

// NON_MATCHING: the pointer and this are in swapped callee-saved registers.
void UnitHeap::free(void* ptr)
{
    if (!ptr)
        return;
    if (!isInclude(ptr) || mFlag.isOnBit(Flag::cDisposing))
        return;

    ConditionalScopedLock<CriticalSection> lock(&mCS, isLockEnabled());
    *static_cast<void**>(ptr) = mFreeList;
    mFreeSize += mUnitSize;
    mFreeList = static_cast<void**>(ptr);
}

void* UnitHeap::resizeFront(void*, size_t)
{
    return nullptr;
}

void* UnitHeap::resizeBack(void*, size_t)
{
    return nullptr;
}

// NON_MATCHING: same free list loop as initialize_ (strength-reduced instead of unrolled).
void UnitHeap::freeAll()
{
    ConditionalScopedLock<CriticalSection> lock(&mCS, isLockEnabled());
    dispose_(nullptr, nullptr);
    initFreeList_();
}

uintptr_t UnitHeap::getStartAddress() const
{
    return reinterpret_cast<uintptr_t>(mStart);
}

uintptr_t UnitHeap::getEndAddress() const
{
    return reinterpret_cast<uintptr_t>(mStart) + mSize;
}

size_t UnitHeap::getSize() const
{
    return mSize;
}

size_t UnitHeap::getFreeSize() const
{
    return mFreeSize;
}

size_t UnitHeap::getMaxAllocatableSize(int) const
{
    return mFreeList ? mUnitSize : 0;
}

// NON_MATCHING: the end address is computed as size + start instead of start + size.
bool UnitHeap::isInclude(const void* ptr) const
{
    const uintptr_t start = reinterpret_cast<uintptr_t>(mUnitAreaStart);
    const uintptr_t end = mUnitAreaSize + start;
    const uintptr_t address = reinterpret_cast<uintptr_t>(ptr);
    return (end > address) & (start <= address);
}

bool UnitHeap::isEmpty() const
{
    const size_t area_size = mUnitAreaSize;
    const size_t free_size = mFreeSize;
    const u32 count = area_size / mUnitSize;
    return free_size == count * mUnitSize;
}

bool UnitHeap::isFreeable() const
{
    return true;
}

bool UnitHeap::isResizable() const
{
    return false;
}

bool UnitHeap::isAdjustable() const
{
    return false;
}

void UnitHeap::dump() const
{
    ConditionalScopedLock<CriticalSection> lock(const_cast<CriticalSection*>(&mCS),
                                                isLockEnabled());
}

void UnitHeap::genInformation_(hostio::Context*) {}

}  // namespace sead
