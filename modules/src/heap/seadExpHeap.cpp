#include <heap/seadExpHeap.h>
#include <heap/seadHeapMgr.h>
#include <math/seadMathCalcCommon.h>
#include <prim/seadPtrUtil.h>
#include <prim/seadScopedLock.h>
#include <string.h>
#include <atomic>

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

ExpHeap* ExpHeap::tryCreate(void* address, size_t size, const SafeString& name, Heap* parent,
                            bool enable_lock)
{
    return sub_7100B04A2C(address, size, name, parent, enable_lock);
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

namespace
{
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

void* ExpHeap::tryAlloc(size_t size, s32 alignment)
{
    HeapMgr* heap_mgr = HeapMgr::sInstancePtr;
    size_t alloc_size = size > 8 ? size : 8;
    const s32 abs_alignment = Mathi::abs(alignment);
    if ((abs_alignment & (abs_alignment - 1)) != 0)
    {
        callAllocFailedCallback(heap_mgr, this, size, alignment, alloc_size, alignment);
        return nullptr;
    }

    alloc_size = (alloc_size + 7) & ~size_t(7);
    if (alloc_size < size)
    {
        callAllocFailedCallback(heap_mgr, this, size, alignment, alloc_size, alignment);
        return nullptr;
    }

    ConditionalScopedLock<CriticalSection> lock(&mCS, isLockEnabled());

    s32 real_alignment = mDirection * alignment;
    MemBlock* block;
    if (real_alignment >= 0)
    {
        if (real_alignment <= 8)
            block = allocFromHead_(alloc_size);
        else
            block = allocFromHead_(alloc_size, real_alignment);
    }
    else
    {
        real_alignment = -real_alignment;
        if (real_alignment <= 8)
            block = allocFromTail_(alloc_size);
        else
            block = allocFromTail_(alloc_size, real_alignment);
    }

    if (!block)
    {
        callAllocFailedCallback(heap_mgr, this, size, alignment, alloc_size, real_alignment);
        return nullptr;
    }

    block->mHeapCheckTag = mHeapCheckTag;
    return reinterpret_cast<u8*>(block) + block->mOffset + sizeof(MemBlock);
}

inline MemBlock* ExpHeap::findFreeMemBlockFromHead_(size_t size, FindMode mode) const
{
    MemBlock* found = nullptr;
    for (auto it = mFreeList.begin(); it != mFreeList.end(); ++it)
    {
        if (it->mSize < size)
            continue;
        if (mode == FindMode::FirstFit)
        {
            found = &*it;
            break;
        }
        if (!found || (mode == FindMode::BestFit && found->mSize > it->mSize) ||
            (mode == FindMode::LargestFit && found->mSize < it->mSize))
            found = &*it;
    }
    return found;
}

MemBlock* ExpHeap::findFreeMemBlockFromHead_(size_t size, s32 alignment, FindMode mode) const
{
    MemBlock* found = nullptr;
    for (auto it = mFreeList.begin(); it != mFreeList.end(); ++it)
    {
        const uintptr_t mask = u32(alignment - 1);
        if (it->mSize < size)
            continue;
        const uintptr_t data = reinterpret_cast<uintptr_t>(&*it) + it->mOffset + sizeof(MemBlock);
        const size_t padding = ((data + mask) & ~mask) - data;
        if (it->mSize < size + padding)
            continue;
        if (mode == FindMode::FirstFit)
        {
            found = &*it;
            break;
        }
        if (!found || (mode == FindMode::BestFit && found->mSize > it->mSize) ||
            (mode == FindMode::LargestFit && found->mSize < it->mSize))
            found = &*it;
    }
    return found;
}

MemBlock* ExpHeap::findFreeMemBlockFromTail_(size_t size, FindMode mode) const
{
    MemBlock* found = nullptr;
    for (MemBlock* block = mFreeList.back(); block; block = mFreeList.prev(block))
    {
        if (block->mSize < size)
            continue;
        if (mode == FindMode::FirstFit)
            return block;
        if (!found || (mode == FindMode::BestFit && found->mSize > block->mSize) ||
            (mode == FindMode::LargestFit && found->mSize < block->mSize))
            found = block;
    }
    return found;
}

// NON_MATCHING: same instructions and structure, but the loop-invariant (0x20 - size) / (alignment - 1) values end up in swapped registers.
MemBlock* ExpHeap::findFreeMemBlockFromTail_(size_t size, s32 alignment, FindMode mode) const
{
    MemBlock* found = nullptr;
    for (MemBlock* block = mFreeList.back(); block; block = mFreeList.prev(block))
    {
        const size_t front_size = sizeof(MemBlock) - size;
        const uintptr_t mask = u32(alignment - 1);
        if (block->mSize < size)
            continue;
        const u32 start =
            u32(front_size + reinterpret_cast<uintptr_t>(block) + block->mSize + block->mOffset);
        const size_t padding = start & mask;
        if (block->mSize < size + padding)
            continue;
        if (mode == FindMode::FirstFit)
            return block;
        if (!found || (mode == FindMode::BestFit && found->mSize > block->mSize) ||
            (mode == FindMode::LargestFit && found->mSize < block->mSize))
            found = block;
    }
    return found;
}

MemBlock* ExpHeap::allocFromHead_(size_t size)
{
    MemBlock* block = findFreeMemBlockFromHead_(size, getFindMode_());
    if (!block)
        return nullptr;

    const size_t block_size = block->mSize;
    block->mSize = size;
    const size_t offset = block->mOffset;
    const size_t remaining = block_size - size;
    MemBlock* next = mFreeList.next(block);
    mFreeList.erase(block);
    pushToUseList_(block);

    if (remaining > sizeof(MemBlock))
    {
        auto* free_block =
            new (reinterpret_cast<void*>(size + reinterpret_cast<uintptr_t>(block) + offset +
                                         sizeof(MemBlock))) MemBlock();
        free_block->mSize = remaining - sizeof(MemBlock);
        free_block->mHeapCheckTag = 0xffff;
        if (next)
            mFreeList.insertBefore(next, free_block);
        else
            mFreeList.pushBack(free_block);
    }
    else if (remaining != 0)
    {
        block->mSize = block_size;
    }
    return block;
}

// NON_MATCHING: same instruction stream apart from callee-saved register numbering and the position of the block size load.
MemBlock* ExpHeap::allocFromHead_(size_t size, s32 alignment)
{
    MemBlock* block = findFreeMemBlockFromHead_(size, alignment, getFindMode_());
    if (!block)
        return nullptr;

    MemBlock* next = mFreeList.next(block);
    const uintptr_t mask = u32(alignment - 1);
    const uintptr_t data = reinterpret_cast<uintptr_t>(block) + block->mOffset + sizeof(MemBlock);
    const size_t padding = ((data + mask) & ~mask) - data;
    const size_t leftover = block->mSize - size;

    if (padding >= 0x10000)
    {
        // The padding does not fit the offset: leave it as its own free block.
        block->mOffset = 0;
        block->mSize = padding - sizeof(MemBlock);
        block = new (reinterpret_cast<void*>(padding + reinterpret_cast<uintptr_t>(block))) MemBlock();
        block->mOffset = 0;
        block->mSize = size;
    }
    else
    {
        const u16 offset = padding;
        block->mOffset = offset;
        if (offset)
            *reinterpret_cast<uintptr_t*>(reinterpret_cast<uintptr_t>(block) + sizeof(MemBlock) +
                                          block->mOffset - sizeof(uintptr_t)) =
                reinterpret_cast<uintptr_t>(block) + 1;
        block->mSize = size;
        mFreeList.erase(block);
    }

    pushToUseList_(block);

    const size_t remaining = leftover - padding;
    if (remaining > sizeof(MemBlock))
    {
        auto* free_block =
            new (reinterpret_cast<u8*>(block) + size + block->mOffset + sizeof(MemBlock)) MemBlock();
        free_block->mSize = remaining - sizeof(MemBlock);
        free_block->mHeapCheckTag = 0xffff;
        if (next)
            mFreeList.insertBefore(next, free_block);
        else
            mFreeList.pushBack(free_block);
    }
    else if (remaining != 0)
    {
        block->mSize = remaining + size;
    }
    return block;
}

MemBlock* ExpHeap::allocFromTail_(size_t size)
{
    MemBlock* block = findFreeMemBlockFromTail_(size, getFindMode_());
    if (!block)
        return nullptr;

    const size_t remaining = block->mSize - size;
    if (remaining > sizeof(MemBlock))
    {
        block->mSize = remaining - sizeof(MemBlock);
        block = new (reinterpret_cast<u8*>(block) + remaining + block->mOffset) MemBlock();
        block->mSize = size;
        pushToUseList_(block);
    }
    else
    {
        mFreeList.erase(block);
        pushToUseList_(block);
    }
    return block;
}

MemBlock* ExpHeap::allocFromTail_(size_t size, s32 alignment)
{
    MemBlock* block = findFreeMemBlockFromTail_(size, alignment, getFindMode_());
    if (!block)
        return nullptr;

    const uintptr_t data = reinterpret_cast<uintptr_t>(block) + sizeof(MemBlock);
    const u32 start = u32(data - size + block->mOffset + block->mSize);
    const uintptr_t mask = u32(alignment - 1);
    const size_t alloc_size = (start & mask) + size;
    const size_t remaining = block->mSize - alloc_size;
    if (remaining > sizeof(MemBlock))
    {
        block->mSize = remaining - sizeof(MemBlock);
        block = new (reinterpret_cast<void*>(remaining + reinterpret_cast<uintptr_t>(block) +
                                             block->mOffset)) MemBlock();
        block->mSize = alloc_size;
        pushToUseList_(block);
    }
    else
    {
        const u16 offset = remaining;
        block->mOffset = offset;
        if (offset)
            reinterpret_cast<uintptr_t*>(data + block->mOffset)[-1] =
                reinterpret_cast<uintptr_t>(block) + 1;
        block->mSize = alloc_size;
        mFreeList.erase(block);
        pushToUseList_(block);
    }
    return block;
}

inline void ExpHeap::checkUseList() const
{
    ConditionalScopedLock<CriticalSection> lock(const_cast<CriticalSection*>(&mCS), isLockEnabled());
    for (auto it = mUseList.begin(); it != mUseList.end(); ++it)
    {
    }
}

inline void ExpHeap::checkFreeList() const
{
    ConditionalScopedLock<CriticalSection> lock(const_cast<CriticalSection*>(&mCS), isLockEnabled());
    for (auto it = mFreeList.begin(); it != mFreeList.end(); ++it)
    {
    }
}

void ExpHeap::free(void* ptr)
{
    freeAndGetAllocatableSize(ptr, 8);
}

size_t ExpHeap::freeAndGetAllocatableSize(void* ptr, s32 alignment)
{
    if (!ptr || !isInclude(ptr) || mFlag.isOnBit(Flag::cDisposing))
        return 0;

    ConditionalScopedLock<CriticalSection> lock(&mCS, isLockEnabled());

    MemBlock* block = MemBlock::FindManageArea(ptr);
    if (block && block->mHeapCheckTag == mHeapCheckTag)
    {
        mUseList.erase(block);
        const size_t size = block->mSize + block->mOffset;
        block->mOffset = 0;
        block->mSize = size;

        const MemBlock* merged = pushToFreeList_(block);
        const s32 abs_alignment = Mathi::abs(alignment);
        size_t allocatable = merged->mSize;
        if (abs_alignment > 8)
        {
            const uintptr_t data =
                reinterpret_cast<uintptr_t>(merged) + merged->mOffset + sizeof(MemBlock);
            const uintptr_t mask = u32(abs_alignment - 1);
            const uintptr_t aligned = (data + mask) & ~mask;
            allocatable = data + allocatable - aligned;
        }
        return allocatable;
    }

    if (!block)
    {
        checkUseList();
        checkFreeList();
    }
    return 0;
}

size_t ExpHeap::getAllocatedSize(void* object)
{
    if (!isInclude(object))
        return 0;
    return MemBlock::FindManageArea(object)->mSize;
}

// NON_MATCHING: one add has its operands swapped (offset + diff instead of diff + offset).
void* ExpHeap::resizeFront(void* ptr, size_t size)
{
    if (!isInclude(ptr))
        return nullptr;

    ConditionalScopedLock<CriticalSection> lock(&mCS, isLockEnabled());

    MemBlock* block = MemBlock::FindManageArea(ptr);
    const size_t new_size = (size + 7) & ~size_t(7);
    if (block->mSize < new_size)
        return nullptr;

    if (block->mSize != new_size)
    {
        const size_t diff = block->mSize - new_size;
        const size_t offset = block->mOffset;
        const size_t front = sizeof(MemBlock) - new_size + block->mSize + offset;
        if (front - sizeof(MemBlock) < sizeof(MemBlock))
        {
            const u16 new_offset = diff + offset;
            block->mOffset = new_offset;
            if (new_offset)
                *reinterpret_cast<uintptr_t*>(reinterpret_cast<uintptr_t>(block) + sizeof(MemBlock) +
                                              block->mOffset - sizeof(uintptr_t)) =
                    reinterpret_cast<uintptr_t>(block) + 1;
        }
        else
        {
            auto* used_block = new (reinterpret_cast<void*>(front - sizeof(MemBlock) +
                                                            reinterpret_cast<uintptr_t>(block)))
                MemBlock();
            used_block->mHeapCheckTag = mHeapCheckTag;
            used_block->mSize = new_size;
            pushToUseList_(used_block);
            mUseList.erase(block);
            block->mSize = front - 2 * sizeof(MemBlock);
            block->mOffset = 0;
            pushToFreeList_(block);
            block = used_block;
        }
    }
    return reinterpret_cast<u8*>(block) + block->mOffset + sizeof(MemBlock);
}

// NON_MATCHING: register assignment of the rounded size and the old size is swapped.
void* ExpHeap::resizeBack(void* ptr, size_t size)
{
    if (!isInclude(ptr))
        return nullptr;

    ConditionalScopedLock<CriticalSection> lock(&mCS, isLockEnabled());

    MemBlock* block = MemBlock::FindManageArea(ptr);
    const size_t old_size = block->mSize;
    const size_t new_size = (size + 7) & ~size_t(7);
    if (old_size < new_size)
        return nullptr;

    if (old_size != new_size)
    {
        const size_t diff = old_size - new_size;
        if (diff < sizeof(MemBlock))
            return reinterpret_cast<u8*>(block) + block->mOffset + sizeof(MemBlock);

        block->mSize = new_size;
        auto* free_block = new (reinterpret_cast<u8*>(block) + new_size + block->mOffset +
                                sizeof(MemBlock)) MemBlock();
        free_block->mSize = diff - sizeof(MemBlock);
        pushToFreeList_(free_block);
    }
    return reinterpret_cast<u8*>(block) + block->mOffset + sizeof(MemBlock);
}

// NON_MATCHING: one callee-saved register more than the original (the block offset is kept across the tryAlloc call).
void* ExpHeap::tryRealloc(void* ptr, size_t size, s32 alignment)
{
    if (!ptr)
        return tryAlloc(size, alignment);

    if (size == 0)
    {
        free(ptr);
        return nullptr;
    }

    if (!isInclude(ptr) || alignment < 0)
        return nullptr;

    ConditionalScopedLock<CriticalSection> lock(&mCS, isLockEnabled());

    MemBlock* block = MemBlock::FindManageArea(ptr);
    const size_t old_size = block->mSize;
    const size_t new_size = (size + 7) & ~size_t(7);
    void* result;
    if (old_size < new_size)
    {
        const u16 offset = block->mOffset;
        result = tryAlloc(new_size, alignment == 0 ? 8 : alignment);
        if (!result)
            return nullptr;
        memcpy(result, reinterpret_cast<u8*>(block) + offset + sizeof(MemBlock), old_size);
        free(ptr);
        return result;
    }

    const uintptr_t data = reinterpret_cast<uintptr_t>(block) + block->mOffset + sizeof(MemBlock);
    if (alignment != 0 && (data & u32(alignment - 1)) != 0)
    {
        result = tryAlloc(new_size, alignment);
        if (!result)
            return nullptr;
        memcpy(result, reinterpret_cast<void*>(data), new_size);
        free(ptr);
        return result;
    }

    const size_t diff = old_size - new_size;
    if (diff > sizeof(MemBlock) - 1)
    {
        block->mSize = new_size;
        auto* free_block = new (reinterpret_cast<u8*>(block) + new_size + block->mOffset + sizeof(MemBlock)) MemBlock();
        free_block->mSize = diff - sizeof(MemBlock);
        pushToFreeList_(free_block);
    }
    return reinterpret_cast<u8*>(block) + block->mOffset + sizeof(MemBlock);
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

// NON_MATCHING: same code; the original duplicates the "unlock the parent" tail after each failing resize and loads the block
// offset / size before the subtractions
// 0x7100b0502c
size_t ExpHeap::adjust()
{
    if (!mParent)
        return mSize;

    ConditionalScopedLock<CriticalSection> lock(&mCS, isLockEnabled());
    Heap* parent = mParent;
    if (parent->isLockEnabled())
        parent->mCS.lock();

    size_t size;
    if (mDirection == cHeapDirection_Forward)
    {
        MemBlock* block = adjustBack_();
        if (block)
        {
            size = reinterpret_cast<uintptr_t>(block) - reinterpret_cast<uintptr_t>(mStart);
            mFreeList.erase(block);
            if (mParent->resizeBack(mStart, size))
                mSize = size;
            else
                size = mSize;
        }
        else
        {
            size = mSize;
        }
    }
    else
    {
        MemBlock* block = adjustFront_();
        size = mSize;
        if (block)
        {
            size = mSize - sizeof(MemBlock) - block->mOffset - block->mSize;
            mFreeList.erase(block);
            void* new_start = mParent->resizeFront(mStart, size);
            if (new_start)
            {
                mSize = size;
                std::atomic_thread_fence(std::memory_order_seq_cst);
                mStart = new_start;
            }
            else
            {
                size = mSize;
            }
        }
    }

    if (parent->isLockEnabled())
        parent->mCS.unlock();
    return size;
}

// 0x7100b04c24
// Flag bit 3 is set (by the tryCreate that takes the heap memory from the caller) when the memory does not come from the parent.
size_t ExpHeap::destroyAndGetAllocatableSize(s32 alignment)
{
    Heap* const parent = mParent;
    void* const start = mStart;
    const BitFlag16 flag = mFlag;
    this->~ExpHeap();

    if (!parent)
        return 0;

    const bool freeable = parent->isFreeable();
    if (flag.isOnBit(3) || !freeable)
        return 0;

    if (ExpHeap* exp_heap = DynamicCast<ExpHeap>(parent))
        return exp_heap->freeAndGetAllocatableSize(start, alignment);

    parent->free(start);
    return 0;
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