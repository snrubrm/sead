#pragma once

#include <math/seadVector.h>

namespace sead
{
/// A line segment between two points.
template <typename Vec>
class Segment
{
public:
    Segment() = default;
    Segment(const Vec& pos0, const Vec& pos1) : mPos0(pos0), mPos1(pos1) {}

    const Vec& getPos0() const { return mPos0; }
    const Vec& getPos1() const { return mPos1; }
    void setPos0(const Vec& pos) { mPos0 = pos; }
    void setPos1(const Vec& pos) { mPos1 = pos; }

private:
    Vec mPos0;
    Vec mPos1;
};

using Segment2f = Segment<Vector2f>;
using Segment3f = Segment<Vector3f>;

}  // namespace sead
