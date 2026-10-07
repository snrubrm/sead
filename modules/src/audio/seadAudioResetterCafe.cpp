#include <audio/seadAudioCafe.h>
#include <nn/atk/detail/driver/HardwareManager.h>
#include <prim/seadRuntimeTypeInfo.h>

namespace sead
{
namespace
{
inline nn::atk::detail::driver::HardwareManager& getHardwareManager()
{
    return nn::atk::detail::Util::Singleton<nn::atk::detail::driver::HardwareManager>::GetInstance();
}
}  // namespace

// 0x7100bb9818
AudioResetterCafe::AudioResetterCafe() = default;

// 0x7100bb9854
void AudioResetterCafe::initialize(AudioMgr& mgr)
{
    AudioResetter::initialize(mgr);
}

// NON_MATCHING: the original tests `mShutdownState != 2` and `volume == 0` as two separate branches (and loads
// the reset state after them); here the compiler fuses the two into one fcmp/ccmp pair.
// 0x7100bb9858
void AudioResetterCafe::calc()
{
    f32 volume = getHardwareManager().GetMasterVolume();

    if (mShutdownState == 1 && volume == 0.0f)
    {
        mShutdownState = 2;
    }
    else if (mShutdownState != 2 && volume == 0.0f && mResetState == 1)
    {
        DynamicCast<AudioPlayerCafe>(mMgr->getPlayer())->stopAll(0);
        mResetState = 2;
    }
}

// 0x7100bb9964
void AudioResetterCafe::reset(s32 frames)
{
    if (mShutdownState == 0 && mResetState == 0)
    {
        mSavedMasterVolume = getHardwareManager().GetMasterVolume();
        getHardwareManager().SetMasterVolume(0.0f, frames);
        AudioResetter::reset(frames);
        mResetState = 1;
    }
}

// 0x7100bb99ec
bool AudioResetterCafe::isResetting() const
{
    if (mResetState != 0)
        return true;
    return AudioResetter::isResetting();
}

// 0x7100bb9a00
bool AudioResetterCafe::isResetDone() const
{
    if (!AudioResetter::isResetDone())
        return false;
    return mResetState == 2;
}

// 0x7100bb9a38
void AudioResetterCafe::recoverReset()
{
    if (mShutdownState == 0)
    {
        f32 volume = mSavedMasterVolume;
        getHardwareManager().SetMasterVolume(volume, 0);
        AudioResetter::recoverReset();
        DynamicCast<AudioPlayerCafe>(mMgr->getPlayer())->unpauseAll(0);
        mResetState = 0;
    }
}

// 0x7100bb9b04
void AudioResetterCafe::shutdown(s32 frames)
{
    if (mShutdownState == 0)
    {
        getHardwareManager().SetMasterVolume(0.0f, frames);
        AudioResetter::shutdown(frames);
        mShutdownState = 1;
    }
}

// 0x7100bb9b50
bool AudioResetterCafe::isShuttingDown() const
{
    if (mShutdownState != 0)
        return true;
    return AudioResetter::isShuttingDown();
}

// 0x7100bb9b64
bool AudioResetterCafe::isShutdownDone() const
{
    if (!AudioResetter::isShutdownDone())
        return false;
    return mShutdownState == 2;
}

}  // namespace sead
