#include <heap/seadExpHeap.h>
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

ExpHeap* ExpHeap::create(size_t size, const SafeString& name, Heap* parent, s32 alignment,
                         HeapDirection direction, bool enable_lock)
{
    return tryCreate(size, name, parent, alignment, direction, enable_lock);
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