#include "SignatureScanner.h"
#include <cassert>
#include <cstdint>

// Memory map helper from coreinit for physical conversion.
#if __WIIU__
    #include <coreinit/memorymap.h>
    // extern "C" uint32_t OSEffectiveToPhysical(uint32_t addr);
#endif

SignatureScanner::SignatureScanner(
    const SignatureDefinition* list,
    uint32_t signatureCount,
    ToPhysicalFunction toPhysicalFunction)
: mSignatureList(list),
  mSignatureCount(signatureCount),
  mMaxSigWords(0),
  mEffToPhys(toPhysicalFunction) {

    if (!mEffToPhys) {
#if defined(__WIIU__)
        mEffToPhys = OSEffectiveToPhysical;
#else
        // For host testing, set to identity function.
        mEffToPhys = [](uintptr_t x){ return x; };
#endif
    }

    if (!mSignatureList || !mSignatureCount) {
        return;
    }
    for (uint32_t i = 0; i < mSignatureCount; ++i) {
        if (mSignatureList[i].wordCount > mMaxSigWords) {
            mMaxSigWords = mSignatureList[i].wordCount;
        }
    }
}

bool SignatureScanner::tryMatchAt(uintptr_t curEff, const SignatureDefinition& sig) const {
    // Compare words forward; all words must fit in range by caller.
    const uint8_t* base = reinterpret_cast<const uint8_t*>(curEff);
    for (uint32_t w = 0; w < sig.wordCount; ++w) {
        uint32_t got = load_be_u32(base + (w << 2));

        /*
        if (w == sig.wordCount - 1) {
            // Special comparison with mask for last word.
            if ((got & sig.lastWordMask) != (sig.words[w] & sig.lastWordMask)) {
                return false;
            }
        } else {
            // Compare full word.
            if (got != sig.words[w]) {
                return false;
            }
        }
        */
        uint32_t need = sig.words[w].value;
        uint32_t mask = sig.words[w].mask;
        // masked compare: only bits set in mask are compared
        if (((got ^ need) & mask) != 0) {
            return false;
        }
    }
    return true;
}

bool SignatureScanner::decodeBLTarget(uintptr_t blEffAddr, uintptr_t& outTargetEff) {
    // BL/B encoding: bits 31..26 opcode=18 (0b010010), bits 25..2 LI (signed), bit1 AA, bit0 LK
    const uint8_t* p = reinterpret_cast<const uint8_t*>(blEffAddr);
    uint32_t instr = load_be_u32(p);

    // Check top 6 bits = 18 (0x12)
    if ( ((instr >> 26) & 0x3F) != 0x12 ) {
        return false; // not a branch
    }
    // We expect LK=1 for BL (link)
    if ( (instr & 0x1) != 0x1 ) {
        return false; // not BL (likely B)
    }

    uint32_t LI = (instr >> 2) & 0x00FFFFFF;     // 24-bit signed
    if (LI & 0x00800000) LI |= 0xFF000000;      // sign-extend
    uint32_t offset = (LI << 2);                 // byte offset
    outTargetEff = blEffAddr + offset;
    return true;
}

bool SignatureScanner::walkBackToPrologue(uintptr_t anyInstrEff,
                                          uintptr_t textBase,
                                          uintptr_t textEnd,
                                          uintptr_t& outStartEff) {
    // Very simple heuristic: look backward for:
    //   mfspr r0, LR  (0x7C0802A6)
    //   stwu  r1, -imm(r1)  (0x9421xxxx)
    // within a reasonable window (e.g., 32 instructions).
    constexpr uint32_t PROLOGUE_MFSPR_LR  = 0x7C0802A6;
    constexpr uint32_t PROLOGUE_STWU_MASK = 0xFFFF0000;
    constexpr uint32_t PROLOGUE_STWU_VAL  = 0x94210000;

    uintptr_t start = anyInstrEff;
    if (start < textBase) {
        start = textBase;
    }

    // Scan up to 32 instructions back.
    constexpr int maxBack = 32;

/*
    for (int i = 0; i < maxBack; ++i) {
        uintptr_t addr = anyInstrEff - (uint32_t(i) << 2);
        if (addr < textBase) {
            break;
        }

        uint32_t w0 = load_be_u32(reinterpret_cast<const uint8_t*>(addr));
        if (w0 == PROLOGUE_MFSPR_LR) {
            // Next instruction should be stwu r1,-X(r1).
            uintptr_t n1Addr = addr + 4;
            if (n1Addr + 4 <= textEnd) {
                uint32_t w1 = load_be_u32(reinterpret_cast<const uint8_t*>(n1Addr));
                if ( (w1 & PROLOGUE_STWU_MASK) == PROLOGUE_STWU_VAL ) {
                    outStartEff = addr;
                    return true;
                }
            }
        }
    }
*/
    // Accept either order: (mfspr then stwu) OR (stwu then mfspr).
    for (int i = 0; i < maxBack; ++i) {
        uintptr_t addr = anyInstrEff - (uint32_t(i) << 2);
        if (addr < textBase) break;

        uint32_t insn = load_be_u32(reinterpret_cast<const uint8_t*>(addr));

        // Case A: mfspr then stwu
        if (insn == PROLOGUE_MFSPR_LR) {
            uint32_t next = load_be_u32(reinterpret_cast<const uint8_t*>(addr + 4));
            if ((next & PROLOGUE_STWU_MASK) == PROLOGUE_STWU_VAL) {
                outStartEff = addr;
                return true;
            }
        }

        // Case B: stwu then (optionally stmw) then mfspr
        if ((insn & PROLOGUE_STWU_MASK) == PROLOGUE_STWU_VAL) {
            uint32_t next = load_be_u32(reinterpret_cast<const uint8_t*>(addr + 4));
            // Allow either direct mfspr, or stmw then mfspr.
            if (next == PROLOGUE_MFSPR_LR) {
                outStartEff = addr;
                return true;
            }
            if ((next & 0xFC000000) == 0xBC000000) { // stmw opcode
                uint32_t next2 = load_be_u32(reinterpret_cast<const uint8_t*>(addr + 8));
                if (next2 == PROLOGUE_MFSPR_LR) {
                    outStartEff = addr;
                    return true;
                }
            }
        }
    }

    // Fallback: fail (caller can choose to use hitEff in this case)
    return false;
}

