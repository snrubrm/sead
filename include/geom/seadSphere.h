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
    // Inline-only names follow the existing getters. ModelRenderUnit C800A0 sets both fields.
    void setCenter(const Vec& center) { mCenter = center; }
    void setRadius(f32 radius) { mRadius = radius; }

private:
    Vec mCenter;
    f32 mRadius;
};

using Sphere2f = Sphere<Vector2f>;
using Sphere3f = Sphere<Vector3f>;

}  // namespace sead
