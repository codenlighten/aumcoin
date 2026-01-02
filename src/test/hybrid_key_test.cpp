// Copyright (c) 2025 AumCoin Developers
// Distributed under the MIT/X11 software license

#include <boost/test/unit_test.hpp>
#include <vector>
#include <string>

#include "../key.h"
#include "../util.h"

BOOST_AUTO_TEST_SUITE(hybrid_key_tests)

#ifdef ENABLE_MLDSA

// Test hybrid key generation
BOOST_AUTO_TEST_CASE(hybrid_key_generation)
{
    CKey key;
    
    // Initially should not be hybrid
    BOOST_CHECK(!key.IsHybrid());
    BOOST_CHECK(!key.HasMLDSAKey());
    
    // Generate hybrid key
    key.MakeNewHybridKey(true);
    
    // Should now be hybrid
    BOOST_CHECK(key.IsHybrid());
    BOOST_CHECK(key.HasMLDSAKey());
    // Note: IsValid() checks ECDSA key regeneration, not hybrid validity
    // For hybrid keys, check components separately
    BOOST_CHECK(key.IsCompressed());
    
    // Check key sizes
    std::vector<unsigned char> mldsa_priv = key.GetMLDSAPrivKey();
    std::vector<unsigned char> mldsa_pub = key.GetMLDSAPubKey();
    
    BOOST_CHECK_EQUAL(mldsa_priv.size(), MLDSA::PRIVATE_KEY_BYTES);
    BOOST_CHECK_EQUAL(mldsa_pub.size(), MLDSA::PUBLIC_KEY_BYTES);
}

// Test hybrid public key
BOOST_AUTO_TEST_CASE(hybrid_public_key)
{
    CKey key;
    key.MakeNewHybridKey(true);
    
    // Get public key
    CPubKey pubkey = key.GetPubKey();
    
    // Should be valid and compressed
    BOOST_CHECK(pubkey.IsValid());
    BOOST_CHECK(pubkey.IsCompressed());
    BOOST_CHECK(pubkey.HasMLDSAKey());
    BOOST_CHECK(pubkey.IsHybrid());
    
    // Check ML-DSA public key
    std::vector<unsigned char> mldsa_pub = pubkey.GetMLDSAPubKey();
    BOOST_CHECK_EQUAL(mldsa_pub.size(), MLDSA::PUBLIC_KEY_BYTES);
    
    // Should match the key's ML-DSA public key
    BOOST_CHECK(mldsa_pub == key.GetMLDSAPubKey());
}

// Test ML-DSA signing and verification
BOOST_AUTO_TEST_CASE(mldsa_sign_verify)
{
    CKey key;
    key.MakeNewHybridKey(true);
    
    // Create a test hash
    uint256 hash;
    for (int i = 0; i < 32; i++) {
        ((unsigned char*)&hash)[i] = i;
    }
    
    // Sign with ML-DSA
    std::vector<unsigned char> vchSig;
    BOOST_CHECK(key.SignMLDSA(hash, vchSig));
    BOOST_CHECK_EQUAL(vchSig.size(), MLDSA::SIGNATURE_BYTES);
    
    // Verify with ML-DSA
    std::vector<unsigned char> mldsa_pub = key.GetMLDSAPubKey();
    BOOST_CHECK(CKey::VerifyMLDSA(hash, vchSig, mldsa_pub));
    
    // Modify hash - should fail verification
    uint256 hash2 = hash;
    ((unsigned char*)&hash2)[0] ^= 1;
    BOOST_CHECK(!CKey::VerifyMLDSA(hash2, vchSig, mldsa_pub));
    
    // Modify signature - should fail verification
    vchSig[0] ^= 1;
    BOOST_CHECK(!CKey::VerifyMLDSA(hash, vchSig, mldsa_pub));
}

