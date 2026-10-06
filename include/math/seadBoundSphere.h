#pragma once

#include <math/seadVector.h>

namespace sead
{
/// A sphere given by its center and radius. Layout and the default state (center at the origin, radius 0) follow
/// gsys::Model::getBounding (0x7100bf97cc) and its callers.
template <typename T>
struct BoundSphere3
{
    using Vector3 = sead::Vector3<T>;

    BoundSphere3() : mCenter(Vector3::zero), mRadius(0) {}
    BoundSphere3(const Vector3& center, T radius) : mCenter(center), mRadius(radius) {}

    const Vector3& getCenter() const { return mCenter; }
    T getRadius() const { return mRadius; }

    void setCenter(const Vector3& center) { mCenter = center; }
    void setRadius(T radius) { mRadius = radius; }

private:
    Vector3 mCenter;
    T mRadius;
};

using BoundSphere3f = BoundSphere3<f32>;

}  // namespace sead
