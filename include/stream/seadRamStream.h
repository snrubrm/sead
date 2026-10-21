#pragma once

#include "stream/seadStream.h"
#include "stream/seadStreamSrc.h"

namespace sead
{
class RamStreamSrc : public StreamSrc
{
public:
    RamStreamSrc(void* buffer, u32 bufferSize);
    ~RamStreamSrc() override;

    u32 read(void* data, u32 size) override;
    u32 write(const void* data, u32 size) override;
    u32 skip(s32 offset) override;
    void rewind() override { mCurrentPos = 0; }
    bool isEOF() override { return mCurrentPos >= mBufferSize; }

    // Inline-only in the original; name is a guess. The replay recorder 0x71008bded4 writes
    // the stream's current position as the file size after XmlDocument::save.
    u32 getCurrentPos() const { return mCurrentPos; }

private:
    u8* mBuffer = nullptr;
    u32 mBufferSize = 0;
    u32 mCurrentPos = 0;
};

class RamReadStream : public ReadStream
{
public:
    RamReadStream(const void* buffer, u32 buffer_size, Stream::Modes mode);
    RamReadStream(const void* buffer, u32 buffer_size, StreamFormat* format);
    ~RamReadStream() override;

private:
    RamStreamSrc mSrc;
};

class RamWriteStream : public WriteStream
{
public:
    RamWriteStream(void* buffer, u32 buffer_size, Stream::Modes mode);
    RamWriteStream(void* buffer, u32 buffer_size, StreamFormat* format);
    ~RamWriteStream() override;

    // Inline-only in the original; name is a guess (see RamStreamSrc::getCurrentPos).
    u32 getCurrentPos() const { return mSrc.getCurrentPos(); }

private:
    RamStreamSrc mSrc;
};

}  // namespace sead
