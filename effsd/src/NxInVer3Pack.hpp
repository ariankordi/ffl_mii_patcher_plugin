#pragma once
/**
 * @file NxInVer3Pack.hpp
 * @brief Core packing/unpacking logic for embedding Switch/"Ver4" common color indices
 *        inside available/unused fields of the 3DS/Wii U Mii data structure as a
 *        contiguous extra-data block. Endian-independent for the internal block.
 *
 * Design goals:
 * - Do not allocate heap memory. Use only compile-time arrays (std::array).
 * - Treat the "ExtraDataBlock" as a raw byte buffer (no std::array exposure).
 * - Create mapping and reverse-mapping tables at compile-time (constexpr).
 * - Place roomIndex/positionInRoom bits at the end of the block and force them non-zero.
 * - Leave spare bits in the block for future checksum/user data growth.
 *
 * Notes:
 * - The Ver3 bitfield struct is inherently compiler/endianness-sensitive in layout.
 *   We only read/write its fields as numbers; the internal ExtraDataBlock remains portable.
 * - We intentionally stop before creatorName (FFLiMiiDataCore).
 */

#include <array>
#include <cstdint>

using u8  = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;

using u64 = std::uint64_t;

// // ---------------------------------------------------------------
// //  3DS/Wii U Data (Ver3StoreData) Structure
// // ---------------------------------------------------------------

// References for the 3DS/Wii U Mii data format,
// aka "Ver3StoreData", "FFLStoreData", "CFLiMiiDataPacket":
// MiiPort: https://github.com/Genwald/MiiPort/blob/4ee38bbb8aa68a2365e9c48d59d7709f760f9b5d/include/mii_ext.h#L170
// Decaf: https://github.com/decaf-emu/decaf-emu/blob/e6c528a20a41c34e0f9eb91dd3da40f119db2dee/src/libdecaf/src/nn/ffl/nn_ffl_miidata.h#L159
// FFL decomp: https://github.com/aboood40091/ffl/blob/73fe9fc70c0f96ebea373122e50f6d3acc443180/include/nn/ffl/FFLiMiiDataCore.h#L886


struct Ver3AuthorId {
    union {
        u8  data[8];
        u16 value16[4];
    };
};

struct Ver3CreateId {
    union {
        u8  data[10];
        u16 value16[5];
    };
};

#define MII_NAME_LENGTH 10
#define MII_CREATORNAME_LENGTH 10

/// Packed structure representing 3DS/Wii U Mii data.
struct Ver3MiiDataCore {
    // 00/0x00 - Lower personal fields.
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    u32 miiVersion     : 8;  // (LSB)
    u32 copyable       : 1;
    u32 ngWord         : 1;
    u32 regionMove     : 2;
    u32 fontRegion     : 2;
    u32 reserved_0     : 2;
    u32 roomIndex      : 4;  ///< Maximum = 9
    u32 positionInRoom : 4;  ///< Maximum = 9
    u32 authorType     : 4;  ///< Logically unused.
    u32 birthPlatform  : 3;
    u32 reserved_1     : 1;  // (MSB)
#else
    u32 reserved_1     : 1;  // (MSB)
    u32 birthPlatform  : 3;
    u32 authorType     : 4;
    u32 positionInRoom : 4;
    u32 roomIndex      : 4;
    u32 reserved_0     : 2;
    u32 fontRegion     : 2;
    u32 regionMove     : 2;
    u32 ngWord         : 1;
    u32 copyable       : 1;
    u32 miiVersion     : 8;  // (LSB)
#endif

    // 04/0x04 - 8 byte value containing the transferable ID.
    Ver3AuthorId authorId;

    // 12/0x0C - 10 byte value identifying the character.
    Ver3CreateId createId;
    u8 reserved_2[2];

    // 24/0x18 - Higher personal fields.
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    u16 gender        : 1;
    u16 birthMonth    : 4;
    u16 birthDay      : 5;
    u16 favoriteColor : 4;
    u16 favorite      : 1;
    u16 padding_0     : 1;
#else
    u16 padding_0     : 1;
    u16 favorite      : 1;
    u16 favoriteColor : 4;
    u16 birthDay      : 5;
    u16 birthMonth    : 4;
    u16 gender        : 1;
#endif

    // 26/0x1A - 10-character name in UTF-16.
    u16 name[MII_NAME_LENGTH];

