#include <geom/seadGeometry.h>

namespace sead
{
namespace
{
template <typename Vec>
f32 calcSquaredDistancePointToSegmentImpl(const Vec& point, const Segment<Vec>& segment, f32* t)
{
    const Vec& pos0 = segment.getPos0();
    const Vec& pos1 = segment.getPos1();
    const Vec direction = pos1 - pos0;
    const Vec to_point = point - pos0;

    Vec closest = pos0;
    f32 ratio;
    const f32 dot = direction.dot(to_point);
    if (dot <= 0.0f)
    {
        ratio = 0.0f;
    }
    else
    {
        const f32 squared_length = direction.dot(direction);
        if (dot >= squared_length)
        {
            ratio = 1.0f;
            closest = pos1;
        }
        else
        {
            ratio = dot / squared_length;
            closest = pos0 + direction * ratio;
        }
    }

    if (t)
        *t = ratio;
    return (closest - point).squaredLength();
}

}  // namespace

// 0x7100b20970
f32 Geometry::calcSquaredDistancePointToLine(const Vector2f& point, const Line<Vector2f>& line, f32* t)
{
    const Vector2f to_point = point - line.getPos();
    const f32 ratio = line.getDir().dot(to_point);
    const Vector2f closest = line.getPos() + line.getDir() * ratio;
    const f32 squared_distance = (point - closest).squaredLength();
    if (t)
        *t = ratio;
    return squared_distance;
}

// 0x7100b20f18
s32 Geometry::calcIntersectionSegmentToPlane(const Segment<Vector3f>& segment,
                                             const Plane3<f32>& plane, f32* t)
{
    const Vector3f direction = segment.getPos1() - segment.getPos0();
    const f32 distance = plane.getNormal().dot(segment.getPos0()) - plane.getD();
    const f32 denominator = plane.getNormal().dot(direction);

    if (denominator == 0.0f)
    {
        if (distance == -0.0f)
        {
            if (t)
                *t = 0.0f;
            return cIntersectionResult_Coincident;
        }
        return cIntersectionResult_None;
    }

    const f32 ratio = -distance / denominator;
    if (!(ratio >= 0.0f && ratio <= 1.0f))
        return cIntersectionResult_None;
    if (t)
        *t = ratio;
    return cIntersectionResult_OnePoint;
}

// NON_MATCHING: the two epsilon comparisons of the tangent case are emitted in the opposite order
// 0x7100b20fcc
s32 Geometry::calcIntersectionRayToSphere(const Ray<Vector3f>& ray, const Sphere<Vector3f>& sphere,
                                          f32* t0, f32* t1)
{
    const Vector3f to_center = ray.getPos() - sphere.getCenter();
    const f32 b = 2.0f * to_center.dot(ray.getDir());
    const f32 c = to_center.squaredLength() - sphere.getRadius() * sphere.getRadius();
    const f32 discriminant = b * b - 4.0f * c;

    if (discriminant > 0.0f)
    {
        if (b > 0.0f && b * b > discriminant)
            return cIntersectionResult_None;

        if (b > 0.0f || b * b < discriminant)
        {
            // The origin is inside of the sphere
            if (t0)
                *t0 = (Mathf::sqrt(discriminant) - b) * 0.5f;
            return cIntersectionResult_OnePoint;
        }

        if (t0 || t1)
        {
            const f32 root = Mathf::sqrt(discriminant);
            if (t0)
                *t0 = (-b - root) * 0.5f;
            if (t1)
                *t1 = (root - b) * 0.5f;
        }
        return cIntersectionResult_TwoPoints;
    }

    if (b > 0.0f || !Mathf::equalsEpsilon(discriminant, 0.0f))
        return cIntersectionResult_None;

    if (t0)
        *t0 = b * -0.5f;
    return cIntersectionResult_OnePoint;
}

// NON_MATCHING: same comparisons; the x term is if-converted (fcsel) and the box bounds are loaded in a different order
// 0x7100b21138
bool Geometry::calcIntersectionSphereToAABB(const Sphere<Vector2f>& sphere, const BoundBox2<f32>& box)
{
    f32 squared_distance = 0.0f;

    const f32 x = sphere.getCenter().x;
    if (x < box.getMin().x)
    {
        const f32 d = box.getMin().x - x;
        squared_distance += d * d;
    }
    else if (x > box.getMax().x)
    {
        const f32 d = box.getMax().x - x;
        squared_distance += d * d;
    }

    const f32 y = sphere.getCenter().y;
    if (y < box.getMin().y)
    {
        const f32 d = box.getMin().y - y;
        squared_distance += d * d;
    }
    else if (y > box.getMax().y)
    {
        const f32 d = box.getMax().y - y;
        squared_distance += d * d;
    }

    return squared_distance <= sphere.getRadius() * sphere.getRadius();
}

// NON_MATCHING: same arithmetic and branches; the registers of the segment end points are allocated differently
// 0x7100b209c0
f32 Geometry::calcSquaredDistancePointToSegment(const Vector2f& point,
                                                const Segment<Vector2f>& segment, f32* t)
{
    return calcSquaredDistancePointToSegmentImpl(point, segment, t);
}

// NON_MATCHING: as the Vector2 version
// 0x7100b20a5c
f32 Geometry::calcSquaredDistancePointToSegment(const Vector3f& point,
                                                const Segment<Vector3f>& segment, f32* t)
{
    return calcSquaredDistancePointToSegmentImpl(point, segment, t);
}

}  // namespace sead
