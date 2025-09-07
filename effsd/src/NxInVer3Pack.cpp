#include "NxInVer3Pack.hpp"
#include <cassert>
#include <cstring> // for std::memset

// // ---------------------------------------------------------------
// //  Group Index Helpers
// // ---------------------------------------------------------------

/// Returns the group index (position in bucket) for a given ver4 index using a reverse map.
template <typename RevT>
constexpr u8 GroupIndexOf(const RevT& rev, u8 ver4Index) {
    return rev.positionInGroup[ver4Index];
}

/// Reconstructs ver4 index from ver3 value and group index, with clamping for safety.
template <typename RevT>
constexpr u8 Ver4FromGroup(const RevT& rev, u8 ver3Value, u8 groupIndex) {
    u16 count = rev.counts[ver3Value];
    assert(count > 0);
    if (groupIndex >= count) {
        // Clamp to a valid entry to avoid out-of-range reads in corrupted cases.
        groupIndex = static_cast<u8>(count - 1);
    }
    return rev.byGroup[ver3Value][groupIndex];
}

// Visible data packing.

void NxInVer3Pack::Pack(const NxExtensionFields& ver4, Ver3MiiDataCore& mii)
{
    // Map ver4 indices to visible ver3 indices that the system supports.
    const u8 faceColorV3    = ToVer3FacelineColorTable[ver4.facelineColor];
    const u8 hairColorV3    = ToVer3HairColorTable[ver4.hairColor];
    const u8 eyeColorV3     = ToVer3EyeColorTable[ver4.eyeColor];
    const u8 eyebrowColorV3 = ToVer3HairColorTable[ver4.eyebrowColor];
    const u8 mouthColorV3   = ToVer3MouthColorTable[ver4.mouthColor];
    const u8 beardColorV3   = ToVer3HairColorTable[ver4.beardColor];
    const u8 glassColorV3   = ToVer3GlassColorTable[ver4.glassColor];
    const u8 glassTypeV3    = ToVer3GlassTypeTable[ver4.glassType];

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
    // Write ver3 visible indices into Mii data.
    mii.faceColor    = static_cast<u8>(faceColorV3);
    mii.hairColor    = static_cast<u8>(hairColorV3);
    mii.eyeColor     = static_cast<u8>(eyeColorV3);
    mii.eyebrowColor = static_cast<u8>(eyebrowColorV3);
    mii.mouthColor   = static_cast<u8>(mouthColorV3);
    mii.beardColor   = static_cast<u8>(beardColorV3);
    mii.glassColor   = static_cast<u8>(glassColorV3);
    mii.glassType    = static_cast<u8>(glassTypeV3);
#pragma GCC diagnostic pop

    // Compute group indices to allow ver4 reconstruction.
    GroupIndices gi{};
    gi.faceGI       = GroupIndexOf(RevFaceColors,  ver4.facelineColor);
    gi.hairGI       = GroupIndexOf(RevHairColors,  ver4.hairColor);
    gi.eyeGI        = GroupIndexOf(RevEyeColors,   ver4.eyeColor);
    gi.browGI       = GroupIndexOf(RevHairColors,  ver4.eyebrowColor);
    gi.mouthGI      = GroupIndexOf(RevMouthColors, ver4.mouthColor);
    gi.beardGI      = GroupIndexOf(RevHairColors,  ver4.beardColor);
    gi.glassColorGI = GroupIndexOf(RevGlassColors, ver4.glassColor);
    gi.glassTypeGI  = GroupIndexOf(RevGlassTypes,  ver4.glassType);

    // Pack group indices into the beginning of ExtraDataBlock.
    ExtraDataBlock block{};
    std::memset(block.data, 0, EXTRA_BYTES_TOTAL);

    EncodeGroupIndices(gi, block);

    // The rest of the 51-bit block remains zero for now (future checksum/user space).
    // Write the block into the Mii struct reserved/padding fields contiguously.
    WriteExtra(mii, block);
}

// Unpacking: read the extra block, and reconstruct Ver4 fields.

