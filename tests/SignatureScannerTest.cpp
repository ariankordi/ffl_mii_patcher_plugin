#include "../src/utils/SignatureScanner.h"
#include "../src/patches.h"
#include <chrono>
#include <filesystem>
#include <fstream>
#include <vector>
// clangd says unknown type name for some reason?
#include <gtest/gtest.h>

using namespace std::chrono;

/// Fake OSEffectiveToPhysical for tests.
static uintptr_t mockEffToPhys(uintptr_t eff) { return eff ^ 0xDEADBEEF; }

class SignatureScannerTest : public ::testing::Test {
protected:
    SignatureScanner* scanner;

    void SetUp() override {
        scanner = new SignatureScanner(&cSignatureSetFFL, mockEffToPhys);
    }
    void TearDown() override {
        delete scanner;
    }
};

/// Helper to load an ELF file entirely into memory.
static std::vector<uint8_t> loadFile(const std::string& path) {
    std::ifstream f(path, std::ios::binary);
    return std::vector<uint8_t>((std::istreambuf_iterator<char>(f)),
                                 std::istreambuf_iterator<char>());
}

// Test scanning a synthetic buffer
/*
TEST_F(SignatureScannerTest, FindsSyntheticPattern) {
    // allocate dummy buffer, insert the pattern
    const size_t bufSize = 0x1000;
    std::vector<uint8_t> buf(bufSize, 0);
    uint32_t textBase = 0x100000; // pretend eff addr

    // Copy signature words into buffer at offset 0x100
    for (uint32_t i=0;i<sigs.defs[0].wordCount;i++) {
        uint32_t val = sigs.defs[0].words[i].value;
        size_t off = 0x100 + i*4;
        buf[off+0] = (val>>24)&0xFF;
        buf[off+1] = (val>>16)&0xFF;
        buf[off+2] = (val>>8)&0xFF;
        buf[off+3] = val&0xFF;
    }

    SignatureMatch matches[4];
    uint32_t found = scanner->scanModule(textBase, buf.size(), matches, 4);

    ASSERT_EQ(found, 1u);
    // EXPECT_EQ(matches[0].name, sigs.defs[0].name);
    EXPECT_EQ(matches[0].effectiveAddress, textBase+0x100+(4*4)); // BL target eff decoded
    EXPECT_EQ(matches[0].physicalAddress, (textBase+0x110) ^ 0xDEADBEEF);
}
*/

// Benchmark over directory of ELFs
TEST_F(SignatureScannerTest, BenchmarkElfs) {
    namespace fs = std::filesystem;
    auto dir = fs::path("test-elfs");
    if (!fs::exists(dir)) {
        GTEST_SKIP() << "There are no files in test-elfs/";
    }

    // TODO: Verify the CORRECT locations of these in tables.

    for (auto& ent : fs::directory_iterator(dir)) {
        if (!ent.is_regular_file()) continue;
        auto buf = loadFile(ent.path().string());
        auto t0 = high_resolution_clock::now();

        SignatureMatch matches[SIGSCAN_MAX_MATCHES];

        const uintptr_t textBase = reinterpret_cast<uintptr_t>(buf.data());
        uint32_t found = scanner->scanModule(textBase, buf.size(), matches, SIGSCAN_MAX_MATCHES);

        for (uint32_t m = 0; m < found; ++m) {
            const SignatureMatch& match = matches[m];
            // Get the offset within the file.
            const uint32_t effectiveOffset =
                // Specifically choose to lose the 64-bit precision, this isn't a pointer anymore.
                static_cast<uint32_t>(match.effectiveAddress - textBase);
            printf("function %s: file offset=0x%08X, value=0x%08X\n", match.pDef->name, effectiveOffset,
                SignatureScanner::load_be_u32(reinterpret_cast<uint8_t*>(match.effectiveAddress)));
        }

        auto t1 = high_resolution_clock::now();
        auto ms = duration_cast<milliseconds>(t1 - t0).count();

        printf("%s: found %i matches in %llu ms\n", ent.path().filename().string().c_str(), found, ms);

        // Make sure all signatures are matched.
        ASSERT_EQ(found, cSignatureSetFFL.count);
    }
}