    // 46/0x2E - Body parameters.
    u8 height;
    u8 build;

    // 48/0x30 - Faceline fields + localonly.
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    u16 localonly     : 1;
    u16 faceType      : 4;
    u16 faceColor     : 3;
    u16 faceTex       : 4;
    u16 faceMake      : 4;
#else
    u16 faceMake      : 4;
    u16 faceTex       : 4;
    u16 faceColor     : 3;
    u16 faceType      : 4;
    u16 localonly     : 1;
#endif

    // 50/0x32 - Hair fields.
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    u16 hairType      : 8;
    u16 hairColor     : 3;
    u16 hairFlip      : 1;
    u16 padding_1     : 4;
#else
    u16 padding_1     : 4;
    u16 hairFlip      : 1;
    u16 hairColor     : 3;
    u16 hairType      : 8;
#endif

    // 52/0x34 - Eye fields part 1.
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    u16 eyeType       : 6;
    u16 eyeColor      : 3;
    u16 eyeScale      : 4;
    u16 eyeAspect     : 3;
#else
    u16 eyeAspect     : 3;
    u16 eyeScale      : 4;
    u16 eyeColor      : 3;
    u16 eyeType       : 6;
#endif

    // 54/0x36 - Eye fields part 2.
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    u16 eyeRotate     : 5;
    u16 eyeX          : 4;
    u16 eyeY          : 5;
    u16 padding_2     : 2;
#else
    u16 padding_2     : 2;
    u16 eyeY          : 5;
    u16 eyeX          : 4;
    u16 eyeRotate     : 5;
#endif

    // 56/0x38 - Eyebrow fields part 1.
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    u16 eyebrowType   : 5;
    u16 eyebrowColor  : 3;
    u16 eyebrowScale  : 4;
    u16 eyebrowAspect : 3;
    u16 padding_3     : 1;
#else
    u16 padding_3     : 1;
    u16 eyebrowAspect : 3;
    u16 eyebrowScale  : 4;
    u16 eyebrowColor  : 3;
    u16 eyebrowType   : 5;
#endif

    // 58/0x3A - Eyebrow fields part 2.
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    u16 eyebrowRotate : 5;
    u16 eyebrowX      : 4;
    u16 eyebrowY      : 5;
    u16 padding_4     : 2;
#else
    u16 padding_4     : 2;
    u16 eyebrowY      : 5;
    u16 eyebrowX      : 4;
    u16 eyebrowRotate : 5;
#endif

    // 60/0x3C - Nose fields.
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    u16 noseType      : 5;
    u16 noseScale     : 4;
    u16 noseY         : 5;
    u16 padding_5     : 2;
#else
    u16 padding_5     : 2;
    u16 noseY         : 5;
    u16 noseScale     : 4;
    u16 noseType      : 5;
#endif

    // 62/0x3E - Mouth fields part 1.
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    u16 mouthType     : 6;
    u16 mouthColor    : 3;
    u16 mouthScale    : 4;
    u16 mouthAspect   : 3;
#else
    u16 mouthAspect   : 3;
    u16 mouthScale    : 4;
    u16 mouthColor    : 3;
    u16 mouthType     : 6;
#endif

    // 64/0x40 - Mouth fields part 2 + mustache type.
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    u16 mouthY        : 5;
    u16 mustacheType  : 3;
    u16 padding_6     : 8;
#else
    u16 padding_6     : 8;
    u16 mustacheType  : 3;
    u16 mouthY        : 5;
#endif

    // 66/0x42 - Beard fields.
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    u16 beardType     : 3;
    u16 beardColor    : 3;
    u16 beardScale    : 4;
    u16 beardY        : 5;
    u16 padding_7     : 1;
#else
    u16 padding_7     : 1;
    u16 beardY        : 5;
    u16 beardScale    : 4;
    u16 beardColor    : 3;
    u16 beardType     : 3;
#endif

    // 68/0x44 - Glass fields.
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    u16 glassType     : 4;
    u16 glassColor    : 3;
    u16 glassScale    : 4;
    u16 glassY        : 5;
#else
    u16 glassY        : 5;
    u16 glassScale    : 4;
    u16 glassColor    : 3;
    u16 glassType     : 4;
#endif

