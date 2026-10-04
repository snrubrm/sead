#pragma once

#include "basis/seadTypes.h"

namespace sead
{
class Heap;

// Wrapper around a loaded message binary (libms, `LMS_*`). Only the parts that are used by the game are declared.
class MessageSetBase
{
public:
    MessageSetBase() = default;
    virtual ~MessageSetBase();  // 0x710136ffcc

    // 0x710136ffd0: opens the message binary `data`; false if it is not valid.
    bool initialize(void* data, Heap* heap);
    // 0x7101370080
    void finalize();
    // 0x71013700c4: size in bytes of the text with the given index (0 if the index is out of range).
    s32 calcTextSizeByIndex(s32 index) const;

protected:
    void* mHandle = nullptr;
    s32 mTextNum = 0;
};

template <typename T>
class MessageSet : public MessageSetBase
{
public:
    // A tag as it appears in the text itself (the char is the tag start / end marker 0xe / 0xf): the processing code
    // gets a pointer into the string. Evidence: eui::ProcessMessageAppTag (0x7100bef968) walks the string and passes
    // the position of a tag to its delegate (start tags: marker, group, type, size of the parameters in bytes, then the
    // parameters; end tags: marker, group, type), eui::TagProcessor::preProcessEuiTag_ tests the type,
    // eui::Grammar::setWordAttrFromTag reads four parameter bytes.
    struct TagInfo
    {
        T marker;
        u16 group;
        u16 type;
        u16 paramSize;

        const u8* getParam() const { return reinterpret_cast<const u8*>(this + 1); }
    };
};
}  // namespace sead
