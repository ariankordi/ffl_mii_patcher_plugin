#include "../src/ffl_patches.h"
#include "../src/utils/SignatureScanner.h"
#include "gtest/gtest.h"
#include <chrono>
#include <filesystem>
#include <fstream>
#include <vector>
// clangd says unknown type name for some reason?
#include <gtest/gtest.h>

using namespace std::chrono;

/// Fake OSEffectiveToPhysical for tests.
static uintptr_t mockEffToPhys(uintptr_t eff) { return eff ^ 0xDEADBEEF; }

class SignatureFFLMatchTest : public ::testing::Test {
protected:
    SignatureScanner *scanner;

    void SetUp() override {
        scanner = new SignatureScanner(cSignaturesFFL.data(),
            cSignaturesFFL.size(), mockEffToPhys);
    }
    void TearDown() override {
        delete scanner;
    }
};

// // ---------------------------------------------------------------
// //  PPC32 ELF Helpers
// // ---------------------------------------------------------------
#pragma pack(push, 1)
struct Elf32Ehdr {
    uint8_t  ident[16];
    uint16_t type;
    uint16_t machine;
    uint32_t version;
    uint32_t entry;
    uint32_t phOff;
    uint32_t shOff;
    uint32_t flags;
    uint16_t ehSize;
    uint16_t phEntSize;
    uint16_t phNum;
    uint16_t shEntSize;
    uint16_t shNum;
    uint16_t shStrNdx;
};

struct Elf32Shdr {
    uint32_t name;
    uint32_t type;
    uint32_t flags;
    uint32_t addr;      ///< effective address in memory
    uint32_t offset;    ///< file offset
    uint32_t size;
    uint32_t link;
    uint32_t info;
    uint32_t addralign;
    uint32_t entSize;
};
#pragma pack(pop)

struct TextSectionView {
    const uint8_t* bytes;     ///< pointer into file buffer
    size_t         size;      ///< bytes length
    uint32_t       shAddr;   ///< effective addr base
    uint32_t       shOffset; ///< file offset base
};

inline uint16_t be16(const uint8_t *p) {
    return (uint16_t(p[0]) << 8) | uint16_t(p[1]);
}
inline uint32_t be32(const uint8_t *p) {
    return (uint32_t(p[0]) << 24) | (uint32_t(p[1]) << 16) |
           (uint32_t(p[2]) << 8)  |  uint32_t(p[3]);
}

bool findExecText(const std::vector<uint8_t>& file, TextSectionView& out) {
    if (file.size() < sizeof(Elf32Ehdr)) {
        return false;
    }

    const Elf32Ehdr eh = {
        .type      = be16(&file[16]),
        .machine   = be16(&file[18]),
        .version   = be32(&file[20]),
        .entry     = be32(&file[24]),
        .phOff     = be32(&file[28]),
        .shOff     = be32(&file[32]),
        .flags     = be32(&file[36]),
        .ehSize    = be16(&file[40]),
        .phEntSize = be16(&file[42]),
        .phNum     = be16(&file[44]),
        .shEntSize = be16(&file[46]),
        .shNum     = be16(&file[48]),
        .shStrNdx  = be16(&file[50]),
    };

    if (eh.shOff == 0 || eh.shEntSize != sizeof(Elf32Shdr) || eh.shNum == 0) {
        return false;
    }

    for (uint16_t i = 0; i < eh.shNum; i++) {
        const uint8_t* base = file.data() + eh.shOff + i * eh.shEntSize;

        Elf32Shdr sh;
        sh.name      = be32(base+0);
        sh.type      = be32(base+4);
        sh.flags     = be32(base+8);
        sh.addr      = be32(base+12);
        sh.offset    = be32(base+16);
        sh.size      = be32(base+20);
        sh.link      = be32(base+24);
        sh.info      = be32(base+28);
        sh.addralign = be32(base+32);
        sh.entSize   = be32(base+36);

        if (sh.type == 1 /*SHT_PROGBITS*/ &&
            (sh.flags & 0x6) == 0x6 /*ALLOC|EXECINSTR*/) {
            if (sh.offset + sh.size > file.size()
                || sh.size < 1024) // hacky basic test for if this is the real section
            {
                continue;
            }
            out.bytes     = file.data();// + sh.offset;
            out.size      = sh.size;
            out.shAddr    = sh.addr;
            out.shOffset  = sh.offset;
            return true;
        }
    }
    return false;
}

