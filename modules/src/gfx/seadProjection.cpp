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

f32 OrthoProjection::getNear() const { return mNear; }
f32 OrthoProjection::getFar() const { return mFar; }
f32 OrthoProjection::getFovy() const { return 0.0f; }
f32 OrthoProjection::getAspect() const { return (mRight - mLeft) / (mTop - mBottom); }

void OrthoProjection::getOffset(Vector2f* offset) const
{
    offset->x = (mLeft + mRight) * 0.5f / (mRight - mLeft);
    offset->y = (mTop + mBottom) * 0.5f / (mTop - mBottom);
}

f32 FrustumProjection::getNear() const { return mNear; }
f32 FrustumProjection::getFar() const { return mFar; }
f32 FrustumProjection::getAspect() const { return (mRight - mLeft) / (mTop - mBottom); }

// NON_MATCHING: equivalent offset arithmetic, with different scheduling.
void FrustumProjection::getOffset(Vector2f* offset) const
{
    offset->x = (mRight + mLeft) * 0.5f / (mRight - mLeft);
    offset->y = (mTop + mBottom) * 0.5f / (mTop - mBottom);
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
