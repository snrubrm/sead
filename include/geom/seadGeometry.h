#pragma once

#include <basis/seadTypes.h>
#include <geom/seadLine.h>
#include <geom/seadPlane.h>
#include <geom/seadRay.h>
#include <geom/seadSegment.h>
#include <geom/seadSphere.h>
#include <math/seadBoundBox.h>
#include <math/seadVector.h>

namespace sead
{
class Geometry
{
public:
    enum IntersectionResult
    {
        cIntersectionResult_None = 0,
        cIntersectionResult_OnePoint = 1,
        cIntersectionResult_TwoPoints = 2,
        /// The segment lies in the plane.
        cIntersectionResult_Coincident = 3,
    };

    /// As calcSquaredDistancePointToSegment for an infinite line. `t` is the (signed) distance of the closest point
    /// from the line position along the direction.
    static f32 calcSquaredDistancePointToLine(const Vector2f& point, const Line<Vector2f>& line, f32* t);
    /// The squared distance between the point and the closest point of the segment. `t` (optional) receives the
    /// position of that point on the segment (0: start, 1: end).
    static f32 calcSquaredDistancePointToSegment(const Vector2f& point, const Segment<Vector2f>& segment,
                                                 f32* t);
    static f32 calcSquaredDistancePointToSegment(const Vector3f& point, const Segment<Vector3f>& segment,
                                                 f32* t);

    static s32 calcIntersectionSegmentToPlane(const Segment<Vector3f>& segment, const Plane3<f32>& plane,
                                              f32* t);
    /// Ray parameters of the (up to two) intersections, in increasing order.
    static s32 calcIntersectionRayToSphere(const Ray<Vector3f>& ray, const Sphere<Vector3f>& sphere,
                                           f32* t0, f32* t1);
    static bool calcIntersectionSphereToAABB(const Sphere<Vector2f>& sphere, const BoundBox2<f32>& box);
};

}  // namespace sead
