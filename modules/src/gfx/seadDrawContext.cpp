#include "gfx/seadDrawContext.h"
#include <nn/gfx/gfx_Device.h>
#include "gfx/nin/seadGraphicsNvn.h"

namespace sead
{
DrawContext::DrawContext()
{
    auto data = mCommandBuffer.ToData();
    data->pNnDevice = GraphicsNvn::instance()->getNnDevice();
    data->pNvnCommandBuffer = nullptr;
    data->state = nn::gfx::CommandBufferImplData<nn::gfx::ApiVariationNvn8>::State_Begun;
}

DrawContext::~DrawContext()
{
    mCommandBuffer.ToData()->state =
        nn::gfx::CommandBufferImplData<nn::gfx::ApiVariationNvn8>::State_NotInitialized;
}
}  // namespace sead
