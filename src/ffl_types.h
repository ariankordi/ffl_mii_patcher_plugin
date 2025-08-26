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
