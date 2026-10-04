#pragma once

#include "controller/seadControllerWrapperBase.h"

namespace sead
{
// A controller wrapper whose pad configuration maps every pad bit to a MASK of controller bits (the
// pad bit is on when any of them is held) instead of ControllerWrapper's single controller bit.
// Evidence: the constructor / calc / setPadConfig (0x7100b1b32c / b1b374 / b1b508) are the
// ControllerWrapper ones with u32 masks at +0x198 (0x80 bytes: sizeof 0x218).
class MaskControllerWrapper : public ControllerWrapperBase
{
    SEAD_RTTI_OVERRIDE(MaskControllerWrapper, ControllerWrapperBase)

public:
    // One bit per controller pad index (1 << index).
    static const u32 cPadMaskDefault[Controller::cPadIdx_Max];

    MaskControllerWrapper();
    ~MaskControllerWrapper() override = default;

    void calc(u32 prev_hold, bool prev_pointer_on) override;

    u32 createPadMaskFromControllerPadMask_(u32 controller_mask) const;
    void setPadConfig(s32 padbit_max, const u32* pad_config, bool enable_stickcross_emulation);

protected:
    u32 mPadConfig[cPadIdx_MaxBase];
};

}  // namespace sead
