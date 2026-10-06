#include <heap/seadFrameHeap.h>
#include <math/seadMathCalcCommon.h>
#include <heap/seadHeapMgr.h>
#include <prim/seadPtrUtil.h>
#include <prim/seadScopedLock.h>

namespace sead
{
namespace
{
// The size of the heap object, which is placed at the start (forward heaps) or at the end (reverse heaps) of the
// memory of the heap.
constexpr size_t cFrameHeapObjectSize = 0xf0;
}  // namespace

FrameHeap* FrameHeap::create(size_t size, const SafeString& name, Heap* parent, s32 alignment,
                             HeapDirection direction, bool enable_lock)
{
    return tryCreate(size, name, parent, alignment, direction, enable_lock);
}

FrameHeap::FrameHeap(const SafeString& name, Heap* parent, void* address, size_t size,
                     HeapDirection direction, bool enable_lock)
    : Heap(name, parent, address, size, direction, enable_lock), mState{}
{
}

FrameHeap::~FrameHeap()
{
    destruct_();
}

inline void* FrameHeap::getAreaStart_() const
{
    return mDirection == cHeapDirection_Forward ? PtrUtil::addOffset(mStart, cFrameHeapObjectSize) :
                                                  mStart;
}

inline void* FrameHeap::getAreaEnd_() const
{
    return PtrUtil::addOffset(mStart, mDirection == cHeapDirection_Forward ?
                                          mSize :
                                          mSize - cFrameHeapObjectSize);
}

FrameHeap* FrameHeap::tryCreate(size_t size, const SafeString& name, Heap* parent, s32 alignment,
                                HeapDirection direction, bool enable_lock)
{
    if (!parent)
    {
        parent = HeapMgr::sInstancePtr->getCurrentHeap();
        if (!parent)
            return nullptr;
    }

    size = (size ? size + 7 : parent->getMaxAllocatableSize(alignment)) & ~size_t(7);
    if (size < cFrameHeapObjectSize)
        return nullptr;

    const u32 abs_alignment = Mathi::abs(alignment);
    if ((abs_alignment & (abs_alignment - 1)) != 0)
        return nullptr;

    void* memory = parent->tryAlloc(size, direction * alignment);
    if (!memory)
        return nullptr;

    const HeapDirection heap_direction =
        parent->mDirection == cHeapDirection_Reverse ? HeapDirection(-direction) : direction;
    FrameHeap* heap;
    if (heap_direction == cHeapDirection_Forward)
        heap = new (memory) FrameHeap(name, parent, memory, size, heap_direction, enable_lock);
    else
        heap = new (PtrUtil::addOffset(memory, size - cFrameHeapObjectSize))
            FrameHeap(name, parent, memory, size, heap_direction, enable_lock);

    {
        ConditionalScopedLock<CriticalSection> lock(&heap->mCS, heap->isLockEnabled());
        heap->mState.mHeadPtr = heap->getAreaStart_();
        heap->mState.mTailPtr = heap->getAreaEnd_();
    }
    parent->pushBackChild_(heap);
    return heap;
}

void FrameHeap::destroy()
{
    Heap* parent = mParent;
    void* start = mStart;
    this->~FrameHeap();
    if (parent && parent->isFreeable())
        parent->free(start);
}

void FrameHeap::freeAll()
{
    ConditionalScopedLock<CriticalSection> lock(&mCS, isLockEnabled());
    dispose_(nullptr, nullptr);
    mState.mHeadPtr = getAreaStart_();
    mState.mTailPtr = getAreaEnd_();
}

void FrameHeap::freeTail()
{
    ConditionalScopedLock<CriticalSection> lock(&mCS, isLockEnabled());
    if (mDirection == cHeapDirection_Forward)
    {
        dispose_(mState.mTailPtr, PtrUtil::addOffset(mStart, mSize));
        mState.mTailPtr = getAreaEnd_();
    }
    else
    {
        dispose_(mStart, mState.mHeadPtr);
        mState.mHeadPtr = getAreaStart_();
    }
}

void FrameHeap::restoreState(const State& state)
{
    ConditionalScopedLock<CriticalSection> lock(&mCS, isLockEnabled());
    if (state.mHeadPtr && state.mHeadPtr != mState.mHeadPtr && isInclude(state.mHeadPtr) &&
        state.mHeadPtr <= mState.mHeadPtr)
    {
        dispose_(state.mHeadPtr, mState.mHeadPtr);
        mState.mHeadPtr = state.mHeadPtr;
    }
    if (state.mTailPtr && state.mTailPtr != mState.mTailPtr && isInclude(state.mTailPtr) &&
        state.mTailPtr >= mState.mTailPtr)
    {
        dispose_(mState.mTailPtr, state.mTailPtr);
        mState.mTailPtr = state.mTailPtr;
    }
}

size_t FrameHeap::adjustBack_()
{
    size_t new_size = reinterpret_cast<uintptr_t>(mState.mHeadPtr) - getStartAddress();
    if (mParent->resizeBack(mStart, new_size))
    {
        mState.mTailPtr = mState.mHeadPtr;
        mSize = new_size;
    }
    else
    {
        new_size = mSize;
    }
    return new_size;
}

void FrameHeap::free(void*) {}

void* FrameHeap::resizeFront(void*, size_t)
{
    return nullptr;
}

void* FrameHeap::resizeBack(void*, size_t)
{
    return nullptr;
}

uintptr_t FrameHeap::getStartAddress() const
{
    return reinterpret_cast<uintptr_t>(mStart);
}

uintptr_t FrameHeap::getEndAddress() const
{
    return reinterpret_cast<uintptr_t>(mStart) + mSize;
}

size_t FrameHeap::getSize() const
{
    return mSize;
}

size_t FrameHeap::getFreeSize() const
{
    ConditionalScopedLock<CriticalSection> lock(const_cast<CriticalSection*>(&mCS), isLockEnabled());
    return reinterpret_cast<uintptr_t>(mState.mTailPtr) - reinterpret_cast<uintptr_t>(mState.mHeadPtr);
}

size_t FrameHeap::getMaxAllocatableSize(int alignment) const
{
    const u32 abs_alignment = Mathi::abs(alignment);
    if ((abs_alignment & (abs_alignment - 1)) != 0)
        return 0;

    ConditionalScopedLock<CriticalSection> lock(const_cast<CriticalSection*>(&mCS), isLockEnabled());
    const uintptr_t mask = abs_alignment - 1;
    const uintptr_t aligned_head = (reinterpret_cast<uintptr_t>(mState.mHeadPtr) + mask) & ~mask;
    const uintptr_t tail = reinterpret_cast<uintptr_t>(mState.mTailPtr);
    size_t result = 0;
    if (aligned_head <= tail)
        result = tail - aligned_head;
    return result;
}

bool FrameHeap::isInclude(const void* ptr) const
{
    const void* start = getAreaStart_();
    const void* end = getAreaEnd_();
    return start <= ptr && ptr < end;
}

bool FrameHeap::isEmpty() const
{
    return mState.mHeadPtr == getAreaStart_() && mState.mTailPtr == getAreaEnd_();
}

bool FrameHeap::isFreeable() const
{
    return false;
}

bool FrameHeap::isResizable() const
{
    return false;
}

bool FrameHeap::isAdjustable() const
{
    return true;
}

void FrameHeap::dump() const
{
    ConditionalScopedLock<CriticalSection> lock(const_cast<CriticalSection*>(&mCS), isLockEnabled());
}

void FrameHeap::genInformation_(hostio::Context* context)
{
    Heap::genInformation_(context);
}
}  // namespace sead