bool SignatureScanner::resolveHit(uintptr_t hitEff,
                                  const SignatureDefinition& sig,
                                  uintptr_t textBase,
                                  uintptr_t textEnd,
                                  uintptr_t& outEff) const {
    switch (sig.resolveMode) {
        case SignatureResolveMode::Direct:
            outEff = hitEff;
            return true;
        case SignatureResolveMode::BranchTarget: {
            // BL is inside the pattern at branchWordIndex
            uintptr_t blEff = hitEff + (sig.branchWordIndex << 2);
            return decodeBLTarget(blEff, outEff);
        }
        case SignatureResolveMode::FunctionStart: {
            uintptr_t startEff = 0;
            if (walkBackToPrologue(hitEff, textBase, textEnd, startEff)) {
                outEff = startEff;
                return true;
            }
            // fallback
            outEff = hitEff;
            return true;
        }
        default:
            return false;
    }
}

uint32_t SignatureScanner::scanModule(uintptr_t textBase,
                                      size_t textSize,
                                      SignatureMatch* outMatches,
                                      uint32_t maxMatches) const {
    if (!mSignatureList || mSignatureCount == 0 ||
        !textBase || textSize < 4 ||
        !outMatches || maxMatches == 0) {
        return 0;
    }

    const uintptr_t textEnd = textBase + textSize;
    uint32_t found = 0;

    // Advance 4 bytes at a time (PPC instruction size).
    // Single pass. For each offset, try each signature whose length fits.
    for (uintptr_t cur = textBase; cur + (mMaxSigWords << 2) <= textEnd; cur += 4) {
        for (uint32_t s = 0; s < mSignatureCount; ++s) {
            const SignatureDefinition& sig = mSignatureList[s];
            const uint32_t patBytes = (sig.wordCount << 2);
            if (cur + patBytes > textEnd) {
                continue;
            }

            // Quick anchor check on last word to minimize work:
            const uintptr_t lastOff = cur + ((sig.wordCount - 1) << 2);
            const uint32_t gotLast = load_be_u32(reinterpret_cast<const uint8_t*>(lastOff));
            const uint32_t needLast = sig.words[sig.wordCount - 1].value;
            const uint32_t maskLast = sig.words[sig.wordCount - 1].mask;
            // const uint32_t maskLast = sig.lastWordMask; // .mask;
            if (((gotLast ^ needLast) & maskLast) != 0) {
                continue;
            }

            // Full masked compare:
            if (!tryMatchAt(cur, sig)) {
                continue;
            }

            // Resolve final function entry effective address:
            uintptr_t resolvedEff = 0;
            if (!resolveHit(cur, sig, textBase, textEnd, resolvedEff)) {
                continue;
            }

            // Compute physical:
            assert(mEffToPhys != nullptr);
            uintptr_t phys = mEffToPhys(resolvedEff);
            if (!phys) {
                continue;
            }

            // Write result.
            outMatches[found].pDef             = &sig;
            outMatches[found].effectiveAddress = resolvedEff;
            outMatches[found].physicalAddress  = phys;
            if (++found >= maxMatches) {
                return found;
            }
        }
    }
    return found;
}
