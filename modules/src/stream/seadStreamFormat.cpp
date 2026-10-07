#include "stream/seadStreamFormat.h"

#include <cstdio>
#include "codec/seadBase64.h"
#include "math/seadMathCalcCommon.h"
#include "prim/seadStringUtil.h"
#include "prim/seadScopedLock.h"
#include "stream/seadStreamSrc.h"
#include "thread/seadMutex.h"

namespace sead
{
u8 BinaryStreamFormat::readU8(StreamSrc* src, Endian::Types endian)
{
    u8 rawValue = 0;
    src->read(&rawValue, sizeof(u8));
    return Endian::toHostU8(endian, rawValue);
}

u16 BinaryStreamFormat::readU16(StreamSrc* src, Endian::Types endian)
{
    u16 rawValue = 0;
    src->read(&rawValue, sizeof(u16));
    return Endian::toHostU16(endian, rawValue);
}

u32 BinaryStreamFormat::readU32(StreamSrc* src, Endian::Types endian)
{
    u32 rawValue = 0;
    src->read(&rawValue, sizeof(u32));
    return Endian::toHostU32(endian, rawValue);
}

u64 BinaryStreamFormat::readU64(StreamSrc* src, Endian::Types endian)
{
    u64 rawValue = 0;
    src->read(&rawValue, sizeof(u64));
    return Endian::toHostU64(endian, rawValue);
}

s8 BinaryStreamFormat::readS8(StreamSrc* src, Endian::Types endian)
{
    s8 rawValue = 0;
    src->read(&rawValue, sizeof(s8));
    return Endian::toHostU8(endian, rawValue);
}

s16 BinaryStreamFormat::readS16(StreamSrc* src, Endian::Types endian)
{
    s16 rawValue = 0;
    src->read(&rawValue, sizeof(s16));
    return Endian::toHostU16(endian, rawValue);
}

s32 BinaryStreamFormat::readS32(StreamSrc* src, Endian::Types endian)
{
    s32 rawValue = 0;
    src->read(&rawValue, sizeof(s32));
    return Endian::toHostU32(endian, rawValue);
}

s64 BinaryStreamFormat::readS64(StreamSrc* src, Endian::Types endian)
{
    s64 rawValue = 0;
    src->read(&rawValue, sizeof(s64));
    return Endian::toHostU64(endian, rawValue);
}

f32 BinaryStreamFormat::readF32(StreamSrc* src, Endian::Types endian)
{
    u32 rawValue = 0;
    src->read(&rawValue, sizeof(f32));
    return Endian::toHostF32(endian, &rawValue);
}

// NOTE: Leaves higher bits of last byte of data at their previous value
void BinaryStreamFormat::readBit(StreamSrc* src, void* data, u32 bits)
{
    u8* dataU8 = static_cast<u8*>(data);

    u32 size = bits / 8;
    src->read(dataU8, size);
    bits -= size * 8;

    if (bits <= 0)
        return;

    u8 lastByte;
    src->read(&lastByte, 1);

    u8 mask = 0xFF << bits;
    dataU8[size] &= mask;
    dataU8[size] |= lastByte & ~mask;
}

// NOTE: If size > str->getBufferSize(), it wraps around and starts reading to the start again.
// if size > str->getBufferSize()*2, the second iteration continues writing out-of-bounds.
void BinaryStreamFormat::readString(StreamSrc* src, BufferedSafeString* str, u32 size)
{
    u32 remainingSize = 0;
    if (size > (u32)str->getBufferSize())
    {
        remainingSize = size - str->getBufferSize();
        size = str->getBufferSize();
    }

    src->read(str->getBuffer(), size);

    if (size + 1 < (u32)str->getBufferSize())
        str->trim(size);
    else
        str->trim(str->getBufferSize() - 1);

    if (remainingSize != 0)
        src->read(str->getBuffer(), remainingSize);
}

u32 BinaryStreamFormat::readMemBlock(StreamSrc* src, void* buffer, u32 size)
{
    return src->read(buffer, size);
}

void BinaryStreamFormat::writeU8(StreamSrc* src, Endian::Types endian, u8 value)
{
    u8 rawValue = Endian::fromHostU8(endian, value);
    src->write(&rawValue, sizeof(u8));
}

void BinaryStreamFormat::writeU16(StreamSrc* src, Endian::Types endian, u16 value)
{
    u16 rawValue = Endian::fromHostU16(endian, value);
    src->write(&rawValue, sizeof(u16));
}

void BinaryStreamFormat::writeU32(StreamSrc* src, Endian::Types endian, u32 value)
{
    u32 rawValue = Endian::fromHostU32(endian, value);
    src->write(&rawValue, sizeof(u32));
}

void BinaryStreamFormat::writeU64(StreamSrc* src, Endian::Types endian, u64 value)
{
    u64 rawValue = Endian::fromHostU64(endian, value);
    src->write(&rawValue, sizeof(u64));
}

void BinaryStreamFormat::writeS8(StreamSrc* src, Endian::Types endian, s8 value)
{
    s8 rawValue = Endian::fromHostS8(endian, value);
    src->write(&rawValue, sizeof(s8));
}

void BinaryStreamFormat::writeS16(StreamSrc* src, Endian::Types endian, s16 value)
{
    s16 rawValue = Endian::fromHostS16(endian, value);
    src->write(&rawValue, sizeof(s16));
}

void BinaryStreamFormat::writeS32(StreamSrc* src, Endian::Types endian, s32 value)
{
    s32 rawValue = Endian::fromHostS32(endian, value);
    src->write(&rawValue, sizeof(s32));
}

void BinaryStreamFormat::writeS64(StreamSrc* src, Endian::Types endian, s64 value)
{
    s64 rawValue = Endian::fromHostS64(endian, value);
    src->write(&rawValue, sizeof(s64));
}

void BinaryStreamFormat::writeF32(StreamSrc* src, Endian::Types endian, f32 value)
{
    u32 rawValue = Endian::fromHostF32(endian, &value);
    src->write(&rawValue, sizeof(f32));
}

// NOTE: Writes extra bits in last byte into stream normally
void BinaryStreamFormat::writeBit(StreamSrc* src, const void* data, u32 bits)
{
    const u8* dataU8 = static_cast<const u8*>(data);

    u8 size = bits / 8;
    src->write(dataU8, size);

    if (size * 8 == bits)
        return;

    const u8& lastByte = dataU8[size];
    src->write(&lastByte, 1);
}

void BinaryStreamFormat::writeString(StreamSrc* src, const SafeString& str, u32 size)
{
    u32 strSize = str.calcLength();
    if (strSize > size)
        strSize = size;

    src->write(str.cstr(), strSize);

    char nullchar = '\0';
    for (; strSize < size; strSize++)
        src->write(&nullchar, 1);
}

void BinaryStreamFormat::writeMemBlock(StreamSrc* src, const void* buffer, u32 size)
{
    src->write(buffer, size);
}

void BinaryStreamFormat::skip(StreamSrc* src, u32 offset)
{
    src->skip(offset);
}

void BinaryStreamFormat::rewind(StreamSrc* src)
{
    src->rewind();
}

// The lock of the shared read buffer of the text formats (0x71025f9568).
static FixedSafeString<1024> sTextReadBuffer;
static Mutex sTextMutex;

// 0x7100b162c8
TextStreamFormat::TextStreamFormat() : mDelimiters(" \t\r\n") {}

u8 TextStreamFormat::readU8(StreamSrc* src, Endian::Types)
{
    ScopedLock<Mutex> lock(&sTextMutex);
    u8 value = 0;
    getNextData_(src);
    StringUtil::tryParseU8(&value, sTextReadBuffer.cstr(), StringUtil::CardinalNumber::BaseAuto);
    return value;
}

u16 TextStreamFormat::readU16(StreamSrc* src, Endian::Types)
{
    ScopedLock<Mutex> lock(&sTextMutex);
    u16 value = 0;
    getNextData_(src);
    StringUtil::tryParseU16(&value, sTextReadBuffer.cstr(), StringUtil::CardinalNumber::BaseAuto);
    return value;
}

u32 TextStreamFormat::readU32(StreamSrc* src, Endian::Types)
{
    ScopedLock<Mutex> lock(&sTextMutex);
    u32 value = 0;
    getNextData_(src);
    StringUtil::tryParseU32(&value, sTextReadBuffer.cstr(), StringUtil::CardinalNumber::BaseAuto);
    return value;
}

u64 TextStreamFormat::readU64(StreamSrc* src, Endian::Types)
{
    ScopedLock<Mutex> lock(&sTextMutex);
    u64 value = 0;
    getNextData_(src);
    StringUtil::tryParseU64(&value, sTextReadBuffer.cstr(), StringUtil::CardinalNumber::BaseAuto);
    return value;
}

s8 TextStreamFormat::readS8(StreamSrc* src, Endian::Types)
{
    ScopedLock<Mutex> lock(&sTextMutex);
    s8 value = 0;
    getNextData_(src);
    StringUtil::tryParseS8(&value, sTextReadBuffer.cstr(), StringUtil::CardinalNumber::BaseAuto);
    return value;
}

s16 TextStreamFormat::readS16(StreamSrc* src, Endian::Types)
{
    ScopedLock<Mutex> lock(&sTextMutex);
    s16 value = 0;
    getNextData_(src);
    StringUtil::tryParseS16(&value, sTextReadBuffer.cstr(), StringUtil::CardinalNumber::BaseAuto);
    return value;
}

s32 TextStreamFormat::readS32(StreamSrc* src, Endian::Types)
{
    ScopedLock<Mutex> lock(&sTextMutex);
    s32 value = 0;
    getNextData_(src);
    StringUtil::tryParseS32(&value, sTextReadBuffer.cstr(), StringUtil::CardinalNumber::BaseAuto);
    return value;
}

s64 TextStreamFormat::readS64(StreamSrc* src, Endian::Types)
{
    ScopedLock<Mutex> lock(&sTextMutex);
    s64 value = 0;
    getNextData_(src);
    StringUtil::tryParseS64(&value, sTextReadBuffer.cstr(), StringUtil::CardinalNumber::BaseAuto);
    return value;
}

f32 TextStreamFormat::readF32(StreamSrc* src, Endian::Types)
{
    ScopedLock<Mutex> lock(&sTextMutex);
    f32 value = 0;
    getNextData_(src);
    if (sTextReadBuffer.calcLength() != 0)
        std::sscanf(sTextReadBuffer.cstr(), "%f", &value);
    return value;
}

void TextStreamFormat::readString(StreamSrc* src, BufferedSafeString* str, u32)
{
    ScopedLock<Mutex> lock(&sTextMutex);
    getNextData_(src);
    str->copy(sTextReadBuffer);
}

// NON_MATCHING: loop induction and register allocation differ.
void TextStreamFormat::readBit(StreamSrc* src, void* buffer, u32 bits)
{
    ScopedLock<Mutex> lock(&sTextMutex);
    getNextData_(src);
    SafeString text = sTextReadBuffer;
    if (text.comparen("0b", 2) == 0)
        text = text.getPart(2);
    // The original loop includes the terminating character in its available count.
    const u32 length = text.calcLength() + 1;
    u8 value = 0;
    u32 count = 0;
    u8* bytes = static_cast<u8*>(buffer);
    for (u32 i = 0; i < length && count < bits; ++i)
    {
        value = (value << 1) | (text.at(i) == '1');
        ++count;
        if ((count & 7) == 0)
        {
            bytes[(count >> 3) - 1] = value;
            value = 0;
        }
    }
    if (count & 7)
    {
        const u32 index = count >> 3;
        bytes[index] = (bytes[index] & (0xff << (count & 7))) | value;
    }
}

u32 TextStreamFormat::readMemBlock(StreamSrc* src, void* buffer, u32 size)
{
    ScopedLock<Mutex> lock(&sTextMutex);
    getNextData_(src);
    const u32 length = sTextReadBuffer.calcLength();
    size_t decoded_size = 0;
    Base64::decode(buffer, size, sTextReadBuffer.cstr(), length, &decoded_size);
    return decoded_size;
}

// NON_MATCHING: the terminating Base64 buffer index width and saved registers differ.
void TextStreamFormat::writeMemBlock(StreamSrc* src, const void* buffer, u32 size)
{
    ScopedLock<Mutex> lock(&sTextMutex);
    sTextReadBuffer.clear();
    const u32 encoded_size = (size / 3 + (size % 3 != 0)) * 4;
    if (encoded_size + 1 < u32(sTextReadBuffer.getBufferSize()))
    {
        char* text = sTextReadBuffer.getBuffer();
        text[encoded_size] = '\0';
        Base64::encode(text, buffer, size, false);
        const u32 length = sTextReadBuffer.calcLength();
        src->write("\"", 1);
        src->write(sTextReadBuffer.cstr(), length);
        src->write("\"", 1);
        src->write(mDelimiters.cstr(), 1);
    }
}

void TextStreamFormat::writeBit(StreamSrc* src, const void* buffer, u32 bits)
{
    ScopedLock<Mutex> lock(&sTextMutex);
    sTextReadBuffer.copy("0b");
    const u8* bytes = static_cast<const u8*>(buffer);
    const u32 byte_count = (bits + 7) / 8;
    for (u32 i = 0; i < byte_count; ++i)
    {
        const s32 byte_bits = Mathu::min(bits - i * 8, 8u);
        for (s32 bit = byte_bits - 1; bit >= 0; --bit)
        {
            if (bytes[i] & (1 << bit))
                sTextReadBuffer.append('1');
            else
                sTextReadBuffer.append('0');
        }
    }
    src->write(sTextReadBuffer.cstr(), bits + 2);
    src->write(mDelimiters.cstr(), 1);
}

void TextStreamFormat::writeDecorationText(StreamSrc* src, const SafeString& text)
{
    const s32 length = text.calcLength();
    src->write(text.cstr(), length);
}

void TextStreamFormat::writeU8(StreamSrc* src, Endian::Types, u8 value)
{
    FixedSafeString<32> text;
    text.format("%u", value);
    const s32 length = text.calcLength();
    src->write(text.cstr(), length);
    src->write(mDelimiters.cstr(), 1);
}

void TextStreamFormat::writeU16(StreamSrc* src, Endian::Types, u16 value)
{
    FixedSafeString<32> text;
    text.format("%u", value);
    const s32 length = text.calcLength();
    src->write(text.cstr(), length);
    src->write(mDelimiters.cstr(), 1);
}

void TextStreamFormat::writeU32(StreamSrc* src, Endian::Types, u32 value)
{
    FixedSafeString<32> text;
    text.format("%u", value);
    const s32 length = text.calcLength();
    src->write(text.cstr(), length);
    src->write(mDelimiters.cstr(), 1);
}

void TextStreamFormat::writeU64(StreamSrc* src, Endian::Types, u64 value)
{
    FixedSafeString<32> text;
    text.format("%llu", static_cast<unsigned long long>(value));
    const s32 length = text.calcLength();
    src->write(text.cstr(), length);
    src->write(mDelimiters.cstr(), 1);
}

void TextStreamFormat::writeS8(StreamSrc* src, Endian::Types, s8 value)
{
    FixedSafeString<32> text;
    text.format("%d", value);
    const s32 length = text.calcLength();
    src->write(text.cstr(), length);
    src->write(mDelimiters.cstr(), 1);
}

void TextStreamFormat::writeS16(StreamSrc* src, Endian::Types, s16 value)
{
    FixedSafeString<32> text;
    text.format("%d", value);
    const s32 length = text.calcLength();
    src->write(text.cstr(), length);
    src->write(mDelimiters.cstr(), 1);
}

void TextStreamFormat::writeS32(StreamSrc* src, Endian::Types, s32 value)
{
    FixedSafeString<32> text;
    text.format("%d", value);
    const s32 length = text.calcLength();
    src->write(text.cstr(), length);
    src->write(mDelimiters.cstr(), 1);
}

void TextStreamFormat::writeS64(StreamSrc* src, Endian::Types, s64 value)
{
    FixedSafeString<32> text;
    text.format("%lld", static_cast<long long>(value));
    const s32 length = text.calcLength();
    src->write(text.cstr(), length);
    src->write(mDelimiters.cstr(), 1);
}

void TextStreamFormat::writeF32(StreamSrc* src, Endian::Types, f32 value)
{
    FixedSafeString<32> text;
    text.format("%.8f", value);
    const s32 length = text.calcLength();
    src->write(text.cstr(), length);
    src->write(mDelimiters.cstr(), 1);
}

// 0x7100b18744
void TextStreamFormat::writeNullChar(StreamSrc* src)
{
    char null_char = '\0';
    src->write(&null_char, 1);
}

// 0x7100b1877c
void TextStreamFormat::skip(StreamSrc* src, u32)
{
    ScopedLock<Mutex> lock(&sTextMutex);
    getNextData_(src);
}

// 0x7100b187c4
void TextStreamFormat::rewind(StreamSrc* src)
{
    src->rewind();
}

// 0x7100b187d4
void TextStreamFormat::flush(StreamSrc*) {}

}  // namespace sead
