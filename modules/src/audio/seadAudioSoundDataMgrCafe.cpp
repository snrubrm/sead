#include <audio/seadAudioCafe.h>
#include <prim/seadRuntimeTypeInfo.h>

namespace sead
{
namespace
{
inline bool isSdkEnabled()
{
    return DynamicCast<AudioSystemCafe>(AudioMgr::instance()->getAudioSystem())->isSdkEnabled();
}
}  // namespace

// 0x7100b97c38 (D2) / 0x7100b97d14 (D0)
AudioFsSoundArchiveCafe::~AudioFsSoundArchiveCafe()
{
    if (IsAvailable())
        Close();
    if (mInfoBlock)
    {
        delete[] mInfoBlock;
        mInfoBlock = nullptr;
    }
    if (mStringBlock)
    {
        delete[] mStringBlock;
        mStringBlock = nullptr;
    }
}

// 0x7100b97e00
bool AudioFsSoundArchiveCafe::open(const void* data)
{
    if (_650)
        _371 = true;
    Open(static_cast<const char*>(data));

    const u32 info_size = mFileHeader.GetInfoBlockSize();
    mInfoBlock = new (mHeap, 0x1000) u8[info_size];
    LoadHeader(mInfoBlock, info_size);

    if (mLoadStrings)
    {
        const u32 string_size = mFileHeader.GetStringBlockSize();
        mStringBlock = new (mHeap, 0x1000) u8[string_size];
        LoadLabelStringData(mStringBlock, string_size);
    }
    return true;
}

// 0x7100b97eb8
void AudioFsSoundArchiveCafe::close()
{
    if (IsAvailable())
        Close();
}

// 0x7100b97ef0 (D2) / 0x7100b97f84 (D0)
AudioMemorySoundArchiveCafe::~AudioMemorySoundArchiveCafe()
{
    if (IsAvailable())
        Finalize();
}

// 0x7100b98030
bool AudioMemorySoundArchiveCafe::open(const void* data)
{
    Initialize(data);
    return true;
}

// 0x7100b9804c
void AudioMemorySoundArchiveCafe::close()
{
    if (IsAvailable())
        Finalize();
}

// 0x7100b98084
AudioSoundDataMgrCafe::AudioSoundDataMgrCafe()
    : nn::atk::SoundDataManager(), mArchive(nullptr), mMemory(nullptr), mSoundHeap(nullptr),
      mContentRootPath(nullptr), mIsInitialized(false)
{
}

// 0x7100b980d0 (D2) / 0x7100b981b8 (D0)
AudioSoundDataMgrCafe::~AudioSoundDataMgrCafe()
{
    if (mIsInitialized)
    {
        Finalize();
        mIsInitialized = false;
    }
    if (mArchive)
    {
        delete mArchive;
        mArchive = nullptr;
    }
    if (mMemory)
    {
        delete[] mMemory;
        mMemory = nullptr;
    }
}

// 0x7100b982b0
void AudioSoundDataMgrCafe::connectSoundHeap(AudioSoundHeapCafe* heap)
{
    if (isSdkEnabled())
    {
        mSoundHeap = heap;
        if (heap)
        {
            if (nn::atk::SoundArchive* archive = getSoundArchive())
                mSoundHeap->setSoundDataManagement(*this, *archive);
        }
    }
}

// 0x7100b98388
nn::atk::SoundArchive* AudioSoundDataMgrCafe::getSoundArchive() const
{
    if (isSdkEnabled())
    {
        if (AudioSoundArchiveBaseCafe* archive = mArchive)
        {
            switch (archive->getType())
            {
            case AudioSoundArchiveBaseCafe::Type::Fs:
                return DynamicCast<AudioFsSoundArchiveCafe>(archive);
            case AudioSoundArchiveBaseCafe::Type::Memory:
                return DynamicCast<AudioMemorySoundArchiveCafe>(archive);
            }
        }
    }
    return nullptr;
}

// 0x7100b98508
void AudioSoundDataMgrCafe::setContentRootPath(const char* path)
{
    mContentRootPath = path;
}

// 0x7100b98510
bool AudioSoundDataMgrCafe::mountSoundArchiveFromFs(const SafeString& path, Heap* heap, bool unknown,
                                                    bool load_strings)
{
    if (isSdkEnabled())
    {
        auto* archive = new (heap, 0x1000) AudioFsSoundArchiveCafe(heap);
        mArchive = archive;
        // (The flags are masked here, like the original does.)
        archive->mLoadStrings = load_strings & 1;
        if (mContentRootPath)
            archive->mContentRoot = mContentRootPath;
        archive->_650 = unknown & 1;
        archive->open(path.cstr());
        return setupManager_(heap);
    }
    return false;
}

// 0x7100b98690
bool AudioSoundDataMgrCafe::setupManager_(Heap* heap)
{
    if (isSdkEnabled())
    {
        const u32 size = GetRequiredMemSize(getSoundArchive());
        mMemory = new (heap, 0x1000) u8[size];
        const bool result = Initialize(getSoundArchive(), mMemory, size);
        mIsInitialized = true;
        return result;
    }
    return false;
}

// 0x7100b98790
bool AudioSoundDataMgrCafe::mountSoundArchiveFromMemory(const void* data, Heap* heap)
{
    if (isSdkEnabled())
    {
        auto* archive = new (heap, 0x1000) AudioMemorySoundArchiveCafe;
        mArchive = archive;
        archive->open(data);
        return setupManager_(heap);
    }
    return false;
}

// 0x7100b988b8
void AudioSoundDataMgrCafe::unmountSoundArchive()
{
    if (isSdkEnabled())
    {
        if (mIsInitialized)
        {
            Finalize();
            mIsInitialized = false;
        }
        if (mArchive)
            delete mArchive;
        mArchive = nullptr;
        if (mMemory)
        {
            delete[] mMemory;
            mMemory = nullptr;
        }
    }
}

// NON_MATCHING: same code; the registers of the arguments are numbered differently.
// 0x7100b9898c
bool AudioSoundDataMgrCafe::loadData(const char* label, u32 load_flag, u32 unknown, AudioSoundHeapCafe* heap)
{
    if (isSdkEnabled())
    {
        if (isSdkEnabled())
        {
            if (!heap)
            {
                heap = mSoundHeap;
                if (!heap)
                    return false;
            }
            if (IsAvailable())
                return LoadData(label, heap, load_flag, unknown);
        }
    }
    return false;
}

}  // namespace sead
