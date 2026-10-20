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

namespace sead::StringUtil
{
namespace
{
// 0x7101eaa570: lower to upper case pairs sorted by the lower case character.
const Char16Pair cToUpperTable[0x56] = {
    {0x0061, 0x0041}, {0x0062, 0x0042}, {0x0063, 0x0043}, {0x0064, 0x0044}, {0x0065, 0x0045},
    {0x0066, 0x0046}, {0x0067, 0x0047}, {0x0068, 0x0048}, {0x0069, 0x0049}, {0x006a, 0x004a},
    {0x006b, 0x004b}, {0x006c, 0x004c}, {0x006d, 0x004d}, {0x006e, 0x004e}, {0x006f, 0x004f},
    {0x0070, 0x0050}, {0x0071, 0x0051}, {0x0072, 0x0052}, {0x0073, 0x0053}, {0x0074, 0x0054},
    {0x0075, 0x0055}, {0x0076, 0x0056}, {0x0077, 0x0057}, {0x0078, 0x0058}, {0x0079, 0x0059},
    {0x007a, 0x005a}, {0x00e0, 0x00c0}, {0x00e1, 0x00c1}, {0x00e2, 0x00c2}, {0x00e3, 0x00c3},
    {0x00e4, 0x00c4}, {0x00e6, 0x00c6}, {0x00e7, 0x00c7}, {0x00e8, 0x00c8}, {0x00e9, 0x00c9},
    {0x00ea, 0x00ca}, {0x00eb, 0x00cb}, {0x00ec, 0x00cc}, {0x00ed, 0x00cd}, {0x00ee, 0x00ce},
    {0x00ef, 0x00cf}, {0x00f1, 0x00d1}, {0x00f2, 0x00d2}, {0x00f3, 0x00d3}, {0x00f4, 0x00d4},
    {0x00f5, 0x00d5}, {0x00f6, 0x00d6}, {0x00f9, 0x00d9}, {0x00fa, 0x00da}, {0x00fb, 0x00db},
    {0x00fc, 0x00dc}, {0x00ff, 0x0178}, {0x0153, 0x0152}, {0x0430, 0x0410}, {0x0431, 0x0411},
    {0x0432, 0x0412}, {0x0433, 0x0413}, {0x0434, 0x0414}, {0x0435, 0x0415}, {0x0436, 0x0416},
    {0x0437, 0x0417}, {0x0438, 0x0418}, {0x0439, 0x0419}, {0x043a, 0x041a}, {0x043b, 0x041b},
    {0x043c, 0x041c}, {0x043d, 0x041d}, {0x043e, 0x041e}, {0x043f, 0x041f}, {0x0440, 0x0420},
    {0x0441, 0x0421}, {0x0442, 0x0422}, {0x0443, 0x0423}, {0x0444, 0x0424}, {0x0445, 0x0425},
    {0x0446, 0x0426}, {0x0447, 0x0427}, {0x0448, 0x0428}, {0x0449, 0x0429}, {0x044a, 0x042a},
    {0x044b, 0x042b}, {0x044c, 0x042c}, {0x044d, 0x042d}, {0x044e, 0x042e}, {0x044f, 0x042f},
    {0x0451, 0x0401},
};
// 0x7101eaa6c8: upper to lower case pairs sorted by the upper case character.
const Char16Pair cToLowerTable[0x56] = {
    {0x0041, 0x0061}, {0x0042, 0x0062}, {0x0043, 0x0063}, {0x0044, 0x0064}, {0x0045, 0x0065},
    {0x0046, 0x0066}, {0x0047, 0x0067}, {0x0048, 0x0068}, {0x0049, 0x0069}, {0x004a, 0x006a},
    {0x004b, 0x006b}, {0x004c, 0x006c}, {0x004d, 0x006d}, {0x004e, 0x006e}, {0x004f, 0x006f},
    {0x0050, 0x0070}, {0x0051, 0x0071}, {0x0052, 0x0072}, {0x0053, 0x0073}, {0x0054, 0x0074},
    {0x0055, 0x0075}, {0x0056, 0x0076}, {0x0057, 0x0077}, {0x0058, 0x0078}, {0x0059, 0x0079},
    {0x005a, 0x007a}, {0x00c0, 0x00e0}, {0x00c1, 0x00e1}, {0x00c2, 0x00e2}, {0x00c3, 0x00e3},
    {0x00c4, 0x00e4}, {0x00c6, 0x00e6}, {0x00c7, 0x00e7}, {0x00c8, 0x00e8}, {0x00c9, 0x00e9},
    {0x00ca, 0x00ea}, {0x00cb, 0x00eb}, {0x00cc, 0x00ec}, {0x00cd, 0x00ed}, {0x00ce, 0x00ee},
    {0x00cf, 0x00ef}, {0x00d1, 0x00f1}, {0x00d2, 0x00f2}, {0x00d3, 0x00f3}, {0x00d4, 0x00f4},
    {0x00d5, 0x00f5}, {0x00d6, 0x00f6}, {0x00d9, 0x00f9}, {0x00da, 0x00fa}, {0x00db, 0x00fb},
    {0x00dc, 0x00fc}, {0x0152, 0x0153}, {0x0178, 0x00ff}, {0x0401, 0x0451}, {0x0410, 0x0430},
    {0x0411, 0x0431}, {0x0412, 0x0432}, {0x0413, 0x0433}, {0x0414, 0x0434}, {0x0415, 0x0435},
    {0x0416, 0x0436}, {0x0417, 0x0437}, {0x0418, 0x0438}, {0x0419, 0x0439}, {0x041a, 0x043a},
    {0x041b, 0x043b}, {0x041c, 0x043c}, {0x041d, 0x043d}, {0x041e, 0x043e}, {0x041f, 0x043f},
    {0x0420, 0x0440}, {0x0421, 0x0441}, {0x0422, 0x0442}, {0x0423, 0x0443}, {0x0424, 0x0444},
    {0x0425, 0x0445}, {0x0426, 0x0446}, {0x0427, 0x0447}, {0x0428, 0x0448}, {0x0429, 0x0449},
    {0x042a, 0x044a}, {0x042b, 0x044b}, {0x042c, 0x044c}, {0x042d, 0x044d}, {0x042e, 0x044e},
    {0x042f, 0x044f},
};
}  // namespace

// 0x7100b12180
char16 toUpperCapital(char16 c)
{
    return replace(c, Buffer<const Char16Pair>(cToUpperTable));
}

// 0x7100b121f8
void toUpperCapitalFirstCharactor(WBufferedSafeString* str)
{
    char16* buffer = str->getBuffer();
    if (buffer[0] != 0)
        buffer[0] = toUpperCapital(buffer[0]);
}

// 0x7100b122a0
char16 toLowerCapital(char16 c)
{
    return replace(c, Buffer<const Char16Pair>(cToLowerTable));
}

// 0x7100b12318
void toLowerCapitalFirstCharactor(WBufferedSafeString* str)
{
    char16* buffer = str->getBuffer();
    if (buffer[0] != 0)
        buffer[0] = toLowerCapital(buffer[0]);
}
}  // namespace sead::StringUtil
