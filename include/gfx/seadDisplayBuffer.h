#pragma once

#include "heap/seadHeap.h"
#include "math/seadVector.h"
#include "prim/seadRuntimeTypeInfo.h"

namespace sead
{
class DisplayBuffer
{
    SEAD_RTTI_BASE(DisplayBuffer)
public:
    DisplayBuffer() = default;

    virtual void initializeImpl_(Heap* heap) = 0;

protected:
    Vector2f mSize = {0.0f, 0.0f};
};
static_assert(sizeof(DisplayBuffer) == 0x10);

}  // namespace sead
