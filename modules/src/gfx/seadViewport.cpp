#include <gfx/seadViewport.h>

namespace sead
{
// NON_MATCHING: the genuine BoundBox2 default initializes bounds before derived fields.
Viewport::Viewport()
    : mDevicePosture(Graphics::sDefaultDevicePosture), mMinDepth(0.0f), mMaxDepth(1.0f)
{
}

Viewport::Viewport(float left, float top, float width, float height)
    : BoundBox2f(left, top, left + width, top + height),
      mDevicePosture(Graphics::sDefaultDevicePosture), mMinDepth(0.0f), mMaxDepth(1.0f)
{
}

Viewport::~Viewport() = default;

// NON_MATCHING: equivalent projection, with different input and bounds load scheduling.
void Viewport::project(Vector2f* out, const Vector3f& point) const
{
    out->x = point.x * getHalfSizeX();
    out->y = point.y * getHalfSizeY();
}

// NON_MATCHING: equivalent projection, with different input and bounds load scheduling.
void Viewport::project(Vector2f* out, const Vector2f& point) const
{
    out->x = point.x * getHalfSizeX();
    out->y = point.y * getHalfSizeY();
}
}  // namespace sead
