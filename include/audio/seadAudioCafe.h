#pragma once

#include <audio/seadAudioMgr.h>
#include <container/seadOffsetList.h>
#include <hostio/seadHostIONode.h>
#include <thread/seadCriticalSection.h>
#include <thread/seadAtomic.h>
#include <nn/atk/SoundSystem.h>
#include <thread/seadThread.h>
#include <nn/atk/SoundArchive.h>
#include <nn/atk/SoundArchivePlayer.h>
#include <nn/atk/SoundDataManager.h>
#include <nn/atk/SoundHandle.h>
#include <nn/atk/SoundHeap.h>
#include <aal/aalMemoryPool.h>

namespace sead
{
/// A task of the AudioTaskThreadCafe (a message of the thread is a pointer to the task).
class AudioTask
{
public:
    /// `is_quitting`: the thread is quitting.
    virtual void taskThreadProc_(bool is_quitting) = 0;

    /// The number of times the task was sent to a thread and has not been run yet.
    Atomic<s32> mNumPending;
};

/// Is called by the AudioTaskThreadCafe around every message (purpose unknown).
class AudioTaskThreadCallback
{
public:
    virtual ~AudioTaskThreadCallback() = default;
    virtual void beforeMessage() = 0;
    virtual void afterMessage() = 0;
};

/// The thread that the audio system runs its tasks on.
class AudioTaskThreadCafe : public Thread
{
public:
    AudioTaskThreadCafe(s32 priority, Heap* heap, const SafeString& name, s32 stack_size, s32 message_queue_size);
    ~AudioTaskThreadCafe() override;

    bool start() override;

protected:
    void calc_(MessageQueue::Element msg) override;

private:
    AudioTaskThreadCallback* mCallback = nullptr;
};
static_assert(sizeof(AudioTaskThreadCafe) == 0x108);

/// The control of the virtual surround of the DRC (purpose of the class is not known: it is an AudioTask that does
/// nothing).
class AudioDrcVsCtrlCafe : public hostio::Node, public AudioTask
{
public:
    AudioDrcVsCtrlCafe() = default;
    virtual ~AudioDrcVsCtrlCafe();

    /// 0x7100b997c0
    void taskThreadProc_(bool is_quitting) override;
};
static_assert(sizeof(AudioDrcVsCtrlCafe) == 0x18);

/// The audio system of the Cafe sound library (nn::atk).
class AudioSystemCafe : public AudioSystem, public hostio::Node
{
    SEAD_RTTI_OVERRIDE(AudioSystemCafe, AudioSystem)
public:
    AudioSystemCafe();
    ~AudioSystemCafe() override;

    void initialize() override;
    void finalize() override;
    /// 0x7100b99284 / 0x7100b992d4
    bool setOutputMode(AudioGlobal::OutputMode mode) override;
    AudioGlobal::OutputMode getOutputMode() const override;
    /// 0x7100b99318 / 0x7100b993ec (declared only)
    bool appendEffect(AudioGlobal::AuxBus bus, AudioFx* effect) override;
    bool appendFxObject(AudioGlobal::AuxBus bus, AudioFxObject* effect) override;
    /// 0x7100b99454 / 0x7100b99488
    void clearEffect(AudioGlobal::AuxBus bus, s32 unused) override;
    bool isFinishedClearEffect(AudioGlobal::AuxBus bus) override;
    /// 0x7100b99524 / 0x7100b99528 / 0x7100b995c0
    void appendSoundFrameCallback(ISoundFrameCallback& callback) override;
    void removeSoundFrameCallback(ISoundFrameCallback& callback) override;
    void clearSoundFrameCallback() override;

    /// 0x7100b994c0 / 0x7100b994c8
    void setHeap(Heap* heap);
    void setCompressor(bool enable);

