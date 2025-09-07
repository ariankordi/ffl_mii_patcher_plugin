#include "src/NxInVer3Pack.hpp"
#include <gtest/gtest.h>
#include <cstring>

/// Initializes Ver3MiiDataCore data to a clean baseline.
/// We deliberately set visible color fields to zero,
/// as NxInVer3Pack::Pack() will overwrite them.
static Ver3MiiDataCore GetCleanData() {
    Ver3MiiDataCore core{};
    std::memset(&core, 0, sizeof(core));

    // Set bias-required fields to a safe non-zero value because some titles verify them.
    core.roomIndex = 1;        // 1..9 acceptable; our writer will overwrite using +1 encoding.
    core.positionInRoom = 1;

    return core;
}

/// Verifies that the data is valid.
/// Currently ensures roomIndex/positionInRoom do not exceed the maximum.
void Verify(const Ver3MiiDataCore& mii) {
    ASSERT_LT(mii.roomIndex, 9);
    ASSERT_LT(mii.positionInRoom, 9);
}

TEST(NxInVer3Pack, Verify_Success_MaxBlock)
{
    Ver3MiiDataCore mii = GetCleanData();

    ExtraDataBlock block{};
    // pretty much write max values to the block
    std::memset(&block, UINT8_MAX, sizeof(block.data));

    NxInVer3Pack::WriteExtra(mii, block);
    Verify(mii);
}

TEST(NxInVer3Pack, RoundTrip_FacelineColor_0_to_9)
{
    Ver3MiiDataCore mii = GetCleanData();
    for (int v = 0; v < FacelineColor_End; ++v) {
        NxExtensionFields in{};
        in.facelineColor = static_cast<u8>(v);
        // Others at zero to keep the test focused per-field.
        in.hairColor = 0; in.eyeColor = 0; in.eyebrowColor = 0;
        in.mouthColor = 0; in.beardColor = 0; in.glassColor = 0; in.glassType = 0;

        NxInVer3Pack::Pack(in, mii);

        NxExtensionFields out{};
        NxInVer3Pack::Unpack(mii, out);

        EXPECT_EQ(out.facelineColor, in.facelineColor) << "facelineColor ver4 mismatch at " << v;
        Verify(mii);
    }
}

TEST(NxInVer3Pack, RoundTrip_HairColor_0_to_99)
{
    Ver3MiiDataCore mii = GetCleanData();
    for (int v = 0; v < CommonColor_End; ++v) {
        NxExtensionFields in{};
        in.hairColor = static_cast<u8>(v);
        in.facelineColor = 0; in.eyeColor = 0; in.eyebrowColor = 0;
        in.mouthColor = 0; in.beardColor = 0; in.glassColor = 0; in.glassType = 0;

        NxInVer3Pack::Pack(in, mii);
        NxExtensionFields out{};
        NxInVer3Pack::Unpack(mii, out);

        EXPECT_EQ(out.hairColor, in.hairColor) << "hairColor ver4 mismatch at " << v;
        Verify(mii);
    }
}

TEST(NxInVer3Pack, RoundTrip_EyeColor_0_to_99)
{
    Ver3MiiDataCore mii = GetCleanData();
    for (int v = 0; v < CommonColor_End; ++v) {
        NxExtensionFields in{};
        in.eyeColor = static_cast<u8>(v);
        NxInVer3Pack::Pack(in, mii);
        NxExtensionFields out{};
        NxInVer3Pack::Unpack(mii, out);
        EXPECT_EQ(out.eyeColor, in.eyeColor) << "eyeColor ver4 mismatch at " << v;
        Verify(mii);
    }
}

TEST(NxInVer3Pack, RoundTrip_EyebrowColor_0_to_99)
{
    Ver3MiiDataCore mii = GetCleanData();
    for (int v = 0; v < CommonColor_End; ++v) {
        NxExtensionFields in{};
        in.eyebrowColor = static_cast<u8>(v);
        NxInVer3Pack::Pack(in, mii);
        NxExtensionFields out{};
        NxInVer3Pack::Unpack(mii, out);
        EXPECT_EQ(out.eyebrowColor, in.eyebrowColor) << "eyebrowColor ver4 mismatch at " << v;
        Verify(mii);
    }
}

TEST(NxInVer3Pack, RoundTrip_MouthColor_0_to_99)
{
    Ver3MiiDataCore mii = GetCleanData();
    for (int v = 0; v < CommonColor_End; ++v) {
        NxExtensionFields in{};
        in.mouthColor = static_cast<u8>(v);
        NxInVer3Pack::Pack(in, mii);
        NxExtensionFields out{};
        NxInVer3Pack::Unpack(mii, out);
        EXPECT_EQ(out.mouthColor, in.mouthColor) << "mouthColor ver4 mismatch at " << v;
        Verify(mii);
    }
}

TEST(NxInVer3Pack, RoundTrip_BeardColor_0_to_99)
{
    Ver3MiiDataCore mii = GetCleanData();
    for (int v = 0; v < CommonColor_End; ++v) {
        NxExtensionFields in{};
        in.beardColor = static_cast<u8>(v);
        NxInVer3Pack::Pack(in, mii);
        NxExtensionFields out{};
        NxInVer3Pack::Unpack(mii, out);
        EXPECT_EQ(out.beardColor, in.beardColor) << "beardColor ver4 mismatch at " << v;
        Verify(mii);
    }
}

TEST(NxInVer3Pack, RoundTrip_GlassColor_0_to_99)
{
    Ver3MiiDataCore mii = GetCleanData();
    for (int v = 0; v < CommonColor_End; ++v) {
        NxExtensionFields in{};
        in.glassColor = static_cast<u8>(v);
        NxInVer3Pack::Pack(in, mii);
        NxExtensionFields out{};
        NxInVer3Pack::Unpack(mii, out);
        EXPECT_EQ(out.glassColor, in.glassColor) << "glassColor ver4 mismatch at " << v;
        Verify(mii);
    }
}

TEST(NxInVer3Pack, RoundTrip_GlassType_0_to_19)
{
    Ver3MiiDataCore mii = GetCleanData();
    for (int v = 0; v < GlassType_End; ++v) {
        NxExtensionFields in{};
        in.glassType = static_cast<u8>(v);
        NxInVer3Pack::Pack(in, mii);
        NxExtensionFields out{};
        NxInVer3Pack::Unpack(mii, out);
        EXPECT_EQ(out.glassType, in.glassType) << "glassType ver4 mismatch at " << v;
        Verify(mii);
    }
}
