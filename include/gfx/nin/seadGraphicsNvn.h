#pragma once

#include <gfx/seadColor.h>
#include <gfx/seadGraphics.h>
#include <thread/seadCriticalSection.h>
#include <nn/gfx/gfx_Types.h>
#include <prim/seadDelegate.h>
#include <thread/seadAtomic.h>
#include "nvn/nvn.h"

namespace sead
{
class DisplayBufferNvn;

class GraphicsNvn : public Graphics
{
public:
    struct CreateArg
    {
        NVNdevice* device;
        u32 _8;
        bool _c;
        bool _d;
    };

    struct NvnDebugCallbackParam
    {
        NVNdebugCallbackSource source;
        NVNdebugCallbackType type;
        s32 id;
        NVNdebugCallbackSeverity severity;
        const char* message;
    };

    GraphicsNvn(const CreateArg& arg);
    ~GraphicsNvn() override;

    void initializeDrawLockContext(Heap*) override;
    void initializeImpl(Heap*) override;

    s32 getNewSamplerId();
    s32 getNewTextureId();

    void setDisplayBufferWindowCrop(s32, s32, s32, s32);
    void getDisplayBufferWindowCrop(s32*, s32*, s32*, s32*) const;

    void registerQueue(NVNqueue*);
    void registerDefaultCommandBuffer(NVNcommandBuffer*);
    void registerDisplayBufferNvn(DisplayBufferNvn*);
    void applyDeferredFinalizes();

    static void nvnDebugCallback(NVNdebugCallbackSource, NVNdebugCallbackType, s32,
                                 NVNdebugCallbackSeverity, const char*, void*);

    static u64 convertGPUTimeStampToSystemTick(const NVNcounterData*);
    static s32 convertNvnDebugLevel(u32);
    void setViewportImpl(f32, f32, f32, f32) override;
    void setScissorImpl(f32, f32, f32, f32) override;
    void setDepthTestEnableImpl(bool) override;
    void setDepthWriteEnableImpl(bool) override;
    void setDepthFuncImpl(Graphics::DepthFunc) override;
    bool setVBlankWaitIntervalImpl(u32) override;
    void setCullingModeImpl(Graphics::CullingMode) override;
    void setBlendEnableImpl(bool) override;
    void setBlendEnableMRTImpl(u32, bool) override;
    void setBlendFactorImpl(Graphics::BlendFactor, Graphics::BlendFactor, Graphics::BlendFactor,
                            Graphics::BlendFactor) override;
    void setBlendFactorMRTImpl(u32, Graphics::BlendFactor, Graphics::BlendFactor,
                               Graphics::BlendFactor, Graphics::BlendFactor) override;
    void setBlendEquationImpl(Graphics::BlendEquation, Graphics::BlendEquation) override;
    void setBlendEquationMRTImpl(u32, Graphics::BlendEquation, Graphics::BlendEquation) override;
    void setBlendConstantColorImpl(sead::Color4f const&) override;
    void waitForVBlankImpl() override;
    void setColorMaskImpl(bool, bool, bool, bool) override;
    void setColorMaskMRTImpl(u32, bool, bool, bool, bool) override;
    void setAlphaTestEnableImpl(bool) override;
    void setAlphaTestFuncImpl(Graphics::AlphaFunc, f32) override;
    void setStencilTestEnableImpl(bool) override;
    void setStencilTestFuncImpl(Graphics::StencilFunc, s32, u32) override;
    void setStencilTestOpImpl(Graphics::StencilOp, Graphics::StencilOp,
                              Graphics::StencilOp) override;
    void setPolygonModeImpl(Graphics::PolygonMode, Graphics::PolygonMode) override;
    void setPolygonOffsetEnableImpl(bool, bool, bool) override;

    NVNdevice* getNvnDevice() const { return mNvnDevice; }
    NVNqueue* getQueue() const { return mNvnQueue; }

    // Inline-only in the original; name is a guess. Screen and FontMgr pass +0x48
    // to NN gfx APIs; initializeImpl constructs its NN device at this address.
    nn::gfx::Device* getNnDevice() const { return mNnDevice; }

    NVNtexturePool* getTexturePool() { return &mNvnTexturePool; }

    s32 getTextureSamplerID() const { return mTextureSamplerID; }

    CriticalSection* getCriticalSection2() { return &mCriticalSection2; }

    static GraphicsNvn* instance() { return (GraphicsNvn*)Graphics::instance(); }

private:
    void defaultNvnDebugCallback_(const NvnDebugCallbackParam&);

    NVNdevice* mNvnDevice;
    NVNqueue* mNvnQueue;  // set by registerQueue; used to present the display buffer textures
    void* _40;
    nn::gfx::Device* mNnDevice;
    void* _50;
    NVNtexturePool mNvnTexturePool;
    void* _78;
    void* _80;
    void* _88;
    void* _90;
    void* _98;
    void* _A0;
    void* _A8;
    void* _B0;
    void* _B8;
    void* _C0;
    void* _C8;
    void* _D0;
    void* _D8;
    void* _E0;
    void* _E8;
    void* _F0;
    s32 mTextureSamplerID;
    u32 mVBlankWaitInterval;
    void* _100;
    Atomic<s32> mSamplerIdCounter;
    Atomic<s32> mTextureIdCounter;
    s32 _110;
    s32 _114;
    CriticalSection mCriticalSection1;
    CriticalSection mCriticalSection2;
    CriticalSection mCriticalSection3;
    Delegate1<GraphicsNvn, const NvnDebugCallbackParam&> mDefaultDebugCallback;
    IDelegate1<const NvnDebugCallbackParam&>* mDebugCallback;
    bool _200;
    bool _201;
    bool _202;
};
static_assert(sizeof(GraphicsNvn) == 0x208);

}  // namespace sead
