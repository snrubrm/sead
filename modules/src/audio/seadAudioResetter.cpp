#include <audio/seadAudioMgr.h>

namespace sead
{
// 0x7100bb9cbc
AudioResetter::AudioResetter() : mMgr(nullptr) {}

// 0x7100bb9cd0
void AudioResetter::initialize(AudioMgr& mgr)
{
    mMgr = &mgr;
}

// 0x7100bb9cd8
void AudioResetter::reset(s32 frames)
{
    if (!mMgr->mSubsets.isEmpty())
    {
        for (auto it = mMgr->mSubsets.begin(); it != mMgr->mSubsets.end(); ++it)
            it->reset(frames);
    }
}

// 0x7100bb9d68
bool AudioResetter::isResetting() const
{
    if (!mMgr->mSubsets.isEmpty())
    {
        for (auto it = mMgr->mSubsets.begin(); it != mMgr->mSubsets.end(); ++it)
        {
            if (it->isResetting())
                return true;
        }
    }
    return false;
}

// 0x7100bb9df8
bool AudioResetter::isResetDone() const
{
    if (!mMgr->mSubsets.isEmpty())
    {
        for (auto it = mMgr->mSubsets.begin(); it != mMgr->mSubsets.end(); ++it)
        {
            if (!it->isResetDone())
                return false;
        }
    }
    return true;
}

// 0x7100bb9e88
void AudioResetter::recoverReset()
{
    if (!mMgr->mSubsets.isEmpty())
    {
        for (auto it = mMgr->mSubsets.begin(); it != mMgr->mSubsets.end(); ++it)
            it->recoverReset();
    }
}

// 0x7100bb9f08
void AudioResetter::shutdown(s32 frames)
{
    if (!mMgr->mSubsets.isEmpty())
    {
        for (auto it = mMgr->mSubsets.begin(); it != mMgr->mSubsets.end(); ++it)
            it->shutdown(frames);
    }
}

// 0x7100bb9f98
bool AudioResetter::isShuttingDown() const
{
    if (!mMgr->mSubsets.isEmpty())
    {
        for (auto it = mMgr->mSubsets.begin(); it != mMgr->mSubsets.end(); ++it)
        {
            if (it->isShuttingDown())
                return true;
        }
    }
    return false;
}

// 0x7100bba028
bool AudioResetter::isShutdownDone() const
{
    if (!mMgr->mSubsets.isEmpty())
    {
        for (auto it = mMgr->mSubsets.begin(); it != mMgr->mSubsets.end(); ++it)
        {
            if (!it->isShutdownDone())
                return false;
        }
    }
    return true;
}

}  // namespace sead
