#include <resource/seadParallelSZSDecompressor.h>
#include <math/seadMathCalcCommon.h>

namespace sead {

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
