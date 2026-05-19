// Copyright (c) 2026 The Aumcoin developers
// Distributed under the MIT/X11 software license.
//
// M2.1: CHybridPubKey — wallet data structure for the quantum-safe
// address format. Holds both the ECDSA pubkey (33 bytes, compressed
// secp256k1) and the ML-DSA-65 pubkey (1952 bytes, FIPS 204) AND
// commits to both via a single 160-bit hash:
//
//   GetID() = Hash160(0x01 || ecdsa_pub || mldsa_pub)
//
// The leading 0x01 algorithm-version byte is the M2.5 plan's
// forward-compatibility hook: future PQ algorithm bundles (ML-DSA-87,
// SLH-DSA, etc.) get distinct algo-id bytes and therefore distinct
// address namespaces by construction.
//
// CHybridPubKey is INTENTIONALLY a separate class from CPubKey.
// CPubKey::GetID() returns Hash160(ecdsa_pub) — the legacy P2PKH
// address. Mixing the two semantics inside one class is what created
// the security gap the original code review flagged. Keeping them
// distinct means a hybrid spend cannot accidentally hash to a legacy
// address (or vice versa).
//
// This commit lands the data structure only. Wiring CHybridPubKey
// into Solver / ExtractDestination / address encoding (bech32m
// "aumq1...") / the CheckSig consensus rule lives in subsequent
// M2.1 / M2.2 PRs.

#ifndef H_AUMCOIN_HYBRIDPUBKEY
#define H_AUMCOIN_HYBRIDPUBKEY

#include <cstdint>
#include <vector>

#include "key.h"           // for CPubKey, CKeyID
#include "serialize.h"
#include "uint256.h"

#ifdef ENABLE_MLDSA
#include "crypto/mldsa.h"  // for MLDSA::PUBLIC_KEY_BYTES
#endif

#ifdef ENABLE_MLDSA

/** Algorithm-version byte for the hybrid pubkey format.
 *  0x01 = secp256k1-ECDSA + ML-DSA-65 (the current and only deployed
 *         bundle). Future bundles get distinct values.
 *  Embedded as the first byte of the hashed preimage; binds the
 *  address to a specific PQ-algorithm tuple. */
static const uint8_t HYBRID_ALGO_ID_ECDSA_MLDSA65 = 0x01;

class CHybridPubKey {
private:
    std::vector<unsigned char> m_ecdsa;   // 33-byte compressed secp256k1 pubkey
    std::vector<unsigned char> m_mldsa;   // 1952-byte ML-DSA-65 pubkey
    uint8_t m_algo_id;                    // currently always HYBRID_ALGO_ID_ECDSA_MLDSA65

public:
    /** Default-constructed CHybridPubKey is INVALID until populated. */
    CHybridPubKey() : m_algo_id(HYBRID_ALGO_ID_ECDSA_MLDSA65) {}

    /** Construct from raw ECDSA + ML-DSA pubkeys. The constructor does
     *  NOT validate the sizes — IsValid() reports that. */
    CHybridPubKey(const std::vector<unsigned char>& ecdsa,
                  const std::vector<unsigned char>& mldsa,
                  uint8_t algo_id = HYBRID_ALGO_ID_ECDSA_MLDSA65)
        : m_ecdsa(ecdsa), m_mldsa(mldsa), m_algo_id(algo_id) {}

    /** True iff sizes match the algorithm-id's expected layout AND the
     *  algorithm-id is one we know about. */
    bool IsValid() const {
        if (m_algo_id != HYBRID_ALGO_ID_ECDSA_MLDSA65) return false;
        if (m_ecdsa.size() != 33) return false;        // require compressed
        if (m_mldsa.size() != MLDSA::PUBLIC_KEY_BYTES) return false;
        return true;
    }

    /** Address commitment: Hash160(algo_id || ecdsa || mldsa).
     *  Distinct from CPubKey::GetID() which hashes only the ECDSA
     *  pubkey — that's the legacy P2PKH address. Mixing the two
     *  hashes is consensus-relevant and intentional: a legacy spend
     *  cannot hash to a hybrid address and vice versa. */
    CKeyID GetID() const;

    /** Raw byte vector for the script-stack push:
     *  [1B algo_id][33B ecdsa_pub][1952B mldsa_pub] = 1986 bytes.
     *  This is what EvalScript will see on the stack at OP_CHECKSIG
     *  time for hybrid outputs (M2.2 dispatches on length). */
    std::vector<unsigned char> Serialize() const;

    /** Parse a 1986-byte stack blob back into a CHybridPubKey.
     *  Returns false on wrong size or unknown algo_id. */
    static bool Parse(const std::vector<unsigned char>& vch,
                      CHybridPubKey& out);

    const std::vector<unsigned char>& GetECDSAPubKey() const { return m_ecdsa; }
    const std::vector<unsigned char>& GetMLDSAPubKey() const { return m_mldsa; }
    uint8_t GetAlgoId() const { return m_algo_id; }

    friend bool operator==(const CHybridPubKey& a, const CHybridPubKey& b) {
        return a.m_algo_id == b.m_algo_id
            && a.m_ecdsa == b.m_ecdsa
            && a.m_mldsa == b.m_mldsa;
    }
    friend bool operator!=(const CHybridPubKey& a, const CHybridPubKey& b) {
        return !(a == b);
    }
};

#endif  // ENABLE_MLDSA

#endif  // H_AUMCOIN_HYBRIDPUBKEY
