#pragma once

#include <audio/seadAudioMgr.h>

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

private:
    u8 _8[0x188 - 0x8];
};
static_assert(sizeof(AudioSystemCafe) == 0x188);

/// TODO: only the constructor is declared (0x7100bb82b4); the class is 0x3c8 bytes (it also derives from the sound
/// library's SoundStartable at offset 8).
class AudioPlayerCafe : public AudioPlayer
{
    SEAD_RTTI_OVERRIDE(AudioPlayerCafe, AudioPlayer)
public:
    AudioPlayerCafe();
    ~AudioPlayerCafe() override;

    void initialize() override;
    void finalize() override;
    void calc() override;

private:
    u8 _8[0x3c8 - 0x8];
};
static_assert(sizeof(AudioPlayerCafe) == 0x3c8);

/// TODO: only the constructor is declared (0x7100bb9818); the class is 0x20 bytes.
class AudioResetterCafe : public AudioResetter
{
public:
    AudioResetterCafe();
    ~AudioResetterCafe() override;

    void initialize(AudioMgr* mgr) override;
    void calc() override;

private:
    u8 _8[0x20 - 0x8];
};
static_assert(sizeof(AudioResetterCafe) == 0x20);

}  // namespace sead
