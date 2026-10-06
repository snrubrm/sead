#include "gfx/seadGraphics.h"

namespace sead
{
Graphics::Graphics() : _20(nullptr), mDrawLockContext(nullptr) {}

Graphics::~Graphics() = default;

void Graphics::initialize(Heap* heap)
{
    mDrawLockContext = new (heap, 8) DrawLockContext;
    initializeDrawLockContext(heap);
    mDrawLockContext->lock();
    initializeImpl(heap);
    mDrawLockContext->unlock();
}

void Graphics::lockDrawContext()
{
    if (_20)
        _20(1);
    else
        mDrawLockContext->lock();
}

void Graphics::unlockDrawContext()
{
    if (_20)
        _20(0);
    else
        mDrawLockContext->unlock();
}

void Graphics::initializeDrawLockContext(Heap* heap)
{
    mDrawLockContext->initialize(heap);
}
}  // namespace sead
