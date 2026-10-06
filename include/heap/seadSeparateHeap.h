#pragma once

#include "container/seadListImpl.h"
#include "heap/seadHeap.h"

namespace sead
{
/// A heap whose management data (the list of allocated blocks) is stored outside of the memory it manages.
/// TODO: incomplete (tryAlloc, create, resizeFront/Back and getMaxAllocatableSize are not implemented).
class SeparateHeap : public Heap
{
    SEAD_RTTI_OVERRIDE(SeparateHeap, Heap)
public:
    /// The management data of an allocated block (or, when unused, the next unused block).
    struct Block
    {
        union
        {
            void* mStart;
            Block* mNextUnused;
        };
        size_t mSize;
        ListNode mNode;

        static Block* fromNode(ListNode* node)
        {
            return reinterpret_cast<Block*>(reinterpret_cast<u8*>(node) - offsetof(Block, mNode));
        }
        static const Block* fromNode(const ListNode* node)
        {
            return reinterpret_cast<const Block*>(reinterpret_cast<const u8*>(node) -
                                                  offsetof(Block, mNode));
        }
    };
    static_assert(sizeof(Block) == 0x20);

    /// The list of allocated blocks, walked through the link nodes embedded in each block.
    class BlockList : public ListImpl
    {
    public:
        ListNode* nodeBegin() const { return mStartEnd.next(); }
        const ListNode* nodeEnd() const { return &mStartEnd; }
        void eraseNode(ListNode* node) { erase(node); }

        Block* blockBegin() const { return Block::fromNode(mStartEnd.next()); }
        Block* blockEnd() const { return Block::fromNode(const_cast<ListNode*>(&mStartEnd)); }
        static Block* blockNext(const Block* block) { return Block::fromNode(block->mNode.next()); }
    };

    static SeparateHeap* create(const SafeString& name, void* start, size_t size, void* work,
                                size_t work_size, bool enable_lock);

    void destroy() override;
    size_t adjust() override;
    void* tryAlloc(size_t size, s32 alignment) override;
    void free(void* ptr) override;
    void* resizeFront(void* ptr, size_t size) override;
    void* resizeBack(void* ptr, size_t size) override;
    void freeAll() override;
    uintptr_t getStartAddress() const override;
    uintptr_t getEndAddress() const override;
    size_t getSize() const override;
    size_t getFreeSize() const override;
    size_t getMaxAllocatableSize(int alignment) const override;
    bool isInclude(const void* ptr) const override;
    bool isEmpty() const override;
    bool isFreeable() const override;
    bool isResizable() const override;
    bool isAdjustable() const override;
    void dump() const override;
    void dumpYAML(WriteStream& stream, int indent) const override;
    void genInformation_(hostio::Context* context) override;

protected:
    ~SeparateHeap() override;

    u8 _dc[4];
    BlockList mBlocks;
    Block* mUnusedBlocks;
};

}  // namespace sead
