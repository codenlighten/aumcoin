// Copyright (c) 2017, 2021 Pieter Wuille (reference implementation)
// Copyright (c) 2026 The Aumcoin developers
// Distributed under the MIT/X11 software license.
//
// M2.1: Bech32 / Bech32m codec for the new quantum-safe address format.
//
// BIP 173: Bech32 (constant 1).
// BIP 350: Bech32m (constant 0x2bc830a3) — fixes a Bech32 mutation flaw and
//          is what we use for new aumcoin hybrid addresses.
//
// This is the data-layer codec only. Higher-level address packing
// (converting 8-bit pubkey-hash bytes ↔ 5-bit groups, prefixing the
// witness version, etc.) lives in src/base58.h / address-handling code.

#ifndef H_AUMCOIN_BECH32
#define H_AUMCOIN_BECH32

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace bech32 {

/** Encoding family. The two differ only in the polymod constant — same
 *  character set, same expansion rules. Mixing them across decode is a
 *  hard error (BIP 350 §3). */
enum class Encoding {
    INVALID,
    BECH32,    //!< BIP 173 (constant 1) — for legacy Segwit V0
    BECH32M,   //!< BIP 350 (constant 0x2bc830a3) — what we use for hybrid addresses
};

/** Encode an HRP + 5-bit data payload into a Bech32(m) string.
 *  Returns empty string on failure (e.g. payload size + HRP > 90 chars).
 *  The data vector contains 5-bit values (0..31); the caller is
 *  responsible for converting 8-bit pubkey-hash bytes via ConvertBits
 *  before passing them in. */
std::string Encode(Encoding encoding, const std::string& hrp,
                   const std::vector<uint8_t>& values);

/** Decode a Bech32(m) string. Returns {Encoding, hrp, 5-bit data}.
 *  Encoding is INVALID on failure (bad checksum, mixed case, oversize,
 *  out-of-range chars, missing separator, empty HRP). */
struct DecodeResult {
    Encoding encoding;
    std::string hrp;
    std::vector<uint8_t> data;
};
DecodeResult Decode(const std::string& str);

/** Convert between different bit-widths (e.g. 8-bit raw bytes → 5-bit
 *  groups for the Bech32 data section, and back). `pad` controls
 *  whether to pad the final partial group with zero bits — must be
 *  true when encoding, false when decoding.
 *  Returns false if `pad` is false and the input doesn't divide evenly. */
bool ConvertBits(std::vector<uint8_t>& out, int outbits,
                 const std::vector<uint8_t>& in, int inbits, bool pad);

}  // namespace bech32

#endif  // H_AUMCOIN_BECH32