    // 70/0x46 - Mole fields.
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    u16 moleType      : 1;
    u16 moleScale     : 4;
    u16 moleX         : 5;
    u16 moleY         : 5;
    u16 padding_8     : 1;
#else
    u16 padding_8     : 1;
    u16 moleY         : 5;
    u16 moleX         : 5;
    u16 moleScale     : 4;
    u16 moleType      : 1;
#endif
    // 72/0x48 - End of core.
};

/**
 * @brief Mii data for database (front/table/official DB).
 * @details Contains core data plus creator name.
 */
struct Ver3MiiDataOfficial
{
    /// 00/0x00
    Ver3MiiDataCore      core;
    /// 72/0x48
    u16                  creatorName[MII_CREATORNAME_LENGTH];
    /// 92/0x5C
};

/**
 * @brief Mii StoreData, including a CRC-16 checksum.
 * @details Includes core data, creator name, and CRC-16/CCITT/XMODEM checksum.
 * Also called: FFLStoreData, CFLiMiiDataPacket, nn::mii::Ver3StoreData
 */
struct Ver3StoreData
{
    /// 00/0x00
    Ver3MiiDataOfficial  official;
    /// 92/0x5C
    u16                  pad; ///< Also called padding_9.
    /// 94/0x5D
    u16                  crc;
    /// 96/0x60
};

static_assert(sizeof(Ver3MiiDataCore) == 72);
static_assert(sizeof(Ver3MiiDataOfficial) == 92);
static_assert(sizeof(Ver3StoreData) == 96);


// // ---------------------------------------------------------------
// //  Model for Ver4/NX Fields
// // ---------------------------------------------------------------

// While this is purely an intermediate representation
// not meant to be stored at all, it actually mirrors
// the structure "nn::mii::detail::NfpStoreDataExtentionRaw"
// which is available in Yuzu source: https://github.com/search?q=%22struct+NfpStoreDataExtension%22&type=code
struct NxExtensionFields {
    u8 facelineColor; // 0..9
    u8 hairColor;     // 0..99
    u8 eyeColor;      // 0..99
    u8 eyebrowColor;  // 0..99
    u8 mouthColor;    // 0..99
    u8 beardColor;    // 0..99
    u8 glassColor;    // 0..99
    u8 glassType;     // 0..19
};


// We construct a single contiguous bitstream from the following in order:
//   [reserved_0:2][authorType:4][reserved_1:1][reserved_2:8+8][padding_0:1][padding_1:4]
//   [padding_2:2][padding_3:1][padding_4:2][padding_5:2][padding_6:8][padding_7:1][padding_8:1]
//   [roomIndex_low3:3] [positionInRoom_low3:3]
//
// Total useful bits = 45 + 3 + 3 = 51 bits -> 7 bytes.
// We place group indices at the start of this block; trailing slack is reserved for future use.
static constexpr int EXTRA_BITS_TOTAL  = 51;
static constexpr int EXTRA_BYTES_TOTAL = (EXTRA_BITS_TOTAL + 7) / 8; // 7 bytes.

struct ExtraDataBlock {
    /// Raw byte buffer, LSB-first bit packing for internal fields.
    u8 data[EXTRA_BYTES_TOTAL];
};

// // ---------------------------------------------------------------
// //  Mapping Tables to Ver3
// // ---------------------------------------------------------------
// Define constants from nn::mii for index bounds.
static constexpr int CommonColor_End = 100;
static constexpr int FacelineColor_End = 10;
static constexpr int GlassType_End = 20;

// Tables are originally from MiiPort: https://github.com/Genwald/MiiPort/blob/4ee38bbb8aa68a2365e9c48d59d7709f760f9b5d/include/convert_mii.h#L18
// The values have been reordered, because the maximum "grouped index" values
// for most part types barely exceeded 31, and after modifications, the
// grouped indices can use 5 bits instead of 6. This gives additional space
// for a checksum/signature, or more user data.

// The tables have additionally been adjusted to give hair/eye/glass one
// more free index to represent placeholder for a value representing
// to use the original Ver3 color present in the data.

