#pragma once

namespace sead
{
class DrawContext;

// Only the draw-context application interface is recovered. The object layout
// and construction are not modeled here.
class GraphicsContext
{
public:
    void apply(DrawContext* context) const;
};
}  // namespace sead
