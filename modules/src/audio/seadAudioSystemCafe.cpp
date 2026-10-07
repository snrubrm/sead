#include <audio/seadAudioCafe.h>
#include <nn/atk/SoundSystem.h>
#include <nn/atk/detail/driver/HardwareManager.h>
#include <nn/atk/detail/driver/SoundThread.h>
#include <prim/seadScopedLock.h>

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

// The body keeps the vtable pointer stores of the destructor (a defaulted or empty destructor does not store them).
// 0x7100b99738 (D2) / 0x7100b99774 (D0)
AudioSystemCafe::~AudioSystemCafe() { ; }

// 0x7100b990d8
void AudioSystemCafe::initialize()
{
    if (mUseTaskThread)
        mTaskThread = new (mHeap, 8)
            AudioTaskThreadCafe(mTaskThreadPriority, mHeap, "sead::AudioTaskThread", 0x2000, 0x40);

    if (mIsSdkEnabled)
    {
        const size_t size = nn::atk::SoundSystem::GetRequiredMemSize(mSoundSystemParam);
        u8* memory = new (mHeap, 0x1000) u8[size];
        mSoundMemory = memory;
        ScopedLock<CriticalSection> lock(&mCS);
        mSoundSystemMemory = memory;
        mSoundSystemMemorySize = size;
    }

    {
        ScopedLock<CriticalSection> lock(&mCS);
        initializeSdk_();
        initializeNw_();
        if (mTaskThread)
            mTaskThread->start();
    }

    if (mIsSdkEnabled)
        getHardwareManager();

    mIsInitialized = true;
}

// 0x7100b991f8
void AudioSystemCafe::finalize()
{
    if (mIsInitialized)
    {
        if (!mIsExternal)
        {
            if (mTaskThread)
                mTaskThread->quitAndWaitDoneSingleThread(false);
            if (mIsSdkEnabled)
                nn::atk::SoundSystem::Finalize();
            finalizeSdk_();
        }
        if (mSoundMemory)
            delete[] mSoundMemory;
        mSoundMemory = nullptr;
        if (mTaskThread)
        {
            delete mTaskThread;
            mTaskThread = nullptr;
        }
        mIsInitialized = false;
    }
}

// 0x7100b994c0
void AudioSystemCafe::setHeap(Heap* heap)
{
    mHeap = heap;
}

// 0x7100b994c8
void AudioSystemCafe::setCompressor(bool enable)
{
    ScopedLock<CriticalSection> lock(&mCS);
    mCompressor = enable;
}

// 0x7100b99504
void AudioSystemCafe::initializeSdk_() {}

// 0x7100b99508
void AudioSystemCafe::finalizeSdk_() {}

// 0x7100b9950c
void AudioSystemCafe::initializeNw_()
{
    if (mIsSdkEnabled)
    {
        const uintptr_t memory = reinterpret_cast<uintptr_t>(mSoundSystemMemory);
        const size_t size = mSoundSystemMemorySize;
        nn::atk::SoundSystem::Initialize(mSoundSystemParam, memory, size);
    }
}

// 0x7100b99524
void AudioSystemCafe::appendSoundFrameCallback(ISoundFrameCallback&) {}

// 0x7100b99528
void AudioSystemCafe::removeSoundFrameCallback(ISoundFrameCallback& callback)
{
    if (!mIsSdkEnabled)
        return;

    {
        ScopedLock<CriticalSection> lock(&mSoundFrameCallbackCS);
        if (mSoundFrameCallbacks.indexOf(&callback) >= 0)
            mSoundFrameCallbacks.erase(&callback);
    }
    if (mSoundFrameCallbacks.size() == 0)
        nn::atk::detail::driver::SoundThread::GetInstance().ClearSoundFrameUserCallback();
}

// 0x7100b995c0
void AudioSystemCafe::clearSoundFrameCallback()
{
    if (!mIsSdkEnabled)
        return;

    {
        ScopedLock<CriticalSection> lock(&mSoundFrameCallbackCS);
        mSoundFrameCallbacks.clear();
    }
    nn::atk::detail::driver::SoundThread::GetInstance().ClearSoundFrameUserCallback();
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
