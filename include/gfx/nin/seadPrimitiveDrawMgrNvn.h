#pragma once

#include <nvn/nvn.h>
#include "devenv/seadFontMgr.h"
#include "gfx/seadColor.h"
#include "heap/seadDisposer.h"
#include "math/seadMatrix.h"
#include "math/seadVector.h"
#include "prim/seadSafeString.h"
#include "thread/seadAtomic.h"

namespace sead
{
class DrawContext;
class Heap;
class Texture;

/// Draws the primitives of PrimitiveDrawer with nvn. Every shape has a vertex and an index buffer
/// that is created when the manager is prepared (some shapes share the vertex buffer).
class PrimitiveDrawMgrNvn
{
    SEAD_SINGLETON_DISPOSER(PrimitiveDrawMgrNvn)
public:
    PrimitiveDrawMgrNvn();

    virtual void prepareFromBinaryImpl(Heap* heap, const void* binary, u32 size);
    virtual void prepareImpl(Heap* heap, const SafeString& path);
    virtual void beginImpl(DrawContext* context, const Matrix34f& camera,
                           const Matrix44f& projection);
    virtual void endImpl(DrawContext* context);
    virtual void drawQuadImpl(DrawContext* context, const Matrix34f& model, const Color4f& color0,
                              const Color4f& color1);
    virtual void drawQuadImpl(DrawContext* context, const Matrix34f& model, const Texture& texture,
                              const Color4f& color0, const Color4f& color1, const Vector2f& uv0,
                              const Vector2f& uv1);
    virtual void drawBoxImpl(DrawContext* context, const Matrix34f& model, const Color4f& color0,
                             const Color4f& color1);
    virtual void drawCubeImpl(DrawContext* context, const Matrix34f& model, const Color4f& color0,
                              const Color4f& color1);
    virtual void drawWireCubeImpl(DrawContext* context, const Matrix34f& model,
                                  const Color4f& color0, const Color4f& color1);
    virtual void drawLineImpl(DrawContext* context, const Matrix34f& model, const Color4f& color0,
                              const Color4f& color1);
    virtual void drawSphere4x8Impl(DrawContext* context, const Matrix34f& model,
                                   const Color4f& color0, const Color4f& color1);
    virtual void drawSphere8x16Impl(DrawContext* context, const Matrix34f& model,
                                    const Color4f& color0, const Color4f& color1);
    virtual void drawDisk16Impl(DrawContext* context, const Matrix34f& model,
                                const Color4f& color0, const Color4f& color1);
    virtual void drawDisk32Impl(DrawContext* context, const Matrix34f& model,
                                const Color4f& color0, const Color4f& color1);
    virtual void drawCircle16Impl(DrawContext* context, const Matrix34f& model,
                                  const Color4f& color);
    virtual void drawCircle32Impl(DrawContext* context, const Matrix34f& model,
                                  const Color4f& color);
    virtual void drawCylinder16Impl(DrawContext* context, const Matrix34f& model,
                                    const Color4f& color0, const Color4f& color1);
    virtual void drawCylinder32Impl(DrawContext* context, const Matrix34f& model,
                                    const Color4f& color0, const Color4f& color1);
    virtual ~PrimitiveDrawMgrNvn();

    void swapUniformBlockBuffer();

private:
    void drawImpl_(NVNcommandBuffer* command_buffer, NVNdrawPrimitive primitive,
                   const Matrix34f& model, const Color4f& color0, const Color4f& color1,
                   NVNbuffer* vertex_buffer, u32 vertex_count, NVNbuffer* index_buffer,
                   u32 index_count, const Texture* texture, const Vector2f* uv0,
                   const Vector2f* uv1);

    NVNprogram mProgram;
    u8 _e8[0x218 - 0xe8];
    NVNvertexAttribState mVertexAttribStates[3];
    u8 _224[0x228 - 0x224];
    NVNvertexStreamState mVertexStreamState;
    u8 _230[0x330 - 0x230];
    NVNbuffer mQuadVertexBuffer;
    NVNbuffer mQuadIndexBuffer;
    NVNbuffer mBoxIndexBuffer;
    NVNbuffer mLineVertexBuffer;
    NVNbuffer mLineIndexBuffer;
    NVNbuffer mCubeVertexBuffer;
    NVNbuffer mCubeIndexBuffer;
    NVNbuffer mWireCubeVertexBuffer;
    NVNbuffer mWireCubeIndexBuffer;
    NVNbuffer mSphere4x8VertexBuffer;
    NVNbuffer mSphere4x8IndexBuffer;
    NVNbuffer mSphere8x16VertexBuffer;
    NVNbuffer mSphere8x16IndexBuffer;
    NVNbuffer mDisk16VertexBuffer;
    NVNbuffer mDisk16IndexBuffer;
    NVNbuffer mDisk32VertexBuffer;
    NVNbuffer mDisk32IndexBuffer;
    NVNbuffer mCircle16IndexBuffer;
    NVNbuffer mCircle32IndexBuffer;
    NVNbuffer mCylinder16VertexBuffer;
    NVNbuffer mCylinder16IndexBuffer;
    NVNbuffer mCylinder32VertexBuffer;
    NVNbuffer mCylinder32IndexBuffer;
    NVNbuffer mUniformBuffer;
    void* _7b0;
    UniformBlockBuffer mUniformBlockBuffer;
    u32 mUniformBlockBufferSize;
    bool _7c4;
    bool _7c5;
};
static_assert(sizeof(PrimitiveDrawMgrNvn) == 0x7c8);

}  // namespace sead
