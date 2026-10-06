#include "gfx/nin/seadDisplayBufferNvn.h"
#include <nvn/nvn_FuncPtrInline.h>
#include "gfx/nin/seadGraphicsNvn.h"

namespace sead
{
// NON_MATCHING: same stores, but the zero size is merged with the neighbouring zero member stores instead
// of the vtable pointer store.
DisplayBufferNvn::DisplayBufferNvn() = default;

void DisplayBufferNvn::presentTextureAndAcquireNext()
{
    nvnQueuePresentTexture(GraphicsNvn::instance()->getQueue(), mWindow, mTextureIndex);
    nvnWindowAcquireTexture(mWindow, mSync, &mTextureIndex);
}

void DisplayBufferNvn::waitAcquireDone()
{
    nvnSyncWait(mSync, u64(-1));
}

void DisplayBufferNvn::setPresentInterval(u8 interval)
{
    mPresentInterval = interval;
    if (mWindow)
        nvnWindowSetPresentInterval(mWindow, interval);
}

void DisplayBufferNvn::setTripleBuffer(bool triple_buffer)
{
    mBufferCount = triple_buffer ? 3 : 2;
}

void DisplayBufferNvn::setWindowCrop(s32 x, s32 y, s32 width, s32 height)
{
    mIsWindowCropChanged = true;
    mWindowCropX = x;
    mWindowCropY = y;
    mWindowCropWidth = width;
    mWindowCropHeight = height;
}

void DisplayBufferNvn::getWindowCrop(s32* x, s32* y, s32* width, s32* height) const
{
    *x = mWindowCropX;
    *y = mWindowCropY;
    *width = mWindowCropWidth;
    *height = mWindowCropHeight;
}

void DisplayBufferNvn::applyChangeWindowCrop()
{
    if (mIsWindowCropChanged)
    {
        nvnWindowSetCrop(mWindow, mWindowCropX, mWindowCropY, mWindowCropWidth, mWindowCropHeight);
        mIsWindowCropChanged = false;
    }
}
}  // namespace sead
