#include <gfx/seadTextWriter.h>
#include <devenv/seadFontMgr.h>
#include <gfx/seadViewport.h>
#include <math/seadMathCalcCommon.h>
#include <prim/seadStringUtil.h>

namespace sead
{
// 0x7100b1f7d0
void TextWriter::setCursorFromTopLeft(const Vector2f& pos)
{
    mCursor.x = pos.x - mViewport->getSizeX() * 0.5f;
    mCursor.y = -pos.y + mViewport->getSizeY() * 0.5f;
}

// 0x7100b1f814
void TextWriter::beginDraw()
{
    mFont->begin(mDrawContext);
    mEndedDrawing = false;
}

// 0x7100b1f848
void TextWriter::endDraw()
{
    mEndedDrawing = true;
    mFont->end(mDrawContext);
}

// 0x7100b1ff24
void TextWriter::printImpl_(const char* str, s32 length, bool flush, BoundBox2f* rect)
{
    char16_t default_buffer[0x200];
    char16_t* buffer = mFormatBuffer;
    s32 buffer_size;
    if (buffer)
    {
        buffer_size = mFormatBufferSize;
    }
    else
    {
        buffer = default_buffer;
        buffer_size = 0x200;
    }

    const s32 size = length < 0 ? buffer_size : Mathi::min(buffer_size, length + 1);
    StringUtil::convertUtf8ToUtf16(buffer, size, str, size - 1);
    printImpl_(buffer, -1, flush, rect);
}

// 0x7100b1f868 (vprintfImpl_ is inlined)
void TextWriter::printf(const char* format, ...)
{
    std::va_list args;
    va_start(args, format);
    vprintfImpl_(format, args, true, nullptr);
    va_end(args);
}

// The UTF-8 text is formatted into the upper half of the (UTF-16) format buffer and converted in place.
void TextWriter::vprintfImpl_(const char* format, std::va_list args, bool flush, BoundBox2f* rect)
{
    char16_t default_buffer[0x200];
    char16_t* buffer = mFormatBuffer;
    s32 buffer_size;
    if (buffer)
    {
        buffer_size = mFormatBufferSize;
    }
    else
    {
        buffer = default_buffer;
        buffer_size = 0x200;
    }

    BufferedSafeString utf8(reinterpret_cast<char*>(buffer) + buffer_size, buffer_size);
    utf8.formatV(format, args);
    StringUtil::convertUtf8ToUtf16(buffer, buffer_size, utf8.cstr(), buffer_size - 1);
    printImpl_(buffer, -1, flush, rect);
}

}  // namespace sead