void NxInVer3Pack::Unpack(const Ver3MiiDataCore& mii, NxExtensionFields& outVer4)
{
    // Extract the contiguous block from the data.
    ExtraDataBlock block{};
    ExtractExtra(mii, block);

    Unpack(block, mii, outVer4);
}

void NxInVer3Pack::Unpack(const ExtraDataBlock& inBlock, const Ver3MiiDataCore& mii, NxExtensionFields& outVer4)
{
    // Read visible ver3 values back from the data for disambiguation.
    GroupIndices gi{};
    NxInVer3Pack::DecodeGroupIndices(inBlock, gi);
    const u8 faceV3  = static_cast<u8>(mii.faceColor);
    const u8 hairV3  = static_cast<u8>(mii.hairColor);
    const u8 eyeV3   = static_cast<u8>(mii.eyeColor);
    const u8 browV3  = static_cast<u8>(mii.eyebrowColor);
    const u8 mouthV3 = static_cast<u8>(mii.mouthColor);
    const u8 beardV3 = static_cast<u8>(mii.beardColor);
    const u8 gcolV3  = static_cast<u8>(mii.glassColor);
    const u8 gtypV3  = static_cast<u8>(mii.glassType);

    // Reconstruct ver4 indices using reverse-mapping buckets.
    outVer4.facelineColor = Ver4FromGroup(RevFaceColors,   faceV3,  gi.faceGI);
    outVer4.hairColor     = Ver4FromGroup(RevHairColors,   hairV3,  gi.hairGI);
    outVer4.eyeColor      = Ver4FromGroup(RevEyeColors,    eyeV3,   gi.eyeGI);
    outVer4.eyebrowColor  = Ver4FromGroup(RevHairColors,   browV3,  gi.browGI);
    outVer4.mouthColor    = Ver4FromGroup(RevMouthColors,  mouthV3, gi.mouthGI);
    outVer4.beardColor    = Ver4FromGroup(RevHairColors,   beardV3, gi.beardGI);
    outVer4.glassColor    = Ver4FromGroup(RevGlassColors,  gcolV3,  gi.glassColorGI);
    outVer4.glassType     = Ver4FromGroup(RevGlassTypes,   gtypV3,  gi.glassTypeGI);
}

void NxInVer3Pack::EncodeGroupIndices(const GroupIndices& gi, ExtraDataBlock& outBlock)
{
    std::memset(outBlock.data, 0, EXTRA_BYTES_TOTAL);
    std::size_t bit = 0;
    PutBits(outBlock.data, bit, FacelineColorBits, gi.faceGI);       bit += FacelineColorBits;
    PutBits(outBlock.data, bit, HairColorBits,     gi.hairGI);       bit += HairColorBits;
    PutBits(outBlock.data, bit, EyeColorBits,      gi.eyeGI);        bit += EyeColorBits;
    PutBits(outBlock.data, bit, EyebrowColorBits,  gi.browGI);       bit += EyebrowColorBits;
    PutBits(outBlock.data, bit, MouthColorBits,    gi.mouthGI);      bit += MouthColorBits;
    PutBits(outBlock.data, bit, BeardColorBits,    gi.beardGI);      bit += BeardColorBits;
    PutBits(outBlock.data, bit, GlassColorBits,    gi.glassColorGI); bit += GlassColorBits;
    PutBits(outBlock.data, bit, GlassTypeBits,     gi.glassTypeGI);  bit += GlassTypeBits;
}

