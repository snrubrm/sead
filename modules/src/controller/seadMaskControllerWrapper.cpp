#include "controller/seadMaskControllerWrapper.h"

#include "prim/seadMemUtil.h"

namespace sead
{
const u32 MaskControllerWrapper::cPadMaskDefault[Controller::cPadIdx_Max] = {
    1 << 0,  1 << 1,  1 << 2,  1 << 3,  1 << 4,  1 << 5,  1 << 6,  1 << 7,  1 << 8,  1 << 9,
    1 << 10, 1 << 11, 1 << 12, 1 << 13, 1 << 14, 1 << 15, 1 << 16, 1 << 17, 1 << 18, 1 << 19,
    1 << 20, 1 << 21, 1 << 22, 1 << 23, 1 << 24, 1 << 25, 1 << 26, 1 << 27};

MaskControllerWrapper::MaskControllerWrapper()
{
    MemUtil::copy(mPadConfig, cPadMaskDefault, sizeof(cPadMaskDefault));
}

void MaskControllerWrapper::calc(u32 prev_hold, bool prev_pointer_on)
{
    if (mIsEnable && mController && mController->isConnected())
    {
        mPadHold = BitFlag32(createPadMaskFromControllerPadMask_(mController->getHoldMask()));

        mLeftStick = mController->getLeftStick();
        mRightStick = mController->getRightStick();
        mLeftAnalogTrigger = mController->getLeftAnalogTrigger();
        mRightAnalogTrigger = mController->getRightAnalogTrigger();

        bool pointer_on = mController->isPointerOn();

        bool touchkey_hold = false;
        if (mTouchKeyBit >= 0)
            touchkey_hold = mPadHold.isOnBit(mTouchKeyBit);

        setPointerWithBound_(pointer_on, touchkey_hold, mController->getPointer());
        updateDerivativeParams_(createPadMaskFromControllerPadMask_(prev_hold), prev_pointer_on);
    }
    else
    {
        setIdle();
    }

    if (isIdle_())
        mIdleFrame++;
    else
        mIdleFrame = 0;
}

void MaskControllerWrapper::setPadConfig(s32 padbit_max, const u32* pad_config,
                                         bool enable_stickcross_emulation)
{
    if (padbit_max > 32)
        return;
    mPadBitMax = padbit_max;

    MemUtil::copy(mPadConfig, pad_config, padbit_max * sizeof(u32));

    mLeftStickCrossStartBit = -1;
    mRightStickCrossStartBit = -1;

    if (enable_stickcross_emulation)
    {
        for (s32 i = 0; i < padbit_max; i++)
        {
            if (pad_config[i] & (1 << Controller::cPadIdx_LeftStickUp))
                mLeftStickCrossStartBit = i;

            else if (pad_config[i] & (1 << Controller::cPadIdx_RightStickUp))
                mRightStickCrossStartBit = i;
        }
    }

    mTouchKeyBit = -1;

    for (s32 i = 0; i < padbit_max; i++)
    {
        if (pad_config[i] & (1 << Controller::cPadIdx_Touch))
        {
            mTouchKeyBit = i;
            break;
        }
    }
}

u32 MaskControllerWrapper::createPadMaskFromControllerPadMask_(u32 controller_mask) const
{
    BitFlag32 controller_pad_mask(controller_mask);
    BitFlag32 pad_mask;

    for (s32 i = 0; i < mPadBitMax; i++)
    {
        if (controller_pad_mask.isOn(mPadConfig[i]))
            pad_mask.setBit(i);
    }

    return pad_mask.getDirect();
}

}  // namespace sead
