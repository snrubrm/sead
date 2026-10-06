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

Projection::~Projection() = default;

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

// NON_MATCHING: natural matrix assignment and posture switch use different loads and scheduling.
void Projection::doUpdateDeviceMatrix(Matrix44f* mtx, const Matrix44f& projection,
                                       Graphics::DevicePosture posture) const
{
    *mtx = projection;
    Vector4f row0 = projection.getRow(0);
    Vector4f row1 = projection.getRow(1);
    switch (posture)
    {
    case Graphics::cDevicePosture_RotateRight:
        row0.negate();
        mtx->setRow(0, row1);
        mtx->setRow(1, row0);
        break;
    case Graphics::cDevicePosture_RotateLeft:
        row1.negate();
        mtx->setRow(0, row1);
        mtx->setRow(1, row0);
        break;
    case Graphics::cDevicePosture_RotateHalfAround:
        row0.negate();
        row1.negate();
        mtx->setRow(0, row0);
        mtx->setRow(1, row1);
        break;
    case Graphics::cDevicePosture_FlipX:
        row0.negate();
        mtx->setRow(0, row0);
        break;
    case Graphics::cDevicePosture_FlipY:
        row1.negate();
        mtx->setRow(1, row1);
        break;
    default:
        break;
    }
    mtx->m[2][0] *= mDeviceZScale;
    mtx->m[2][1] *= mDeviceZScale;
    mtx->m[2][2] = (mtx->m[2][2] + mtx->m[3][2] * mDeviceZOffset) * mDeviceZScale;
    mtx->m[2][3] = mtx->m[2][3] * mDeviceZScale + mtx->m[3][3] * mDeviceZOffset;
}

PerspectiveProjection::PerspectiveProjection()
    : mNear(1.0f), mFar(10000.0f), mAspect(4.0f / 3.0f), mOffset(Vector2f::zero)
{
    setFovy_(Mathf::pi() / 4.0f);
}

PerspectiveProjection::PerspectiveProjection(f32 near, f32 far, f32 fovy_rad, f32 aspect)
    : mNear(near), mFar(far), mAspect(aspect), mOffset(Vector2f::zero)
{
    setFovy_(fovy_rad);
}

f32 PerspectiveProjection::getNear() const { return mNear; }
f32 PerspectiveProjection::getFar() const { return mFar; }
f32 PerspectiveProjection::getFovy() const { return mFovyRad; }
f32 PerspectiveProjection::getAspect() const { return mAspect; }
u32 PerspectiveProjection::getProjectionType() const { return 0; }

void PerspectiveProjection::setFovy_(f32 fovy_rad)
{
    mFovyRad = fovy_rad;
    mFovySin = Mathf::sin(fovy_rad * 0.5f);
    mFovyCos = Mathf::cos(fovy_rad * 0.5f);
    mFovyTan = Mathf::tan(fovy_rad * 0.5f);
    setDirty();
}

void PerspectiveProjection::set(f32 near, f32 far, f32 fovy_rad, f32 aspect)
{
    setNear(near);
    setFar(far);
    setFovy_(fovy_rad);
    setAspect(aspect);
}

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

OrthoProjection::OrthoProjection() : mNear(0.0f), mFar(1.0f)
{
    setTBLR(0.5f, -0.5f, -0.5f, 0.5f);
}

OrthoProjection::OrthoProjection(f32 near, f32 far, f32 top, f32 bottom, f32 left, f32 right)
    : mNear(near), mFar(far)
{
    setTBLR(top, bottom, left, right);
}

// NON_MATCHING: viewport bounds are computed together before the member stores.
OrthoProjection::OrthoProjection(f32 near, f32 far, const Viewport& viewport)
    : mNear(near), mFar(far)
{
    setByViewport(viewport);
    setDevicePosture(viewport.getDevicePosture());
}

// NON_MATCHING: the bound values use different floating-point registers.
void OrthoProjection::setByViewport(const Viewport& viewport)
{
    setTBLR(viewport.getSizeY() * 0.5f, viewport.getSizeY() * -0.5f,
            viewport.getSizeX() * -0.5f, viewport.getSizeX() * 0.5f);
}

f32 OrthoProjection::getNear() const { return mNear; }
f32 OrthoProjection::getFar() const { return mFar; }
f32 OrthoProjection::getFovy() const { return 0.0f; }
f32 OrthoProjection::getAspect() const { return (mRight - mLeft) / (mTop - mBottom); }
u32 OrthoProjection::getProjectionType() const { return 1; }

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

FrustumProjection::FrustumProjection(f32 near, f32 far, f32 top, f32 bottom, f32 left,
                                     f32 right)
    : mNear(near), mFar(far), mTop(top), mBottom(bottom), mLeft(left), mRight(right)
{
    setDirty();
}

f32 FrustumProjection::getNear() const { return mNear; }
f32 FrustumProjection::getFar() const { return mFar; }
f32 FrustumProjection::getAspect() const { return (mRight - mLeft) / (mTop - mBottom); }
u32 FrustumProjection::getProjectionType() const { return 0; }

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

DirectProjection::DirectProjection()
    : mProjectionMatrix(Matrix44f::ident), mNear(0.0f), mFar(0.0f), mFovy(0.0f),
      mAspect(0.0f), mOffset(0.0f, 0.0f), _f0(true)
{
    setDirty();
}

