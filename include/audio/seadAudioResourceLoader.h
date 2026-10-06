#pragma once

#include <prim/seadRuntimeTypeInfo.h>

namespace sead
{
class AudioMgr;

/// Loads the audio resources (sound archive, sound data) when the AudioMgr is prepared.
class AudioResourceLoader
{
    SEAD_RTTI_BASE(AudioResourceLoader)
public:
    virtual ~AudioResourceLoader() {}

    virtual void initialize(AudioMgr& mgr) = 0;
    virtual void load() = 0;
    virtual void finalize() = 0;
};

}  // namespace sead
