#include <heap/seadSeparateHeap.h>
#include <prim/seadScopedLock.h>

namespace sead
{
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
