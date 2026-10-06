#include "framework/seadTask.h"
#include "framework/seadMethodTreeMgr.h"

namespace sead
{
Task::Task(const TaskConstructArg& arg, const char* name) : TaskBase(arg, name)
{
    mCalcNode.bind(Delegate<Task>{this, &Task::calc}, name);
    mDrawNode.bind(Delegate<Task>{this, &Task::draw}, name);
}

Task::~Task() = default;

void Task::calc() {}

void Task::draw() {}

void Task::attachCalcImpl()
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

void Task::attachDrawImpl()
{
    ScopedLock<CriticalSection> lock(&getMethodTreeMgr()->mCS);
    TaskBase* parent_task = parent() ? parent()->value() : nullptr;
    if (mTag == cSystem)
        attachMethodWithCheck(2, &mDrawNode);
    else if (parent_task)
        parent_task->getMethodTreeNode(3)->pushFrontChild(&mDrawNode);
    else
        attachMethodWithCheck(3, &mDrawNode);
}

void Task::detachCalcImpl()
{
    mCalcNode.detachAll();
}

void Task::detachDrawImpl()
{
    mDrawNode.detachAll();
}

void Task::pauseCalc(bool b)
{
    if (b)
        mCalcNode.setPauseFlag(MethodTreeNode::cPause_Self);
    else
        mCalcNode.setPauseFlag(MethodTreeNode::cPause_None);
}

void Task::pauseDraw(bool b)
{
    if (b)
        mDrawNode.setPauseFlag(MethodTreeNode::cPause_Self);
    else
        mDrawNode.setPauseFlag(MethodTreeNode::cPause_None);
}

void Task::pauseCalcRec(bool b)
{
    if (b)
        mCalcNode.setPauseFlag(MethodTreeNode::cPause_Both);
    else
        mCalcNode.setPauseFlag(MethodTreeNode::cPause_None);
}

void Task::pauseDrawRec(bool b)
{
    if (b)
        mDrawNode.setPauseFlag(MethodTreeNode::cPause_Both);
    else
        mDrawNode.setPauseFlag(MethodTreeNode::cPause_None);
}

void Task::pauseCalcChild(bool b)
{
    if (b)
        mCalcNode.setPauseFlag(MethodTreeNode::cPause_Child);
    else
        mCalcNode.setPauseFlag(MethodTreeNode::cPause_None);
}

void Task::pauseDrawChild(bool b)
{
    if (b)
        mDrawNode.setPauseFlag(MethodTreeNode::cPause_Child);
    else
        mDrawNode.setPauseFlag(MethodTreeNode::cPause_None);
}

const RuntimeTypeInfo::Interface* Task::getCorrespondingMethodTreeMgrTypeInfo() const
{
    return MethodTreeMgr::getRuntimeTypeInfoStatic();
}

MethodTreeNode* Task::getMethodTreeNode(s32 method_type)
{
    switch (method_type)
    {
    case 0:
    case 1:
        return &mCalcNode;
    case 2:
    case 3:
    case 4:
        return &mDrawNode;
    default:
        return nullptr;
    }
}
}  // namespace sead
