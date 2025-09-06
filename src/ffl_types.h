#pragma once
#include <array>
#include <cstdint>

/// https://github.com/aboood40091/ffl/blob/73fe9fc70c0f96ebea373122e50f6d3acc443180/include/nn/ffl/FFLColor.h#L10
typedef struct alignas(16) FFLColor
{
    float r;
    float g;
    float b;
    float a;
}
FFLColor;
static_assert(sizeof(FFLColor) == 0x10);

/// https://github.com/ariankordi/ffl/blob/97eecdf3688f92c4c95cecf5d6ab3e84c0ee42c0/include/nn/ffl/FFLModulateParam.h#L11
typedef enum FFLModulateMode
{
    FFL_MODULATE_MODE_CONSTANT        = 0,
    FFL_MODULATE_MODE_TEXTURE_DIRECT  = 1,
    FFL_MODULATE_MODE_RGB_LAYERED     = 2,
    FFL_MODULATE_MODE_ALPHA           = 3,
    FFL_MODULATE_MODE_LUMINANCE_ALPHA = 4,
    FFL_MODULATE_MODE_ALPHA_OPA       = 5
}
FFLModulateMode;

/// https://github.com/aboood40091/ffl/blob/73fe9fc70c0f96ebea373122e50f6d3acc443180/include/nn/ffl/FFLModulateParam.h#L52
typedef enum FFLModulateType
{
    FFL_MODULATE_TYPE_SHAPE_FACELINE    =  0,
    FFL_MODULATE_TYPE_SHAPE_BEARD       =  1,
    FFL_MODULATE_TYPE_SHAPE_NOSE        =  2,
    FFL_MODULATE_TYPE_SHAPE_FOREHEAD    =  3,
    FFL_MODULATE_TYPE_SHAPE_HAIR        =  4,
    FFL_MODULATE_TYPE_SHAPE_CAP         =  5,
    FFL_MODULATE_TYPE_SHAPE_MASK        =  6,
    FFL_MODULATE_TYPE_SHAPE_NOSELINE    =  7,
    FFL_MODULATE_TYPE_SHAPE_GLASS       =  8,
    FFL_MODULATE_TYPE_MUSTACHE          =  9,
    FFL_MODULATE_TYPE_MOUTH             = 10,
    FFL_MODULATE_TYPE_EYEBROW           = 11,
    FFL_MODULATE_TYPE_EYE               = 12,
    FFL_MODULATE_TYPE_MOLE              = 13,
    FFL_MODULATE_TYPE_FACE_MAKE         = 14,
    FFL_MODULATE_TYPE_FACE_LINE         = 15,
    FFL_MODULATE_TYPE_FACE_BEARD        = 16,
    FFL_MODULATE_TYPE_FILL              = 17,

    FFL_MODULATE_TYPE_SHAPE_MAX         = FFL_MODULATE_TYPE_SHAPE_GLASS + 1
}
FFLModulateType;

/// https://github.com/aboood40091/ffl/blob/73fe9fc70c0f96ebea373122e50f6d3acc443180/include/nn/ffl/FFLModulateParam.h#L79
typedef struct FFLModulateParam
{
    FFLModulateMode     mode;
    FFLModulateType     type;
    const FFLColor*     pColorR;
    const FFLColor*     pColorG;
    const FFLColor*     pColorB;
    const /*GX2Texture*/void*   pGX2Texture;
}
FFLModulateParam;
#ifdef __WIIU__
static_assert(sizeof(FFLModulateParam) == 0x18);
#endif

typedef struct FFLiCharInfo
{
    int miiVersion;

    struct {
        int type;
        int color;
        int texture;
        int make;
    } faceline;

    struct {
        int type;
        int color;
        int flip;
    } hair;

    struct {
        int type;
        int color;
        int scale;
        int aspect;
        int rotate;
        int x;
        int y;
    } eye;

    struct {
        int type;
        int color;
        int scale;
        int aspect;
        int rotate;
        int x;
        int y;
    } eyebrow;

    struct {
        int type;
        int scale;
        int y;
    } nose;

    struct {
        int type;
        int color;
        int scale;
        int aspect;
        int y;
    } mouth;

    struct {
        int mustache;
        int type;
        int color;
        int scale;
        int y;
    } beard;

    struct {
        int type;
        int color;
        int scale;
        int y;
    } glass;

    struct {
        int type;
        int scale;
        int x;
        int y;
    } mole;

    struct {
        int height;
        int build;
    } body;

    struct {
        char16_t name[11];
        char16_t creator[11];
        int gender;
        int birthMonth;
        int birthDay;
        int favoriteColor;
        bool favorite;
        bool copyable;
        bool ngWord;
        bool localonly;
        int regionMove;
        int fontRegion;
        int roomIndex;
        int positionInRoom;
        int birthPlatform;
    } personal;

    uint8_t createID[10];
    uint16_t padding_0;
    int authorType;
    uint8_t authorID[8];
}
FFLiCharInfo;

