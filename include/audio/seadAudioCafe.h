#pragma once

#include <audio/seadAudioMgr.h>
#include <nn/atk/SoundArchivePlayer.h>

namespace sead
{
/// TODO: only the constructor is declared (0x7100b99018); the class is 0x188 bytes.
class AudioSystemCafe : public AudioSystem
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

    /// False if the sound library (nn::atk) is not used (the system then only keeps the settings).
    bool isSdkEnabled() const { return mIsSdkEnabled; }

private:
    u8 _8[0x180 - 0x8];
    bool mIsSdkEnabled;
    u8 _181[0x188 - 0x181];
};
static_assert(sizeof(AudioSystemCafe) == 0x188);

/// TODO: only the constructor is declared (0x7100bb82b4); the class is 0x3c8 bytes.
class AudioPlayerCafe : public AudioPlayer, public nn::atk::SoundArchivePlayer
{
    SEAD_RTTI_OVERRIDE(AudioPlayerCafe, AudioPlayer)
public:
    AudioPlayerCafe();
    ~AudioPlayerCafe() override;

    void initialize() override;
    void finalize() override;
    void calc() override;

    /// 0x7100bb8510
    void stopAll(s32 frames);
    /// 0x7100bb8f50
    void unpauseAll(s32 frames);

private:
    u8 _2f0[0x310 - 0x2f0];
    bool mIsPaused;
    u8 _311[0x3c8 - 0x311];
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
