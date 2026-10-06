#include <framework/nx/seadGameFrameworkNx.h>
#include <framework/nx/seadPerformanceMgrNx.h>

namespace sead
{
// 0x7100af8510 (D1) / 0x7100af8528 (D0)
// The body keeps the store of the GameFrameworkNx vtable pointer before the tail call to ~GameFramework (a defaulted or
// empty destructor drops it).
GameFrameworkNx::~GameFrameworkNx() { ; }

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

// 0x7100af9a60
void GameFrameworkNx::setCaption(const SafeString&) {}

}  // namespace sead
