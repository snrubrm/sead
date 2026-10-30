#include <gfx/seadPrimitiveRenderer.h>

namespace sead
{
SEAD_SINGLETON_DISPOSER_IMPL(PrimitiveRenderer)

PrimitiveRenderer::PrimitiveRenderer()
    : mDrawer(nullptr), mCameraMatrix(Matrix34f::ident), mProjection(Matrix44f::ident),
      mModelMatrix(Matrix34f::ident)
{
}

PrimitiveRenderer::~PrimitiveRenderer() {}

void PrimitiveRenderer::prepare(Heap* heap, const SafeString& path)
{
    PrimitiveDrawer::prepareMgr(heap, path);
}
}  // namespace sead
