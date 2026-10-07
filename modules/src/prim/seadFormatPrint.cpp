#include <prim/seadFormatPrint.h>
#include <prim/seadStringUtil.h>
#include <string.h>

namespace sead
{
// NON_MATCHING: the original builds the sub-buffer at mPos (the BufferedSafeString local) in a different order of stores
// 0x7100b0bac4
void StringPrintOutput::write(const char* string, s32 size)
{
    // The part of the buffer that follows the characters written so far.
    char* top = mBuffer->getBuffer();
    BufferedSafeString part = mPos >= 0 && mBuffer->getBufferSize() > mPos ?
                                  BufferedSafeString(top + mPos, mBuffer->getBufferSize() - mPos) :
                                  BufferedSafeString(nullptr, 0);
    mPos += part.cutOffCopy(string, size);
}

// NON_MATCHING: the original builds the sub-buffer at mPos (the BufferedSafeString local) in a different order of stores
// 0x7100b0bc2c
void StringCutOffPrintOutput::write(const char* string, s32 size)
{
    char* top = mBuffer->getBuffer();
    BufferedSafeString part = mPos >= 0 && mBuffer->getBufferSize() > mPos ?
                                  BufferedSafeString(top + mPos, mBuffer->getBufferSize() - mPos) :
                                  BufferedSafeString(nullptr, 0);
    mPos += part.cutOffCopy(string, size);
}

// 0x7100b0c528
StringCutOffPrintOutput::~StringCutOffPrintOutput() {}

// 0x7100b0bd94
void PrintFormatter::flush()
{
    if (!mFormatStr)
        return;

    mX = false;
    while (mPos < mFormatStrLength)
    {
        char format[32];
        proceedToFormatMark_(format);
    }
}

// NON_MATCHING: same control flow; the registers of format / str / length are allocated differently
// 0x7100b0bde0
bool PrintFormatter::proceedToFormatMark_(char* format)
{
    format[0] = '\0';
    if (!mFormatStr)
        return false;
    if (mX)
        return true;
    if (mPos >= mFormatStrLength)
        return false;

    const char* str = &mFormatStr[mPos];
    while (true)
    {
        s32 length = 0;
        while (str[length] != '\0' && str[length] != '%')
            ++length;

        if (str[length] == '\0')
        {
            if (length >= 1)
            {
                mPrintOutput->write(str, length);
                mPos += length;
                return false;
            }
            return false;
        }

        if (str[length + 1] != '%')
        {
            if (length > 0)
            {
                mPrintOutput->write(str, length);
                mPos += length;
                str += length;
                length = 0;
            }

            const char mark = str[1];
            s32 consumed = length + 2;
            if (mark == '<')
            {
                mX = true;
            }
            else if (mark == '@')
            {
            }
            else
            {
                format[0] = '%';
                s32 index = length + 1;
                while (true)
                {
                    const char c = str[index];
                    bool is_format_char;
                    switch (c)
                    {
                    case ' ':
                    case '#':
                    case '+':
                    case '-':
                    case '.':
                    case 'L':
                    case 'h':
                    case 'l':
                        is_format_char = true;
                        break;
                    default:
                        is_format_char = c == 'z' || (c >= '0' && c <= '9');
                        break;
                    }

                    if (is_format_char)
                    {
                        if (index >= 0x1f)
                        {
                            format[0] = '\0';
                            mPos = mFormatStrLength;
                            break;
                        }
                        format[index] = c;
                        ++index;
                        ++consumed;
                        continue;
                    }

                    if (c == '\0')
                    {
                        format[0] = '\0';
                        mPos = mFormatStrLength;
                        break;
                    }
                    format[index] = c;
                    format[consumed] = '\0';
                    break;
                }
            }
            mPos += consumed;
            return true;
        }

        // "%%": the text up to and including the first '%'
        mPrintOutput->write(str, length + 1);
        str += length + 2;
        mPos += length + 2;
    }
}

// 0x7100b0bfd8
PrintFormatter& PrintFormatter::operator<<(const char* string)
{
    if (mFormatStr)
    {
        char format[32];
        if (proceedToFormatMark_(format))
            outputString_(format[0] == '\0' ? nullptr : format, mPrintOutput, string, -1);
    }
    else
    {
        mFormatStr = string;
        mFormatStrLength = __builtin_strlen(string);
    }
    return *this;
}

// 0x7100b0c058
void PrintFormatter::outputString_(const char* format, PrintOutput* output, const char* string,
                                   s32 length)
{
    if (!format || format[1] == 's')
    {
        output->write(string, length == -1 ? s32(__builtin_strlen(string)) : length);
        return;
    }

    if (format[__builtin_strlen(format) - 1] != 's')
    {
        FixedSafeString<128> buffer;
        const s32 written = buffer.format(format, string);
        output->write(buffer.cstr(), written);
        return;
    }

    // A width ("%-10s" or "%10s"): pad with spaces by the display width of the string in Shift-JIS.
    FixedSafeString<128> sjis;
    const s32 sjis_length = StringUtil::convertUtf8ToSjis(sjis.getBuffer(), sjis.getBufferSize(), string, -1);
    if (format[1] == '-')
    {
        output->write(string, length == -1 ? __builtin_strlen(string) : length);
        s32 padding = StringUtil::parseS32(SafeString(format + 2), StringUtil::CardinalNumber::Base10) - sjis_length;
        for (s32 i = 0; i < padding; ++i)
            output->write(" ", 1);
    }
    else
    {
        s32 padding = StringUtil::parseS32(SafeString(format + 1), StringUtil::CardinalNumber::Base10) - sjis_length;
        for (s32 i = 0; i < padding; ++i)
            output->write(" ", 1);
        output->write(string, length == -1 ? __builtin_strlen(string) : length);
    }
}

// 0x7100b0c320
StringPrintFormatter::StringPrintFormatter(BufferedSafeString* string)
    : PrintFormatter(nullptr, &mOutput), mOutput(string)
{
}

// 0x7100b0c35c
StringCutOffPrintFormatter::StringCutOffPrintFormatter(BufferedSafeString* string)
    : PrintFormatter(nullptr, &mOutput), mOutput(string)
{
}

// 0x7100b0c398
template <>
void PrintFormatter::out<u32>(const u32& value, const char* format, PrintOutput* output)
{
    FixedSafeString<32> buffer;
    s32 length;
    if (format)
        length = buffer.format(format, value);
    else
        length = buffer.format("%d", value);
    output->write(buffer.cstr(), length);
}

// 0x7100b0c46c
template <>
void PrintFormatter::OutImpl<char, SafeStringBase>::out(const SafeStringBase<char>& string,
                                                        const char* format, PrintOutput* output)
{
    outputString_(format, output, string.cstr(), string.calcLength());
}

}  // namespace sead
