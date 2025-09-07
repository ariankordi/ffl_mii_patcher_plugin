#include "src/NxInVer3Pack.hpp"
#include <cstdio>
#include <cstring>
#include <cstdlib>

// #include "mii_SwapEndian.h"

static void ShowUsage(const char* prog) {
    std::fprintf(stderr,
        "Usage:\n"
        "  %s pack <input_mii_file> <output_mii_file> <facelineColor> <hairColor> <eyeColor> <eyebrowColor> <mouthColor> <beardColor> <glassColor> <glassType>\n"
        "  %s unpack <input_mii_file>\n",
        prog, prog);
}

static bool ReadMiiFile(const char* path, Ver3MiiDataCore& out) {
    FILE* f = nullptr;
    if (std::strcmp(path, "-") == 0) {
        f = stdin;
    } else {
        f = std::fopen(path, "rb");
    }
    if (!f) {
        std::perror("fopen");
        return false;
    }
    size_t got = std::fread(&out, 1, sizeof(out), f);
    if (f != stdin) {
        std::fclose(f);
    }
    if (got != sizeof(out)) {
        std::fprintf(stderr, "Error: file too small or truncated.\n");
        return false;
    }

#if __BYTE_ORDER__ != __ORDER_LITTLE_ENDIAN__
    SwapEndian(&out);
#endif

    return true;
}

static bool WriteMiiFile(const char* path, const Ver3MiiDataCore& in) {
    FILE* f = nullptr;
    if (std::strcmp(path, "-") == 0) {
        f = stdout;
    } else {
        f = std::fopen(path, "wb");
    }
    if (!f) {
        std::perror("fopen");
        return false;
    }
    size_t wrote = std::fwrite(&in, 1, sizeof(in), f);
    if (f != stdout) {
        std::fclose(f);
    }
    if (wrote != sizeof(in)) {
        std::fprintf(stderr, "Error: failed to write all data.\n");
        return false;
    }
    return true;
}

int main(int argc, char** argv) {
    if (argc < 3) {
        ShowUsage(argv[0]);
        return 1;
    }

    const char* mode = argv[1];

    if (std::strcmp(mode, "pack") == 0) {
        if (argc != 12) {
            ShowUsage(argv[0]);
            return 1;
        }

        Ver3MiiDataCore mii{};
        if (!ReadMiiFile(argv[2], mii)) {
            return 1;
        }

        NxExtensionFields fields{};
        fields.facelineColor = static_cast<u8>(std::atoi(argv[4]));
        fields.hairColor     = static_cast<u8>(std::atoi(argv[5]));
        fields.eyeColor      = static_cast<u8>(std::atoi(argv[6]));
        fields.eyebrowColor  = static_cast<u8>(std::atoi(argv[7]));
        fields.mouthColor    = static_cast<u8>(std::atoi(argv[8]));
        fields.beardColor    = static_cast<u8>(std::atoi(argv[9]));
        fields.glassColor    = static_cast<u8>(std::atoi(argv[10]));
        fields.glassType     = static_cast<u8>(std::atoi(argv[11]));

        // Bounds checking.
        if (fields.facelineColor >= FacelineColor_End)  { std::fprintf(stderr, "facelineColor out of range (0-9)\n"); return 1; }
        if (fields.hairColor >= CommonColor_End) { std::fprintf(stderr, "hairColor out of range (0-99)\n"); return 1; }
        if (fields.eyeColor >= CommonColor_End)  { std::fprintf(stderr, "eyeColor out of range (0-99)\n"); return 1; }
        if (fields.eyebrowColor >= CommonColor_End) { std::fprintf(stderr, "eyebrowColor out of range (0-99)\n"); return 1; }
        if (fields.mouthColor >= CommonColor_End) { std::fprintf(stderr, "mouthColor out of range (0-99)\n"); return 1; }
        if (fields.beardColor >= CommonColor_End) { std::fprintf(stderr, "beardColor out of range (0-99)\n"); return 1; }
        if (fields.glassColor >= CommonColor_End) { std::fprintf(stderr, "glassColor out of range (0-99)\n"); return 1; }
        if (fields.glassType >= GlassType_End)  { std::fprintf(stderr, "glassType out of range (0-19)\n"); return 1; }

        NxInVer3Pack::Pack(fields, mii);

        if (!WriteMiiFile(argv[3], mii)) {
            return 1;
        }

    } else if (std::strcmp(mode, "unpack") == 0) {
        if (argc != 3) {
            ShowUsage(argv[0]);
            return 1;
        }

        Ver3MiiDataCore mii{};
        if (!ReadMiiFile(argv[2], mii)) {
            return 1;
        }

        NxExtensionFields out{};
        NxInVer3Pack::Unpack(mii, out);

        std::printf("Faceline Color: %u\n", out.facelineColor);
        std::printf("Hair Color:     %u\n", out.hairColor);
        std::printf("Eye Color:      %u\n", out.eyeColor);
        std::printf("Eyebrow Color:  %u\n", out.eyebrowColor);
        std::printf("Mouth Color:    %u\n", out.mouthColor);
        std::printf("Beard Color:    %u\n", out.beardColor);
        std::printf("Glass Color:    %u\n", out.glassColor);
        std::printf("Glass Type:     %u\n", out.glassType);

    } else {
        ShowUsage(argv[0]);
        return 1;
    }

    return 0;
}
