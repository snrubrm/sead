#pragma once

#include <nvn/nvn.h>
#include "gfx/seadFrameBuffer.h"

namespace sead
{
class Heap;

class FrameBufferNvn : public FrameBuffer
{
    SEAD_RTTI_OVERRIDE(FrameBufferNvn, FrameBuffer)
public:
    ~FrameBufferNvn() override;

    void copyToDisplayBuffer(DrawContext* draw_context,
                             const DisplayBuffer* display_buffer) const override;
    void clear(DrawContext* draw_context, u32 clr_flag, const Color4f& color, f32 depth,
               u32 stencil) const override;
    void bindImpl_(DrawContext* draw_context) const override;

    static FrameBufferNvn* create(Heap* heap, const Vector2f& size, u32 color_format,
                                  u32 depth_format);

private:
    NVNtexture* mColorTarget;
    NVNtexture* mDepthTarget;
};
static_assert(sizeof(FrameBufferNvn) == 0x30);

}  // namespace sead
