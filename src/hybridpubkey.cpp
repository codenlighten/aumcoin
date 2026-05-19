// Copyright (c) 2026 The Aumcoin developers
// Distributed under the MIT/X11 software license.
//
// M2.1: CHybridPubKey implementation. See hybridpubkey.h for the rationale.

#include "hybridpubkey.h"

#ifdef ENABLE_MLDSA

#include "util.h"  // for Hash160

std::vector<unsigned char> CHybridPubKey::Serialize() const {
    std::vector<unsigned char> out;
    out.reserve(1 + m_ecdsa.size() + m_mldsa.size());
    out.push_back(m_algo_id);
    out.insert(out.end(), m_ecdsa.begin(), m_ecdsa.end());
    out.insert(out.end(), m_mldsa.begin(), m_mldsa.end());
    return out;
}

CKeyID CHybridPubKey::GetID() const {
    // Hash160(algo_id || ecdsa || mldsa). The algo_id prefix is the
    // forward-compatibility hook (M2.5 / M2.6): different algorithm
    // bundles produce different address namespaces by construction,
    // so a future ML-DSA-87 address can never collide with today's
    // ML-DSA-65 address even if every other component is identical.
    return CKeyID(Hash160(Serialize()));
}

bool CHybridPubKey::Parse(const std::vector<unsigned char>& vch,
                          CHybridPubKey& out) {
    // Currently only the 0x01 (secp256k1-ECDSA + ML-DSA-65) layout is
    // defined. Length = 1 + 33 + 1952 = 1986.
    static const size_t kExpectedLen = 1 + 33 + MLDSA::PUBLIC_KEY_BYTES;
    if (vch.size() != kExpectedLen) return false;
    if (vch[0] != HYBRID_ALGO_ID_ECDSA_MLDSA65) return false;

    out.m_algo_id = vch[0];
    out.m_ecdsa.assign(vch.begin() + 1, vch.begin() + 1 + 33);
    out.m_mldsa.assign(vch.begin() + 1 + 33, vch.end());
    return true;
}

#endif  // ENABLE_MLDSA
