#include <audio/seadAudioCafe.h>
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