void NxInVer3Pack::DecodeGroupIndices(const ExtraDataBlock& inBlock, GroupIndices& gi)
{
    std::size_t bit = 0;
    gi.faceGI       = static_cast<u8>(GetBits(inBlock.data, bit, FacelineColorBits)); bit += FacelineColorBits;
    gi.hairGI       = static_cast<u8>(GetBits(inBlock.data, bit, HairColorBits));     bit += HairColorBits;
    gi.eyeGI        = static_cast<u8>(GetBits(inBlock.data, bit, EyeColorBits));      bit += EyeColorBits;
    gi.browGI       = static_cast<u8>(GetBits(inBlock.data, bit, EyebrowColorBits));  bit += EyebrowColorBits;
    gi.mouthGI      = static_cast<u8>(GetBits(inBlock.data, bit, MouthColorBits));    bit += MouthColorBits;
    gi.beardGI      = static_cast<u8>(GetBits(inBlock.data, bit, BeardColorBits));    bit += BeardColorBits;
    gi.glassColorGI = static_cast<u8>(GetBits(inBlock.data, bit, GlassColorBits));    bit += GlassColorBits;
    gi.glassTypeGI  = static_cast<u8>(GetBits(inBlock.data, bit, GlassTypeBits));     bit += GlassTypeBits;
}

// ExtraDataBlock contiguous extraction.

void NxInVer3Pack::ExtractExtra(const Ver3MiiDataCore& mii, ExtraDataBlock& out)
{
    std::memset(out.data, 0, EXTRA_BYTES_TOTAL);
    std::size_t bit = 0;

    // Strict order as documented. We read the current numeric field values as bits.
    PutBits(out.data, bit, 2,  static_cast<u64>(mii.reserved_0));      bit += 2;
    PutBits(out.data, bit, 4,  static_cast<u64>(mii.authorType));      bit += 4;
    PutBits(out.data, bit, 1,  static_cast<u64>(mii.reserved_1));      bit += 1;
    PutBits(out.data, bit, 8,  static_cast<u64>(mii.reserved_2[0]));   bit += 8;
    PutBits(out.data, bit, 8,  static_cast<u64>(mii.reserved_2[1]));   bit += 8;

    PutBits(out.data, bit, 1,  static_cast<u64>(mii.padding_0));       bit += 1;
    PutBits(out.data, bit, 4,  static_cast<u64>(mii.padding_1));       bit += 4;
    PutBits(out.data, bit, 2,  static_cast<u64>(mii.padding_2));       bit += 2;
    PutBits(out.data, bit, 1,  static_cast<u64>(mii.padding_3));       bit += 1;
    PutBits(out.data, bit, 2,  static_cast<u64>(mii.padding_4));       bit += 2;
    PutBits(out.data, bit, 2,  static_cast<u64>(mii.padding_5));       bit += 2;
    PutBits(out.data, bit, 8,  static_cast<u64>(mii.padding_6));       bit += 8;
    PutBits(out.data, bit, 1,  static_cast<u64>(mii.padding_7));       bit += 1;
    PutBits(out.data, bit, 1,  static_cast<u64>(mii.padding_8));       bit += 1;

    // Only 3 bits (0-7) are used in roomIndex/positionInRoom.
    /*
    // If fields are zero or out of range, treat as 1 (encoded zero) defensively.
    u32 room = (mii.roomIndex == 0 || mii.roomIndex > 9) ? 1u : static_cast<u32>(mii.roomIndex);
    u32 pos  = (mii.positionInRoom == 0 || mii.positionInRoom > 9) ? 1u : static_cast<u32>(mii.positionInRoom);
    PutBits(out.data, bit, 3, static_cast<u64>((room - 1) & 0x7));     bit += 3;
    PutBits(out.data, bit, 3, static_cast<u64>((pos  - 1) & 0x7));     bit += 3;
    */
    PutBits(out.data, bit, 3,  static_cast<u64>(mii.roomIndex) & 0x7);      bit += 3;
    PutBits(out.data, bit, 3,  static_cast<u64>(mii.positionInRoom) & 0x7); bit += 3;

    // We ignore any remaining bits; total must be <= 51.
    (void)bit;
}

// ExtraDataBlock writing.