    /// False if the sound library (nn::atk) is not used (the system then only keeps the settings).
    bool isSdkEnabled() const { return mIsSdkEnabled; }

protected:
    /// 0x7100b99504 / 0x7100b99508 / 0x7100b9950c (initializeNw_ declared only)
    virtual void initializeSdk_();
    virtual void finalizeSdk_();
    virtual void initializeNw_();

private:
    /// The memory that the sound library is initialised with (initializeNw_).
    void* mSoundSystemMemory;
    size_t mSoundSystemMemorySize;
    nn::atk::SoundSystem::SoundSystemParam mSoundSystemParam;
    u32 _90;
    u32 _94;
    u32 _98;
    bool mCompressor;
    u8 _9d[0xa0 - 0x9d];
    CriticalSection mCS;
    /// Whether the sound library is shut down by someone else (finalize then only frees the memory).
    bool mIsExternal;
    u8 _e1[0xe4 - 0xe1];
    u32 _e4;
    Heap* mHeap;
    /// The memory of the sound library.
    u8* mSoundMemory;
    bool mIsInitialized;
    u8 _f9[0x100 - 0xf9];
    OffsetList<ISoundFrameCallback> mSoundFrameCallbacks;
    CriticalSection mSoundFrameCallbackCS;
    AudioTaskThreadCafe* mTaskThread;
    /// Whether initialize creates the AudioTaskThreadCafe.
    bool mUseTaskThread;
    u8 _161[0x164 - 0x161];
    s32 mTaskThreadPriority;
    AudioDrcVsCtrlCafe mDrcVsCtrl;
    bool mIsSdkEnabled;
    u8 _181[0x188 - 0x181];
};
static_assert(sizeof(AudioSystemCafe) == 0x188);

/// A handle to a sound of the audio player.
class SoundHandle : public nn::atk::SoundHandle
{
};

/// A sound archive of the audio system (the base of the file system and the memory archives).
class AudioSoundArchiveBaseCafe
{
    SEAD_RTTI_BASE(AudioSoundArchiveBaseCafe)
public:
    enum class Type : s32
    {
        Fs = 0,
        Memory = 1,
    };

    explicit AudioSoundArchiveBaseCafe(Type type) : mType(type) {}
    virtual ~AudioSoundArchiveBaseCafe() = default;

    virtual bool open(const void* data) = 0;
    virtual void close() = 0;

    Type getType() const { return mType; }

private:
    Type mType;
};
static_assert(sizeof(AudioSoundArchiveBaseCafe) == 0x10);

/// A sound archive that is read from a file (0x658 bytes).
class AudioFsSoundArchiveCafe : public AudioSoundArchiveBaseCafe, public nn::atk::FsSoundArchive
{
    SEAD_RTTI_OVERRIDE(AudioFsSoundArchiveCafe, AudioSoundArchiveBaseCafe)
public:
    explicit AudioFsSoundArchiveCafe(Heap* heap)
        : AudioSoundArchiveBaseCafe(Type::Fs), mHeap(heap), mInfoBlock(nullptr), mStringBlock(nullptr),
          mContentRoot("/vol/content"), _650(false)
    {
    }
    ~AudioFsSoundArchiveCafe() override;

    /// 0x7100b97e00 / 0x7100b97eb8: `data` is the path of the archive file.
    bool open(const void* data) override;
    void close() override;

    Heap* mHeap;
    u8* mInfoBlock;
    u8* mStringBlock;
    /// Whether the label strings are loaded too.
    bool mLoadStrings;
    const char* mContentRoot;
    bool _650;
};
static_assert(sizeof(AudioFsSoundArchiveCafe) == 0x658);

/// A sound archive that is in memory (0x308 bytes).
class AudioMemorySoundArchiveCafe : public AudioSoundArchiveBaseCafe, public nn::atk::MemorySoundArchive
{
    SEAD_RTTI_OVERRIDE(AudioMemorySoundArchiveCafe, AudioSoundArchiveBaseCafe)
public:
    AudioMemorySoundArchiveCafe() : AudioSoundArchiveBaseCafe(Type::Memory) {}
    ~AudioMemorySoundArchiveCafe() override;

    /// 0x7100b98030 / 0x7100b9804c: `data` is the archive.
    bool open(const void* data) override;
    void close() override;
};
static_assert(sizeof(AudioMemorySoundArchiveCafe) == 0x308);

class AudioSoundHeapCafe;

/// Manages the data of the sound archive (0x268 bytes).
class AudioSoundDataMgrCafe : public nn::atk::SoundDataManager
{
public:
    AudioSoundDataMgrCafe();
    ~AudioSoundDataMgrCafe() override;

    /// 0x7100b982b0 / 0x7100b98388
    void connectSoundHeap(AudioSoundHeapCafe* heap);
    nn::atk::SoundArchive* getSoundArchive() const;
    /// 0x7100b98508: the root directory of the content (the default is "/vol/content").
    void setContentRootPath(const char* path);
    /// 0x7100b98510 / 0x7100b98790 / 0x7100b988b8 / 0x7100b9898c
    bool mountSoundArchiveFromFs(const SafeString& path, Heap* heap, bool unknown, bool load_strings);
    bool mountSoundArchiveFromMemory(const void* data, Heap* heap);
    void unmountSoundArchive();
    bool loadData(const char* label, u32 load_flag, u32 unknown, AudioSoundHeapCafe* heap);

private:
    /// 0x7100b98690
    bool setupManager_(Heap* heap);

