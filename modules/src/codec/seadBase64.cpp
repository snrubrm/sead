#include "codec/seadBase64.h"

namespace sead
{
// NON_MATCHING: the complete-triplet loop uses different pointer registers and tail setup.
void Base64::encode(char* dst, const void* src, size_t length, bool url_safe)
{
    const char* alphabet = url_safe ? "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_" :
                                      "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    const u8* data = static_cast<const u8*>(src);
    size_t complete_length = length / 3 * 3;
    for (size_t i = 0; i < complete_length; i += 3) {
        dst[0] = alphabet[data[i] >> 2];
        dst[1] = alphabet[((data[i] & 3) << 4) | (data[i + 1] >> 4)];
        dst[2] = alphabet[((data[i + 1] & 15) << 2) | (data[i + 2] >> 6)];
        dst[3] = alphabet[data[i + 2] & 63];
        dst += 4;
    }
    data += complete_length;
    switch (length - complete_length) {
    case 1:
        dst[0] = alphabet[data[0] >> 2];
        dst[1] = alphabet[(data[0] & 3) << 4];
        dst[2] = '=';
        dst[3] = '=';
        break;
    case 2:
        dst[0] = alphabet[data[0] >> 2];
        dst[1] = alphabet[((data[0] & 3) << 4) | (data[1] >> 4)];
        dst[2] = alphabet[(data[1] & 15) << 2];
        dst[3] = '=';
        break;
    }
}

// NON_MATCHING: character range branches and quartet pointer updates differ.
bool Base64::decode(void* dst, size_t dst_size, const char* src, size_t src_size,
                    size_t* decoded_size)
{
    u8* output = static_cast<u8*>(dst);
    size_t written = 0;
    for (size_t read = 0; read < src_size;) {
        // The native reader skips at most two leading control bytes per quartet.
        if (u8(*src) < 0x20) {
            if (++read >= src_size)
                break;
            ++src;
            if (u8(*src) < 0x20) {
                if (++read >= src_size)
                    break;
                ++src;
            }
        }
        read += 4;
        if (read > src_size)
            return false;
        u32 values[4];
        for (s32 i = 0; i < 4; ++i) {
            const u8 ch = u8(src[i]);
            if (ch >= 'A' && ch <= 'Z')
                values[i] = ch - 'A';
            else if (ch >= 'a' && ch <= 'z')
                values[i] = ch - 'a' + 26;
            else if (ch >= '0' && ch <= '9')
                values[i] = ch - '0' + 52;
            else if (ch == '+' || ch == '-')
                values[i] = 62;
            else if (ch == '/' || ch == '_')
                values[i] = 63;
            else
                values[i] = 64;
        }
        if (written >= dst_size)
            return false;
        output[written++] = (values[0] << 2) | ((values[1] & 63) >> 4);
        if (values[2] < 64) {
            if (written >= dst_size)
                return false;
            output[written++] = ((values[1] & 63) << 4) | (values[2] >> 2);
            if (values[3] < 64) {
                if (written >= dst_size)
                    return false;
                output[written++] = (values[2] << 6) | values[3];
            }
        }
        src += 4;
    }
    if (decoded_size)
        *decoded_size = written;
    return true;
}
}  // namespace sead
