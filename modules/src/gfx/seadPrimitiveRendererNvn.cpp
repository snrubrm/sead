#include <gfx/seadPrimitiveRenderer.h>

namespace sead
{
PrimitiveRenderer::~PrimitiveRenderer() {}

void PrimitiveRenderer::prepare(Heap* heap, const SafeString& path)
{
    PrimitiveDrawer::prepareMgr(heap, path);
}
}  // namespace sead