    AudioSoundArchiveBaseCafe* mArchive;
    u8* mMemory;
    AudioSoundHeapCafe* mSoundHeap;
    const char* mContentRootPath;
    bool mIsInitialized;
};
static_assert(sizeof(AudioSoundDataMgrCafe) == 0x268);

/// The heap that the sound data is loaded to (0x60 bytes).
class AudioSoundHeapCafe : public nn::atk::SoundHeap, public hostio::Node
{
public:
    /// Uses the largest free block of the heap (the current heap if `heap` is null) if `size` is 0.
    AudioSoundHeapCafe(size_t size, Heap* heap);
    ~AudioSoundHeapCafe() override;

    /// 0x7100b99010
    void setSoundDataManagement(nn::atk::SoundDataManager& data_manager, nn::atk::SoundArchive& archive);

private:
    u8* mMemory;
    nn::atk::SoundDataManager* mDataManager;
    nn::atk::SoundArchive* mArchive;
};
static_assert(sizeof(AudioSoundHeapCafe) == 0x60);

/// The player of the audio system (0x3c8 bytes).
class AudioPlayerCafe : public AudioPlayer, public nn::atk::SoundArchivePlayer, public hostio::Node
{
    SEAD_RTTI_OVERRIDE(AudioPlayerCafe, AudioPlayer)
public:
    /// The sizes of the buffers of setupDataManagement.
    struct DataManagementSetupParam
    {
        /// The size of the stream buffer is the size that the sound library needs multiplied by this.
        f32 stream_buffer_scale;
        u32 _4;
        u32 _8;
        Heap* heap;
    };
    static_assert(sizeof(DataManagementSetupParam) == 0x18);

    AudioPlayerCafe();
    ~AudioPlayerCafe() override;

    void initialize() override;
    void finalize() override;
    void calc() override;
    bool startSound(SoundHandle* handle, u32 id) override;
    bool startSound(SoundHandle* handle, const char* label) override;
    bool holdSound(SoundHandle* handle, u32 id) override;
    bool holdSound(SoundHandle* handle, const char* label) override;
    u32 getSoundCount() const override;
    const char* getSoundName(u32 id) const override;
    u32 getSoundId(const char* label) const override;
    nn::Result detail_SetupSound(nn::atk::SoundHandle* handle, u32 sound_id, bool hold, const char* label,
                                 const StartInfo* start_info) override;

    /// 0x7100bb8510
    void stopAll(s32 frames);
    /// 0x7100bb8f50
    void unpauseAll(s32 frames);

    /// 0x7100bb8840 / 0x7100bb91cc / 0x7100bb92b0 / 0x7100bb95ac
    void shutdownDataManagement();
    void createSoundHeap(size_t size, Heap* heap);
    bool setupDataManagement(u32 stream_buffer_size, u32 stream_cache_size, u32 setup_size, Heap* heap);
    bool setupDataManagement(const DataManagementSetupParam& param);

    AudioSoundDataMgrCafe* getDataManager() const { return mDataMgr; }

private:
    /// 0x7100bb93bc
    bool setupDataManagementInner_(const nn::atk::SoundArchive& archive, u32 stream_buffer_size,
                                   u32 stream_cache_size, u32 setup_size, Heap* heap);

    void* mSetupBuffer;
    u32 mSetupBufferSize;
    void* mStreamBuffer;
    u32 mStreamBufferSize;
    u32 mRequiredStreamBufferSize;
    void* mStreamCacheBuffer;
    u32 mStreamCacheBufferSize;
    AudioSoundDataMgrCafe* mDataMgr;
    AudioSoundHeapCafe* mSoundHeap;
    bool mIsPaused;
    /// While it is set, no sound can be started.
    bool _311;
    CriticalSection mCS;
    bool mUseCS;
    aal::MemoryPool mMemoryPool;
};
static_assert(sizeof(AudioPlayerCafe) == 0x3c8);

/// Fades the master volume out and back in around a reset (and when shutting down), and stops / unpauses the sounds.
class AudioResetterCafe : public AudioResetter
{
public:
    AudioResetterCafe();
    ~AudioResetterCafe() override = default;

    void initialize(AudioMgr& mgr) override;
    void calc() override;
    void reset(s32 frames) override;
    bool isResetting() const override;
    bool isResetDone() const override;
    void recoverReset() override;
    void shutdown(s32 frames) override;
    bool isShuttingDown() const override;
    bool isShutdownDone() const override;

private:
    /// 0: idle, 1: fading out / paused, 2: reset done
    s32 mResetState = 0;
    /// 0: idle, 1: fading out, 2: shutdown done
    s32 mShutdownState = 0;
    /// The master volume before the reset, restored by recoverReset.
    f32 mSavedMasterVolume = 1.0f;
};
static_assert(sizeof(AudioResetterCafe) == 0x20);

}  // namespace sead