void NxInVer3Pack::WriteExtra(Ver3MiiDataCore& mii, const ExtraDataBlock& in)
{
    std::size_t bit = 0;

    // For all assignments below, values may or may not be changed.
    // GCC complains about this depsite the explicit static_cast, clang doesn't.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
    // Read fields back from the block, then assign into struct fields.

    mii.reserved_0 = static_cast<u32>(GetBits(in.data, bit, 2));   bit += 2;
    // authorType is an unused field always unread and 0. There is even an enum
    // for it, but it only has one value "0 = Normal". It was presumably for UGC.
    mii.authorType = static_cast<u32>(GetBits(in.data, bit, 4));   bit += 4;
    mii.reserved_1 = static_cast<u32>(GetBits(in.data, bit, 1));   bit += 1;

    mii.reserved_2[0] = static_cast<u8>(GetBits(in.data, bit, 8)); bit += 8;
    mii.reserved_2[1] = static_cast<u8>(GetBits(in.data, bit, 8)); bit += 8;

    mii.padding_0 = static_cast<u16>(GetBits(in.data, bit, 1));     bit += 1;
    mii.padding_1 = static_cast<u16>(GetBits(in.data, bit, 4));     bit += 4;
    mii.padding_2 = static_cast<u16>(GetBits(in.data, bit, 2));     bit += 2;
    mii.padding_3 = static_cast<u16>(GetBits(in.data, bit, 1));     bit += 1;
    mii.padding_4 = static_cast<u16>(GetBits(in.data, bit, 2));     bit += 2;
    mii.padding_5 = static_cast<u16>(GetBits(in.data, bit, 2));     bit += 2;
    mii.padding_6 = static_cast<u16>(GetBits(in.data, bit, 8));     bit += 8;
    mii.padding_7 = static_cast<u16>(GetBits(in.data, bit, 1));     bit += 1;
    mii.padding_8 = static_cast<u16>(GetBits(in.data, bit, 1));     bit += 1;

    // roomIndex/positionInRoom are only used on the 3DS database.
    // They should otherwise be unused on Wii U and in data transmission.
    // Their max value is 9, otherwise verification fails. So, only 3 bits are used.
    // Because these can be non-zero, they are put at the end.
    // They may eventually be removed as well if they

    mii.roomIndex      = static_cast<u32>(GetBits(in.data, bit, 3)); bit += 3;
    mii.positionInRoom = static_cast<u32>(GetBits(in.data, bit, 3)); bit += 3;
#pragma GCC diagnostic pop

    (void)bit;
}

extern "C" {
    void NxInVer3Pack_Pack(const NxExtensionFields* in, Ver3MiiDataCore* out) {
        NxInVer3Pack::Pack(*in, *out);
    }
    void NxInVer3Pack_Unpack(const Ver3MiiDataCore* in, NxExtensionFields* out) {
        NxInVer3Pack::Unpack(*in, *out);
    }
}

/*
 * Usage in JS:
 * > emcc -s WASM=1 -s SINGLE_FILE=1 -s MALLOC=emmalloc -s INITIAL_HEAP=64kb -s STRICT=1 -s MINIMAL_RUNTIME=2 -s EXPORTED_FUNCTIONS="['_NxInVer3Pack_Pack','_NxInVer3Pack_Unpack','_malloc','_free']" -sEXPORTED_RUNTIME_METHODS="['ccall','HEAPU8']" -s MODULARIZE=1 -sEXPORT_KEEPALIVE=1 -O2 NxInVer3Pack.cpp
 * Call like so:
async function main() {
    const mod = await Module();

    const customFields = new Uint8Array(8);
    customFields[0] = 9; // facelineColor
    customFields[1] = 99; // hairColor
    customFields[2] = 43; // eyeColor
    customFields[3] = 99; // eyebrowColor
    customFields[4] = 8; // mouthColor
    customFields[5] = 0; // beardColor
    customFields[6] = 23; // glassColor
    customFields[7] = 19; // glassType

    const storeData = new Uint8Array(72);
    const storeDataPtr = mod._malloc(storeData.length);
    mod.HEAPU8.set(storeData, storeDataPtr);

    mod.ccall('NxInVer3Pack_Pack',
        null, ['array', 'number'], // Output must be pointer.
        [customFields, storeDataPtr]);

    storeData.set(mod.HEAPU8.subarray(storeDataPtr, storeDataPtr + storeData.length));
    mod._free(storeDataPtr);

    console.log(customFields, storeData); // Results.
}
main();
*/
