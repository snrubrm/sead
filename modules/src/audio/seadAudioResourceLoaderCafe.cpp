#include <audio/seadAudioResourceLoaderCafe.h>

namespace sead
{
// 0x7100b97604
AudioResourceLoaderCafe::AudioResourceLoaderCafe() = default;

// The body keeps the vtable pointer store of the destructor (a defaulted or empty destructor does not store it).
// 0x7100b97658 (D2) / 0x7100b9766c (D0)
AudioResourceLoaderCafe::~AudioResourceLoaderCafe() { ; }

// 0x7100b979ac
void AudioResourceLoaderCafe::setHeap(Heap* heap)
{
    mHeap = heap;
}

// 0x7100b979b4
void AudioResourceLoaderCafe::setStreamBufferSizeRate(f32 rate)
{
    mStreamBufferSizeRate = rate;
}

// 0x7100b979bc
void AudioResourceLoaderCafe::setStreamReadCacheSize(u32 size)
{
    mStreamReadCacheSize = size;
}

// 0x7100b979c4
void AudioResourceLoaderCafe::setSoundHeapSize(u32 size)
{
    mSoundHeapSize = size;
}

// 0x7100b979cc
void AudioResourceLoaderCafe::setArchiveOnFs(const SafeString& path)
{
    mArchiveOnFs = path;
    mArchiveSource = 1;
}

}  // namespace sead
