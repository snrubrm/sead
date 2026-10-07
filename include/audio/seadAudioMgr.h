#pragma once

#include <container/seadOffsetList.h>
#include <heap/seadDisposer.h>
#include <hostio/seadHostIONode.h>
#include <prim/seadRuntimeTypeInfo.h>

namespace sead
{
class AudioFx;
class AudioFxObject;
class ISoundFrameCallback;
class AudioMgr;
class AudioResourceLoader;
class SoundHandle;
class AudioSettingParameter;
class Heap;

struct AudioGlobal
{
    /// The names of the values are not known (aal::Settings converts its output mode with the table {1, 0, 2}; 4 is
    /// returned for values that are out of range).
    enum OutputMode : u32
    {
    };

    /// The auxiliary buses (0 - 2 of the main output, 3 - 5 of the other output; the names are not known).
    enum AuxBus : u32
    {
    };
};

/// The low level audio system (the sound library). TODO: only the virtual functions up to setOutputMode.
class AudioSystem
{
    SEAD_RTTI_BASE(AudioSystem)
public:
    virtual ~AudioSystem() = default;
    virtual void initialize() = 0;
    virtual void finalize() = 0;
    virtual bool setOutputMode(AudioGlobal::OutputMode mode) = 0;
    virtual AudioGlobal::OutputMode getOutputMode() const = 0;
    virtual bool appendEffect(AudioGlobal::AuxBus bus, AudioFx* effect) = 0;
    virtual bool appendFxObject(AudioGlobal::AuxBus bus, AudioFxObject* effect) = 0;
    virtual void clearEffect(AudioGlobal::AuxBus bus, s32 unused) = 0;
    virtual bool isFinishedClearEffect(AudioGlobal::AuxBus bus) = 0;
    virtual void appendSoundFrameCallback(ISoundFrameCallback& callback) = 0;
    virtual void removeSoundFrameCallback(ISoundFrameCallback& callback) = 0;
    virtual void clearSoundFrameCallback() = 0;
};


/// Plays the sounds. The base class does nothing (no sound can be started).
class AudioPlayer
{
    SEAD_RTTI_BASE(AudioPlayer)
public:
    virtual ~AudioPlayer() = default;
    virtual void initialize() {}
    virtual void finalize() {}
    virtual void calc() {}
    virtual bool startSound(SoundHandle*, u32) { return false; }
    virtual bool startSound(SoundHandle*, const char*) { return false; }
    virtual bool holdSound(SoundHandle*, u32) { return false; }
    virtual bool holdSound(SoundHandle*, const char*) { return false; }
    virtual u32 getSoundCount() const { return 0; }
    virtual const char* getSoundName(u32) const { return nullptr; }
    virtual u32 getSoundId(const char*) const { return 0xffffffff; }
};

/// Resets the audio system (e.g. when the output device changes). The base class forwards everything to the
/// subsets of the AudioMgr.
class AudioResetter
{
public:
    AudioResetter();
    virtual ~AudioResetter() = default;
    virtual void initialize(AudioMgr& mgr);
    virtual void calc() = 0;
    virtual void reset(s32 frames);
    virtual bool isResetting() const;
    virtual bool isResetDone() const;
    virtual void recoverReset();
    virtual void shutdown(s32 frames);
    virtual bool isShuttingDown() const;
    virtual bool isShutdownDone() const;

protected:
    AudioMgr* mMgr;
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
    /// Purpose unknown (virtual function 8 and 9, in between calc and reset).
    virtual void unknown8_() = 0;
    virtual void unknown9_() = 0;
    virtual void reset(s32 frames) = 0;
    virtual bool isResetting() const = 0;
    virtual bool isResetDone() const = 0;
    virtual void recoverReset() = 0;
    virtual void shutdown(s32 frames) = 0;
    virtual bool isShuttingDown() const = 0;
    virtual bool isShutdownDone() const = 0;

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

    AudioSystem* getAudioSystem() const { return mAudioSystem; }
    AudioResetter* getResetter() const { return mResetter; }
    AudioPlayer* getPlayer() const { return mPlayer; }

private:
    friend class AudioResetter;

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
