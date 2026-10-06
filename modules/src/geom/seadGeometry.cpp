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
