#include <framework/seadFramework.h>
#include <framework/seadMethodTreeMgr.h>
#include <framework/seadTaskMgr.h>
#include <gfx/seadFrameBuffer.h>
#include <heap/seadExpHeap.h>
#include <heap/seadHeap.h>
#include <random/seadGlobalRandom.h>

namespace sead
{
Framework::InitializeArg::InitializeArg() : heap_size(0x3000000), arena(NULL)
{
}

Framework::RunArg::RunArg() : prepare_stack_size(0), prepare_priority(-1)
{
}

Framework::CreateSystemTaskArg::CreateSystemTaskArg()
    : hostio_parameter(NULL), heap(NULL), infloop_detection_span(), infloop_unk(0x1000)
{
}

void Framework::initialize(const InitializeArg& arg)
{
    if (arg.arena)
        HeapMgr::initialize(arg.arena);
    else
        HeapMgr::initialize(arg.heap_size);

    Heap* heap = HeapMgr::instance()->getRootHeap(0);

    {
        Heap* threadHeap = ExpHeap::create(0, "sead::ThreadMgr", heap);

        ThreadMgr::createInstance(threadHeap);
        ThreadMgr::instance()->initialize(threadHeap);

        threadHeap->adjust();
    }

    GlobalRandom::createInstance(heap);
}

Framework::Framework()
    : mReserveReset(false), mResetParameter(NULL), mResetEvent(), mTaskMgr(NULL),
      mMethodTreeMgr(NULL), mMethodTreeMgrHeap(NULL)
{
}

Framework::~Framework()
{
    if (mTaskMgr != NULL)
    {
        mTaskMgr->finalize();
        delete mTaskMgr;
        mTaskMgr = NULL;
    }

    if (mMethodTreeMgr != NULL)
    {
        delete mMethodTreeMgr;
        mMethodTreeMgr = NULL;
    }

    if (mMethodTreeMgrHeap != NULL)
        mMethodTreeMgrHeap->destroy();
}

LogicalFrameBuffer* Framework::getMethodLogicalFrameBuffer(s32 method) const
{
    return getMethodFrameBuffer(method);
}

bool Framework::setProcessPriority(ProcessPriority priority)
{
    return false;
}

void Framework::reserveReset(void* parameter)
{
    mReserveReset = true;
    mResetParameter = parameter;
}

void Framework::procReset_()
{
    if (mReserveReset)
    {
        mResetEvent.emit(mResetParameter);
        mTaskMgr->destroyAllAndCreateRoot();
        mReserveReset = false;
        mResetParameter = NULL;
    }
}

void Framework::createSystemTasks(TaskBase* parent, const CreateSystemTaskArg& arg) {}

void Framework::initRun_(Heap* heap) {}

void Framework::quitRun_(Heap* heap) {}

void Framework::runImpl_() {}

}  // namespace sead
