#pragma once

#include <container/seadOffsetList.h>
#include <framework/seadTaskParameter.h>
#include <prim/seadRuntimeTypeInfo.h>

namespace sead
{
class AudioMgr;
class AudioPlayer;
class AudioResetter;
class AudioResourceLoader;
class AudioSubsetBase;
class AudioSystem;

/// The parts that the AudioMgr is set up with (see AudioMgr::prepare).
class AudioSettingParameter : public TaskParameter
{
    SEAD_RTTI_OVERRIDE(AudioSettingParameter, TaskParameter)
public:
    AudioSettingParameter();
    virtual ~AudioSettingParameter() = default;

    void setAudioSystem(AudioSystem* system);
    void setPlayer(AudioPlayer* player);
    void setResourceLoader(AudioResourceLoader* loader);
    void appendSubset(AudioSubsetBase* subset);

private:
    friend class AudioMgr;

    AudioSystem* mAudioSystem = nullptr;
    AudioResetter* mResetter = nullptr;
    AudioPlayer* mPlayer = nullptr;
    AudioResourceLoader* mResourceLoader = nullptr;
    /// The subsets are linked through a node at offset 8 of AudioSubsetBase.
    OffsetList<AudioSubsetBase> mSubsets;
};

}  // namespace sead
