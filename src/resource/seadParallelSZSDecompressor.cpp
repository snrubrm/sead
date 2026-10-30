#include <resource/seadParallelSZSDecompressor.h>
#include <math/seadMathCalcCommon.h>

namespace sead {

ParallelSZSDecompressor::DecompThread::DecompThread(Heap* heap, s32 priority)
    : Thread("DecompThread", heap, priority, MessageQueue::BlockType::Blocking, 0x7fffffff, 0x1000,
             0x20),
      mInput(nullptr), mInputSize(0), mChunkSize(0), mResult(0)
{
    mChunkFinished.initialize(false);
    mFinished.initialize(false);
    mFinished.setSignal();
}

ParallelSZSDecompressor::ParallelSZSDecompressor(u32 workSize, s32 threadPriority, Heap* heap,
                                                 u8* workBuffer, const CoreIdMask& mask)
    : Decompressor("szs"), mWorkBuffer(workBuffer), mThread(heap, threadPriority)
{
    mWorkSize = Mathu::roundUp(workSize, 32);
    mThread.setAffinity(mask);
    mThread.start();
}

ParallelSZSDecompressor::~ParallelSZSDecompressor()
{
    // called through a pointer in the original (not devirtualised)
    (&mThread)->quitAndWaitDoneSingleThread(false);
}

ParallelSZSDecompressor::DecompThread::~DecompThread() = default;

void ParallelSZSDecompressor::DecompThread::calc_(MessageQueue::Element message)
{
    if (mResult < 0)
        return;
    const s32 chunk = message - 1;
    const u8* input = (chunk & 1) ? mInput + mChunkSize : mInput;
    const u32 size = Mathu::min(mChunkSize, mInputSize - mChunkSize * chunk);
    const s32 result = SZSDecompressor::streamDecomp(&mContext, input, size);
    if (result <= 0)
    {
        if (result < 0)
            mResult = result;
        mFinished.setSignal();
    }
    mChunkFinished.setSignal();
}

}  // namespace sead
