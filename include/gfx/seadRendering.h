#pragma once

namespace sead
{
class PrimitiveDrawer;

namespace Rendering
{
/// Draws a geometric shape (sead::Capsule, ...) with the primitive drawer.
template <typename Shape, typename Color>
void draw(PrimitiveDrawer* drawer, const Shape& shape, const Color& color);

}  // namespace Rendering
}  // namespace sead
