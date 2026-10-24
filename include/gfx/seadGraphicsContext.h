#pragma once

#include "basis/seadTypes.h"
#include "prim/seadBitFlag.h"

namespace sead
{
class DrawContext;

class GraphicsContext
{
public:
    GraphicsContext();
    void apply(DrawContext* context) const;

    void setDepthTestEnable(bool enable) { mDepthTestEnable = enable; }
    void setDepthWriteEnable(u8 enable) { mDepthWriteEnable = enable; }
    void setStencilTestEnable(bool enable) { mStencilTestEnable = enable; }
    // Inline-only in the original; name is a guess. Native b1cef8 passes
    // +0x65 directly to NVN SetCullFace (0 none, 1 front, 2 back, 3 both),
    // and independent draw caller 1059c00 explicitly writes the back value.
    void setCullFace(u8 face) { mCullFace = face; }
    // Inline-only in the original; names are guesses. agl::cull::OcclusionQueryDrawer::draw
    // 0x71013ec42c clears the whole blend-enable mask (+0x4) and the channel mask (+0x1c) and sets
    // the depth clamp byte (+0x6e).
    void setBlendEnableMask(u32 mask) { mBlendEnableMask = mask; }
    void setColorMask(u32 mask) { mChannelMask = mask; }
    // Inline-only in the original; name is a guess. OcclusionQueryDrawer::drawDebug 0x71013ec75c
    // ORs 0xf into the channel mask (all four channels of target 0).
    void setColorMask(u32 target, bool r, bool g, bool b, bool a)
    {
        const u32 shift = target * 4;
        const u32 bits = (r ? 1 : 0) | (g ? 2 : 0) | (b ? 4 : 0) | (a ? 8 : 0);
        mChannelMask = (mChannelMask & ~(0xfu << shift)) | (bits << shift);
    }
    // Inline-only in the original; name is a guess. Native apply 0x7100b1cef8 reads only the
    // first byte, but ModelRenderCache::reset 0x7100c80574 stores the same mode into both
    // adjacent bytes at +0x6b / +0x6c, like Graphics::setPolygonMode's front/back pair.
    void setPolygonMode(u8 mode, u8 back_mode) { mPolygonMode = mode; _6c = back_mode; }
    // Native 0x7100b1cef8 reads bit 0 of the byte at +0x6d; ModelRenderCache::reset
    // 0x7100c80574 changes that bit in place, so the field is a flag byte.
    void setPolygonOffsetEnable(bool enable) { mPolygonOffsetEnable.changeBit(0, enable); }
    void setDepthClampEnable(bool enable) { mDepthClampEnable = enable; }
    // ModelRenderCache's constructor 0x7100c80100 writes the source/destination factors of all
    // eight blend targets and the equation of the targets after the first one.
    void setBlendControl(u32 target, u8 src_rgb, u8 src_alpha, u8 dst_rgb, u8 dst_alpha,
                         u8 equation_rgb, u8 equation_alpha)
    {
        BlendControl& control = mBlendControl[target];
        control.src_rgb = src_rgb;
        control.src_alpha = src_alpha;
        control.dst_rgb = dst_rgb;
        control.dst_alpha = dst_alpha;
        control.equation_rgb = equation_rgb;
        control.equation_alpha = equation_alpha;
    }
    // Writes the depth comparison byte (+0x64) and the byte at +0x66.
    void setDepthFuncMaybe(u8 func, u8 unk66)
    {
        mDepthFunc = func;
        _66 = unk66;
    }
    // ModelRenderCache's constructor also clears the byte at +0x2 and writes 0.5f at +0x18.
    // The meanings of both fields are unrecovered.
    void setUnk2Maybe(u8 value) { _2 = value; }
    void setBlendAlphaMaybe(f32 value) { _18 = value; }
    void setBlendEnable(bool enable, u32 target)
    {
        if (enable)
            mBlendEnableMask |= 1u << target;
        else
            mBlendEnableMask &= ~(1u << target);
    }

private:
    // B1CEF8 passes these six bytes to NVN blend-function/equation setters.
    struct BlendControl
    {
        u8 src_rgb = 5;
        u8 src_alpha = 5;
        u8 dst_rgb = 6;
        u8 dst_alpha = 6;
        u8 equation_rgb = 1;
        u8 equation_alpha = 1;
    };

    bool mDepthTestEnable = true;
    u8 mDepthWriteEnable = 1;
    u8 _2 = 0;
    bool mStencilTestEnable = false;
    u32 mBlendEnableMask = 0xff;
    f32 mBlendColor[4] = {1, 1, 1, 1};
    f32 _18 = 0;
    u32 mChannelMask = 0xffffffff;
    u32 mStencilRef = 0;
    u32 mStencilValueMask = 0xffffffff;
    BlendControl mBlendControl[8];
    f32 mPolygonOffsetFactor = 0;
    f32 mPolygonOffsetUnits = 0;
    f32 mPolygonOffsetClamp = 0;
    u8 mDepthFunc = 4;
    u8 mCullFace = 2;
    u8 _66 = 5;
    u8 mStencilFailOp = 1;
    u8 mStencilDepthFailOp = 1;
    u8 mStencilDepthPassOp = 1;
    u8 mStencilFunc = 1;
    u8 mPolygonMode = 2;
    u8 _6c = 2;
    sead::BitFlag8 mPolygonOffsetEnable;
    bool mDepthClampEnable = false;
    u32 _70 = 0xffffffff;
};
// LightMapMgr's six constructors use a 0x74 stride; OcclusionRenderer's two
// contexts independently end at the next member with the same stride.
static_assert(sizeof(GraphicsContext) == 0x74);
}  // namespace sead
