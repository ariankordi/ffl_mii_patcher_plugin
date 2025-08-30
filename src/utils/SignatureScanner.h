#pragma once
#include <cstdint>
#include <cstddef>
#include <cstring>

/// Maximum limits to avoid dynamic allocation.
#define SIGSCAN_MAX_SIGNATURES  16
#define SIGSCAN_MAX_WORDS       16
#define SIGSCAN_MAX_MATCHES     32

/// How to resolve a pattern hit to the actual function entry.
enum SignatureResolveMode {
    Direct = 0,      ///< The match start is the entrypoint.
    BranchTarget,    ///< Pattern contains a BL; use its branch target as entry.
    FunctionStart    ///< Pattern is inside function; walk back to prologue.
};

/// One 32-bit word of a signature (value + mask).
struct SignatureWord {
    uint32_t value;  ///< Bits to match after masking.
    uint32_t mask;   ///< 1-bits = compare; 0-bits = wildcard.
};

/// Static description of one signature.
struct SignatureDefinition {
    const char*          name;                      ///< Descriptive name for logs.
    /// Optional information on how to use this definition.
    /// This is assumed to be of type function_replacement_data_t for libfunctionpatcher.
    void*                pHookInfo;   ///< Optional hook info (e.g., pointer to prebuilt replacement desc)
    // uint32_t             words[SIGSCAN_MAX_WORDS];  ///< 32-bit pattern words.
    SignatureWord        words[SIGSCAN_MAX_WORDS];  ///< Pattern words.
    uint32_t             wordCount;                 ///< Number of valid words.
    SignatureResolveMode resolveMode;               ///< How to compute function entry.
    uint32_t             branchWordIndex;           ///< Index of BL inside words[] (if used).
    // uint32_t             lastWordMask;              ///< Mask to apply only on the last word.
};

/// Result of resolving a signature.
struct SignatureMatch {
    const SignatureDefinition* pDef;  ///< SignatureDefinition that was matched.
    uintptr_t   effectiveAddress;     ///< Final resolved effective/virtual address.
    uintptr_t   physicalAddress;      ///< Physical address via OSEffectiveToPhysical.
};

typedef uintptr_t (*ToPhysicalFunction)(uintptr_t);

/// Scanner class for locating function entrypoints in modules.
/// Designed for PowerPC-specific code.
class SignatureScanner {
public:
    /// Construct with a list of signatures.
    explicit SignatureScanner(
        const SignatureDefinition* list,
        uint32_t signatureCount,
        ToPhysicalFunction toPhysicalFunction = nullptr
    );

    /**
     * @brief Scan one module's .text range once and resolve all known signatures.
     * @param textBase   Effective base address of .text (must be 4-byte aligned).
     * @param textSize   Size in bytes of .text (multiple of 4 recommended).
     * @param outMatches Caller-provided array to receive matches.
     * @param maxMatches Capacity of outMatches (SIGSCAN_MAX_MATCHES is typical).
     * @return Number of matches written to outMatches.
     */
    uint32_t scanModule(uintptr_t textBase,
                        size_t textSize,
                        SignatureMatch* pOutMatches,
                        uint32_t maxMatches) const;

    /// Helper to load a big-endian u32 value.
    /// This should get optimized to an actual word load on PPC.
    static inline uint32_t load_be_u32(const uint8_t* p) { // Used to be private
        return (uint32_t(p[0]) << 24) | (uint32_t(p[1]) << 16) |
               (uint32_t(p[2]) <<  8) |  uint32_t(p[3]);
    }
private:
    const SignatureDefinition* mSignatureList;
    const uint32_t             mSignatureCount;
    uint32_t                   mMaxSigWords; ///< Max wordCount across all signatures.
    /// Pointer for function to convert effective to physical addresses.
    ToPhysicalFunction         mEffToPhys;

    /// Decode a BL instruction and compute branch target.
    static bool decodeBLTarget(uintptr_t instrEffAddr, uintptr_t& outTargetEff);
    /// Take an address and compute the prologue/function start.
    static bool walkBackToPrologue(uintptr_t anyInstrEff, uintptr_t textBase, uintptr_t textEnd, uintptr_t& outStartEff);

    bool tryMatchAt(uintptr_t curEff, const SignatureDefinition& sig) const;
    bool resolveHit(uintptr_t hitEff, const SignatureDefinition& sig, uintptr_t textBase, uintptr_t textEnd, uintptr_t& outEff) const;
};
