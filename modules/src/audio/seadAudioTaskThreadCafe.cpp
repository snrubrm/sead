#include <audio/seadAudioCafe.h>

namespace sead
{
// 0x7100b99854
AudioTaskThreadCafe::AudioTaskThreadCafe(s32 priority, Heap* heap, const SafeString& name, s32 stack_size,
                                         s32 message_queue_size)
    : Thread(name, heap, priority, MessageQueue::BlockType::Blocking, 0x7fffffff, stack_size, message_queue_size)
{
}

// 0x7100b998b4 (D2) / 0x7100b99908 (D0)
AudioTaskThreadCafe::~AudioTaskThreadCafe()
{
    if (!isDone())
        quitAndWaitDoneSingleThread(false);
}

// 0x7100b99964
bool AudioTaskThreadCafe::start()
{
    return Thread::start();
}

// 0x7100b99968
void AudioTaskThreadCafe::calc_(MessageQueue::Element msg)
{
    if (msg)
    {
        if (mCallback)
            mCallback->beforeMessage();

        auto* task = reinterpret_cast<AudioTask*>(msg);
        task->run(mState == State::cQuitting);
        task->mNumPending.decrement();

        if (mCallback)
            mCallback->afterMessage();
    }
}

}  // namespace sead
