#pragma once

#include <math/seadVector.h>

namespace sead
{
/// A circle (Vector2) or sphere (Vector3): center and radius.
template <typename Vec>
class Sphere
{
public:
    Sphere() = default;
    Sphere(const Vec& center, f32 radius) : mCenter(center), mRadius(radius) {}

    const Vec& getCenter() const { return mCenter; }
    f32 getRadius() const { return mRadius; }

private:
    Vec mCenter;
    f32 mRadius;
};

using Sphere2f = Sphere<Vector2f>;
using Sphere3f = Sphere<Vector3f>;

}  // namespace sead
