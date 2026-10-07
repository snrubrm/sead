#include <framework/nx/seadGameFrameworkNx.h>
#include <framework/nx/seadPerformanceMgrNx.h>
#include <framework/seadSingleScreenMethodTreeMgr.h>
#include <framework/seadTaskMgr.h>
#include <gfx/nin/seadDisplayBufferNvn.h>
#include <gfx/nin/seadGraphicsNvn.h>
#include <nn/os.h>
#include <time/seadTickSpan.h>

namespace sead
{
// 0x7100af8510 (D1) / 0x7100af8528 (D0)
// The body keeps the store of the GameFrameworkNx vtable pointer before the tail call to ~GameFramework (a defaulted or
// empty destructor drops it).
GameFrameworkNx::~GameFrameworkNx() { ; }

// 0x7100af842c
void GameFrameworkNx::initialize(const Framework::InitializeArg& arg)
{
    GameFramework::initialize(arg);
    PerformanceMgrNx::initialize();
}

// 0x7100af8fa8
void GameFrameworkNx::procFrame_()
{
    if ((mGpuTimeStampFlags & 3) != 1)
    {
        if (mUseGPURequest != 0)
        {
            if (mUseGPURequest == 1)
                mUseGPU = 1;
            else if (mUseGPURequest == 2)
                mUseGPU = 0;
            mUseGPURequest = 0;
        }

        if (mDisplayStarted == 1)
            mDisplayStarted = 2;
    }

    mTaskMgr->afterCalc();
    procDraw_();
    procCalc_();
    procReset_();
    waitForGpuDone_();
    nn::os::GetSystemTick();
    setGpuTimeStamp_();

    if ((mGpuTimeStampFlags & 3) != 3)
    {
        mFrameDuration = nn::os::GetSystemTick().value - mPrevFrameTick;
        mPrevFrameTick = nn::os::GetSystemTick().value;
    }

    if (mGpuTimeStampFlags & 1)
        mGpuTimeStampFlags ^= 2;
}

// 0x7100af9380
void GameFrameworkNx::procCalc_()
{
    mTaskMgr->beforeCalc();
    DynamicCast<SingleScreenMethodTreeMgr>(mMethodTreeMgr)->calc();
}

// 0x7100af9828
void GameFrameworkNx::setGpuTimeStamp_()
{
    if ((mGpuTimeStampFlags & 3) == 3)
        return;
    GraphicsNvn::convertGPUTimeStampToSystemTick(_160);
    GraphicsNvn::convertGPUTimeStampToSystemTick(_160 + 1);
}

// 0x7100af8ef0
void GameFrameworkNx::outOfMemoryCallback_(NVNcommandBuffer*, NVNcommandBufferMemoryEvent, size_t,
                                           void*)
{
}

// 0x7100af8ef4
void GameFrameworkNx::presentAsync_(Thread*, long)
{
    present_();
}

// 0x7100af8f00
void GameFrameworkNx::requestChangeUseGPU(bool use_gpu)
{
    mUseGPURequest = use_gpu ? 1 : 2;
}

// 0x7100af8f18
void GameFrameworkNx::runImpl_()
{
    waitStartDisplayLoop_();
    mPrevFrameTick = nn::os::GetSystemTick().value;
    mainLoop_();
}

// 0x7100af8f54
MethodTreeMgr* GameFrameworkNx::createMethodTreeMgr_(Heap* heap)
{
    return new (heap, 8) SingleScreenMethodTreeMgr;
}

// 0x7100af8f14
void GameFrameworkNx::initRun_(Heap*)
{
    PerformanceMgrNx::printPerformance();
}

// 0x7100af8f84
void GameFrameworkNx::mainLoop_()
{
    while (true)
        procFrame_();
}

// 0x7100af9870
FrameBuffer* GameFrameworkNx::getMethodFrameBuffer(int method) const
{
    if (method >= 2 && method <= 4)
        return mMethodFrameBuffer;
    return nullptr;
}

// 0x7100af988c
LogicalFrameBuffer* GameFrameworkNx::getMethodLogicalFrameBuffer(int method) const
{
    return method >= 2 && method <= 4 ? const_cast<LogicalFrameBuffer*>(&mMethodLogicalFrameBuffer) :
                                        nullptr;
}

// 0x7100af9520
void GameFrameworkNx::swapBuffer_()
{
    if (mDisplayStarted == 2)
        mDisplayBuffer->presentTextureAndAcquireNext();
}

// 0x7100af9a34
float GameFrameworkNx::calcFps()
{
    return static_cast<f32>(TickSpan::makeFromSeconds(1).toS64()) / static_cast<f32>(static_cast<s64>(mFrameDuration));
}

// 0x7100af9a60
void GameFrameworkNx::setCaption(const SafeString&) {}

}  // namespace sead
