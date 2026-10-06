#pragma once

#include <audio/seadAudioResourceLoader.h>
#include <prim/seadSafeString.h>

namespace sead
{
class Heap;

/// TODO: incomplete (initialize and load are not implemented, the members are guessed from the constructor and the
/// setters).
class AudioResourceLoaderCafe : public AudioResourceLoader
{
    SEAD_RTTI_OVERRIDE(AudioResourceLoaderCafe, AudioResourceLoader)
public:
    AudioResourceLoaderCafe();
    ~AudioResourceLoaderCafe() override;

    void initialize(AudioMgr& mgr) override;
    void load() override;
    void finalize() override {}

    void setHeap(Heap* heap);
    void setStreamBufferSizeRate(f32 rate);
    void setStreamReadCacheSize(u32 size);
    void setSoundHeapSize(u32 size);
    /// Loads the sound archive from the file system (the path of the archive is given).
    void setArchiveOnFs(const SafeString& path);

private:
    s32 mArchiveSource = 0;
    Heap* mHeap = nullptr;
    u32 mSoundHeapSize = 0;
    u32 _1c = 0;
    f32 mStreamBufferSizeRate = 1.0f;
    u32 mStreamReadCacheSize = 0;
    u32 _28 = 0;
    bool _2c = true;
    SafeString mArchiveOnFs;
    SafeString _40;
    void* _50 = nullptr;
    void* _58 = nullptr;
    bool _60 = false;
};

}  // namespace sead
