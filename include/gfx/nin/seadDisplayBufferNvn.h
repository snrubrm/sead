#pragma once

#include <container/seadSafeArray.h>
#include <nvn/nvn.h>
#include "gfx/seadDisplayBuffer.h"

namespace sead
{
class DisplayBufferNvn : public DisplayBuffer
{
    SEAD_RTTI_OVERRIDE(DisplayBufferNvn, DisplayBuffer)
public:
    DisplayBufferNvn();

    void initializeImpl_(Heap* heap) override;

    void presentTextureAndAcquireNext();
    void waitAcquireDone();
    void setPresentInterval(u8 interval);
    void setTripleBuffer(bool triple_buffer);
    void setWindowCrop(s32 x, s32 y, s32 width, s32 height);
    void getWindowCrop(s32* x, s32* y, s32* width, s32* height) const;
    void applyChangeWindowCrop();

    const Vector2f& getSize() const { return mSize; }
    /// The texture that is currently acquired from the window.
    NVNtexture* getAcquiredTexture() const { return mTextures[mTextureIndex]; }

private:
    NVNwindow* mWindow = nullptr;
    NVNsync* mSync = nullptr;
    s32 mTextureIndex = 0;
    SafeArray<NVNtexture*, 3> mTextures = {};
    void* mNativeWindow = nullptr;
    u8 mPresentInterval = 1;
    u8 mBufferCount = 2;
    bool mIsWindowCropChanged = false;
    s32 mWindowCropX = 0;
    s32 mWindowCropY = 0;
    s32 mWindowCropWidth = 0;
    s32 mWindowCropHeight = 0;
};
static_assert(sizeof(DisplayBufferNvn) == 0x60);

}  // namespace sead
