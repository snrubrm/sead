#include <prim/seadStringUtil.h>
#include <limits.h>

namespace sead::StringUtil
{
// Shared parser implementations; the public tryParse functions are branch thunks.
bool sub_7100B0E4CC(u32* out, const SafeString& str, CardinalNumber base);
bool sub_7100B0E9D0(s32* out, const SafeString& str, CardinalNumber base);
bool sub_7100B10424(f32* out, const SafeString& str);

bool tryParseU32(u32* out, const SafeString& str, CardinalNumber base)
{
    return sub_7100B0E4CC(out, str, base);
}

u32 parseU32(const SafeString& str, CardinalNumber base)
{
    u32 value = 0;
    tryParseU32(&value, str, base);
    return value;
}

bool tryParseS32(s32* out, const SafeString& str, CardinalNumber base)
{
    return sub_7100B0E9D0(out, str, base);
}

s32 parseS32(const SafeString& str, CardinalNumber base)
{
    s32 value = 0;
    tryParseS32(&value, str, base);
    return value;
}

bool tryParseF32(f32* out, const SafeString& str)
{
    return sub_7100B10424(out, str);
}

f32 parseF32(const SafeString& str)
{
    f32 value = 0;
    tryParseF32(&value, str);
    return value;
}

bool sub_7100B0DA68(u64* out, const SafeString& str, CardinalNumber base);

bool tryParseU64(u64* out, const SafeString& str, CardinalNumber base)
{
    return sub_7100B0DA68(out, str, base);
}

bool sub_7100B0DF84(s64* out, const SafeString& str, CardinalNumber base);

bool tryParseS64(s64* out, const SafeString& str, CardinalNumber base)
{
    return sub_7100B0DF84(out, str, base);
}

bool sub_7100B0EEFC(u16* out, const SafeString& str, CardinalNumber base);

bool tryParseU16(u16* out, const SafeString& str, CardinalNumber base)
{
    return sub_7100B0EEFC(out, str, base);
}

bool sub_7100B0F434(s16* out, const SafeString& str, CardinalNumber base);

bool tryParseS16(s16* out, const SafeString& str, CardinalNumber base)
{
    return sub_7100B0F434(out, str, base);
}

bool sub_7100B0F998(u8* out, const SafeString& str, CardinalNumber base);

bool tryParseU8(u8* out, const SafeString& str, CardinalNumber base)
{
    return sub_7100B0F998(out, str, base);
}

bool sub_7100B0FEC8(s8* out, const SafeString& str, CardinalNumber base);

bool tryParseS8(s8* out, const SafeString& str, CardinalNumber base)
{
    return sub_7100B0FEC8(out, str, base);
}

char16 replace(char16 c, const Buffer<const Char16Pair>& sorted_table)
{
    if (sorted_table.size() == 0)
        return c;

    const s32 idx =
        sorted_table.binarySearch(Char16Pair{c, 0}, [](const Char16Pair* p1, const Char16Pair* p2) {
            return p1->before - p2->before;
        });
    if (idx < 0)
        return c;

    return sorted_table[idx].after;
}
}  // namespace sead::StringUtil

namespace sead::StringUtil
{
s32 vsnprintf(char* s, size_t n, const char* format, va_list args) {
    if (!n)
        return -1;
    const s32 result = ::vsnprintf(s, n, format, args);
    if (result < 0 || size_t(result) >= n)
        s[n - 1] = SafeString::cNullChar;
    return size_t(result) < n ? result : s32(n - 1);
}
// NON_MATCHING: output-count registers and capacity-exhaustion scheduling differ.
s32 convertUtf8ToUtf16(char16* dst, u32 dst_len, const char* src, s32 src_len) {
    size_t written = 0;
    if (!dst_len || src_len < -1)
        return written;
    if (src_len == -1)
        src_len = INT_MAX;
    s32 read = 0;
    while (src_len > read) {
        const char* current = src + read;
        u32 codepoint = u8(*current);
        if (!codepoint)
            break;
        s32 bytes = 1;
        if (codepoint & 0x80) {
            if ((codepoint & 0xe0) == 0xc0) {
                if (!current[1])
                    break;
                codepoint = ((codepoint & 0x1f) << 6) | (u8(current[1]) & 0x3f);
                bytes = 2;
            } else if ((codepoint & 0xf0) == 0xe0) {
                if (!current[1] || !current[2])
                    break;
                codepoint = (codepoint << 12) | ((u8(current[1]) & 0x3f) << 6) |
                            (u8(current[2]) & 0x3f);
                bytes = 3;
            } else {
                break;
            }
        }
        dst[written++] = codepoint;
        read += bytes;
        if (written >= dst_len) {
            --written;
            break;
        }
    }
    dst[written] = 0;
    return written;
}

// NON_MATCHING: capacity-exhaustion and output-pointer scheduling differ.
s32 convertUtf16ToUtf8(char* dst, u32 dst_len, const char16* src, s32 src_len) {
    if (dst_len == 0 || src_len < -1)
        return 0;
    if (src_len == -1)
        src_len = INT_MAX;
    u32 written = 0;
    s32 read = 0;
    while (read < src_len && src[read] != 0) {
        char* current = dst + written;
        const u16 c = src[read];
        u32 width;
        if (c <= 0x7f) {
            current[0] = c;
            width = 1;
        } else if (c <= 0x7ff) {
            if (written + 1 >= dst_len) {
                --written;
                break;
            }
            current[0] = 0xc0 | (c >> 6);
            current[1] = 0x80 | (c & 0x3f);
            width = 2;
        } else {
            if (written + 2 >= dst_len) {
                --written;
                break;
            }
            current[0] = 0xe0 | (c >> 12);
            current[1] = 0x80 | ((c >> 6) & 0x3f);
            current[2] = 0x80 | (c & 0x3f);
            width = 3;
        }
        written += width;
        ++read;
        if (written >= dst_len) {
            --written;
            break;
        }
    }
    dst[written] = 0;
    return written;
}
}
