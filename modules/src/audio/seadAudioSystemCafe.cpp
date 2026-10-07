#include <audio/seadAudioCafe.h>
#include <nn/atk/SoundSystem.h>
#include <nn/atk/detail/driver/HardwareManager.h>

namespace sead
{
namespace
{
inline nn::atk::detail::driver::HardwareManager& getHardwareManager()
{
    return nn::atk::detail::Util::Singleton<nn::atk::detail::driver::HardwareManager>::GetInstance();
}

// The output modes of the sound library and of AudioGlobal are in a different order (the table is its own inverse).
inline u32 convertOutputMode(s32 mode)
{
    static const u32 cOutputModes[3] = {1, 0, 2};
    return cOutputModes[mode];
}

inline nn::atk::AuxBus convertAuxBus(AudioGlobal::AuxBus bus)
{
    static const nn::atk::AuxBus cAuxBuses[6] = {
        nn::atk::AuxBus::AuxBus_A, nn::atk::AuxBus::AuxBus_B, nn::atk::AuxBus::AuxBus_C,
        nn::atk::AuxBus::AuxBus_A, nn::atk::AuxBus::AuxBus_B, nn::atk::AuxBus::AuxBus_C};
    return bus <= 5 ? cAuxBuses[static_cast<s32>(bus)] : nn::atk::AuxBus::AuxBus_A;
}
}  // namespace

// NON_MATCHING: same logic; the original sets the false result and a copy of `this` first, before the checks
// (here each early exit sets its own result).
// 0x7100b99284
bool AudioSystemCafe::setOutputMode(AudioGlobal::OutputMode mode)
{
    bool result = false;
    if (mode <= 2)
    {
        if (mIsSdkEnabled)
        {
            const nn::atk::OutputMode atk_mode = nn::atk::OutputMode(convertOutputMode(mode));
            getHardwareManager().SetOutputMode(atk_mode, nn::atk::OutputDevice(0));
            result = true;
        }
    }
    return result;
}

// 0x7100b99454
void AudioSystemCafe::clearEffect(AudioGlobal::AuxBus bus, s32)
{
    if (mIsSdkEnabled)
        nn::atk::SoundSystem::ClearEffect(convertAuxBus(bus), nn::atk::OutputDevice(0));
}

// 0x7100b99488
bool AudioSystemCafe::isFinishedClearEffect(AudioGlobal::AuxBus bus)
{
    if (mIsSdkEnabled)
        return nn::atk::SoundSystem::IsClearEffectFinished(convertAuxBus(bus), nn::atk::OutputDevice(0));
    return true;
}

// 0x7100b992d4
AudioGlobal::OutputMode AudioSystemCafe::getOutputMode() const
{
    if (mIsSdkEnabled)
    {
        s32 mode = getHardwareManager().GetOutputMode();
        if (static_cast<u32>(mode) <= 2)
            return AudioGlobal::OutputMode(convertOutputMode(mode));
    }
    return AudioGlobal::OutputMode(4);
}

}  // namespace sead