static constexpr auto ToVer3HairColorTable = std::array<u8, CommonColor_End>({
    /* 0:  */ 0, 1, 2, 3, 4, 5, 6, 7, 0, 4, 3, 5, 4, 5 /* < 13, orig. val: 4 */, 6, 2, 0, 6, 4, 3, 2, 2, 7, 3, 2, 2,
    /* 26: */ 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 4, 4, 4, 4, 4, 4, 4, 0, 0, 4, 4,
    /* 52: */ 4, 4, 4, 4, 0, 0, 0, 5 /* < 59, orig. val: 4 */, 4, 4, 4, 4, 4, 5, 5, 5, 4, 4, 7 /* 70, < orig. val: 4 */, 4, 4, 4, 4, 5, 7, 5,
    /* 78: */ 7, 7, 7, 7, 7, 6, 7, 7, 7, 7, 7, 3, 7, 7, 7, 7, 7, 0, 4, 4, 4, 4,
});

static constexpr auto ToVer3EyeColorTable = std::array<u8, CommonColor_End>({
    /* 0:  */ 0, 2, 2, 2, 1, 3, 2, 3, 0, 1, 2, 3, 4, 5, 2, 2, 4, 2, 1, 2, 2, 2, 2, 2, 2, 2,
    /* 26: */ 2, 1 /* < 27, orig. val: 2 */, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1 /* < 37, orig. val: 2 */, 0, 0, 4, 4, 4, 4, 4, 4, 4, 1, 0, 4, 4, 4,
    /* 52: */ 4, 4, 4, 4, 0, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 3, 3, 3,
    /* 78: */ 3, 3, 3, 3, 3, 2, 2, 3, 3, 3, 3, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1,
});

static constexpr auto ToVer3MouthColorTable = std::array<u8, CommonColor_End>({
    /* 0:  */ 4, 4, 4, 4, 4, 4, 4, 3, 4, 4, 4, 4, 4, 4, 4, 1, 4, 4, 4, 0, 1, 2, 3, 4, 4, 2,
    /* 26: */ 3, 3, 4, 4, 4, 4, 1, 4, 4, 2, 3, 3, 4, 4, 4, 4, 4, 4, 4, 3, 3, 3, 4, 4, 4, 3,
    /* 52: */ 3, 3, 3, 3, 4, 4, 4, 4, 4, 3, 3, 3, 3, 4, 4, 4, 4, 3, 3, 3, 3, 3, 3, 4, 4, 3,
    /* 78: */ 3, 3, 3, 3, 3, 4, 3, 3, 3, 3, 3, 4, 0, 3, 3, 3, 3, 4, 3, 3, 3, 3,
});

static constexpr auto ToVer3GlassColorTable = std::array<u8, CommonColor_End>({
    /* 0:  */ 0, 1, 1, 1, 5, 1, 1, 4, 0, 5, 1, 1, 3, 5, 1, 2, 3, 4, 5, 4, 2, 2, 4, 4, 2, 2,
    /* 26: */ 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
    /* 52: */ 3, 3, 3, 3, 0, 0, 0, 5, 5, 5, 5, 5, 5, 0, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 4 /* < 77, orig. val: 5 */,
    /* 78: */ 5, 5, 5, 5, 5, 1, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5,
});

static constexpr auto ToVer3FacelineColorTable = std::array<u8, FacelineColor_End>({
    0, 1, 2, 3, 4, 5, 0, 1, 5, 5
});

static constexpr auto ToVer3GlassTypeTable = std::array<u8, GlassType_End>({
    0, 1, 2, 3, 4, 5, 6, 7, 8, 1, 2, 1, 3, 7, 7, 6, 7, 8, 7, 7
});

// // ---------------------------------------------------------------
// //  Compile-time utilities
// // ---------------------------------------------------------------
constexpr std::size_t CeilLog2(std::size_t n) {
    // Returns minimum number of bits to represent [0..n-1].
    if (n <= 1) {
        return 0;
    }
    std::size_t bits = 0;
    std::size_t v = 1;
    while (v < n) {
        v <<= 1;
        ++bits;
    }
    return bits;
}

template <std::size_t TableN, std::size_t Ver3MaxPlus1>
struct StaticReverseMap {
    std::array<u16, Ver3MaxPlus1> counts{};  ///< Number of ver4 indices in each ver3 bucket.
    /// Lists of ver4 indices per ver3 value.
    std::array<std::array<u8, TableN>, Ver3MaxPlus1> byGroup{};
    /// For each ver4 index, its position within its ver3 bucket.
    std::array<u8, TableN> positionInGroup{};
    u16 maxGroupSize = 0;                    ///< Maximum bucket size over all ver3 values.
};