// NON_MATCHING: the matrix setter remains an out-of-line call instead of being inlined.
DirectProjection::DirectProjection(const Matrix44f& mtx, Graphics::DevicePosture posture)
    : mNear(0.0f), mFar(0.0f), mFovy(0.0f), mAspect(0.0f), mOffset(0.0f, 0.0f), _f0(true)
{
    setProjectionMatrix(mtx, posture);
}

// NON_MATCHING: natural matrix assignment and row operations use separate component loads.
void DirectProjection::setProjectionMatrix(const Matrix44f& mtx,
                                           Graphics::DevicePosture posture)
{
    mProjectionMatrix = mtx;
    Vector4f row0 = mtx.getRow(0);
    Vector4f row1 = mtx.getRow(1);
    switch (posture)
    {
    case Graphics::cDevicePosture_RotateRight:
        row1.negate();
        mProjectionMatrix.setRow(0, row1);
        mProjectionMatrix.setRow(1, row0);
        break;
    case Graphics::cDevicePosture_RotateLeft:
        row0.negate();
        mProjectionMatrix.setRow(0, row1);
        mProjectionMatrix.setRow(1, row0);
        break;
    case Graphics::cDevicePosture_RotateHalfAround:
        row0.negate();
        row1.negate();
        mProjectionMatrix.setRow(0, row0);
        mProjectionMatrix.setRow(1, row1);
        break;
    case Graphics::cDevicePosture_FlipX:
        row0.negate();
        mProjectionMatrix.setRow(0, row0);
        break;
    case Graphics::cDevicePosture_FlipY:
        row1.negate();
        mProjectionMatrix.setRow(1, row1);
        break;
    default:
        break;
    }
    setDirty();
    _f0 = true;
}

f32 DirectProjection::getNear() const { return mNear; }
f32 DirectProjection::getFar() const { return mFar; }
f32 DirectProjection::getFovy() const { return mFovy; }
f32 DirectProjection::getAspect() const { return mAspect; }
u32 DirectProjection::getProjectionType() const { return 2; }

void DirectProjection::getOffset(Vector2f* offset) const
{
    offset->x = mOffset.x;
    offset->y = mOffset.y;
}

// NON_MATCHING: complete attribute recovery, with different matrix-load scheduling.
void DirectProjection::updateAttributesForDirectProjection()
{
    if (!_f0)
        return;

    Matrix44f inverse;
    inverse.setInverse(mProjectionMatrix);
    const Vector4f clip[8] = {
        {-1.0f, -1.0f, -1.0f, 1.0f}, {-1.0f, 1.0f, -1.0f, 1.0f},
        {1.0f, 1.0f, -1.0f, 1.0f}, {1.0f, -1.0f, -1.0f, 1.0f},
        {-1.0f, -1.0f, 1.0f, 1.0f}, {-1.0f, 1.0f, 1.0f, 1.0f},
        {1.0f, 1.0f, 1.0f, 1.0f}, {1.0f, -1.0f, 1.0f, 1.0f},
    };
    Vector3f camera[8];
    for (s32 i = 0; i < 8; ++i)
    {
        const Vector4f& point = clip[i];
        const f32 inverse_w = 1.0f / (inverse.m[3][0] * point.x +
            inverse.m[3][1] * point.y + inverse.m[3][2] * point.z +
            inverse.m[3][3] * point.w);
        camera[i].x = (inverse.m[0][0] * point.x + inverse.m[0][1] * point.y +
            inverse.m[0][2] * point.z + inverse.m[0][3] * point.w) * inverse_w;
        camera[i].y = (inverse.m[1][0] * point.x + inverse.m[1][1] * point.y +
            inverse.m[1][2] * point.z + inverse.m[1][3] * point.w) * inverse_w;
        camera[i].z = (inverse.m[2][0] * point.x + inverse.m[2][1] * point.y +
            inverse.m[2][2] * point.z + inverse.m[2][3] * point.w) * inverse_w;
    }

    mNear = -camera[0].z;
    mFar = -camera[4].z;
    const f32 height = camera[1].y - camera[0].y;
    const f32 width = camera[2].x - camera[0].x;
    mAspect = width / height;
    mOffset.x = (camera[0].x + camera[2].x) * 0.5f / width;
    mOffset.y = (camera[1].y + camera[0].y) * 0.5f / height;
    if (Mathf::abs(camera[0].x - camera[4].x) > 0.0001f ||
        Mathf::abs(camera[1].x - camera[5].x) > 0.0001f ||
        Mathf::abs(camera[2].x - camera[6].x) > 0.0001f ||
        Mathf::abs(camera[3].x - camera[7].x) > 0.0001f)
        mFovy = 2.0f * Mathf::atan2(height * 0.5f, mNear);
    else
        mFovy = 0.0f;
    _f0 = false;
}

// NON_MATCHING: natural Matrix44 assignment copies components separately.
void DirectProjection::doUpdateMatrix(Matrix44f* mtx) const
{
    *mtx = mProjectionMatrix;
}

void DirectProjection::doScreenPosToCameraPosTo(Vector3f* camera_pos,
                                                const Vector3f& screen_pos) const
{
    Matrix44f inverse;
    inverse.setInverse(mProjectionMatrix);
    camera_pos->setMul(inverse, screen_pos);
}

}  // namespace sead
