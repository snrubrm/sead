#pragma once

#include <math/seadVector.h>

namespace sead
{
/// An infinite line given by a point and a (unit length) direction.
template <typename Vec>
class Line
{
public:
    Line() = default;
    Line(const Vec& pos, const Vec& dir) : mPos(pos), mDir(dir) {}

    const Vec& getPos() const { return mPos; }
    const Vec& getDir() const { return mDir; }

private:
    Vec mPos;
    Vec mDir;
};

using Line2f = Line<Vector2f>;
using Line3f = Line<Vector3f>;

}  // namespace sead
