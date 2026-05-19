// Copyright (c) 2026 The Aumcoin developers
// Distributed under the MIT/X11 software license.
//
// M2.1 Slice 3: hybrid-address bech32m codec.

#include "hybridaddress.h"

#include "bech32.h"
#include "uint256.h"
#include "util.h"   // for fTestNet

#include <cstring>
#include <vector>

namespace HybridAddress {

const char* HRP_MAINNET = "aumq";
const char* HRP_TESTNET = "taumq";

const char* ActiveHRP() {
    return fTestNet ? HRP_TESTNET : HRP_MAINNET;
}

std::string Encode(const CKeyID& id) {
    // CKeyID is uint160 → 20 bytes.
    std::vector<uint8_t> payload(20);
    std::memcpy(payload.data(), &id, 20);

    std::vector<uint8_t> five;
    if (!bech32::ConvertBits(five, 5, payload, 8, true)) return "";

    return bech32::Encode(bech32::Encoding::BECH32M, ActiveHRP(), five);
}

bool Decode(const std::string& str, CKeyID& id_out) {
    bech32::DecodeResult r = bech32::Decode(str);
    if (r.encoding != bech32::Encoding::BECH32M) return false;
    if (r.hrp != ActiveHRP()) return false;

    std::vector<uint8_t> eight;
    if (!bech32::ConvertBits(eight, 8, r.data, 5, false)) return false;
    if (eight.size() != 20) return false;

    std::memcpy(&id_out, eight.data(), 20);
    return true;
}

}  // namespace HybridAddress
