#include "gfx/nin/seadPrimitiveDrawMgrNvn.h"
#include <nvn/nvn_FuncPtrInline.h>
#include "gfx/seadDrawContext.h"
#include "math/seadMatrix.hpp"
#include "math/seadMatrixCalcCommon.hpp"
#include "prim/seadPtrUtil.h"

namespace sead
{
SEAD_SINGLETON_DISPOSER_IMPL(PrimitiveDrawMgrNvn)

PrimitiveDrawMgrNvn::PrimitiveDrawMgrNvn() : _7b0(nullptr), mUniformBlockBuffer(0, 0), mUniformBlockBufferSize(0x32000), _7c4(false), _7c5(false)
{
}

PrimitiveDrawMgrNvn::~PrimitiveDrawMgrNvn() = default;

// NON_MATCHING: the projection * camera multiplication is inlined (vectorised) in the original; here it stays a call
// 0x7100b021d8
void PrimitiveDrawMgrNvn::beginImpl(DrawContext* context, const Matrix34f& camera,
                                    const Matrix44f& projection)
{
    if (_7c4)
        return;

    const u32 offset = mUniformBlockBuffer.fetchAdd(0x100);
    if (offset + 0x100 - mUniformBlockBuffer.get_4() > mUniformBlockBufferSize)
    {
        _7c4 = true;
        return;
    }

    const u32 position = offset % mUniformBlockBufferSize;
    NVNcommandBuffer* command_buffer = context->getCommandBuffer()->ToData()->pNvnCommandBuffer;
    nvnCommandBufferBindProgram(command_buffer, &mProgram, 0x1f);
    nvnCommandBufferBindVertexAttribState(command_buffer, 3, mVertexAttribStates);
    nvnCommandBufferBindVertexStreamState(command_buffer, 1, &mVertexStreamState);

    static_cast<Matrix44f*>(PtrUtil::addOffset(_7b0, position))->setMul(projection, camera);
    nvnCommandBufferBindUniformBuffer(command_buffer, NVN_SHADER_STAGE_VERTEX, 0,
                                      nvnBufferGetAddress(&mUniformBuffer) + position, 0x40);
}

void PrimitiveDrawMgrNvn::swapUniformBlockBuffer()
{
    mUniformBlockBuffer.swap(mUniformBlockBuffer.get_0(), mUniformBlockBufferSize);
    _7c4 = false;
}

void PrimitiveDrawMgrNvn::endImpl(DrawContext*) {}

void PrimitiveDrawMgrNvn::drawQuadImpl(DrawContext* context, const Matrix34f& model,
                                       const Texture& texture, const Color4f& color0,
                                       const Color4f& color1, const Vector2f& uv0,
                                       const Vector2f& uv1)
{
    drawImpl_(context->getCommandBuffer()->ToData()->pNvnCommandBuffer, NVN_DRAW_PRIMITIVE_TRIANGLES,
              model, color0, color1, &mQuadVertexBuffer, 4, &mQuadIndexBuffer, 6, &texture, &uv0,
              &uv1);
}

void PrimitiveDrawMgrNvn::drawQuadImpl(DrawContext* context, const Matrix34f& model,
                                       const Color4f& color0, const Color4f& color1)
{
    drawImpl_(context->getCommandBuffer()->ToData()->pNvnCommandBuffer, NVN_DRAW_PRIMITIVE_TRIANGLES,
              model, color0, color1, &mQuadVertexBuffer, 4, &mQuadIndexBuffer, 6, nullptr, nullptr, nullptr);
}

void PrimitiveDrawMgrNvn::drawBoxImpl(DrawContext* context, const Matrix34f& model,
                                      const Color4f& color0, const Color4f& color1)
{
    drawImpl_(context->getCommandBuffer()->ToData()->pNvnCommandBuffer, NVN_DRAW_PRIMITIVE_LINE_LOOP,
              model, color0, color1, &mQuadVertexBuffer, 4, &mBoxIndexBuffer, 4, nullptr, nullptr, nullptr);
}

void PrimitiveDrawMgrNvn::drawCubeImpl(DrawContext* context, const Matrix34f& model,
                                       const Color4f& color0, const Color4f& color1)
{
    drawImpl_(context->getCommandBuffer()->ToData()->pNvnCommandBuffer, NVN_DRAW_PRIMITIVE_TRIANGLES,
              model, color0, color1, &mCubeVertexBuffer, 8, &mCubeIndexBuffer, 36, nullptr, nullptr, nullptr);
}

void PrimitiveDrawMgrNvn::drawWireCubeImpl(DrawContext* context, const Matrix34f& model,
                                           const Color4f& color0, const Color4f& color1)
{
    drawImpl_(context->getCommandBuffer()->ToData()->pNvnCommandBuffer, NVN_DRAW_PRIMITIVE_LINE_LOOP,
              model, color0, color1, &mWireCubeVertexBuffer, 8, &mWireCubeIndexBuffer, 17, nullptr, nullptr, nullptr);
}

void PrimitiveDrawMgrNvn::drawLineImpl(DrawContext* context, const Matrix34f& model,
                                       const Color4f& color0, const Color4f& color1)
{
    drawImpl_(context->getCommandBuffer()->ToData()->pNvnCommandBuffer, NVN_DRAW_PRIMITIVE_LINE_LOOP,
              model, color0, color1, &mLineVertexBuffer, 2, &mLineIndexBuffer, 2, nullptr, nullptr, nullptr);
}

void PrimitiveDrawMgrNvn::drawSphere4x8Impl(DrawContext* context, const Matrix34f& model,
                                            const Color4f& color0, const Color4f& color1)
{
    drawImpl_(context->getCommandBuffer()->ToData()->pNvnCommandBuffer, NVN_DRAW_PRIMITIVE_TRIANGLES,
              model, color0, color1, &mSphere4x8VertexBuffer, 34, &mSphere4x8IndexBuffer, 192, nullptr, nullptr, nullptr);
}

void PrimitiveDrawMgrNvn::drawSphere8x16Impl(DrawContext* context, const Matrix34f& model,
                                             const Color4f& color0, const Color4f& color1)
{
    drawImpl_(context->getCommandBuffer()->ToData()->pNvnCommandBuffer, NVN_DRAW_PRIMITIVE_TRIANGLES,
              model, color0, color1, &mSphere8x16VertexBuffer, 130, &mSphere8x16IndexBuffer, 768, nullptr, nullptr, nullptr);
}

void PrimitiveDrawMgrNvn::drawDisk16Impl(DrawContext* context, const Matrix34f& model,
                                         const Color4f& color0, const Color4f& color1)
{
    drawImpl_(context->getCommandBuffer()->ToData()->pNvnCommandBuffer, NVN_DRAW_PRIMITIVE_TRIANGLES,
              model, color0, color1, &mDisk16VertexBuffer, 17, &mDisk16IndexBuffer, 48, nullptr, nullptr, nullptr);
}

void PrimitiveDrawMgrNvn::drawDisk32Impl(DrawContext* context, const Matrix34f& model,
                                         const Color4f& color0, const Color4f& color1)
{
    drawImpl_(context->getCommandBuffer()->ToData()->pNvnCommandBuffer, NVN_DRAW_PRIMITIVE_TRIANGLES,
              model, color0, color1, &mDisk32VertexBuffer, 33, &mDisk32IndexBuffer, 96, nullptr, nullptr, nullptr);
}

void PrimitiveDrawMgrNvn::drawCircle16Impl(DrawContext* context, const Matrix34f& model,
                                           const Color4f& color)
{
    drawImpl_(context->getCommandBuffer()->ToData()->pNvnCommandBuffer, NVN_DRAW_PRIMITIVE_LINE_LOOP,
              model, color, color, &mDisk16VertexBuffer, 17, &mCircle16IndexBuffer, 16, nullptr, nullptr, nullptr);
}

void PrimitiveDrawMgrNvn::drawCircle32Impl(DrawContext* context, const Matrix34f& model,
                                           const Color4f& color)
{
    drawImpl_(context->getCommandBuffer()->ToData()->pNvnCommandBuffer, NVN_DRAW_PRIMITIVE_LINE_LOOP,
              model, color, color, &mDisk32VertexBuffer, 33, &mCircle32IndexBuffer, 32, nullptr, nullptr, nullptr);
}

void PrimitiveDrawMgrNvn::drawCylinder16Impl(DrawContext* context, const Matrix34f& model,
                                             const Color4f& color0, const Color4f& color1)
{
    drawImpl_(context->getCommandBuffer()->ToData()->pNvnCommandBuffer, NVN_DRAW_PRIMITIVE_TRIANGLES,
              model, color0, color1, &mCylinder16VertexBuffer, 34, &mCylinder16IndexBuffer, 192, nullptr, nullptr, nullptr);
}

void PrimitiveDrawMgrNvn::drawCylinder32Impl(DrawContext* context, const Matrix34f& model,
                                             const Color4f& color0, const Color4f& color1)
{
    drawImpl_(context->getCommandBuffer()->ToData()->pNvnCommandBuffer, NVN_DRAW_PRIMITIVE_TRIANGLES,
              model, color0, color1, &mCylinder32VertexBuffer, 66, &mCylinder32IndexBuffer, 384, nullptr, nullptr, nullptr);
}
}  // namespace sead
