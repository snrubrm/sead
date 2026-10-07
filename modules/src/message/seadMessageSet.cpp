#include <message/seadMessageSet.h>
#include <heap/seadHeap.h>

// The message library (libms). The names are the ones of the functions in the binary.
extern "C" {
void LMS_SetMemFuncs(void* (*alloc)(size_t), void (*free)(void*));
void* LMS_InitMessage(void* data);
void LMS_CloseMessage(void* message);
s32 LMS_GetTextNum(void* message);
s32 LMS_GetTextSize(void* message, s32 index);
}

namespace sead
{
Heap* MessageProject::sHeap;

// 0x7101370060
void* MessageProject::allocForLibms_(size_t size)
{
    return new (sHeap, 8) u8[size];
}

// 0x7101370074
void MessageProject::freeForLibms_(void* ptr)
{
    if (ptr)
        delete[] static_cast<u8*>(ptr);
}

// 0x710136ffcc
MessageSetBase::~MessageSetBase() = default;

// 0x710136ffd0
bool MessageSetBase::initialize(void* data, Heap* heap)
{
    MessageProject::sHeap = heap;
    LMS_SetMemFuncs(&MessageProject::allocForLibms_, &MessageProject::freeForLibms_);
    mHandle = LMS_InitMessage(data);
    const s32 num = LMS_GetTextNum(mHandle);
    s32 text_num = num;
    if (num < 0)
    {
        LMS_CloseMessage(mHandle);
        text_num = 0;
        mHandle = nullptr;
    }
    mTextNum = text_num;
    LMS_SetMemFuncs(nullptr, nullptr);
    MessageProject::sHeap = nullptr;
    return num >= 0;
}

// 0x7101370080
void MessageSetBase::finalize()
{
    LMS_SetMemFuncs(nullptr, &MessageProject::freeForLibms_);
    LMS_CloseMessage(mHandle);
    mHandle = nullptr;
    mTextNum = 0;
    LMS_SetMemFuncs(nullptr, nullptr);
}

// 0x71013700c4
s32 MessageSetBase::calcTextSizeByIndex(s32 index) const
{
    if (static_cast<u32>(index) < static_cast<u32>(mTextNum))
        return LMS_GetTextSize(mHandle, index);
    return 0;
}

}  // namespace sead
