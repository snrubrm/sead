#include "framework/seadCalculateTask.h"
#include "framework/seadMethodTreeMgr.h"

namespace sead
{
CalculateTask::CalculateTask(const TaskConstructArg& arg) : TaskBase(arg)
{
    mCalcNode.bind(sead::Delegate<CalculateTask>{this, &CalculateTask::calc}, "CalculateTask");
}

CalculateTask::CalculateTask(const TaskConstructArg& arg, const char* name) : TaskBase(arg, name)
{
    mCalcNode.bind(sead::Delegate<CalculateTask>{this, &CalculateTask::calc}, name);
}

CalculateTask::~CalculateTask() = default;

void CalculateTask::calc() {}

void CalculateTask::attachCalcImpl()
{
    ScopedLock<CriticalSection> lock(&getMethodTreeMgr()->mCS);
    TaskBase* parent_task = parent() ? parent()->value() : nullptr;
    if (mTag == cSystem)
        attachMethodWithCheck(0, &mCalcNode);
    else if (parent_task)
        parent_task->getMethodTreeNode(1)->pushBackChild(&mCalcNode);
    else
        attachMethodWithCheck(1, &mCalcNode);
}

void CalculateTask::attachDrawImpl() {}

void CalculateTask::detachCalcImpl()
{
    mCalcNode.detachAll();
}

void CalculateTask::detachDrawImpl() {}

void CalculateTask::pauseCalc(bool b)
{
    if (b)
        mCalcNode.setPauseFlag(MethodTreeNode::cPause_Self);
    else
        mCalcNode.setPauseFlag(MethodTreeNode::cPause_None);
}

void CalculateTask::pauseDraw(bool) {}

void CalculateTask::pauseCalcRec(bool b)
{
    if (b)
        mCalcNode.setPauseFlag(MethodTreeNode::cPause_Both);
    else
        mCalcNode.setPauseFlag(MethodTreeNode::cPause_None);
}

void CalculateTask::pauseDrawRec(bool) {}

void CalculateTask::pauseCalcChild(bool b)
{
    if (b)
        mCalcNode.setPauseFlag(MethodTreeNode::cPause_Child);
    else
        mCalcNode.setPauseFlag(MethodTreeNode::cPause_None);
}

void CalculateTask::pauseDrawChild(bool) {}

const RuntimeTypeInfo::Interface* CalculateTask::getCorrespondingMethodTreeMgrTypeInfo() const
{
    return MethodTreeMgr::getRuntimeTypeInfoStatic();
}

MethodTreeNode* CalculateTask::getMethodTreeNode(s32 method_type)
{
    return u32(method_type) < 2 ? &mCalcNode : nullptr;
}
}  // namespace sead
