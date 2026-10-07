#include <controller/nin/seadNinJoyNpadDevice.h>
#include <prim/seadScopedLock.h>

namespace sead
{
// 0x7100b2bbd0 (D2) / 0x7100b2c108 (D0)
NinJoyNpadDevice::VibrationThread::~VibrationThread() = default;

// 0x7100b2bef8
void NinJoyNpadDevice::setNpadIdUpdateNum(u32 num)
{
    if (num <= 8)
        mNpadIdUpdateNum = num;
}

// 0x7100b2bf08
void NinJoyNpadDevice::setSupportedNpadStyleSet(nn::hid::NpadStyleSet style_set)
{
    nn::hid::SetSupportedNpadStyleSet(style_set);
}

// 0x7100b2bf10
void NinJoyNpadDevice::setNpadJoyHoldType(nn::hid::NpadJoyHoldType type)
{
    mNpadJoyHoldType = type;
    nn::hid::SetNpadJoyHoldType(type);
}

// 0x7100b2bf1c
void NinJoyNpadDevice::setNpadJoyAssignmentModeDual(s32 id)
{
    if (u32(id) <= 7)
    {
        const u32 npad_id = id;
        nn::hid::SetNpadJoyAssignmentModeDual(npad_id);
    }
}

// 0x7100b2c058
// NON_MATCHING: same code; the original calls unlock() (not as a tail call) in the empty case and shares the epilogue
void NinJoyNpadDevice::VibrationThread::calc_(s64)
{
    mCS.lock();
    if (mRequests.empty())
    {
        mCS.unlock();
        return;
    }

    const Request request = mRequests.popFront();
    mCS.unlock();
    nn::hid::SendVibrationValue(request.handle, request.value);
}
}  // namespace sead
