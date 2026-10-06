#include <gfx/seadCamera.h>
#include <gfx/seadProjection.h>
#include <gfx/seadPrimitiveRenderer.h>
#include "gfx/nin/seadPrimitiveDrawMgrNvn.h"

namespace sead
{
PrimitiveDrawer::PrimitiveDrawer(DrawContext* context)
    : mModel(const_cast<Matrix34f*>(&Matrix34f::ident)),
      mCamera(const_cast<Matrix34f*>(&Matrix34f::ident)),
      mProjection(const_cast<Matrix44f*>(&Matrix44f::ident)), mDrawContext(context)
{
}

PrimitiveDrawer::~PrimitiveDrawer() = default;

void PrimitiveDrawer::prepareMgr(Heap* heap, const SafeString& path)
{
    PrimitiveDrawMgrNvn::createInstance(heap)->prepareImpl(heap, path);
}

void PrimitiveDrawer::setCamera(const Camera* camera)
{
    mCamera = const_cast<Matrix34f*>(&camera->getMatrix());
}

void PrimitiveDrawer::setProjection(const Projection* projection)
{
    mProjection = const_cast<Matrix44f*>(&projection->getDeviceProjectionMatrix());
}

void PrimitiveDrawer::setModelMatrix(const Matrix34f* model_mtx)
{
    mModel = const_cast<Matrix34f*>(model_mtx);
}

void PrimitiveDrawer::begin()
{
    PrimitiveDrawMgrNvn::instance()->beginImpl(mDrawContext, *mCamera, *mProjection);
}

void PrimitiveDrawer::end()
{
    PrimitiveDrawMgrNvn::instance()->endImpl(mDrawContext);
}

void PrimitiveDrawer::drawQuad(const Color4f& color0, const Color4f& color1)
{
    PrimitiveDrawMgrNvn::instance()->drawQuadImpl(mDrawContext, *mModel, color0, color1);
}

void PrimitiveDrawer::drawBox(const Color4f& color0, const Color4f& color1)
{
    PrimitiveDrawMgrNvn::instance()->drawBoxImpl(mDrawContext, *mModel, color0, color1);
}

void PrimitiveDrawer::QuadArg::setColor(const Color4f& color0, const Color4f& color1)
{
    mHorizontal = false;
    mColor0 = color0;
    mColor1 = color1;
}

void PrimitiveDrawer::QuadArg::setColorHorizontal(const Color4f& color0, const Color4f& color1)
{
    mHorizontal = true;
    mColor0 = color0;
    mColor1 = color1;
}

// NON_MATCHING: same vector operations, but the scheduler places the lane inserts of the product differently.
void PrimitiveDrawer::drawWireCube(const CubeArg& arg)
{
    Matrix34f local;
    local.makeST(arg.mSize, arg.mCenter);
    Matrix34f mtx;
    mtx.setMul(*mModel, local);
    PrimitiveDrawMgrNvn::instance()->drawWireCubeImpl(mDrawContext, mtx, arg.mColor0, arg.mColor1);
}

// NON_MATCHING: same vector operations, but the scheduler places the lane inserts of the product differently.
void PrimitiveDrawer::drawSphere4x8(const Vector3f& pos, f32 radius, const Color4f& color)
{
    const f32 diameter = radius + radius;
    Matrix34f mtx;
    Matrix34f local;
    local.makeST(Vector3f(diameter, diameter, diameter), pos);
    mtx.setMul(*mModel, local);
    PrimitiveDrawMgrNvn::instance()->drawSphere4x8Impl(mDrawContext, mtx, color, color);
}

// NON_MATCHING: same vector operations, but the scheduler places the lane inserts of the product differently.
void PrimitiveDrawer::drawSphere8x16(const Vector3f& pos, f32 radius, const Color4f& color)
{
    const f32 diameter = radius + radius;
    Matrix34f local;
    local.makeST(Vector3f(diameter, diameter, diameter), pos);
    Matrix34f mtx;
    mtx.setMul(*mModel, local);
    PrimitiveDrawMgrNvn::instance()->drawSphere8x16Impl(mDrawContext, mtx, color, color);
}

// NON_MATCHING: same vector operations, but the scheduler places the lane inserts of the product differently.
void PrimitiveDrawer::drawCylinder32(const Vector3f& pos, f32 radius, f32 height,
                                     const Color4f& color)
{
    const f32 diameter = radius + radius;
    Matrix34f local;
    local.makeST(Vector3f(diameter, height, diameter), pos);
    Matrix34f mtx;
    mtx.setMul(*mModel, local);
    PrimitiveDrawMgrNvn::instance()->drawCylinder32Impl(mDrawContext, mtx, color, color);
}

void PrimitiveDrawer::drawLine(const Vector3f& from, const Vector3f& to, const Color4f& color)
{
    drawLine(from, to, color, color);
}
}  // namespace sead