template <std::size_t TableN, std::size_t Ver3MaxPlus1>
constexpr StaticReverseMap<TableN, Ver3MaxPlus1>
BuildReverseMap(const std::array<u8, TableN>& toVer3)
{
    StaticReverseMap<TableN, Ver3MaxPlus1> out{};
    for (std::size_t i = 0; i < TableN; ++i) {
        u8 v3 = toVer3[i];
        // Bounds must be respected at compile-time. We assume correctness of input tables.
        std::size_t pos = out.counts[v3];
        out.byGroup[v3][pos] = static_cast<u8>(i);
        out.positionInGroup[i] = static_cast<u8>(pos);
        out.counts[v3] = static_cast<u16>(pos + 1);
    }
    // Compute maximum group size.
    u16 maxSz = 0;
    for (std::size_t v3 = 0; v3 < Ver3MaxPlus1; ++v3) {
        if (out.counts[v3] > maxSz) {
            maxSz = out.counts[v3];
        }
    }
    out.maxGroupSize = maxSz;
    return out;
}

// Define constants for Ver3 bounds.
static constexpr int Ver3HairColor_End = 8;
static constexpr int Ver3EyeColor_End = 6;
static constexpr int Ver3MouthColor_End = 5;
static constexpr int Ver3GlassColor_End = 6;
static constexpr int Ver3FacelineColor_End = 6;
static constexpr int Ver3GlassType_End = 9;

// Concrete reverse maps for our tables.
static constexpr auto RevHairColors  = BuildReverseMap<ToVer3HairColorTable.size(), Ver3HairColor_End>(ToVer3HairColorTable);
static constexpr auto RevEyeColors   = BuildReverseMap<ToVer3EyeColorTable.size(), Ver3EyeColor_End>(ToVer3EyeColorTable);
static constexpr auto RevMouthColors = BuildReverseMap<ToVer3MouthColorTable.size(), Ver3MouthColor_End>(ToVer3MouthColorTable);
static constexpr auto RevGlassColors = BuildReverseMap<ToVer3GlassColorTable.size(), Ver3GlassType_End>(ToVer3GlassColorTable);
static constexpr auto RevFaceColors  = BuildReverseMap<ToVer3FacelineColorTable.size(), Ver3FacelineColor_End>(ToVer3FacelineColorTable);
static constexpr auto RevGlassTypes  = BuildReverseMap<ToVer3GlassTypeTable.size(), Ver3GlassType_End>(ToVer3GlassTypeTable);

// Bit widths required by each group index (compile-time).
static constexpr std::size_t FacelineColorBits = CeilLog2(RevFaceColors.maxGroupSize);
static_assert(FacelineColorBits == 2);
static constexpr std::size_t HairColorBits     = CeilLog2(RevHairColors.maxGroupSize);
static_assert(HairColorBits == 5);
static constexpr std::size_t EyeColorBits      = CeilLog2(RevEyeColors.maxGroupSize);
static_assert(EyeColorBits == 5);
static constexpr std::size_t EyebrowColorBits  = CeilLog2(RevHairColors.maxGroupSize);
static_assert(EyebrowColorBits == 5); // Shares hair table.
static constexpr std::size_t MouthColorBits    = CeilLog2(RevMouthColors.maxGroupSize);
static_assert(MouthColorBits == 6);
static constexpr std::size_t BeardColorBits    = CeilLog2(RevHairColors.maxGroupSize);
static_assert(BeardColorBits == 5); // Shares hair table.
static constexpr std::size_t GlassColorBits    = CeilLog2(RevGlassColors.maxGroupSize);
static_assert(GlassColorBits == 5);
static constexpr std::size_t GlassTypeBits     = CeilLog2(RevGlassTypes.maxGroupSize);
static_assert(GlassTypeBits == 3);

static_assert(EXTRA_BYTES_TOTAL == 7, "ExtraDataBlock byte size must be 7.");
// Sum used for group indices. We keep it small to leave slack in the extra block.
static constexpr std::size_t UsedIndexBits =
    FacelineColorBits + HairColorBits + EyeColorBits + EyebrowColorBits + MouthColorBits + BeardColorBits + GlassColorBits + GlassTypeBits;

