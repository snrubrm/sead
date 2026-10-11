#pragma once

#include <math/seadMatrix.h>
#include <math/seadVector.h>

namespace sead
{
// Descriptive name: native 136e120/136e18c/136e2c0 construct and query this
// perspective frustum in each of the five CameraLookAt visibility consumers.
class Frustum
{
public:
    Frustum();
    void set(const Matrix34f& view, f32 fovy, f32 aspect, f32 near, f32 far);
    bool isInside(const Vector3f& position, f32 radius, f32 near, f32 far) const;

private:
    Vector3f mRight;
    Vector3f mUp;
    Vector3f mForward;
    Vector3f mOrigin;
    f32 mHorizontalTangent = 0.0f;
    f32 mHorizontalNormalLength = 0.0f;
    f32 mVerticalTangent = 0.0f;
    f32 mVerticalNormalLength = 0.0f;
    f32 mNear = 0.0f;
    f32 mFar = 0.0f;
};
static_assert(sizeof(Frustum) == 0x48, "Frustum size");

}  // namespace sead
