#include <audio/seadAudioCafe.h>
#include <aal/aalMemoryPoolManager.h>
#include <aal/aalSystem.h>
#include <aal/aalSystemAccessor.h>
#include <nn/atk/SoundPlayer.h>
#include <nn/atk/SoundSystem.h>
#include <prim/seadRuntimeTypeInfo.h>
#include <prim/seadScopedLock.h>

namespace sead
{
// The id of the first sound player of the sound archive.
static constexpr u32 cSoundPlayerIdBase = 0x4000000;

namespace
{
inline bool isSdkEnabled()
{
    return DynamicCast<AudioSystemCafe>(AudioMgr::instance()->getAudioSystem())->isSdkEnabled();
}
}  // namespace

// NON_MATCHING: same stores; the original initialises the members of the memory pool between the two clears of its
// name string (here the pool is constructed in one piece).
// 0x7100bb82b4
AudioPlayerCafe::AudioPlayerCafe()
    : mSetupBuffer(nullptr), mSetupBufferSize(0), mStreamBuffer(nullptr), mStreamBufferSize(0),
      mRequiredStreamBufferSize(0), mStreamCacheBuffer(nullptr), mStreamCacheBufferSize(0), mDataMgr(nullptr),
      mSoundHeap(nullptr), mIsPaused(false), _311(false), mCS(), mUseCS(false), mMemoryPool()
{
    mDataMgr = new AudioSoundDataMgrCafe;
}

// 0x7100bb83b4 (D1) / 0x7100bb85ec (D0)
AudioPlayerCafe::~AudioPlayerCafe()
{
    stopAll(0);
    if (mSetupBuffer)
    {
        delete[] static_cast<u8*>(mSetupBuffer);
        mSetupBuffer = nullptr;
    }
    if (mStreamBuffer)
    {
        delete[] static_cast<u8*>(mStreamBuffer);
        mStreamBuffer = nullptr;
    }
    if (mStreamCacheBuffer)
    {
        delete[] static_cast<u8*>(mStreamCacheBuffer);
        mStreamCacheBuffer = nullptr;
    }
    if (mSoundHeap)
    {
        delete mSoundHeap;
        mSoundHeap = nullptr;
    }
    if (mDataMgr)
    {
        delete mDataMgr;
        mDataMgr = nullptr;
    }
}

// 0x7100bb8638
void AudioPlayerCafe::initialize() {}

// 0x7100bb863c
void AudioPlayerCafe::finalize()
{
    if (isSdkEnabled())
    {
        if (IsAvailable())
        {
            stopAll(0);
            if (isSdkEnabled())
            {
                mDataMgr->connectSoundHeap(nullptr);
                if (mSoundHeap)
                {
                    delete mSoundHeap;
                    mSoundHeap = nullptr;
                }
            }
            shutdownDataManagement();
            mDataMgr->unmountSoundArchive();
        }
    }
}

// 0x7100bb8840
void AudioPlayerCafe::shutdownDataManagement()
{
    if (isSdkEnabled())
    {
        Finalize();
        if (mStreamBuffer)
        {
            delete[] static_cast<u8*>(mStreamBuffer);
            mStreamBuffer = nullptr;
        }
        if (mSetupBuffer)
        {
            delete[] static_cast<u8*>(mSetupBuffer);
            mSetupBuffer = nullptr;
        }
    }
}

// 0x7100bb8900
void AudioPlayerCafe::calc()
{
    if (isSdkEnabled())
    {
        if (nn::atk::SoundSystem::IsInitialized())
        {
            if (mUseCS)
            {
                ScopedLock<CriticalSection> lock(&mCS);
                Update();
            }
            else
            {
                Update();
            }
        }
    }
}

// 0x7100bb89dc
bool AudioPlayerCafe::startSound(SoundHandle* handle, u32 id)
{
    if (isSdkEnabled())
        return StartSound(handle, id, nullptr).IsSuccess();
    return false;
}

// 0x7100bb8aa8
bool AudioPlayerCafe::startSound(SoundHandle* handle, const char* label)
{
    if (isSdkEnabled())
        return StartSound(handle, label, nullptr).IsSuccess();
    return false;
}

// 0x7100bb8b74
bool AudioPlayerCafe::holdSound(SoundHandle* handle, u32 id)
{
    if (isSdkEnabled())
        return HoldSound(handle, id, nullptr).IsSuccess();
    return false;
}

// 0x7100bb8c40
bool AudioPlayerCafe::holdSound(SoundHandle* handle, const char* label)
{
    if (isSdkEnabled())
        return HoldSound(handle, label, nullptr).IsSuccess();
    return false;
}

// 0x7100bb8d0c
u32 AudioPlayerCafe::getSoundCount() const
{
    if (isSdkEnabled())
    {
        if (nn::atk::SoundArchive* archive = mDataMgr->getSoundArchive())
            return archive->GetSoundCount();
    }
    return 0;
}

// 0x7100bb8dc0
const char* AudioPlayerCafe::getSoundName(u32 id) const
{
    if (isSdkEnabled())
    {
        if (nn::atk::SoundArchive* archive = mDataMgr->getSoundArchive())
            return archive->GetItemLabel(id);
    }
    return nullptr;
}

// 0x7100bb8e88
u32 AudioPlayerCafe::getSoundId(const char* label) const
{
    if (isSdkEnabled())
    {
        if (nn::atk::SoundArchive* archive = mDataMgr->getSoundArchive())
            return archive->GetItemId(label);
    }
    return 0xffffffff;
}

// 0x7100bb9034 (and the thunk 0x7100bb91ac)
nn::Result AudioPlayerCafe::detail_SetupSound(nn::atk::SoundHandle* handle, u32 sound_id, bool hold,
                                              const char* label, const StartInfo* start_info)
{
    if (isSdkEnabled())
    {
        DynamicCast<AudioSystemCafe>(AudioMgr::instance()->getAudioSystem());
        if (_311)
            return nn::result::detail::ConstructResult(0x80);
        if (AudioMgr::instance()->getResetter()->isResetting())
            return nn::result::detail::ConstructResult(0x80);
        return SoundArchivePlayer::detail_SetupSound(handle, sound_id, hold, label, start_info);
    }
    return nn::result::detail::ConstructResult(0x80);
}

// 0x7100bb91cc
void AudioPlayerCafe::createSoundHeap(size_t size, Heap* heap)
{
    if (isSdkEnabled())
    {
        mSoundHeap = new (heap, 0x20) AudioSoundHeapCafe(size, heap);
        mDataMgr->connectSoundHeap(mSoundHeap);
    }
}

// 0x7100bb92b0
bool AudioPlayerCafe::setupDataManagement(u32 stream_buffer_size, u32 stream_cache_size, u32 setup_size,
                                          Heap* heap)
{
    if (isSdkEnabled())
    {
        nn::atk::SoundArchive* archive = mDataMgr->getSoundArchive();
        mRequiredStreamBufferSize = GetRequiredStreamBufferSize(archive);
        return setupDataManagementInner_(
            *archive, mRequiredStreamBufferSize == 0 ? 0 : mRequiredStreamBufferSize + stream_buffer_size,
            stream_cache_size, setup_size, heap);
    }
    return false;
}

// 0x7100bb93bc
bool AudioPlayerCafe::setupDataManagementInner_(const nn::atk::SoundArchive& archive, u32 stream_buffer_size,
                                                u32 stream_cache_size, u32 setup_size, Heap* heap)
{
    if (isSdkEnabled())
    {
        const size_t memory_size = GetRequiredMemSize(&archive, setup_size);
        const u32 required_size = memory_size;
        mSetupBuffer = new (heap, 0x1000) u8[required_size];
        if (stream_buffer_size)
        {
            stream_buffer_size = (stream_buffer_size + 0xfff) & ~0xfffu;
            mStreamBuffer = new (heap, 0x1000) u8[stream_buffer_size];
            mMemoryPool.name.format("StreamBuffer");
            mMemoryPool.memory = mStreamBuffer;
            mMemoryPool.size = stream_buffer_size;
            aal::SystemAccessor::getSystem()->mMemoryPoolManager->requestAttachMemoryPool(&mMemoryPool);
            if (stream_cache_size)
            {
                mStreamCacheBufferSize = GetRequiredStreamCacheSize(&archive, (stream_cache_size + 0xfff) & ~0xfffu);
                mStreamCacheBuffer = new (heap, 0x1000) u8[mStreamCacheBufferSize];
            }
        }
        else
        {
            mStreamBuffer = nullptr;
        }

        InitializeParam param;
        param.archive = &archive;
        param.data_manager = mDataMgr;
        param.setup_buffer = mSetupBuffer;
        param.setup_buffer_size = required_size;
        param.stream_buffer = mStreamBuffer;
        param.stream_buffer_size = stream_buffer_size;
        param.stream_cache_buffer = mStreamCacheBuffer;
        param.stream_cache_buffer_size = mStreamCacheBufferSize;
        param._58 = setup_size;
        const bool result = Initialize(param);
        mSetupBufferSize = memory_size;
        mStreamBufferSize = stream_buffer_size;
        return result;
    }
    return false;
}

// 0x7100bb95ac
bool AudioPlayerCafe::setupDataManagement(const DataManagementSetupParam& param)
{
    if (isSdkEnabled())
    {
        nn::atk::SoundArchive* archive = mDataMgr->getSoundArchive();
        mRequiredStreamBufferSize = GetRequiredStreamBufferSize(archive);
        return setupDataManagementInner_(*archive,
                                         mRequiredStreamBufferSize == 0 ?
                                             0 :
                                             static_cast<u32>(mRequiredStreamBufferSize * param.stream_buffer_scale),
                                         param._4, param._8, param.heap);
    }
    return false;
}

// 0x7100bb8510
void AudioPlayerCafe::stopAll(s32 frames)
{
    if (DynamicCast<AudioSystemCafe>(AudioMgr::instance()->getAudioSystem())->isSdkEnabled())
    {
        u32 count = GetSoundPlayerCount();
        for (u32 i = 0; i < count; ++i)
            GetSoundPlayer(cSoundPlayerIdBase + i)->StopAllSound(frames);
    }
}

// 0x7100bb8f50
void AudioPlayerCafe::unpauseAll(s32 frames)
{
    if (DynamicCast<AudioSystemCafe>(AudioMgr::instance()->getAudioSystem())->isSdkEnabled())
    {
        u32 count = GetSoundPlayerCount();
        if (count != 0)
        {
            for (u32 i = 0; i < count; ++i)
                GetSoundPlayer(cSoundPlayerIdBase + i)->PauseAllSound(false, frames);
            mIsPaused = false;
        }
    }
}

}  // namespace sead
