#include <gfx/seadViewport.h>
#include <gfx/seadDrawContext.h>
#include <gfx/seadFrameBuffer.h>
#include <gfx/seadProjection.h>
#include <nvn/nvn_FuncPtrInline.h>

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
// NON_MATCHING: natural vector copy and posture cases schedule bounds loads differently.
void Viewport::getOnFrameBufferPos(Vector2f* out, const LogicalFrameBuffer& buffer) const
{
    out->set(getMin());
    switch (mDevicePosture)
    {
    case Graphics::cDevicePosture_RotateRight:
        out->x = getMin().y;
        out->y = buffer.getVirtualSize().y - getSizeX() - getMin().x;
        break;
    case Graphics::cDevicePosture_RotateLeft:
        out->x = buffer.getVirtualSize().x - getSizeY() - getMin().y;
        out->y = getMin().x;
        break;
    case Graphics::cDevicePosture_RotateHalfAround:
        out->x = buffer.getVirtualSize().x - getSizeX() - getMin().x;
        out->y = buffer.getVirtualSize().y - getSizeY() - getMin().y;
        break;
    case Graphics::cDevicePosture_FlipX:
        out->x = buffer.getVirtualSize().x - getSizeX() - getMin().x;
        break;
    case Graphics::cDevicePosture_FlipY:
        out->y = buffer.getVirtualSize().y - getSizeY() - getMin().y;
        break;
    default:
        break;
    }
    out->x /= buffer.getVirtualSize().x;
    out->y /= buffer.getVirtualSize().y;
    out->x *= buffer.getPhysicalArea().getSizeX();
    out->y *= buffer.getPhysicalArea().getSizeY();
    out->x += buffer.getPhysicalArea().getMin().x;
    out->y += buffer.getPhysicalArea().getMin().y;
}

void Viewport::getOnFrameBufferSize(Vector2f* out, const LogicalFrameBuffer& buffer) const
{
    out->set(getSizeX(), getSizeY());
    if (mDevicePosture == Graphics::cDevicePosture_RotateRight ||
        mDevicePosture == Graphics::cDevicePosture_RotateLeft)
        out->set(out->y, out->x);
    out->x /= buffer.getVirtualSize().x;
    out->y /= buffer.getVirtualSize().y;
    out->x *= buffer.getPhysicalArea().getSizeX();
    out->y *= buffer.getPhysicalArea().getSizeY();
}
// NON_MATCHING: the original reloads the position from the stack for the second call (here the converted values are
// kept in registers).
// 0x7100b20164
void Viewport::apply(DrawContext* context, const LogicalFrameBuffer& buffer) const
{
    Vector2f pos;
    getOnFrameBufferPos(&pos, buffer);
    Vector2f size;
    getOnFrameBufferSize(&size, buffer);
    pos.y = buffer.getPhysicalArea().getSizeY() - size.y - pos.y;

    NVNcommandBuffer* command_buffer = context->getCommandBuffer()->ToData()->pNvnCommandBuffer;
    nvnCommandBufferSetScissor(command_buffer, s32(pos.x), s32(pos.y), u32(size.x), u32(size.y));
    nvnCommandBufferSetViewport(command_buffer, s32(pos.x), s32(pos.y), u32(size.x), u32(size.y));
    nvnCommandBufferSetDepthRange(command_buffer, mMinDepth, mMaxDepth);
}

// 0x7100b20440
void Viewport::applyViewport(DrawContext* context, const LogicalFrameBuffer& buffer) const
{
    Vector2f pos;
    getOnFrameBufferPos(&pos, buffer);
    Vector2f size;
    getOnFrameBufferSize(&size, buffer);
    pos.y = buffer.getPhysicalArea().getSizeY() - size.y - pos.y;

    NVNcommandBuffer* command_buffer = context->getCommandBuffer()->ToData()->pNvnCommandBuffer;
    nvnCommandBufferSetViewport(command_buffer, s32(pos.x), s32(pos.y), u32(size.x), u32(size.y));
    nvnCommandBufferSetDepthRange(command_buffer, mMinDepth, mMaxDepth);
}

// 0x7100b20510
void Viewport::applyScissor(DrawContext* context, const LogicalFrameBuffer& buffer) const
{
    Vector2f pos;
    getOnFrameBufferPos(&pos, buffer);
    Vector2f size;
    getOnFrameBufferSize(&size, buffer);
    pos.y = buffer.getPhysicalArea().getSizeY() - size.y - pos.y;

    NVNcommandBuffer* command_buffer = context->getCommandBuffer()->ToData()->pNvnCommandBuffer;
    nvnCommandBufferSetScissor(command_buffer, s32(pos.x), s32(pos.y), u32(size.x), u32(size.y));
}

// NON_MATCHING: natural vector construction combines bounds/input loads differently.
void Viewport::unproject(Vector3f* out, const Vector2f& point, const Projection& projection,
                         const Camera& camera) const
{
    const Vector3f screen_pos(point.x / getHalfSizeX(), point.y / getHalfSizeY(), 0.0f);
    projection.unproject(out, screen_pos, camera);
}

// NON_MATCHING: natural vector construction combines bounds/input loads differently.
void Viewport::unprojectRay(Ray<Vector3f>* out, const Vector2f& point,
                            const Projection& projection, const Camera& camera) const
{
    const Vector3f screen_pos(point.x / getHalfSizeX(), point.y / getHalfSizeY(), 0.0f);
    projection.unprojectRay(out, screen_pos, camera);
}
}  // namespace sead
