#pragma once

#include "basis/seadTypes.h"
#include "container/seadListImpl.h"
#include "container/seadOffsetList.h"

namespace sead
{
class MemBlock
{
public:
    static MemBlock* FindManageArea(void* ptr);

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
