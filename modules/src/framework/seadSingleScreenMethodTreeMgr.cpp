#include "framework/seadSingleScreenMethodTreeMgr.h"

namespace sead
{
SingleScreenMethodTreeMgr::~SingleScreenMethodTreeMgr() = default;

void SingleScreenMethodTreeMgr::attachMethod(s32 index, MethodTreeNode* node)
{
    switch (index)
    {
    case 0:
        mSystemCalcNode.pushBackChild(node);
        break;
    case 1:
        mAppCalcNode.pushBackChild(node);
        break;
    case 2:
        mDrawNode0.pushFrontChild(node);
        break;
    case 3:
        mDrawNode1.pushFrontChild(node);
        break;
    case 4:
        mDrawNode2.pushFrontChild(node);
        break;
    }
}

MethodTreeNode* SingleScreenMethodTreeMgr::getRootMethodTreeNode(s32 index)
{
    switch (index)
    {
    case 0:
        return &mSystemCalcNode;
    case 1:
        return &mAppCalcNode;
    case 2:
        return &mDrawNode0;
    case 3:
        return &mDrawNode1;
    case 4:
        return &mDrawNode2;
    default:
        return nullptr;
    }
}

void SingleScreenMethodTreeMgr::pauseAll(bool pause)
{
    if (pause)
    {
        mCalcRootNode.setPauseFlag(MethodTreeNode::cPause_Both);
        mDrawRootNode.setPauseFlag(MethodTreeNode::cPause_Both);
    }
    else
    {
        mCalcRootNode.setPauseFlag(MethodTreeNode::cPause_None);
        mDrawRootNode.setPauseFlag(MethodTreeNode::cPause_None);
    }
}

void SingleScreenMethodTreeMgr::pauseAppCalc(bool pause)
{
    if (pause)
        mAppCalcNode.setPauseFlag(MethodTreeNode::cPause_Both);
    else
        mAppCalcNode.setPauseFlag(MethodTreeNode::cPause_None);
}

void SingleScreenMethodTreeMgr::calc()
{
    mCalcRootNode.call();
}

void SingleScreenMethodTreeMgr::draw()
{
    mDrawRootNode.call();
}
}  // namespace sead
