#pragma once

#include <basis/seadTypes.h>
#include <geom/seadSegment.h>

namespace sead
{
/// All the points within `radius` of a segment.
template <typename Vec>
class Capsule
{
public:
    Capsule() = default;
    Capsule(const Segment<Vec>& segment, f32 radius) : mSegment(segment), mRadius(radius) {}

    const Segment<Vec>& getSegment() const { return mSegment; }
    f32 getRadius() const { return mRadius; }

private:
    Segment<Vec> mSegment;
    f32 mRadius = 0.0f;
};

using Capsule3f = Capsule<Vector3f>;

}  // namespace sead
