#pragma once

namespace sead
{
class PrimitiveDrawer;

namespace Rendering
{
/// Draws a geometric shape (sead::Capsule, ...) with the primitive drawer.
template <typename Shape, typename Color>
void draw(PrimitiveDrawer* drawer, const Shape& shape, const Color& color);

// Native 136DD5C accepts separate endpoint colors; complete ShapeCapsule
// consumer B9B0E0 forwards one color through the single-color overload.
template <typename Shape, typename Color0, typename Color1>
void draw(PrimitiveDrawer* drawer, const Shape& shape, const Color0& color0, const Color1& color1);

}  // namespace Rendering
}  // namespace sead
