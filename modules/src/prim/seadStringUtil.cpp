#include <prim/seadStringUtil.h>

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