/// Convert offset in the file back to an effective offset using the chosen .text.
static uint32_t fileOffToEff(uint32_t off, const TextSectionView &sec) {
    return (off - sec.shOffset) + sec.shAddr;
}

/// Helper to load an ELF file entirely into memory.
static std::vector<uint8_t> loadFile(const std::string &path) {
    std::ifstream f(path, std::ios::binary);
    return std::vector<uint8_t>(
            std::istreambuf_iterator<char>(f),
            std::istreambuf_iterator<char>());
}

// // ---------------------------------------------------------------
// //  Extra Verification Helpers
// // ---------------------------------------------------------------

/// Verify that each signature in the set matches exactly once in the module.
static void verifySignatureUniqueness(const SignatureDefinition* defs,
                                      uint32_t defCount,
                                      const SignatureMatch* matches,
                                      uint32_t found,
                                      const TextSectionView& text) {
    for (uint32_t d = 0; d < defCount; ++d) {
        const SignatureDefinition& def = defs[d];

        uint32_t occurrences = 0;
        for (uint32_t m = 0; m < found; ++m) {
            if (matches[m].pDef == &def) {
                occurrences++;
            }
        }

        EXPECT_EQ(occurrences, 1) << "Signature '" << def.name
                                  << "' matched " << occurrences << " times";

        // Extra sanity: re-scan bytes manually to confirm only one match.
        uint32_t verifyCount = 0;
        for (size_t off = 0; off + def.wordCount * 4 <= text.size; off += 4) {
            bool ok = true;
            for (uint32_t w = 0; w < def.wordCount; ++w) {
                uint32_t word = SignatureScanner::load_be_u32(text.bytes + off + w*4);
                if ((word & def.words[w].mask)
                        != (def.words[w].value & def.words[w].mask)) {
                    ok = false;
                    break;
                }
            }
            if (ok) {
                verifyCount++;
            }
        }
        EXPECT_EQ(verifyCount, 1) << "Manual rescan found " << verifyCount
                                  << " matches for " << def.name;
    }
}

struct ExpectedSymbol {
    const char* name;
    uint32_t expectedEffective;
};

struct KnownProgram {
    /// Base name of the binary, e.g. "ffl_app"
    const char* basenameContains;
    const ExpectedSymbol* symbols;
    uint32_t symbolCount;
};

// // ---------------------------------------------------------------
// //  Program Symbol Tables
// // ---------------------------------------------------------------

static constexpr std::array cFFLAppSymbols = std::to_array<ExpectedSymbol>({
    { "FFLiVerifyCharInfoWithReason", 0x021c5b74 },
    { "FFLiMiiDataCore2CharInfo__FP12FFLiCharInfoRC15", 0x021d3018 },
    { "FFLiGetHairColor__Fi", 0x021c8ec4 },
    { "FFLiGetBeardColor_Fi", 0x021c8ec4 },
    { "FFLiGetSrgbFetchEyebrowColor__Fi", 0x021c8c1c },
    { "FFLiGetSrgbFetchMustacheColor__Fi", 0x021c8c60 },
    { "FFLiGetSrgbFetchBeardColor__Fi", 0x021c8bd8 },
    { "FFLiGetGlassColor__Fi", 0x021c8ef8 },
    { "FFLiGetFacelineColor__Fi", 0x021c8e90 },
    { "FFLiGetSrgbFetchFacelineColor__Fi", 0x021c8b94 },
    { "FFLiInitModulateEye__FP16FFLModulateParamiT2RC11_GX2Texture", 0x021d4204 },
    { "FFLiInitModulateMouth__FP16FFLModulateParamiRC11_GX2Texture", 0x021d412c }
});

static constexpr std::array cFFLAppJpnV0Symbols = std::to_array<ExpectedSymbol>({
    { "FFLiVerifyCharInfoWithReason", 0x021c270c },
    { "FFLiMiiDataCore2CharInfo__FP12FFLiCharInfoRC15", 0x021cec54 },
    { "FFLiGetHairColor__Fi", 0x021c5a30 },
    { "FFLiGetBeardColor_Fi", 0x021c5a30 },
    { "FFLiGetSrgbFetchEyebrowColor__Fi", 0x021C5788 },
    { "FFLiGetSrgbFetchMustacheColor__Fi", 0x021C57CC },
    { "FFLiGetSrgbFetchBeardColor__Fi", 0x021c5744 },
    { "FFLiGetGlassColor__Fi", 0x021c5a64 },
    { "FFLiGetFacelineColor__Fi", 0x021c59fc },
    { "FFLiGetSrgbFetchFacelineColor__Fi", 0x021C5700 },
    { "FFLiInitModulateEye__FP16FFLModulateParamiT2RC11_GX2Texture", 0x021cf59c },
    { "FFLiInitModulateMouth__FP16FFLModulateParamiRC11_GX2Texture", 0x021cf4c4 }
});


