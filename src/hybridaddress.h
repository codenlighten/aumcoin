// Copyright (c) 2026 The Aumcoin developers
// Distributed under the MIT/X11 software license.
//
// M2.1 Slice 3: encode/decode the new quantum-safe address format.
//
//   <network HRP> '1' <bech32m-encoded 20-byte CKeyID> <6-char checksum>
//
// Mainnet HRP is "aumq" (Aumcoin Quantum). Testnet is "taumq".
// The payload is the 20-byte CKeyID returned by CHybridPubKey::GetID()
// — i.e. Hash160(algo_id || ecdsa_pub || mldsa_pub).
//
// This module is the codec layer only. Wiring it into Solver /
// ExtractDestination / wallet RPCs lives in subsequent M2.1 / M2.2 slices.
//
// Design choices:
//   - Separate from CBitcoinAddress (which stays base58 for legacy
//     P2PKH / P2SH). Mixing bech32m payloads into CBase58Data would
//     have to special-case at every call site; cleaner to introduce
//     a sibling type and let callers branch on which one to use.
//   - No witness version byte. Unlike BIP 350 §3 (which prefixes a
//     witness version 0..16 before the program), aumcoin's hybrid
//     address is a flat 20-byte hash. The algorithm-version byte
//     (0x01 = ECDSA + ML-DSA-65) is committed INSIDE the Hash160
//     preimage via CHybridPubKey::Serialize(), not exposed in the
//     bech32m address. Future algo bumps get a new HRP if/when they
//     ever need a distinct address space externally (M2.6).

#ifndef H_AUMCOIN_HYBRIDADDRESS
#define H_AUMCOIN_HYBRIDADDRESS

#include "key.h"   // for CKeyID

#include <string>

namespace HybridAddress {

/** HRPs. Mainnet is "aumq", testnet is "taumq". */
extern const char* HRP_MAINNET;
extern const char* HRP_TESTNET;

/** Pick the active HRP based on the global fTestNet flag (matches
 *  the convention already used by CBitcoinAddress). */
const char* ActiveHRP();

/** Encode a 20-byte CKeyID as a bech32m string.
 *  Returns empty string on failure. */
std::string Encode(const CKeyID& id);

/** Decode a bech32m hybrid-address string into a CKeyID.
 *  Returns false on:
 *    - bech32m decode failure (bad checksum, mixed case, …)
 *    - wrong HRP for the active network
 *    - payload size != 20 bytes after 5-bit → 8-bit unpacking
 *    - input was bech32 (not bech32m) — must match exactly. */
bool Decode(const std::string& str, CKeyID& id_out);

}  // namespace HybridAddress

#endif  // H_AUMCOIN_HYBRIDADDRESS
