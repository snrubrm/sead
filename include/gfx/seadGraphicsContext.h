#pragma once

#include "basis/seadTypes.h"

namespace sead
{
class DrawContext;

class GraphicsContext
{
public:
    GraphicsContext();
    void apply(DrawContext* context) const;

    void setDepthTestEnable(bool enable) { mDepthTestEnable = enable; }
    void setDepthWriteEnable(bool enable) { mDepthWriteEnable = enable; }
    void setStencilTestEnable(bool enable) { mStencilTestEnable = enable; }
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
    bool mDepthWriteEnable = true;
    u8 _2 = 0;
    bool mStencilTestEnable = false;
    u32 mBlendEnableMask = 0xff;
    f32 mBlendColor[4] = {1, 1, 1, 1};
    u32 _18 = 0;
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
    bool mPolygonOffsetEnable = false;
    bool mDepthClampEnable = false;
    u32 _70 = 0xffffffff;
};
// LightMapMgr's six constructors use a 0x74 stride; OcclusionRenderer's two
// contexts independently end at the next member with the same stride.
static_assert(sizeof(GraphicsContext) == 0x74);
}  // namespace sead
