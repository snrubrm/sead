#pragma once

#include "basis/seadTypes.h"
#include "container/seadListImpl.h"
#include "container/seadOffsetList.h"

namespace sead
{
class MemBlock
{
public:
    // The block is either directly in front of the data or referenced by a tagged pointer that is
    // stored right before the (aligned) data.
    static MemBlock* FindManageArea(void* ptr)
    {
        const uintptr_t tagged = reinterpret_cast<uintptr_t*>(ptr)[-1];
        if (tagged & 1)
            return reinterpret_cast<MemBlock*>(tagged - 1);
        return reinterpret_cast<MemBlock*>(static_cast<u8*>(ptr) - sizeof(MemBlock));
    }

    MemBlock() : mListNode(), mHeapCheckTag(0), mOffset(0), mSize(0) {}

    static u32 getOffset() { return offsetof(MemBlock, mListNode); }

protected:
    friend class ExpHeap;

    ListNode mListNode;
    u16 mHeapCheckTag;
    u16 mOffset;
    size_t mSize;
};
static_assert(sizeof(MemBlock) == 0x20, "sead::MemBlock size mismatch");

using MemBlockList = OffsetList<MemBlock>;
}  // namespace sead