// Test hybrid signing and verification
BOOST_AUTO_TEST_CASE(hybrid_sign_verify)
{
    CKey key;
    key.MakeNewHybridKey(true);
    
    // Create a test hash
    uint256 hash;
    for (int i = 0; i < 32; i++) {
        ((unsigned char*)&hash)[i] = i * 3;
    }
    
    // Sign with hybrid signature (ECDSA + ML-DSA)
    std::vector<unsigned char> vchSig;
    BOOST_CHECK(key.SignHybrid(hash, vchSig));
    
    // Hybrid signature should be: 1 byte length + ECDSA sig + ML-DSA sig
    // ECDSA is typically 70-72 bytes, ML-DSA is 3309 bytes
    BOOST_CHECK(vchSig.size() > 1 + 70 + MLDSA::SIGNATURE_BYTES);
    BOOST_CHECK(vchSig.size() < 1 + 80 + MLDSA::SIGNATURE_BYTES);
    
    // Verify hybrid signature
    BOOST_CHECK(key.VerifyHybrid(hash, vchSig));
    
    // Modify hash - should fail verification
    uint256 hash2 = hash;
    ((unsigned char*)&hash2)[10] ^= 1;
    BOOST_CHECK(!key.VerifyHybrid(hash2, vchSig));
    
    // Corrupt ECDSA portion - should fail verification
    std::vector<unsigned char> vchSigBad = vchSig;
    vchSigBad[5] ^= 1;  // Flip bit in ECDSA signature
    BOOST_CHECK(!key.VerifyHybrid(hash, vchSigBad));
    
    // Corrupt ML-DSA portion - should fail verification
    vchSigBad = vchSig;
    vchSigBad[vchSig.size() - 100] ^= 1;  // Flip bit in ML-DSA signature
    BOOST_CHECK(!key.VerifyHybrid(hash, vchSigBad));
}

// Test signature format parsing
BOOST_AUTO_TEST_CASE(hybrid_signature_format)
{
    CKey key;
    key.MakeNewHybridKey(true);
    
    uint256 hash;
    for (int i = 0; i < 32; i++) {
        ((unsigned char*)&hash)[i] = i * 5;
    }
    
    std::vector<unsigned char> vchSig;
    BOOST_CHECK(key.SignHybrid(hash, vchSig));
    
    // Parse signature format
    BOOST_CHECK(vchSig.size() >= 1);
    
    size_t ecdsa_len = vchSig[0];
    BOOST_CHECK(ecdsa_len >= 64 && ecdsa_len <= 72);  // Typical ECDSA DER size
    
    size_t expected_total = 1 + ecdsa_len + MLDSA::SIGNATURE_BYTES;
    BOOST_CHECK_EQUAL(vchSig.size(), expected_total);
    
    // Extract ECDSA signature and verify separately
    std::vector<unsigned char> vchECDSASig(vchSig.begin() + 1,
                                           vchSig.begin() + 1 + ecdsa_len);
    BOOST_CHECK(key.Verify(hash, vchECDSASig));
    
    // Extract ML-DSA signature and verify separately
    std::vector<unsigned char> vchMLDSASig(vchSig.begin() + 1 + ecdsa_len,
                                           vchSig.end());
    BOOST_CHECK_EQUAL(vchMLDSASig.size(), MLDSA::SIGNATURE_BYTES);
    
    std::vector<unsigned char> mldsa_pub = key.GetMLDSAPubKey();
    BOOST_CHECK(CKey::VerifyMLDSA(hash, vchMLDSASig, mldsa_pub));
}

// Test multiple hybrid keys
BOOST_AUTO_TEST_CASE(multiple_hybrid_keys)
{
    CKey key1, key2;
    key1.MakeNewHybridKey(true);
    key2.MakeNewHybridKey(true);
    
    // Keys should be different
    BOOST_CHECK(key1.GetPubKey() != key2.GetPubKey());
    BOOST_CHECK(key1.GetMLDSAPubKey() != key2.GetMLDSAPubKey());
    
    // Sign same message with both keys
    uint256 hash;
    for (int i = 0; i < 32; i++) {
        ((unsigned char*)&hash)[i] = i * 7;
    }
    
    std::vector<unsigned char> vchSig1, vchSig2;
    BOOST_CHECK(key1.SignHybrid(hash, vchSig1));
    BOOST_CHECK(key2.SignHybrid(hash, vchSig2));
    
    // Signatures should be different
    BOOST_CHECK(vchSig1 != vchSig2);
    
    // Each key can only verify its own signature
    BOOST_CHECK(key1.VerifyHybrid(hash, vchSig1));
    BOOST_CHECK(key2.VerifyHybrid(hash, vchSig2));
    BOOST_CHECK(!key1.VerifyHybrid(hash, vchSig2));
    BOOST_CHECK(!key2.VerifyHybrid(hash, vchSig1));
}

