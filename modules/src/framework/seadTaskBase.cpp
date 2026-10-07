#include "framework/seadTaskBase.h"
#include "framework/seadFramework.h"
#include "framework/seadMethodTreeMgr.h"
#include "framework/seadTaskMgr.h"

namespace sead
{
// 0x7100afcc00
TaskBase* TaskClassID::create(const TaskConstructArg& arg) const
{
    switch (mType)
    {
    case Type::cInt:
        if (sCreateFromInt)
            return sCreateFromInt(mID.mInt, arg);
        return nullptr;
    case Type::cFactory:
        return mID.mFactory(arg);
    case Type::cString:
        if (sCreateFromString)
            return sCreateFromString(mID.mString, arg);
        return nullptr;
    default:
        return nullptr;
    }
}

TaskBase::CreateArg::CreateArg() = default;

TaskBase::CreateArg::CreateArg(const TaskClassID& factory) : factory(factory) {}

TaskBase::SystemMgrTaskArg::SystemMgrTaskArg(const TaskClassID& classID) : MgrTaskArg(classID)
{
    const s32 n = HeapMgr::getRootHeapNum();
    for (s32 i = 0; i < n; ++i)
        heap_policies.mPolicies[i].adjust = true;
    for (s32 i = 0; i < n; ++i)
        heap_policies.mPolicies[i].dont_create = i != 0;
    tag = cSystem;
}

TaskBase::TaskBase(const TaskConstructArg& arg, const char* name)
    : TTreeNode<TaskBase*>(this), mParameter(arg.param), mTaskListNode(this)
{
    mHeapArray = *arg.heap_array;
    mTaskMgr = arg.mgr;
    mState = cCreated;
    mTag = cApp;
    mClassID = TaskClassID();
    mInternalFlag.makeAllZero();
    setName(name);
}

TaskBase::~TaskBase()
{
    if (mTaskMgr)
    {
        while (child())
            mTaskMgr->destroyTaskSync(child()->value());
    }
    mState = cDead;
    detachAll();
    mTaskListNode.erase();
}

void TaskBase::attachCalcDraw()
{
    attachCalcImpl();
    attachDrawImpl();
}

void TaskBase::prepare() {}

void TaskBase::enterCommon()
{
    attachCalcImpl();
    attachDrawImpl();
    pauseCalc(false);
    pauseDraw(false);
    enter();
}

void TaskBase::enter() {}

void TaskBase::exit() {}

void TaskBase::onEvent(const TaskEvent&) {}

void TaskBase::adjustHeapWithSlackWithoutLock_(s32 index, u32 slack)
{
    Heap* heap = mHeapArray.getHeap(index);
    if (heap && !mHeapArray.mAdjusted[index])
    {
        mHeapArray.mAdjusted[index] = true;
        void* slack_block = slack ? heap->tryAlloc(slack, 8) : nullptr;
        heap->adjust();
        if (slack_block)
            heap->free(slack_block);
    }
}

void TaskBase::adjustHeapAll()
{
    ScopedLock<CriticalSection> lock(&mTaskMgr->mCriticalSection);
    for (s64 i = 0; i < HeapMgr::getRootHeapNum(); ++i)
    {
        Heap* heap = mHeapArray.getHeap(i);
        if (heap && !mHeapArray.mAdjusted[i])
        {
            mHeapArray.mAdjusted[i] = true;
            heap->adjust();
        }
    }
}

void TaskBase::doneDestroy()
{
    mInternalFlag.setBit(2);
}

Framework* TaskBase::getFramework() const
{
    return mTaskMgr->mParentFramework;
}

MethodTreeMgr* TaskBase::getMethodTreeMgr() const
{
    return mTaskMgr->mParentFramework->mMethodTreeMgr;
}

void TaskBase::attachMethodWithCheck(s32 method_type, MethodTreeNode* node)
{
    getMethodTreeMgr()->attachMethod(method_type, node);
}

void TaskBase::pauseCalcChild(bool) {}

void TaskBase::pauseDrawChild(bool) {}

void TaskBase::onDestroy()
{
    doneDestroy();
}
}  // namespace sead
