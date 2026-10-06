#pragma once

#include <framework/seadMethodTree.h>
#include <framework/seadMethodTreeMgr.h>

namespace sead
{
/// A method tree manager for a single screen: one calc tree and one draw tree. Methods are attached
/// to one of five child nodes (the first two are calc nodes, the other three are draw nodes).
class SingleScreenMethodTreeMgr : public MethodTreeMgr
{
    SEAD_RTTI_OVERRIDE(SingleScreenMethodTreeMgr, MethodTreeMgr)
public:
    SingleScreenMethodTreeMgr();
    ~SingleScreenMethodTreeMgr() override;

    void attachMethod(s32 index, MethodTreeNode* node) override;
    MethodTreeNode* getRootMethodTreeNode(s32 index) override;
    void pauseAll(bool pause) override;
    void pauseAppCalc(bool pause) override;

    void calc();
    void draw();

private:
    MethodTreeNode mCalcRootNode;
    MethodTreeNode mSystemCalcNode;
    MethodTreeNode mAppCalcNode;
    MethodTreeNode mDrawRootNode;
    MethodTreeNode mDrawNode0;
    MethodTreeNode mDrawNode1;
    MethodTreeNode mDrawNode2;
};
static_assert(sizeof(SingleScreenMethodTreeMgr) == 0x470);

}  // namespace sead
