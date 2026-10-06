#pragma once

#include <basis/seadTypes.h>
#include <geom/seadSegment.h>
#include <math/seadVector.h>

namespace sead
{
class Geometry
{
public:
    /// The squared distance between the point and the closest point of the segment. `t` (optional) receives the
    /// position of that point on the segment (0: start, 1: end).
    static f32 calcSquaredDistancePointToSegment(const Vector2f& point, const Segment<Vector2f>& segment,
                                                 f32* t);
    static f32 calcSquaredDistancePointToSegment(const Vector3f& point, const Segment<Vector3f>& segment,
                                                 f32* t);
};

}  // namespace sead