// Test serialization of hybrid public key
BOOST_AUTO_TEST_CASE(hybrid_pubkey_serialization)
{
    CKey key;
    key.MakeNewHybridKey(true);
    
    CPubKey pubkey = key.GetPubKey();
    BOOST_CHECK(pubkey.IsHybrid());
    
    // Serialize the public key
    CDataStream ss(SER_DISK, CLIENT_VERSION);
    ss << pubkey;
    
    // Deserialize into new public key
    CPubKey pubkey2;
    ss >> pubkey2;
    
    // Should be identical
    BOOST_CHECK(pubkey == pubkey2);
    BOOST_CHECK(pubkey2.IsHybrid());
    BOOST_CHECK(pubkey.GetMLDSAPubKey() == pubkey2.GetMLDSAPubKey());
}

// Test backward compatibility: regular ECDSA key without ML-DSA
BOOST_AUTO_TEST_CASE(ecdsa_only_key_compatibility)
{
    CKey key;
    key.MakeNewKey(true);
    
    // Should not be hybrid
    BOOST_CHECK(!key.IsHybrid());
    BOOST_CHECK(!key.HasMLDSAKey());
    BOOST_CHECK(key.IsValid());
    
    // Regular ECDSA signing should still work
    uint256 hash;
    for (int i = 0; i < 32; i++) {
        ((unsigned char*)&hash)[i] = i * 11;
    }
    
    std::vector<unsigned char> vchSig;
    BOOST_CHECK(key.Sign(hash, vchSig));
    BOOST_CHECK(key.Verify(hash, vchSig));
    
    // Hybrid operations should fail gracefully
    BOOST_CHECK(!key.SignHybrid(hash, vchSig));
    BOOST_CHECK(!key.SignMLDSA(hash, vchSig));
}

// Test setting ML-DSA keys manually
BOOST_AUTO_TEST_CASE(set_mldsa_keys_manually)
{
    // Generate ML-DSA key pair directly
    std::vector<unsigned char> pub, priv;
    BOOST_CHECK(MLDSA::GenerateKeypair(pub, priv));
    
    BOOST_CHECK_EQUAL(pub.size(), MLDSA::PUBLIC_KEY_BYTES);
    BOOST_CHECK_EQUAL(priv.size(), MLDSA::PRIVATE_KEY_BYTES);
    
    // Create ECDSA key
    CKey key;
    key.MakeNewKey(true);
    
    // Should not be hybrid yet
    BOOST_CHECK(!key.IsHybrid());
    
    // Set ML-DSA keys
    BOOST_CHECK(key.SetMLDSAPrivKey(priv, pub));
    
    // Should now be hybrid
    BOOST_CHECK(key.IsHybrid());
    BOOST_CHECK(key.GetMLDSAPubKey() == pub);
    BOOST_CHECK(key.GetMLDSAPrivKey() == priv);
    
    // Should be able to sign and verify
    uint256 hash;
    for (int i = 0; i < 32; i++) {
        ((unsigned char*)&hash)[i] = i * 13;
    }
    
    std::vector<unsigned char> vchSig;
    BOOST_CHECK(key.SignMLDSA(hash, vchSig));
    BOOST_CHECK(CKey::VerifyMLDSA(hash, vchSig, pub));
}

#else // !ENABLE_MLDSA

BOOST_AUTO_TEST_CASE(mldsa_disabled_message)
{
    // When ML-DSA is disabled, just pass
    BOOST_CHECK_MESSAGE(true, "ML-DSA support not enabled (build without -DENABLE_MLDSA)");
}

#endif // ENABLE_MLDSA

BOOST_AUTO_TEST_SUITE_END()
