#include "gfx/nin/seadFrameBufferNvn.h"
#include <nvn/nvn_FuncPtrInline.h>
#include "gfx/nin/seadDisplayBufferNvn.h"
#include "gfx/seadDrawContext.h"

namespace sead
{
FrameBufferNvn::~FrameBufferNvn() = default;

void FrameBufferNvn::copyToDisplayBuffer(DrawContext* draw_context,
                                         const DisplayBuffer* display_buffer) const
{
    const auto* display = static_cast<const DisplayBufferNvn*>(display_buffer);
    const BoundBox2f& area = getPhysicalArea();

    NVNcopyRegion src_region = {0, 0, 0, s32(area.getMax().x - area.getMin().x),
                                s32(area.getMax().y - area.getMin().y), 1};
    NVNcopyRegion dst_region = {0, 0, 0, s32(display->getSize().x), s32(display->getSize().y), 1};

    nvnCommandBufferCopyTextureToTexture(
        draw_context->getCommandBuffer()->ToData()->pNvnCommandBuffer, mColorTarget, nullptr,
        &src_region, display->getAcquiredTexture(), nullptr, &dst_region,
        NVN_COPY_FLAGS_LINEAR_FILTER_BIT);
}

void FrameBufferNvn::clear(DrawContext* draw_context, u32 clr_flag, const Color4f& color,
                           f32 depth, u32 stencil) const
{
    if (clr_flag & cColor)
        nvnCommandBufferClearColor(draw_context->getCommandBuffer()->ToData()->pNvnCommandBuffer,
                                   0, &color.r, NVN_CLEAR_COLOR_MASK_RGBA);
    if (clr_flag & (cDepth | cStencil))
        nvnCommandBufferClearDepthStencil(
            draw_context->getCommandBuffer()->ToData()->pNvnCommandBuffer, depth,
            (clr_flag & cDepth) != 0, stencil, (clr_flag & cStencil) != 0);
}

void FrameBufferNvn::bindImpl_(DrawContext* draw_context) const
{
    nvnCommandBufferSetRenderTargets(draw_context->getCommandBuffer()->ToData()->pNvnCommandBuffer,
                                     1, &mColorTarget, nullptr, mDepthTarget, nullptr);
}
}  // namespace sead
