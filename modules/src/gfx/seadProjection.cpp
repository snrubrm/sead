#include <gfx/seadProjection.h>

#include <gfx/seadCamera.h>
#include <gfx/seadViewport.h>

namespace sead
{
Projection::Projection()
{
    mDevicePosture = Graphics::sDefaultDevicePosture;
    mDeviceZScale = Graphics::sDefaultDeviceZScale;
    mDeviceZOffset = Graphics::sDefaultDeviceZOffset;
}

void Projection::updateAttributesForDirectProjection() {}

const Matrix44f& Projection::getProjectionMatrix() const
{
    updateMatrixImpl_();
    return mMatrix;
}

void Projection::updateMatrixImpl_() const
{
    if (mDirty)
    {
        doUpdateMatrix(const_cast<Matrix44f*>(&mMatrix));
        mDirty = false;
        mDeviceDirty = true;
    }

    if (mDeviceDirty)
    {
        doUpdateDeviceMatrix(const_cast<Matrix44f*>(&mDeviceMatrix), mMatrix, mDevicePosture);
        mDeviceDirty = false;
    }
}

Matrix44f* Projection::getProjectionMatrixMutable()
{
    updateMatrixImpl_();
    return &mMatrix;
}

const Matrix44f& Projection::getDeviceProjectionMatrix() const
{
    updateMatrixImpl_();
    return mDeviceMatrix;
}

void Projection::cameraPosToScreenPos(Vector3f* screen_pos, const Vector3f& camera_pos) const
{
    screen_pos->setMul(getProjectionMatrix(), camera_pos);
}

void Projection::screenPosToCameraPos(Vector3f* camera_pos, const Vector3f& screen_pos) const
{
    doScreenPosToCameraPosTo(camera_pos, screen_pos);
}

void Projection::screenPosToCameraPos(Vector3f* camera_pos, const Vector2f& screen_pos) const
{
    screenPosToCameraPos(camera_pos, {screen_pos.x, screen_pos.y, 0.0f});
}

void Projection::project(Vector2f* dst, const Vector3f& camera_pos, const Viewport& viewport) const
{
    Vector3f screen_pos;
    cameraPosToScreenPos(&screen_pos, camera_pos);
    viewport.project(dst, screen_pos);
}

void Projection::unproject(Vector3f* world_pos, const Vector3f& screen_pos,
                           const Camera& camera) const
{
    Vector3f camera_pos;
    screenPosToCameraPos(&camera_pos, screen_pos);
    camera.cameraPosToWorldPosByMatrix(world_pos, camera_pos);
}

void Projection::unprojectRay(Ray<Vector3f>* dst, const Vector3f& screen_pos,
                              const Camera& camera) const
{
    Vector3f camera_pos;
    screenPosToCameraPos(&camera_pos, screen_pos);
    camera.unprojectRayByMatrix(dst, camera_pos);
}

f32 PerspectiveProjection::getNear() const { return mNear; }
f32 PerspectiveProjection::getFar() const { return mFar; }
f32 PerspectiveProjection::getFovy() const { return mFovyRad; }
f32 PerspectiveProjection::getAspect() const { return mAspect; }

void PerspectiveProjection::getOffset(Vector2f* offset) const
{
    offset->x = mOffset.x;
    offset->y = mOffset.y;
}

void PerspectiveProjection::doScreenPosToCameraPosTo(Vector3f* camera_pos,
                                                     const Vector3f& screen_pos) const
{
    camera_pos->set(0.0f, 0.0f, -mNear);
    camera_pos->y = (2.0f * mNear * mFovyTan) * 0.5f * (screen_pos.y + 2.0f * mOffset.y);
    camera_pos->x = (2.0f * mNear * mFovyTan * mAspect) * 0.5f *
                    (screen_pos.x + 2.0f * mOffset.x);
}

// NON_MATCHING: equivalent projection coefficients, with different row-write scheduling.
void PerspectiveProjection::doUpdateMatrix(Matrix44f* mtx) const
{
    const f32 height = 2.0f * mNear * mFovyTan;
    const f32 width = height * mAspect;
    const f32 left = width * mOffset.x - width * 0.5f;
    const f32 right = width * 0.5f + width * mOffset.x;
    const f32 bottom = height * mOffset.y - height * 0.5f;
    const f32 top = height * 0.5f + height * mOffset.y;
    const f32 inverse_width = 1.0f / (right - left);
    const f32 inverse_height = 1.0f / (top - bottom);
    const f32 inverse_depth = 1.0f / (mFar - mNear);
    mtx->setRow(0, {2.0f * mNear * inverse_width, 0.0f,
                    (right + left) * inverse_width, 0.0f});
    mtx->setRow(1, {0.0f, inverse_height * (2.0f * mNear),
                    (top + bottom) * inverse_height, 0.0f});
    mtx->setRow(2, {0.0f, 0.0f, -inverse_depth * (mFar + mNear),
                    -inverse_depth * (2.0f * mFar * mNear)});
    mtx->setRow(3, {0.0f, 0.0f, -1.0f, 0.0f});
}

f32 OrthoProjection::getNear() const { return mNear; }
f32 OrthoProjection::getFar() const { return mFar; }
f32 OrthoProjection::getFovy() const { return 0.0f; }
f32 OrthoProjection::getAspect() const { return (mRight - mLeft) / (mTop - mBottom); }

void OrthoProjection::getOffset(Vector2f* offset) const
{
    offset->x = (mLeft + mRight) * 0.5f / (mRight - mLeft);
    offset->y = (mTop + mBottom) * 0.5f / (mTop - mBottom);
}

// NON_MATCHING: equivalent screen conversion, with different arithmetic scheduling.
void OrthoProjection::doScreenPosToCameraPosTo(Vector3f* camera_pos,
                                               const Vector3f& screen_pos) const
{
    camera_pos->x = (mRight + mLeft) * 0.5f + screen_pos.x * (mRight - mLeft) * 0.5f;
    camera_pos->y = (mTop + mBottom) * 0.5f + screen_pos.y * (mTop - mBottom) * 0.5f;
    camera_pos->z = -mNear;
}

void OrthoProjection::setTBLR(f32 top, f32 bottom, f32 left, f32 right)
{
    mTop = top;
    mBottom = bottom;
    mLeft = left;
    mRight = right;
    setDirty();
}

// NON_MATCHING: equivalent projection coefficients, with different row-write scheduling.
void OrthoProjection::doUpdateMatrix(Matrix44f* mtx) const
{
    const f32 half_width = (mRight - mLeft) * 0.5f;
    const f32 half_height = (mTop - mBottom) * 0.5f;
    const f32 inverse_depth = 1.0f / (mFar - mNear);
    mtx->setRow(0, {1.0f / half_width, 0.0f, 0.0f,
                    (mLeft + mRight) * -0.5f / half_width});
    mtx->setRow(1, {0.0f, 1.0f / half_height, 0.0f,
                    (mTop + mBottom) * -0.5f / half_height});
    mtx->setRow(2, {0.0f, 0.0f, inverse_depth * -2.0f,
                    -inverse_depth * (mNear + mFar)});
    mtx->setRow(3, {0.0f, 0.0f, 0.0f, 1.0f});
}

f32 FrustumProjection::getNear() const { return mNear; }
f32 FrustumProjection::getFar() const { return mFar; }
f32 FrustumProjection::getAspect() const { return (mRight - mLeft) / (mTop - mBottom); }

f32 FrustumProjection::getFovy() const
{
    return 2.0f * Mathf::atan2((mTop - mBottom) * 0.5f, getNear());
}

// NON_MATCHING: equivalent offset arithmetic, with different scheduling.
void FrustumProjection::getOffset(Vector2f* offset) const
{
    offset->x = (mRight + mLeft) * 0.5f / (mRight - mLeft);
    offset->y = (mTop + mBottom) * 0.5f / (mTop - mBottom);
}

// NON_MATCHING: equivalent screen conversion, with different arithmetic scheduling.
void FrustumProjection::doScreenPosToCameraPosTo(Vector3f* camera_pos,
                                                 const Vector3f& screen_pos) const
{
    camera_pos->z = -mNear;
    camera_pos->x = (mRight + mLeft) * 0.5f + (mRight - mLeft) * screen_pos.x * 0.5f;
    camera_pos->y = (mTop + mBottom) * 0.5f + (mTop - mBottom) * screen_pos.y * 0.5f;
}

// NON_MATCHING: equivalent projection coefficients, with different row-write scheduling.
void FrustumProjection::doUpdateMatrix(Matrix44f* mtx) const
{
    const f32 inverse_width = 1.0f / (mRight - mLeft);
    const f32 inverse_height = 1.0f / (mTop - mBottom);
    const f32 inverse_depth = 1.0f / (mFar - mNear);
    mtx->setRow(0, {2.0f * mNear * inverse_width, 0.0f,
                    inverse_width * (mLeft + mRight), 0.0f});
    mtx->setRow(1, {0.0f, inverse_height * (2.0f * mNear),
                    inverse_height * (mTop + mBottom), 0.0f});
    mtx->setRow(2, {0.0f, 0.0f, -inverse_depth * (mFar + mNear),
                    -inverse_depth * (2.0f * mFar * mNear)});
    mtx->setRow(3, {0.0f, 0.0f, -1.0f, 0.0f});
}

f32 DirectProjection::getNear() const { return mNear; }
f32 DirectProjection::getFar() const { return mFar; }
f32 DirectProjection::getFovy() const { return mFovy; }
f32 DirectProjection::getAspect() const { return mAspect; }

void DirectProjection::getOffset(Vector2f* offset) const
{
    offset->x = mOffset.x;
    offset->y = mOffset.y;
}

// NON_MATCHING: natural Matrix44 assignment copies components separately.
void DirectProjection::doUpdateMatrix(Matrix44f* mtx) const
{
    *mtx = mProjectionMatrix;
}

}  // namespace sead
