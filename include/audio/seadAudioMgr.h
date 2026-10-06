#pragma once

#include <container/seadOffsetList.h>
#include <heap/seadDisposer.h>
#include <hostio/seadHostIONode.h>
#include <prim/seadRuntimeTypeInfo.h>

namespace sead
{
class AudioMgr;
class AudioResourceLoader;
class AudioSettingParameter;
class Heap;

/// The low level audio system (the sound library). TODO: only the virtual functions that the AudioMgr calls.
class AudioSystem
{
    SEAD_RTTI_BASE(AudioSystem)
public:
    virtual ~AudioSystem() = default;
    virtual void initialize() = 0;
    virtual void finalize() = 0;
};

/// Plays the sounds. TODO: only the virtual functions that the AudioMgr calls.
class AudioPlayer
{
    SEAD_RTTI_BASE(AudioPlayer)
public:
    virtual ~AudioPlayer() = default;
    virtual void initialize() = 0;
    virtual void finalize() = 0;
    virtual void calc() = 0;
};

/// Resets the audio system (e.g. when the output device changes).
class AudioResetter
{
public:
    virtual ~AudioResetter() = default;
    virtual void initialize(AudioMgr* mgr) = 0;
    virtual void calc() = 0;
};

/// A part of the audio system that is set up and shut down by the AudioMgr (kept in a list through the node at
/// offset 8).
class AudioSubsetBase
{
    SEAD_RTTI_BASE(AudioSubsetBase)
public:
    virtual ~AudioSubsetBase() = default;
    /// Purpose unknown.
    virtual void unknown4_() = 0;
    virtual void initialize(AudioMgr* mgr, Heap* heap) = 0;
    virtual void finalize() = 0;
    virtual void calc() = 0;

private:
    friend class AudioMgr;
    friend class AudioSettingParameter;

    ListNode mListNode;
};

/// Owns the parts of the audio system (system, player, resetter, resource loader and the subsets) that are
/// given by an AudioSettingParameter or created by default (Cafe versions).
class AudioMgr : public hostio::Node
{
    SEAD_SINGLETON_DISPOSER(AudioMgr)
public:
    AudioMgr() { mSubsets.initOffset(8); }
    virtual ~AudioMgr();

    void prepare(AudioSettingParameter* parameter, Heap* heap);
    void exit();
    void calc();

private:
    AudioSystem* mAudioSystem = nullptr;
    AudioResetter* mResetter = nullptr;
    AudioPlayer* mPlayer = nullptr;
    AudioResourceLoader* mResourceLoader = nullptr;
    OffsetList<AudioSubsetBase> mSubsets;
    Heap* mHeap = nullptr;
    bool mPrepared = false;
    bool mOwnsAudioSystem = false;
    bool mOwnsResetter = false;
    bool mOwnsPlayer = false;
};

}  // namespace sead
