#include <audio/seadAudioCafe.h>
#include <heap/seadHeapMgr.h>

namespace sead
{
// 0x7100b98ed4
AudioSoundHeapCafe::AudioSoundHeapCafe(size_t size, Heap* heap)
    : nn::atk::SoundHeap(), mMemory(nullptr), mDataManager(nullptr), mArchive(nullptr)
{
    if (!heap)
        heap = HeapMgr::instance()->getCurrentHeap();
    if (!size)
        size = heap->getMaxAllocatableSize(0x1000) - 0x1000;
    mMemory = new (heap, 0x1000) u8[size];
    Create(mMemory, size);
}

// 0x7100b98f78 (D2) / 0x7100b98fc0 (D0)
AudioSoundHeapCafe::~AudioSoundHeapCafe()
{
    Destroy();
    if (mMemory)
        delete[] mMemory;
}

// 0x7100b99010
void AudioSoundHeapCafe::setSoundDataManagement(nn::atk::SoundDataManager& data_manager,
                                                nn::atk::SoundArchive& archive)
{
    mDataManager = &data_manager;
    mArchive = &archive;
}

}  // namespace sead
