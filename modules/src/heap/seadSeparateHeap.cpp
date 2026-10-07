#include <heap/seadSeparateHeap.h>
#include <math/seadMathCalcCommon.h>
#include <prim/seadPtrUtil.h>
#include <prim/seadScopedLock.h>

namespace sead
{
// NON_MATCHING: same code; the original loop that chains the unused blocks is unrolled differently (and the block
// address is computed before the lock is taken).
// 0x710136a55c: `memory` holds the heap object followed by the array of the blocks (0x110 bytes + n * 0x20 bytes);
// `start` / `size` is the memory that the heap manages.
SeparateHeap* SeparateHeap::create(const SafeString& name, void* memory, size_t memory_size,
                                   void* start, size_t size, bool enable_lock)
{
    SeparateHeap* heap = new (memory) SeparateHeap(name, start, size, enable_lock);
    ConditionalScopedLock<CriticalSection> lock(&heap->mCS, heap->isLockEnabled());
    const s32 block_num = (memory_size - sizeof(SeparateHeap)) / sizeof(Block);
    Block* blocks = static_cast<Block*>(PtrUtil::addOffset(heap, sizeof(SeparateHeap)));
    if (blocks)
    {
        const s32 last = block_num - 1;
        if (last >= 0)
        {
            heap->mUnusedBlocks = blocks;
            for (s32 i = 0; i < last; ++i)
                blocks[i].mNextUnused = &blocks[i + 1];
            blocks[last].mNextUnused = nullptr;
            heap->mBlockArray = blocks;
            heap->mBlockNum = block_num;
        }
    }
    return heap;
}

SeparateHeap::SeparateHeap(const SafeString& name, void* start, size_t size, bool enable_lock)
    : Heap(name, nullptr, start, size, cHeapDirection_Forward, enable_lock), mUnusedBlocks(nullptr),
      mBlockArray(nullptr), mBlockNum(0)
{
}

SeparateHeap::~SeparateHeap()
{
    destruct_();
}

void SeparateHeap::destroy()
{
    Heap* parent = mParent;
    this->~SeparateHeap();
    if (parent && parent->isFreeable())
        parent->free(this);
}

size_t SeparateHeap::adjust()
{
    return getSize();
}

// NON_MATCHING: the original walks the list with a block pointer and a node pointer live at once; this keeps a single induction variable.
void SeparateHeap::free(void* ptr)
{
    if (!ptr)
        return;

    ConditionalScopedLock<CriticalSection> lock(&mCS, isLockEnabled());
    Block* block = mBlocks.blockBegin();
    for (; block != mBlocks.blockEnd(); block = BlockList::blockNext(block))
    {
        if (block->mStart == ptr)
            break;
    }
    if (block)
    {
        mBlocks.eraseNode(&block->mNode);
        block->mNextUnused = mUnusedBlocks;
        mUnusedBlocks = block;
    }
}

void SeparateHeap::freeAll()
{
    ConditionalScopedLock<CriticalSection> lock(&mCS, isLockEnabled());
    dispose_(nullptr, nullptr);
    ListNode* next;
    for (ListNode* node = mBlocks.nodeBegin(); node != mBlocks.nodeEnd(); node = next)
    {
        next = node->next();
        mBlocks.eraseNode(node);
        Block* block = Block::fromNode(node);
        block->mNextUnused = mUnusedBlocks;
        mUnusedBlocks = block;
    }
}

uintptr_t SeparateHeap::getStartAddress() const
{
    return reinterpret_cast<uintptr_t>(mStart);
}

uintptr_t SeparateHeap::getEndAddress() const
{
    return reinterpret_cast<uintptr_t>(mStart) + getSize();
}

size_t SeparateHeap::getSize() const
{
    return mSize;
}

// NON_MATCHING: same list walk difference as free(): the original keeps both the node and the block pointer.
size_t SeparateHeap::getFreeSize() const
{
    size_t used = 0;
    for (const Block* block = mBlocks.blockBegin(); block != mBlocks.blockEnd();
         block = BlockList::blockNext(block))
        used += block->mSize;
    return mSize - used;
}

// NON_MATCHING: same list walk difference as free() and getFreeSize() (the original keeps both the node and the block
// pointer).
// 0x710136ad1c: the largest gap between the (address ordered) blocks, after aligning the start of the gap.
size_t SeparateHeap::getMaxAllocatableSize(int alignment) const
{
    const uintptr_t mask = u32(alignment - 1);
    uintptr_t cursor = reinterpret_cast<uintptr_t>(mStart);
    size_t max_size = 0;

    for (const Block* block = mBlocks.blockBegin(); block != mBlocks.blockEnd();
         block = BlockList::blockNext(block))
    {
        const uintptr_t aligned = (cursor + mask) & ~mask;
        const uintptr_t block_start = reinterpret_cast<uintptr_t>(block->mStart);
        if (aligned < block_start)
        {
            const size_t gap = block_start - aligned;
            if (max_size < gap)
                max_size = gap;
        }
        cursor = block_start + block->mSize;
    }

    const uintptr_t end = reinterpret_cast<uintptr_t>(mStart) + mSize;
    const uintptr_t aligned = (cursor + mask) & ~mask;
    if (aligned < end)
    {
        const size_t gap = end - aligned;
        if (max_size < gap)
            max_size = gap;
    }
    return max_size;
}

bool SeparateHeap::isInclude(const void* ptr) const
{
    const uintptr_t start = reinterpret_cast<uintptr_t>(mStart);
    const uintptr_t address = reinterpret_cast<uintptr_t>(ptr);
    return start <= address && start + mSize > address;
}

bool SeparateHeap::isEmpty() const
{
    return mBlocks.size() == 0;
}

bool SeparateHeap::isFreeable() const
{
    return true;
}

bool SeparateHeap::isResizable() const
{
    return true;
}

bool SeparateHeap::isAdjustable() const
{
    return false;
}

// NON_MATCHING: the (empty here) block loop is not reproduced; the original body prints each block.
void SeparateHeap::dump() const
{
    ConditionalScopedLock<CriticalSection> lock(const_cast<CriticalSection*>(&mCS),
                                                isLockEnabled());
    ConditionalScopedLock<CriticalSection> lock2(const_cast<CriticalSection*>(&mCS),
                                                 isLockEnabled());
    for (const Block* block = mBlocks.blockBegin(); block != mBlocks.blockEnd();
         block = BlockList::blockNext(block))
    {
    }
}

void SeparateHeap::genInformation_(hostio::Context*) {}

}  // namespace sead
