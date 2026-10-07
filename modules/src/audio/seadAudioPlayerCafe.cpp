#include <audio/seadAudioCafe.h>
#include <nn/atk/SoundPlayer.h>
#include <prim/seadRuntimeTypeInfo.h>

namespace sead
{
// The id of the first sound player of the sound archive.
static constexpr u32 cSoundPlayerIdBase = 0x4000000;

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