// // ---------------------------------------------------------------
// //  Bit Pack/Unpack Helpers
// // ---------------------------------------------------------------

/// Writes `width` LSBs of `value` at bit offset `bitOff` into `dst` (LSB-first).
inline void PutBits(u8* dst, std::size_t bitOff, std::size_t width, std::uint64_t value) {
    // We avoid branches and write byte-wise. This function is endian-independent.
    std::size_t bit = bitOff;
    for (std::size_t i = 0; i < width; ++i, ++bit) {
        std::size_t byteIndex = bit >> 3;
        std::size_t bitIndex  = bit & 7;
        u8 mask = static_cast<u8>(1u << bitIndex);
        if (value & (1ull << i)) {
            dst[byteIndex] = static_cast<u8>(dst[byteIndex] | mask);
        } else {
            dst[byteIndex] = static_cast<u8>(dst[byteIndex] & static_cast<u8>(~mask));
        }
    }
}

/// Reads `width` bits at bit offset `bitOff` from `src` (LSB-first).
inline std::uint64_t GetBits(const u8* src, std::size_t bitOff, std::size_t width) {
    std::uint64_t out = 0;
    std::size_t bit = bitOff;
    for (std::size_t i = 0; i < width; ++i, ++bit) {
        std::size_t byteIndex = bit >> 3;
        std::size_t bitIndex  = bit & 7;
        u8 mask = static_cast<u8>(1u << bitIndex);
        if (src[byteIndex] & mask) {
            out |= (1ull << i);
        }
    }
    return out;
}

// // ---------------------------------------------------------------
// //  ExtraDataBlock <-> Ver3 fields (contiguous).
// // ---------------------------------------------------------------
class NxInVer3Pack {
public:
    struct GroupIndices {
        u8 faceGI;
        u8 hairGI;
        u8 eyeGI;
        u8 browGI;
        u8 mouthGI;
        u8 beardGI;
        u8 glassColorGI;
        u8 glassTypeGI;
    };

    // Public packing/unpacking API.

    /// @brief Packs Ver4/NX (Switch) indices into Ver3MiiDataCore.
    /// @detail
    /// - Converts ver4 to ver3 visible indices with ToVer3 tables.
    /// - Builds group indices and packs them into the ExtraDataBlock.
    /// - Writes the block into available fields of the Ver3 struct.
    static void Pack(const NxExtensionFields& ver4, Ver3MiiDataCore& mii);
    /// @brief Unpacks Ver4/NX (Switch) indices from Ver3MiiDataCore.
    /// @detail
    /// - Reads ExtraDataBlock from the data.
    /// - Unpacks group indices and reconstructs ver4 indices using reverse maps.
    static void Unpack(const Ver3MiiDataCore& mii, NxExtensionFields& outVer4);

    /// Overload to unpack Ver4/NX (Switch) indices from Ver3MiiDataCore
    /// using an already extracted ExtraDataBlock.
    static void Unpack(const ExtraDataBlock& inBlock, const Ver3MiiDataCore& mii, NxExtensionFields& outVer4);

    /// @brief Extracts the contiguous 51-bit extra block from the Ver3 data into `out`.
    /// @detail The order and widths are fixed as documented above.
    static void ExtractExtra(const Ver3MiiDataCore& mii, ExtraDataBlock& outBlock);
    /// @brief Writes the contiguous 51-bit extra block from `in` into the Ver3MiiDataCore fields.
    /// @detail The last 6 bits target roomIndex/positionInRoom.
    /// Three bits are used for each for a range of 0..7 to not exceed the max of 9.
    static void WriteExtra(Ver3MiiDataCore& mii, const ExtraDataBlock& inBlock);

    // Staging.
    /// Packs the Ver4/NX (Switch) grouped indices into the ExtraDataBlock.
    static void EncodeGroupIndices(const GroupIndices& gi, ExtraDataBlock& outBlock);
    /// Unpacks the Ver4/NX (Switch) grouped indices from the ExtraDataBlock.
    static void DecodeGroupIndices(const ExtraDataBlock& inBlock, GroupIndices& gi);
};

// C ABI wrappers.
extern "C" {
    void NxInVer3Pack_Pack(const NxExtensionFields* in, Ver3MiiDataCore* out);
    void NxInVer3Pack_Unpack(const Ver3MiiDataCore* in, NxExtensionFields* out);
}
