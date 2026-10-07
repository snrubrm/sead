#include <audio/seadAudioCafe.h>
#include <audio/seadAudioMgr.h>
#include <audio/seadAudioResourceLoader.h>
#include <audio/seadAudioSettingParameter.h>
#include <heap/seadHeapMgr.h>

namespace sead
{
SEAD_SINGLETON_DISPOSER_IMPL(AudioMgr)

// 0x7100b99bd8 (D2) / 0x7100b99dd0 (D0)
AudioMgr::~AudioMgr()
{
    exit();

    if (mOwnsAudioSystem && mAudioSystem)
    {
        delete mAudioSystem;
        mAudioSystem = nullptr;
    }
    if (mOwnsPlayer && mPlayer)
    {
        delete mPlayer;
        mPlayer = nullptr;
    }
    if (mOwnsResetter && mResetter)
    {
        delete mResetter;
        mResetter = nullptr;
    }
}

// 0x7100b99df4
void AudioMgr::prepare(AudioSettingParameter* parameter, Heap* heap)
{
    mHeap = heap ? heap : HeapMgr::instance()->getCurrentHeap();

    if (parameter)
    {
        mAudioSystem = parameter->mAudioSystem;
        mResetter = parameter->mResetter;
        mPlayer = parameter->mPlayer;
        mResourceLoader = parameter->mResourceLoader;
        while (!parameter->mSubsets.isEmpty())
            mSubsets.pushBack(parameter->mSubsets.popFront());
    }

    if (!mAudioSystem)
    {
        mAudioSystem = new (heap, 8) AudioSystemCafe;
        mOwnsAudioSystem = true;
    }
    if (!mPlayer)
    {
        mPlayer = new (heap, 8) AudioPlayerCafe;
        mOwnsPlayer = true;
    }
    if (!mResetter)
    {
        mResetter = new (heap, 8) AudioResetterCafe;
        mOwnsResetter = true;
    }

    if (mAudioSystem)
        mAudioSystem->initialize();
    if (mPlayer)
        mPlayer->initialize();
    if (mResetter)
        mResetter->initialize(*this);
    if (mResourceLoader)
        mResourceLoader->initialize(*this);
    if (!mSubsets.isEmpty())
    {
        for (auto it = mSubsets.begin(); it != mSubsets.end(); ++it)
            it->initialize(this, heap);
    }
    if (mResourceLoader)
        mResourceLoader->load();
    mPrepared = true;
}

// 0x7100b99d0c
void AudioMgr::exit()
{
    if (mPrepared)
    {
        if (mPlayer)
            mPlayer->finalize();
        if (mResourceLoader)
            mResourceLoader->finalize();
        if (!mSubsets.isEmpty())
        {
            for (auto it = mSubsets.begin(); it != mSubsets.end(); ++it)
                it->finalize();
        }
        if (mAudioSystem)
            mAudioSystem->finalize();
        mPrepared = false;
    }
}

// 0x7100b9a014
void AudioMgr::calc()
{
    if (mPrepared)
    {
        if (mPlayer)
            mPlayer->calc();
        if (mResetter)
            mResetter->calc();
        if (!mSubsets.isEmpty())
        {
            for (auto it = mSubsets.begin(); it != mSubsets.end(); ++it)
                it->calc();
        }
    }
}

}  // namespace sead
