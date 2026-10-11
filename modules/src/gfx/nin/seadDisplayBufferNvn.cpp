#include "gfx/nin/seadDisplayBufferNvn.h"
#include <nvn/nvn_FuncPtrInline.h>
#include <math/seadMathCalcCommon.h>
#include "gfx/nin/seadGraphicsNvn.h"

namespace sead
{
// NON_MATCHING: same stores, but the zero size is merged with the neighbouring zero member stores instead
// of the vtable pointer store.
DisplayBufferNvn::DisplayBufferNvn() = default;

// NON_MATCHING: builder stack lifetimes, storage alignment width and loop branches differ.
void DisplayBufferNvn::initializeImpl_(Heap* heap)
{
    const u32 width = mSize.x;
    const u32 height = mSize.y;
    auto* device = GraphicsNvn::instance()->getNvnDevice();
    auto* pool = new (heap, 8) NVNmemoryPool;
    NVNtextureBuilder texture_builders[3];
    size_t pool_size = 0;
    const s32 texture_count = Mathi::min(mBufferCount, 3);
    for (s32 i = 0; i < texture_count; ++i)
    {
        nvnTextureBuilderSetDevice(&texture_builders[i], device);
        nvnTextureBuilderSetDefaults(&texture_builders[i]);
        nvnTextureBuilderSetSize2D(&texture_builders[i], width, height);
        nvnTextureBuilderSetTarget(&texture_builders[i], NVN_TEXTURE_TARGET_2D);
        nvnTextureBuilderSetFormat(&texture_builders[i], NVN_FORMAT_RGBA8);
        nvnTextureBuilderSetFlags(&texture_builders[i], NVN_TEXTURE_FLAGS_DISPLAY);
        const size_t size = nvnTextureBuilderGetStorageSize(&texture_builders[i]);
        const s32 alignment = nvnTextureBuilderGetStorageAlignment(&texture_builders[i]);
        pool_size += MathCalcCommon<size_t>::roundUp(size, alignment);
    }
    pool_size = MathCalcCommon<size_t>::roundUp(pool_size, 0x1000);

    NVNmemoryPoolBuilder pool_builder;
    nvnMemoryPoolBuilderSetDefaults(&pool_builder);
    nvnMemoryPoolBuilderSetDevice(&pool_builder, GraphicsNvn::instance()->getNvnDevice());
    nvnMemoryPoolBuilderSetFlags(&pool_builder,
        NVN_MEMORY_POOL_FLAGS_CPU_CACHED | NVN_MEMORY_POOL_FLAGS_GPU_CACHED);
    nvnMemoryPoolBuilderSetStorage(&pool_builder, new (heap, 0x1000) u8[pool_size], pool_size);
    nvnMemoryPoolInitialize(pool, &pool_builder);

    size_t offset = 0;
    const s32 initialized_texture_count = Mathi::min(mBufferCount, 3);
    for (s32 i = 0; i < initialized_texture_count; ++i)
    {
        mTextures[i] = new (heap, 8) NVNtexture;
        nvnTextureBuilderSetStorage(&texture_builders[i], pool, offset);
        const size_t size = nvnTextureBuilderGetStorageSize(&texture_builders[i]);
        const s32 alignment = nvnTextureBuilderGetStorageAlignment(&texture_builders[i]);
        offset += MathCalcCommon<size_t>::roundUp(size, alignment);
        nvnTextureInitialize(mTextures[i], &texture_builders[i]);
    }

    mWindow = new (heap, 8) NVNwindow;
    NVNwindowBuilder window_builder;
    nvnWindowBuilderSetDefaults(&window_builder);
    nvnWindowBuilderSetDevice(&window_builder, GraphicsNvn::instance()->getNvnDevice());
    nvnWindowBuilderSetNativeWindow(&window_builder, mNativeWindow);
    nvnWindowBuilderSetTextures(&window_builder, mBufferCount, mTextures.getBufferPtr());
    nvnWindowBuilderSetPresentInterval(&window_builder, mPresentInterval);
    nvnWindowInitialize(mWindow, &window_builder);
    nvnQueueAcquireTexture(GraphicsNvn::instance()->getQueue(), mWindow, &mTextureIndex);
    mWindowCropX = 0;
    mWindowCropY = 0;
    mWindowCropWidth = width;
    mWindowCropHeight = height;
    mSync = new (heap, 8) NVNsync;
    nvnSyncInitialize(mSync, device);
}

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