static constexpr std::array FFLiVerifyReasonStrings = std::to_array<const char*>({
    "FFLI_VERIFY_REASON_SUCCESS",
    "FFLI_VERIFY_REASON_FACELINE_TYPE",
    "FFLI_VERIFY_REASON_FACELINE_COLOR",
    "FFLI_VERIFY_REASON_FACELINE_TEXTURE",
    "FFLI_VERIFY_REASON_FACELINE_MAKE",
    "FFLI_VERIFY_REASON_HAIR_TYPE",
    "FFLI_VERIFY_REASON_HAIR_COLOR",
    "FFLI_VERIFY_REASON_HAIR_FLIP",
    "FFLI_VERIFY_REASON_EYE_TYPE",
    "FFLI_VERIFY_REASON_EYE_COLOR",
    "FFLI_VERIFY_REASON_EYE_SCALE",
    "FFLI_VERIFY_REASON_EYE_ASPECT",
    "FFLI_VERIFY_REASON_EYE_ROTATE",
    "FFLI_VERIFY_REASON_EYE_X",
    "FFLI_VERIFY_REASON_EYE_Y",
    "FFLI_VERIFY_REASON_EYEBROW_TYPE",
    "FFLI_VERIFY_REASON_EYEBROW_COLOR",
    "FFLI_VERIFY_REASON_EYEBROW_SCALE",
    "FFLI_VERIFY_REASON_EYEBROW_ASPECT",
    "FFLI_VERIFY_REASON_EYEBROW_ROTATE",
    "FFLI_VERIFY_REASON_EYEBROW_X",
    "FFLI_VERIFY_REASON_EYEBROW_Y",
    "FFLI_VERIFY_REASON_NOSE_TYPE",
    "FFLI_VERIFY_REASON_NOSE_SCALE",
    "FFLI_VERIFY_REASON_NOSE_Y",
    "FFLI_VERIFY_REASON_MOUTH_TYPE",
    "FFLI_VERIFY_REASON_MOUTH_COLOR",
    "FFLI_VERIFY_REASON_MOUTH_SCALE",
    "FFLI_VERIFY_REASON_MOUTH_ASPECT",
    "FFLI_VERIFY_REASON_MOUTH_Y",
    "FFLI_VERIFY_REASON_BEARD_TYPE",
    "FFLI_VERIFY_REASON_BEARD_COLOR",
    "FFLI_VERIFY_REASON_MUSTACHE_TYPE",
    "FFLI_VERIFY_REASON_MUSTACHE_SCALE",
    "FFLI_VERIFY_REASON_MUSTACHE_Y",
    "FFLI_VERIFY_REASON_GLASS_TYPE",
    "FFLI_VERIFY_REASON_GLASS_COLOR",
    "FFLI_VERIFY_REASON_GLASS_SCALE",
    "FFLI_VERIFY_REASON_GLASS_Y",
    "FFLI_VERIFY_REASON_MOLE_TYPE",
    "FFLI_VERIFY_REASON_MOLE_SCALE",
    "FFLI_VERIFY_REASON_MOLE_X",
    "FFLI_VERIFY_REASON_MOLE_Y",
    "FFLI_VERIFY_REASON_HEIGHT",
    "FFLI_VERIFY_REASON_BUILD",
    "FFLI_VERIFY_REASON_NAME",
    "FFLI_VERIFY_REASON_CREATORNAME",
    "FFLI_VERIFY_REASON_GENDER",
    "FFLI_VERIFY_REASON_BIRTHDAY",
    "FFLI_VERIFY_REASON_FAVORITECOLOR",
    "FFLI_VERIFY_REASON_REGIONMOVE",
    "FFLI_VERIFY_REASON_FONTREGION",
    "FFLI_VERIFY_REASON_ROOM_INDEX",
    "FFLI_VERIFY_REASON_POSITION_IN_ROOM",
    "FFLI_VERIFY_REASON_BIRTH_PLATFORM",
    "FFLI_VERIFY_REASON_CREATEID"
});
