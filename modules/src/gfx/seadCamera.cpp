#include "gfx/seadCamera.h"
#include "basis/seadRawPrint.h"
#include "gfx/seadProjection.h"

namespace sead
{
Camera::~Camera() = default;

void Camera::getWorldPosByMatrix(Vector3f* dst) const
{
    dst->set(-mMatrix.m[0][0] * mMatrix.m[0][3] - mMatrix.m[1][0] * mMatrix.m[1][3] -
                 mMatrix.m[2][0] * mMatrix.m[2][3],
             -mMatrix.m[0][1] * mMatrix.m[0][3] - mMatrix.m[1][1] * mMatrix.m[1][3] -
                 mMatrix.m[2][1] * mMatrix.m[2][3],
             -mMatrix.m[0][2] * mMatrix.m[0][3] - mMatrix.m[1][2] * mMatrix.m[1][3] -
                 mMatrix.m[2][2] * mMatrix.m[2][3]);
}

void Camera::getLookVectorByMatrix(Vector3f* dst) const
{
    dst->set(mMatrix.m[2][0], mMatrix.m[2][1], mMatrix.m[2][2]);
}

void Camera::getRightVectorByMatrix(Vector3f* dst) const
{
    dst->set(mMatrix.m[0][0], mMatrix.m[0][1], mMatrix.m[0][2]);
}

void Camera::getUpVectorByMatrix(Vector3f* dst) const
{
    dst->set(mMatrix.m[1][0], mMatrix.m[1][1], mMatrix.m[1][2]);
}

void Camera::worldPosToCameraPosByMatrix(Vector3f* dst, const Vector3f& world_pos) const
{
    dst->setMul(mMatrix, world_pos);
}

// NON_MATCHING: equivalent inverse transform, with different arithmetic scheduling.
void Camera::cameraPosToWorldPosByMatrix(Vector3f* dst, const Vector3f& camera_pos) const
{
    Vector3f world_pos;
    getWorldPosByMatrix(&world_pos);
    const Vector3f pos = camera_pos;
    dst->set(world_pos.x + mMatrix.m[1][0] * pos.y + mMatrix.m[2][0] * pos.z +
                 mMatrix.m[0][0] * pos.x,
             world_pos.y + mMatrix.m[1][1] * pos.y + mMatrix.m[2][1] * pos.z +
                 mMatrix.m[0][1] * pos.x,
             world_pos.z + mMatrix.m[1][2] * pos.y + mMatrix.m[2][2] * pos.z +
                 mMatrix.m[0][2] * pos.x);
}

void Camera::projectByMatrix(Vector2f* dst, const Vector3f& world_pos,
                             const Projection& projection, const Viewport& viewport) const
{
    Vector3f camera_pos;
    worldPosToCameraPosByMatrix(&camera_pos, world_pos);
    projection.project(dst, camera_pos, viewport);
}

LookAtCamera::~LookAtCamera() = default;

LookAtCamera::LookAtCamera(const Vector3f& pos, const Vector3f& at, const Vector3f& up)
    : mPos(pos), mAt(at), mUp(up)
{
    SEAD_ASSERT(mPos != mAt);
    mUp.normalize();
}

// NON_MATCHING: equivalent look-at matrix, with different row-write scheduling.
void LookAtCamera::doUpdateMatrix(Matrix34f* dst) const
{
    if (mPos == mAt)
        return;

    Vector3f look = mPos - mAt;
    look.normalize();
    Vector3f right = mUp.cross(look);
    right.normalize();
    const Vector3f up = look.cross(right);
    dst->setRow(0, {right.x, right.y, right.z, -right.dot(mPos)});
    dst->setRow(1, {up.x, up.y, up.z, -up.dot(mPos)});
    dst->setRow(2, {look.x, look.y, look.z, -look.dot(mPos)});
}

OrthoCamera::~OrthoCamera() = default;

DirectCamera::~DirectCamera() = default;

void DirectCamera::doUpdateMatrix(Matrix34f* dst) const
{
    *dst = mDirectMatrix;
}

}  // namespace sead
