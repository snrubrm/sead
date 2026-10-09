#include "gfx/seadGraphicsContext.h"
#include "gfx/seadDrawContext.h"
#include <nvn/nvn_FuncPtrInline.h>

namespace sead
{
// NON_MATCHING: initialization of the byte-sized blend states is scheduled differently.
GraphicsContext::GraphicsContext() = default;

// NON_MATCHING: the NVN state objects and target loops use different stack/register scheduling.
void GraphicsContext::apply(DrawContext* context) const
{
    NVNcommandBuffer* command_buffer = context->getCommandBuffer()->ToData()->pNvnCommandBuffer;
    NVNblendState blend;
    NVNcolorState color;
    nvnBlendStateSetDefaults(&blend);
    nvnColorStateSetDefaults(&color);
    if (mBlendEnableMask)
    {
        for (s32 target = 0; target < 8; ++target)
        {
            if (!(mBlendEnableMask & (1u << target)))
                continue;
            nvnColorStateSetBlendEnable(&color, target, true);
            nvnBlendStateSetBlendTarget(&blend, target);
            const BlendControl& control = mBlendControl[target];
            nvnBlendStateSetBlendFunc(&blend, NVNblendFunc(control.src_rgb),
                                     NVNblendFunc(control.dst_rgb),
                                     NVNblendFunc(control.src_alpha),
                                     NVNblendFunc(control.dst_alpha));
            nvnBlendStateSetBlendEquation(&blend, NVNblendEquation(control.equation_rgb),
                                         NVNblendEquation(control.equation_alpha));
            nvnCommandBufferBindBlendState(command_buffer, &blend);
        }
    }
    nvnColorStateSetLogicOp(&color, NVN_LOGIC_OP_COPY);
    nvnCommandBufferBindColorState(command_buffer, &color);
    nvnCommandBufferSetBlendColor(command_buffer, mBlendColor);

    NVNchannelMaskState channel_mask;
    for (s32 target = 0; target < 8; ++target)
    {
        const u32 mask = mChannelMask >> (target * 4);
        nvnChannelMaskStateSetChannelMask(&channel_mask, target, mask & 1, (mask >> 1) & 1,
                                         (mask >> 2) & 1, (mask >> 3) & 1);
    }
    nvnCommandBufferBindChannelMaskState(command_buffer, &channel_mask);

    NVNdepthStencilState depth_stencil;
    nvnDepthStencilStateSetDepthTestEnable(&depth_stencil, mDepthTestEnable);
    nvnDepthStencilStateSetDepthWriteEnable(&depth_stencil, mDepthWriteEnable);
    nvnDepthStencilStateSetDepthFunc(&depth_stencil, NVNdepthFunc(mDepthFunc));
    nvnDepthStencilStateSetStencilTestEnable(&depth_stencil, mStencilTestEnable);
    nvnDepthStencilStateSetStencilFunc(&depth_stencil, NVN_FACE_FRONT_AND_BACK,
                                     NVNstencilFunc(mStencilFunc));
    nvnDepthStencilStateSetStencilOp(&depth_stencil, NVN_FACE_FRONT_AND_BACK,
                                   NVNstencilOp(mStencilFailOp), NVNstencilOp(mStencilDepthFailOp),
                                   NVNstencilOp(mStencilDepthPassOp));
    nvnCommandBufferBindDepthStencilState(command_buffer, &depth_stencil);
    if (mStencilTestEnable)
    {
        nvnCommandBufferSetStencilValueMask(command_buffer, NVN_FACE_FRONT_AND_BACK,
                                          mStencilValueMask);
        nvnCommandBufferSetStencilMask(command_buffer, NVN_FACE_FRONT_AND_BACK, 0xff);
        nvnCommandBufferSetStencilRef(command_buffer, NVN_FACE_FRONT_AND_BACK, mStencilRef);
    }

    NVNpolygonState polygon;
    nvnPolygonStateSetCullFace(&polygon, NVNface(mCullFace));
    nvnPolygonStateSetFrontFace(&polygon, NVN_FRONT_FACE_CCW);
    nvnPolygonStateSetPolygonMode(&polygon, NVNpolygonMode(mPolygonMode));
    nvnPolygonStateSetPolygonOffsetEnables(&polygon,
                                         mPolygonOffsetEnable ? NVN_POLYGON_OFFSET_ENABLE_FILL : 0);
    nvnCommandBufferBindPolygonState(command_buffer, &polygon);
    nvnCommandBufferSetPolygonOffsetClamp(command_buffer, mPolygonOffsetFactor, mPolygonOffsetUnits,
                                        mPolygonOffsetClamp);
    nvnCommandBufferSetDepthClamp(command_buffer, mDepthClampEnable);
}
}  // namespace sead
