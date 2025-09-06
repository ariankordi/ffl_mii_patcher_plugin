/**
 * @file base64enc.h
 * @brief Simple Base64 encoder utility.
 *
 * When calling base64_encode, you must allocate the
 * output buffer yourself. The size can be calculated
 * using BASE64_ENCODED_SIZE(strlen("string")).
 */

#include <stddef.h>

/**
 * @brief Calculates the maximum Base64 encoded length for a given number of bytes.
 * @details Base64 expands every 3 bytes into 4 characters, plus padding.
 * Add 1 extra for null terminator.
 * @param n Number of input bytes.
 * @return Maximum required output size in bytes.
 */
#define BASE64_ENCODED_SIZE(n) ((((n) + 2) / 3) * 4 + 1)

/**
 * @brief Encodes a binary buffer into Base64 text.
 * @param input Pointer to raw input bytes.
 * @param len Number of bytes in input.
 * @param output Pointer to destination buffer (must be at least BASE64_ENCODED_SIZE(len)).
 */
static void base64_encode(const unsigned char* input, size_t len, char* output) {
    static const char cBase64Alphabet[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";

    size_t outIndex = 0;
    size_t i = 0;

    while (i + 2 < len) {
        // Take 3 bytes and split into 4 groups of 6 bits.
        int triple = (input[i] << 16) | (input[i + 1] << 8) | input[i + 2];
        output[outIndex++] = cBase64Alphabet[(triple >> 18) & 0x3F];
        output[outIndex++] = cBase64Alphabet[(triple >> 12) & 0x3F];
        output[outIndex++] = cBase64Alphabet[(triple >> 6)  & 0x3F];
        output[outIndex++] = cBase64Alphabet[triple & 0x3F];
        i += 3;
    }

    // Handle remaining 1 or 2 bytes with padding.
    if (i < len) {
        int triple = input[i] << 16;
        if (i + 1 < len) {
            triple |= input[i + 1] << 8;
        }

        output[outIndex++] = cBase64Alphabet[(triple >> 18) & 0x3F];
        output[outIndex++] = cBase64Alphabet[(triple >> 12) & 0x3F];

        if (i + 1 < len) {
            output[outIndex++] = cBase64Alphabet[(triple >> 6) & 0x3F];
            output[outIndex++] = '=';
        } else {
            output[outIndex++] = '=';
            output[outIndex++] = '=';
        }
    }

    // Null terminate.
    output[outIndex] = '\0';
}
