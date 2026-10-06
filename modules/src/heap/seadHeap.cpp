#include <heap/seadHeap.h>
#include <heap/seadHeapMgr.h>
#include <prim/seadScopedLock.h>

namespace sead
{
Heap::Heap(const SafeString& name, Heap* parent, void* address, size_t size,
           HeapDirection direction, bool enable_lock)
    : IDisposer(parent, HeapNullOption::UseSpecifiedOrContainHeap), INamable(name), mStart(address),
      mSize(size), mParent(parent), mDirection(direction), mCS(parent)
{
    mFlag.makeAllZero();
    mFlag.setBit(Flag::cEnableWarning);
    u32 tag;
    do
        tag = HeapMgr::sHeapCheckTag.fetchAdd(1);
    while ((tag & 0xffff) == 0xffff);
    mHeapCheckTag = tag;
    enableLock(enable_lock);

    ConditionalScopedLock<CriticalSection> lock(&mCS, isLockEnabled());
    mChildren.initOffset(offsetof(Heap, mListNode));
    mDisposerList.initOffset(IDisposer::getListNodeOffset());
}

Heap::~Heap() = default;

void Heap::destruct_()
{
    ConditionalScopedLock<CriticalSection> lock(&mCS, isLockEnabled());
    dispose_(nullptr, nullptr);
    HeapMgr::removeFromFindContainHeapCache_(this);
    Heap* parent = mParent;
    if (parent)
    {
        ScopedLock<CriticalSection> tree_lock(&HeapMgr::sHeapTreeLockCS);
        ConditionalScopedLock<CriticalSection> parent_lock(&parent->mCS, parent->isLockEnabled());
        parent->mChildren.erase(this);
    }
    else
    {
        HeapMgr::removeRootHeap(this);
    }
}

void Heap::dispose_(const void* begin, const void* end)
{
    mFlag.setBit(Flag::cDisposing);
    const bool dispose_all = !begin && !end;
    auto it = mDisposerList.begin();
    while (it != mDisposerList.end())
    {
        IDisposer* disposer = &*it;
        if (disposer->mDisposerHeap &&
            (dispose_all || (disposer >= begin && disposer < end)))
        {
            disposer->~IDisposer();
            it = mDisposerList.begin();
        }
        else
        {
            ++it;
        }
    }
    mFlag.resetBit(Flag::cDisposing);
}

void Heap::genInformation_(hostio::Context*) {}

void Heap::makeMetaString_(BufferedSafeString* string)
{
    const f32 free_size = getFreeSize();
    const f32 size = getSize();
    string->format("$SEAD_META_HEAP_%03d", s32((1.0f - free_size / size) * 10) * 10);
}

void Heap::pushBackChild_(Heap* child)
{
    ScopedLock<CriticalSection> tree_lock(&HeapMgr::sHeapTreeLockCS);
    ConditionalScopedLock<CriticalSection> lock(&mCS, isLockEnabled());
    mChildren.pushBack(child);
}

void Heap::appendDisposer_(IDisposer* disposer)
{
    ConditionalScopedLock<CriticalSection> lock(&mCS, isLockEnabled());
    mDisposerList.pushBack(disposer);
}

void Heap::removeDisposer_(IDisposer* disposer)
{
    ConditionalScopedLock<CriticalSection> lock(&mCS, isLockEnabled());
    mDisposerList.erase(disposer);
}

Heap* Heap::findContainHeap_(const void* ptr)
{
    if (!isInclude(ptr))
        return nullptr;

    for (auto it = mChildren.begin(); it != mChildren.end(); ++it)
    {
        if (it->isInclude(ptr))
            return it->findContainHeap_(ptr);
    }

    return this;
}

}  // namespace sead
