#pragma once

#include <mc/seadCoreInfo.h>
#include <resource/seadDecompressor.h>
#include <resource/seadSZSDecompressor.h>
#include <thread/seadEvent.h>
#include <thread/seadThread.h>

namespace sead
{
class ParallelSZSDecompressor : public Decompressor
{
public:
    ParallelSZSDecompressor(u32 workSize, s32 threadPriority, sead::Heap* heap, u8* workBuffer,
                            const CoreIdMask& mask);
    ~ParallelSZSDecompressor() override;

    u8* tryDecompFromDevice(const ResourceMgr::LoadArg& loadArg, Resource* resource, u32* outSize,
                            u32* outAllocSize, bool* outAllocated) override;

    void setDivSize(u32);

private:
    // Constructed in the parent constructor at +0x88. The thread handles chunks of the
    // compressed work buffer and signals the two events used by the parent.
    class DecompThread : public Thread
    {
    public:
        DecompThread(Heap* heap, s32 priority);
        ~DecompThread() override;

    private:
        // NON_MATCHING: the compiler uses a different result test and instruction schedule.
        void calc_(MessageQueue::Element message) override;

        /* 0x100 */ SZSDecompressor::DecompContext mContext;
        /* 0x120 */ Event mFinished;
        /* 0x150 */ Event mChunkFinished;
        /* 0x180 */ u8* mInput;
        /* 0x188 */ u32 mInputSize;
        /* 0x18c */ u32 mChunkSize;
        /* 0x190 */ s32 mResult;
    };
    static_assert(sizeof(DecompThread) == 0x198);

    /* 0x78 */ u32 mWorkSize;
    /* 0x80 */ u8* mWorkBuffer;
    /* 0x88 */ DecompThread mThread;
};

static_assert(sizeof(ParallelSZSDecompressor) == 0x220);

}  // namespace sead
