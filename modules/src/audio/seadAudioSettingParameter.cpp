#include <audio/seadAudioSettingParameter.h>

namespace sead
{
// 0x7100b9a0c0
AudioSettingParameter::AudioSettingParameter()
{
    mSubsets.initOffset(8);
}

// 0x7100b9a0ec
void AudioSettingParameter::setAudioSystem(AudioSystem* system)
{
    mAudioSystem = system;
}

// 0x7100b9a0f4
void AudioSettingParameter::setPlayer(AudioPlayer* player)
{
    mPlayer = player;
}

// 0x7100b9a0fc
void AudioSettingParameter::setResourceLoader(AudioResourceLoader* loader)
{
    mResourceLoader = loader;
}

// 0x7100b9a104
void AudioSettingParameter::appendSubset(AudioSubsetBase* subset)
{
    mSubsets.pushBack(subset);
}

}  // namespace sead
