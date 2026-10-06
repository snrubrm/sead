#include "gfx/nin/seadGraphicsNvn.h"
#include <nvn/nvn_FuncPtrInline.h>

namespace sead
{
GraphicsNvn::GraphicsNvn(const CreateArg& arg)
    : mNvnDevice(arg.device), _38(nullptr), _40(nullptr), mNnDevice(nullptr), _50(nullptr),
      mTextureSamplerID(0), mVBlankWaitInterval(0), mSamplerIdCounter(0), mTextureIdCounter(0),
      _110(arg._8), _114(0x1000),
      mDefaultDebugCallback(this, &GraphicsNvn::defaultNvnDebugCallback_),
      mDebugCallback(&mDefaultDebugCallback), _200(false), _201(arg._c), _202(arg._d)
{
    nvnDeviceSetWindowOriginMode(mNvnDevice, NVN_WINDOW_ORIGIN_MODE_UPPER_LEFT);
}

GraphicsNvn::~GraphicsNvn() = default;

void GraphicsNvn::defaultNvnDebugCallback_(const NvnDebugCallbackParam&) {}

void GraphicsNvn::initializeDrawLockContext(Heap* heap)
{
    Graphics::initializeDrawLockContext(heap);
}

s32 GraphicsNvn::getNewSamplerId()
{
    return mSamplerIdCounter.increment();
}

s32 GraphicsNvn::getNewTextureId()
{
    return mTextureIdCounter.increment();
}

void GraphicsNvn::applyDeferredFinalizes()
{
    if (_201)
        nvnDeviceApplyDeferredFinalizes(mNvnDevice, 5);
}

void GraphicsNvn::nvnDebugCallback(NVNdebugCallbackSource source, NVNdebugCallbackType type,
                                   s32 id, NVNdebugCallbackSeverity severity, const char* message,
                                   void*)
{
    if (GraphicsNvn* graphics = instance())
    {
        NvnDebugCallbackParam param{source, type, id, severity, message};
        if (graphics->mDebugCallback)
            graphics->mDebugCallback->invoke(param);
    }
}

s32 GraphicsNvn::convertNvnDebugLevel(u32 level)
{
    switch (level)
    {
    case 0:
        return 0x20;
    case 1:
        return 0x40;
    case 2:
        return 1;
    case 3:
        return 4;
    case 4:
        return 0x10;
    default:
        return 0x20;
    }
}

void GraphicsNvn::setViewportImpl(f32, f32, f32, f32) {}
void GraphicsNvn::setScissorImpl(f32, f32, f32, f32) {}
void GraphicsNvn::setDepthTestEnableImpl(bool) {}
void GraphicsNvn::setDepthWriteEnableImpl(bool) {}
void GraphicsNvn::setDepthFuncImpl(Graphics::DepthFunc) {}
void GraphicsNvn::setCullingModeImpl(Graphics::CullingMode) {}
void GraphicsNvn::setBlendEnableImpl(bool) {}
void GraphicsNvn::setBlendEnableMRTImpl(u32, bool) {}
void GraphicsNvn::setBlendFactorImpl(Graphics::BlendFactor, Graphics::BlendFactor,
                                     Graphics::BlendFactor, Graphics::BlendFactor)
{
}
void GraphicsNvn::setBlendFactorMRTImpl(u32, Graphics::BlendFactor, Graphics::BlendFactor,
                                        Graphics::BlendFactor, Graphics::BlendFactor)
{
}
void GraphicsNvn::setBlendEquationImpl(Graphics::BlendEquation, Graphics::BlendEquation) {}
void GraphicsNvn::setBlendEquationMRTImpl(u32, Graphics::BlendEquation, Graphics::BlendEquation) {}
void GraphicsNvn::setBlendConstantColorImpl(const Color4f&) {}
void GraphicsNvn::waitForVBlankImpl() {}
void GraphicsNvn::setColorMaskImpl(bool, bool, bool, bool) {}
void GraphicsNvn::setColorMaskMRTImpl(u32, bool, bool, bool, bool) {}
void GraphicsNvn::setAlphaTestEnableImpl(bool) {}
void GraphicsNvn::setAlphaTestFuncImpl(Graphics::AlphaFunc, f32) {}
void GraphicsNvn::setStencilTestEnableImpl(bool) {}
void GraphicsNvn::setStencilTestFuncImpl(Graphics::StencilFunc, s32, u32) {}
void GraphicsNvn::setStencilTestOpImpl(Graphics::StencilOp, Graphics::StencilOp,
                                       Graphics::StencilOp)
{
}
void GraphicsNvn::setPolygonModeImpl(Graphics::PolygonMode, Graphics::PolygonMode) {}
void GraphicsNvn::setPolygonOffsetEnableImpl(bool, bool, bool) {}
}  // namespace sead