static KnownProgram cKnownPrograms[] = {
    { "ffl_app.", cFFLAppSymbols.data(), cFFLAppSymbols.size() },
    { "ffl_app_jpn_v0", cFFLAppJpnV0Symbols.data(), cFFLAppJpnV0Symbols.size() }
};

// // ---------------------------------------------------------------
// //  Test Entrypoint
// // ---------------------------------------------------------------

TEST_F(SignatureFFLMatchTest, BenchmarkElfs) {
    namespace fs = std::filesystem;
    auto dir     = fs::path("test-elfs");
    if (!fs::exists(dir)) {
        GTEST_SKIP() << "There are no files in test-elfs/";
    }

    for (auto &ent : fs::directory_iterator(dir)) {
        if (!ent.is_regular_file()) {
            continue;
        }

        printf("open elf file: %s\n", ent.path().c_str());
        auto file = loadFile(ent.path().string());

        TextSectionView text{};
        ASSERT_TRUE(findExecText(file, text)) << "No exec .text in " << ent.path();

        // Find known program and set known if it exists.
        std::optional<std::reference_wrapper<const KnownProgram>> known;
        for (const auto& cKnown : cKnownPrograms) {
            const std::string baseName = ent.path().filename().string();
            if (baseName.find(cKnown.basenameContains) == std::string::npos) {
                continue;
            }
            known = cKnown; // set local known
        }

        // Start timer
        auto t0 = high_resolution_clock::now();

        SignatureMatch matches[SIGSCAN_MAX_MATCHES];

        ASSERT_TRUE(text.bytes != nullptr);
        const uintptr_t fileBase = reinterpret_cast<uintptr_t>(text.bytes);
        uint32_t found = scanner->scanModule(
            fileBase, text.size,
            /*matches*/ matches,
            /*cap*/ SIGSCAN_MAX_MATCHES);

        auto t1 = high_resolution_clock::now();
        auto ms = duration_cast<milliseconds>(t1 - t0).count();

        printf("%s: found %u matches in %lld ms\n",
               ent.path().filename().string().c_str(), found, (long long) ms);

        verifySignatureUniqueness(cSignaturesFFL.data(),
            cSignaturesFFL.size(), matches, found, text);

        // Print the matches. Compare the real locations.
        for (uint32_t m = 0; m < found; ++m) {
            const SignatureMatch &match = matches[m];
            const uint32_t fileOffset = static_cast<uint32_t>(match.effectiveAddress - fileBase);
            const uint32_t effectiveAddr = fileOffToEff(fileOffset, text);
            const uint32_t word0 = SignatureScanner::load_be_u32(
                    //const_cast<uint8_t*>(text.bytes + (fileOff + text.shOffset)));
                    reinterpret_cast<uint8_t*>(match.effectiveAddress));
            printf("  - %s: fileOffset=0x%08X effective=0x%08X word0=0x%08X\n",
                   match.pDef->name, fileOffset, effectiveAddr, word0);

            // Find the found symbol in the known table.
            if (!known.has_value()) {
                continue;
            }
            const KnownProgram& k = known.value();
            for (uint32_t i = 0; i < k.symbolCount; ++i) {
                const ExpectedSymbol& sym = k.symbols[i];
                if (strcmp(matches[m].pDef->name, sym.name) != 0) {
                    continue;
                }

                // sym is the matched symbol.
                if (sym.expectedEffective != effectiveAddr) {
                    EXPECT_TRUE(sym.expectedEffective == effectiveAddr)
                        << "Mismatch for " << sym.name << std::endl
                        << "Expected: 0x" << std::hex << sym.expectedEffective
                        << " but got: 0x" << std::hex << effectiveAddr;;
                }
                //ASSERT_EQ(effectiveAddr, sym.expectedEffective)
                //    << "Mismatch for " << sym.name << " in " << sym.name;
            }
        }

        // Each signature should match least once in .text for these targets.
        ASSERT_EQ(found, cSignaturesFFL.size());
    }
}
