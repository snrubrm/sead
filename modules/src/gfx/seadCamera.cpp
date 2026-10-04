#include "gfx/seadCamera.h"
#include "basis/seadRawPrint.h"

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

LookAtCamera::~LookAtCamera() = default;

LookAtCamera::LookAtCamera(const Vector3f& pos, const Vector3f& at, const Vector3f& up)
    : mPos(pos), mAt(at), mUp(up)
{
    SEAD_ASSERT(mPos != mAt);
    mUp.normalize();
}

OrthoCamera::~OrthoCamera() = default;

DirectCamera::~DirectCamera() = default;

void DirectCamera::doUpdateMatrix(Matrix34f* dst) const
{
    *dst = mDirectMatrix;
}

}  // namespace sead
