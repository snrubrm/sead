#pragma once

#include <math/seadVector.h>

namespace sead
{
/// A half line given by its origin and a (unit length) direction.
template <typename Vec>
class Ray
{
public:
    Ray() = default;
    Ray(const Vec& pos, const Vec& dir) : mPos(pos), mDir(dir) {}

    const Vec& getPos() const { return mPos; }
    const Vec& getDir() const { return mDir; }
    void setPos(const Vec& pos) { mPos = pos; }
    void setDir(const Vec& dir) { mDir = dir; }

private:
    Vec mPos;
    Vec mDir;
};

using Ray2f = Ray<Vector2f>;
using Ray3f = Ray<Vector3f>;

}  // namespace sead
