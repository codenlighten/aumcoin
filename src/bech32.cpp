// Copyright (c) 2017, 2021 Pieter Wuille (reference implementation)
// Copyright (c) 2026 The Aumcoin developers
// Distributed under the MIT/X11 software license.
//
// M2.1: Bech32 / Bech32m codec implementation. See bech32.h for the
// API contract and a pointer to BIPs 173 / 350.

#include "bech32.h"

#include <algorithm>
#include <cctype>

namespace bech32 {
namespace {

// BIP 173 character set. Position-in-string = 5-bit value 0..31.
// "1" is the HRP/data separator and is NOT a value-set character.
const char* CHARSET = "qpzry9x8gf2tvdw0s3jn54khce6mua7l";

// Inverse of CHARSET; -1 for non-chars and the separator.
const int8_t CHARSET_REV[128] = {
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    15, -1, 10, 17, 21, 20, 26, 30,  7,  5, -1, -1, -1, -1, -1, -1,
    -1, 29, -1, 24, 13, 25,  9,  8, 23, -1, 18, 22, 31, 27, 19, -1,
     1,  0,  3, 16, 11, 28, 12, 14,  6,  4,  2, -1, -1, -1, -1, -1,
    -1, 29, -1, 24, 13, 25,  9,  8, 23, -1, 18, 22, 31, 27, 19, -1,
     1,  0,  3, 16, 11, 28, 12, 14,  6,  4,  2, -1, -1, -1, -1, -1,
};

// Polymod step (BIP 173 §3).
uint32_t PolymodStep(uint32_t pre) {
    static const uint32_t GEN[5] = {
        0x3b6a57b2UL, 0x26508e6dUL, 0x1ea119faUL, 0x3d4233ddUL, 0x2a1462b3UL,
    };
    uint8_t b = pre >> 25;
    uint32_t chk = ((pre & 0x1ffffffUL) << 5);
    for (int i = 0; i < 5; ++i) {
        if ((b >> i) & 1) chk ^= GEN[i];
    }
    return chk;
}

// Encoding-specific polymod final constant.
uint32_t EncodingConstant(Encoding e) {
    switch (e) {
        case Encoding::BECH32:  return 1u;
        case Encoding::BECH32M: return 0x2bc830a3u;
        default: return 0;
    }
}

// HRP expansion: high-3-bits of each char, separator 0, low-5-bits of each char.
std::vector<uint8_t> ExpandHRP(const std::string& hrp) {
    std::vector<uint8_t> ret;
    ret.reserve(hrp.size() * 2 + 1);
    for (char c : hrp) ret.push_back(static_cast<uint8_t>(c) >> 5);
    ret.push_back(0);
    for (char c : hrp) ret.push_back(static_cast<uint8_t>(c) & 0x1f);
    return ret;
}

uint32_t VerifyChecksum(const std::string& hrp,
                        const std::vector<uint8_t>& values) {
    uint32_t chk = 1;
    for (uint8_t v : ExpandHRP(hrp)) chk = PolymodStep(chk) ^ v;
    for (uint8_t v : values) chk = PolymodStep(chk) ^ v;
    return chk;
}

std::vector<uint8_t> CreateChecksum(Encoding e, const std::string& hrp,
                                    const std::vector<uint8_t>& values) {
    uint32_t chk = 1;
    for (uint8_t v : ExpandHRP(hrp)) chk = PolymodStep(chk) ^ v;
    for (uint8_t v : values) chk = PolymodStep(chk) ^ v;
    for (int i = 0; i < 6; ++i) chk = PolymodStep(chk);
    chk ^= EncodingConstant(e);
    std::vector<uint8_t> ret(6);
    for (size_t i = 0; i < 6; ++i)
        ret[i] = (chk >> (5 * (5 - i))) & 0x1f;
    return ret;
}

}  // namespace

std::string Encode(Encoding encoding, const std::string& hrp,
                   const std::vector<uint8_t>& values) {
    if (encoding == Encoding::INVALID) return "";
    if (hrp.empty() || hrp.size() > 83) return "";
    // BIP 173 caps total length at 90.
    if (hrp.size() + 1 + values.size() + 6 > 90) return "";
    // HRP must be lowercase 33..126 ASCII.
    for (char c : hrp) {
        if (c < 33 || c > 126) return "";
        if (c >= 'A' && c <= 'Z') return "";  // require lowercase
    }
    for (uint8_t v : values) if (v >= 32) return "";

    std::vector<uint8_t> checksum = CreateChecksum(encoding, hrp, values);
    std::string ret = hrp + "1";
    ret.reserve(ret.size() + values.size() + checksum.size());
    for (uint8_t v : values) ret += CHARSET[v];
    for (uint8_t v : checksum) ret += CHARSET[v];
    return ret;
}

DecodeResult Decode(const std::string& str) {
    DecodeResult fail{Encoding::INVALID, "", {}};
    if (str.size() > 90 || str.size() < 8) return fail;

    bool has_lower = false, has_upper = false;
    for (char c : str) {
        if (c < 33 || c > 126) return fail;
        if (c >= 'a' && c <= 'z') has_lower = true;
        if (c >= 'A' && c <= 'Z') has_upper = true;
    }
    if (has_lower && has_upper) return fail;  // mixed case banned by BIP 173

    std::string lower;
    lower.reserve(str.size());
    for (char c : str)
        lower += (c >= 'A' && c <= 'Z') ? static_cast<char>(c + 32) : c;

    // Find the LAST '1' — HRP can contain neither '1' nor data-set
    // characters that would also appear as data, so this is unambiguous.
    auto sep = lower.rfind('1');
    if (sep == std::string::npos || sep == 0 ||
        sep + 7 > lower.size()) {
        return fail;
    }

    std::string hrp = lower.substr(0, sep);
    std::vector<uint8_t> data;
    data.reserve(lower.size() - sep - 1);
    for (size_t i = sep + 1; i < lower.size(); ++i) {
        int8_t v = (static_cast<unsigned char>(lower[i]) < 128)
                       ? CHARSET_REV[static_cast<unsigned char>(lower[i])]
                       : -1;
        if (v < 0) return fail;
        data.push_back(static_cast<uint8_t>(v));
    }

    uint32_t chk = VerifyChecksum(hrp, data);
    Encoding e = Encoding::INVALID;
    if (chk == EncodingConstant(Encoding::BECH32))  e = Encoding::BECH32;
    if (chk == EncodingConstant(Encoding::BECH32M)) e = Encoding::BECH32M;
    if (e == Encoding::INVALID) return fail;

    // Strip the 6-byte checksum from the returned data.
    data.resize(data.size() - 6);
    return {e, hrp, data};
}

bool ConvertBits(std::vector<uint8_t>& out, int outbits,
                 const std::vector<uint8_t>& in, int inbits, bool pad) {
    int acc = 0;
    int bits = 0;
    const int maxv = (1 << outbits) - 1;
    const int max_acc = (1 << (inbits + outbits - 1)) - 1;
    for (uint8_t v : in) {
        if ((v >> inbits) != 0) return false;
        acc = ((acc << inbits) | v) & max_acc;
        bits += inbits;
        while (bits >= outbits) {
            bits -= outbits;
            out.push_back((acc >> bits) & maxv);
        }
    }
    if (pad) {
        if (bits) out.push_back((acc << (outbits - bits)) & maxv);
    } else if (bits >= inbits || ((acc << (outbits - bits)) & maxv)) {
        return false;
    }
    return true;
}

}  // namespace bech32
