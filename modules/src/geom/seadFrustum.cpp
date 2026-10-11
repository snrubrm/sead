#include <cmath>
#include "geom/seadFrustum.h"

namespace sead
{
Frustum::Frustum()
    : mRight(Vector3f::ex), mUp(Vector3f::ey), mForward(Vector3f::ez), mOrigin(Vector3f::zero)
{
}

// NON_MATCHING: matrix row copies use different load/store grouping and arithmetic scheduling.
void Frustum::set(const Matrix34f& view, f32 fovy, f32 aspect, f32 near, f32 far)
{
    const Vector4f right = view.getRow(0);
    const Vector4f up = view.getRow(1);
    const Vector4f back = view.getRow(2);
    mRight.set(right.x, right.y, right.z);
    mUp.set(up.x, up.y, up.z);
    mForward.set(-back.x, -back.y, -back.z);
    mOrigin.set(-right.x * right.w - up.x * up.w - back.x * back.w,
                -right.y * right.w - up.y * up.w - back.y * back.w,
                -right.z * right.w - up.z * up.w - back.z * back.w);
    mVerticalTangent = std::tan(fovy * 0.5f);
    mVerticalNormalLength = std::sqrt(mVerticalTangent * mVerticalTangent + 1.0f);
    mHorizontalTangent = mVerticalTangent * aspect;
    mHorizontalNormalLength = std::sqrt(mHorizontalTangent * mHorizontalTangent + 1.0f);
    mNear = near;
    mFar = far;
}

bool Frustum::isInside(const Vector3f& position, f32 radius, f32 near, f32 far) const
{
    const Vector3f relative = position - mOrigin;
    const f32 depth = relative.dot(mForward);
    if (depth < near - radius)
        return false;
    if (far > 0.0f && far + radius < depth)
        return false;

    f32 horizontal = relative.dot(mRight);
    horizontal = horizontal > 0.0f ? horizontal : -horizontal;
    if (depth * mHorizontalTangent + mHorizontalNormalLength * radius < horizontal)
        return false;

    f32 vertical = relative.dot(mUp);
    vertical = vertical > 0.0f ? vertical : -vertical;
    return !(depth * mVerticalTangent + mVerticalNormalLength * radius < vertical);
}
}  // namespace sead
