#include <gfx/seadRendering.h>
#include <geom/seadCapsule.h>
#include <gfx/seadColor.h>

namespace sead::Rendering {

template <typename Shape, typename Color>
void draw(PrimitiveDrawer* drawer, const Shape& shape, const Color& color) {
    draw<Shape, Color, Color>(drawer, shape, color, color);
}

template void draw<Capsule3f, Color4f>(PrimitiveDrawer* drawer, const Capsule3f& shape,
                                      const Color4f& color);

}  // namespace sead::Rendering
