// Copyright (c) 2026 AumCoin Developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/test/unit_test.hpp>
#include "../key.h"
#include "../script.h"
#include "../crypto/mldsa.h"
#include "../util.h"
#include <vector>

BOOST_AUTO_TEST_SUITE(mldsa_multisig_tests)

// Test 1: Create 2-of-3 multisig script
BOOST_AUTO_TEST_CASE(create_2of3_multisig_script)
{
    printf("\n=== Test 1: Create 2-of-3 Multisig Script ===\n");
    
    // Generate 3 hybrid keys
    CKey keyAlice, keyBob, keyCarol;
    keyAlice.MakeNewHybridKey();
    keyBob.MakeNewHybridKey();
    keyCarol.MakeNewHybridKey();
    
    // Verify hybrid keys were created
    BOOST_CHECK(keyAlice.HasMLDSAKey());
    BOOST_CHECK(keyBob.HasMLDSAKey());
    BOOST_CHECK(keyCarol.HasMLDSAKey());
    
    // Get ML-DSA public keys
    std::vector<std::vector<unsigned char> > vchMLDSAPubKeys;
    vchMLDSAPubKeys.push_back(keyAlice.GetPubKey().GetMLDSAPubKey());
    vchMLDSAPubKeys.push_back(keyBob.GetPubKey().GetMLDSAPubKey());
    vchMLDSAPubKeys.push_back(keyCarol.GetPubKey().GetMLDSAPubKey());
    
    BOOST_CHECK_EQUAL(vchMLDSAPubKeys[0].size(), (size_t)MLDSA::PUBLIC_KEY_BYTES);
    BOOST_CHECK_EQUAL(vchMLDSAPubKeys[1].size(), (size_t)MLDSA::PUBLIC_KEY_BYTES);
    BOOST_CHECK_EQUAL(vchMLDSAPubKeys[2].size(), (size_t)MLDSA::PUBLIC_KEY_BYTES);
    
    // Create 2-of-3 multisig script
    CScript redeemScript = CreateMLDSAMultisigScript(2, vchMLDSAPubKeys);
    
    // Verify script was created
    BOOST_CHECK(!redeemScript.empty());
    
    // Verify script size (approximately 6KB for 3 keys)
    // 3 keys × 1952 bytes + opcodes ≈ 6000 bytes
    printf("Redeem script size: %zu bytes\n", redeemScript.size());
    BOOST_CHECK(redeemScript.size() > 5800);  // At least 5.8KB
    BOOST_CHECK(redeemScript.size() < 6500);  // Less than 6.5KB
    
    // Create P2SH address from redeem script
    CScriptID scriptID = redeemScript.GetID();
    
    printf("✅ 2-of-3 multisig script created successfully\n");
    printf("   Script size: %zu bytes\n", redeemScript.size());
    printf("   Script ID: %s\n", scriptID.ToString().c_str());
}

// Test 2: Invalid parameters (nRequired > N)
BOOST_AUTO_TEST_CASE(invalid_nrequired_too_large)
{
    printf("\n=== Test 2: Invalid nRequired (3-of-2) ===\n");
    
    // Generate 2 keys
    CKey key1, key2;
    key1.MakeNewHybridKey();
    key2.MakeNewHybridKey();
    
    std::vector<std::vector<unsigned char> > pubkeys;
    pubkeys.push_back(key1.GetPubKey().GetMLDSAPubKey());
    pubkeys.push_back(key2.GetPubKey().GetMLDSAPubKey());
    
    // Try to create 3-of-2 (should fail)
    bool caught_error = false;
    try {
        CScript script = CreateMLDSAMultisigScript(3, pubkeys);
    } catch (std::runtime_error& e) {
        caught_error = true;
        printf("✅ Correctly rejected invalid nRequired: %s\n", e.what());
    }
    
    BOOST_CHECK(caught_error);
}

// Test 3: Invalid parameters (nRequired = 0)
BOOST_AUTO_TEST_CASE(invalid_nrequired_zero)
{
    printf("\n=== Test 3: Invalid nRequired (0-of-3) ===\n");
    
    CKey key1, key2, key3;
    key1.MakeNewHybridKey();
    key2.MakeNewHybridKey();
    key3.MakeNewHybridKey();
    
    std::vector<std::vector<unsigned char> > pubkeys;
    pubkeys.push_back(key1.GetPubKey().GetMLDSAPubKey());
    pubkeys.push_back(key2.GetPubKey().GetMLDSAPubKey());
    pubkeys.push_back(key3.GetPubKey().GetMLDSAPubKey());
    
    // Try to create 0-of-3 (should fail)
    bool caught_error = false;
    try {
        CScript script = CreateMLDSAMultisigScript(0, pubkeys);
    } catch (std::runtime_error& e) {
        caught_error = true;
        printf("✅ Correctly rejected nRequired=0: %s\n", e.what());
    }
    
    BOOST_CHECK(caught_error);
}

// Test 4: Invalid public key size
BOOST_AUTO_TEST_CASE(invalid_pubkey_size)
{
    printf("\n=== Test 4: Invalid Public Key Size ===\n");
    
    std::vector<std::vector<unsigned char> > pubkeys;
    
    // Add invalid pubkey (wrong size)
    std::vector<unsigned char> badKey(100);  // Should be 1952 bytes
    pubkeys.push_back(badKey);
    
    // Try to create script (should fail)
    bool caught_error = false;
    try {
        CScript script = CreateMLDSAMultisigScript(1, pubkeys);
    } catch (std::runtime_error& e) {
        caught_error = true;
        printf("✅ Correctly rejected invalid key size: %s\n", e.what());
    }
    
    BOOST_CHECK(caught_error);
}

// Test 5: Various multisig combinations (1-of-1, 1-of-2, 2-of-2, 3-of-5)
BOOST_AUTO_TEST_CASE(various_multisig_combinations)
{
    printf("\n=== Test 5: Various Multisig Combinations ===\n");
    
    // Generate 5 keys and verify they have ML-DSA data
    std::vector<CKey> keys;
    for (int i = 0; i < 5; ++i)
    {
        CKey key;
        key.MakeNewHybridKey();
        
        // Verify key has ML-DSA component
        BOOST_CHECK(key.HasMLDSAKey());
        std::vector<unsigned char> mldsa_pub = key.GetMLDSAPubKey();
        BOOST_CHECK_EQUAL(mldsa_pub.size(), (size_t)MLDSA::PUBLIC_KEY_BYTES);
        
        keys.push_back(key);
    }
    
    printf("✅ Generated 5 hybrid keys successfully\n");
    
    // Test 1-of-1
    {
        std::vector<std::vector<unsigned char> > pubkeys;
        std::vector<unsigned char> pk = keys[0].GetPubKey().GetMLDSAPubKey();
        BOOST_CHECK_EQUAL(pk.size(), (size_t)MLDSA::PUBLIC_KEY_BYTES);
        pubkeys.push_back(pk);
        CScript script = CreateMLDSAMultisigScript(1, pubkeys);
        BOOST_CHECK(!script.empty());
        printf("✅ 1-of-1: %zu bytes\n", script.size());
    }
    
    // Test 1-of-2
    {
        std::vector<std::vector<unsigned char> > pubkeys;
        pubkeys.push_back(keys[0].GetPubKey().GetMLDSAPubKey());
        pubkeys.push_back(keys[1].GetPubKey().GetMLDSAPubKey());
        CScript script = CreateMLDSAMultisigScript(1, pubkeys);
        BOOST_CHECK(!script.empty());
        printf("✅ 1-of-2: %zu bytes\n", script.size());
    }
    
    // Test 2-of-2
    {
        std::vector<std::vector<unsigned char> > pubkeys;
        pubkeys.push_back(keys[0].GetPubKey().GetMLDSAPubKey());
        pubkeys.push_back(keys[1].GetPubKey().GetMLDSAPubKey());
        CScript script = CreateMLDSAMultisigScript(2, pubkeys);
        BOOST_CHECK(!script.empty());
        printf("✅ 2-of-2: %zu bytes\n", script.size());
    }
    
    // Test 3-of-5
    {
        std::vector<std::vector<unsigned char> > pubkeys;
        for (int i = 0; i < 5; ++i)
            pubkeys.push_back(keys[i].GetPubKey().GetMLDSAPubKey());
        CScript script = CreateMLDSAMultisigScript(3, pubkeys);
        BOOST_CHECK(!script.empty());
        printf("✅ 3-of-5: %zu bytes\n", script.size());
        
        // 5 keys × 1952 bytes ≈ 9.5KB
        BOOST_CHECK(script.size() > 9500);
        BOOST_CHECK(script.size() < 10500);
    }
}

// Test 6: Script structure verification
BOOST_AUTO_TEST_CASE(verify_script_structure)
{
    printf("\n=== Test 6: Verify Script Structure ===\n");
    
    // Create simple 2-of-3
    CKey key1, key2, key3;
    key1.MakeNewHybridKey();
    key2.MakeNewHybridKey();
    key3.MakeNewHybridKey();
    
    std::vector<std::vector<unsigned char> > pubkeys;
    pubkeys.push_back(key1.GetPubKey().GetMLDSAPubKey());
    pubkeys.push_back(key2.GetPubKey().GetMLDSAPubKey());
    pubkeys.push_back(key3.GetPubKey().GetMLDSAPubKey());
    
    CScript script = CreateMLDSAMultisigScript(2, pubkeys);
    
    // Parse script opcodes
    std::vector<unsigned char> vch;
    CScript::const_iterator pc = script.begin();
    opcodetype opcode;
    
    // First opcode should be OP_2 (nRequired)
    BOOST_CHECK(script.GetOp(pc, opcode, vch));
    BOOST_CHECK_EQUAL(opcode, OP_2);
    printf("✅ First opcode: OP_2 (nRequired)\n");
    
    // Next should be first pubkey (1952 bytes)
    BOOST_CHECK(script.GetOp(pc, opcode, vch));
    BOOST_CHECK_EQUAL(vch.size(), (size_t)MLDSA::PUBLIC_KEY_BYTES);
    printf("✅ First pubkey: %zu bytes\n", vch.size());
    
    // Followed by OP_CHECKMLDSASIG
    BOOST_CHECK(script.GetOp(pc, opcode, vch));
    BOOST_CHECK_EQUAL(opcode, OP_CHECKMLDSASIG);
    printf("✅ OP_CHECKMLDSASIG found\n");
    
    // Followed by OP_IF
    BOOST_CHECK(script.GetOp(pc, opcode, vch));
    BOOST_CHECK_EQUAL(opcode, OP_IF);
    printf("✅ OP_IF found\n");
    
    // Pattern continues...
    printf("✅ Script structure verified\n");
}

// Test 7: Maximum keys (15-of-15)
BOOST_AUTO_TEST_CASE(maximum_keys_15of15)
{
    printf("\n=== Test 7: Maximum Keys (15-of-15) ===\n");
    
    // Generate 15 keys
    std::vector<std::vector<unsigned char> > pubkeys;
    for (int i = 0; i < 15; ++i)
    {
        CKey key;
        key.MakeNewHybridKey();
        pubkeys.push_back(key.GetPubKey().GetMLDSAPubKey());
    }
    
    // Create 15-of-15 script
    CScript script = CreateMLDSAMultisigScript(15, pubkeys);
    BOOST_CHECK(!script.empty());
    
    // Size: 15 keys × 1952 bytes ≈ 29KB
    printf("✅ 15-of-15 script created: %zu bytes\n", script.size());
    BOOST_CHECK(script.size() > 29000);
    BOOST_CHECK(script.size() < 31000);
}

// Test 8: Too many keys (16-of-16 should fail)
BOOST_AUTO_TEST_CASE(too_many_keys)
{
    printf("\n=== Test 8: Too Many Keys (16-of-16 should fail) ===\n");
    
    // Generate 16 keys
    std::vector<std::vector<unsigned char> > pubkeys;
    for (int i = 0; i < 16; ++i)
    {
        CKey key;
        key.MakeNewHybridKey();
        pubkeys.push_back(key.GetPubKey().GetMLDSAPubKey());
    }
    
    // Try to create 16-of-16 (should fail - limit is 15)
    bool caught_error = false;
    try {
        CScript script = CreateMLDSAMultisigScript(16, pubkeys);
    } catch (std::runtime_error& e) {
        caught_error = true;
        printf("✅ Correctly rejected 16 keys: %s\n", e.what());
    }
    
    BOOST_CHECK(caught_error);
}

// Test 9: P2SH address generation
BOOST_AUTO_TEST_CASE(p2sh_address_generation)
{
    printf("\n=== Test 9: P2SH Address Generation ===\n");
    
    // Create 2-of-3
    CKey key1, key2, key3;
    key1.MakeNewHybridKey();
    key2.MakeNewHybridKey();
    key3.MakeNewHybridKey();
    
    std::vector<std::vector<unsigned char> > pubkeys;
    pubkeys.push_back(key1.GetPubKey().GetMLDSAPubKey());
    pubkeys.push_back(key2.GetPubKey().GetMLDSAPubKey());
    pubkeys.push_back(key3.GetPubKey().GetMLDSAPubKey());
    
    CScript redeemScript = CreateMLDSAMultisigScript(2, pubkeys);
    
    // Create P2SH scriptPubKey
    CScriptID scriptID = redeemScript.GetID();
    CScript scriptPubKey;
    scriptPubKey << OP_HASH160 << scriptID << OP_EQUAL;
    
    BOOST_CHECK_EQUAL(scriptPubKey.size(), (size_t)23);  // Standard P2SH size
    printf("✅ P2SH scriptPubKey created: %zu bytes\n", scriptPubKey.size());
    printf("   Script ID: %s\n", scriptID.ToString().c_str());
    
    // Verify scriptPubKey structure
    std::vector<unsigned char> vch;
    CScript::const_iterator pc = scriptPubKey.begin();
    opcodetype opcode;
    
    BOOST_CHECK(scriptPubKey.GetOp(pc, opcode, vch));
    BOOST_CHECK_EQUAL(opcode, OP_HASH160);
    
    BOOST_CHECK(scriptPubKey.GetOp(pc, opcode, vch));
    BOOST_CHECK_EQUAL(vch.size(), (size_t)20);  // 160-bit hash
    
    BOOST_CHECK(scriptPubKey.GetOp(pc, opcode, vch));
    BOOST_CHECK_EQUAL(opcode, OP_EQUAL);
    
    printf("✅ P2SH structure verified\n");
}

// Test 10: Reproducibility (same keys → same script)
BOOST_AUTO_TEST_CASE(script_reproducibility)
{
    printf("\n=== Test 10: Script Reproducibility ===\n");
    
    // Generate keys
    CKey key1, key2, key3;
    key1.MakeNewHybridKey();
    key2.MakeNewHybridKey();
    key3.MakeNewHybridKey();
    
    std::vector<std::vector<unsigned char> > pubkeys;
    pubkeys.push_back(key1.GetPubKey().GetMLDSAPubKey());
    pubkeys.push_back(key2.GetPubKey().GetMLDSAPubKey());
    pubkeys.push_back(key3.GetPubKey().GetMLDSAPubKey());
    
    // Create script twice
    CScript script1 = CreateMLDSAMultisigScript(2, pubkeys);
    CScript script2 = CreateMLDSAMultisigScript(2, pubkeys);
    
    // Should be identical
    BOOST_CHECK(script1 == script2);
    BOOST_CHECK_EQUAL(script1.size(), script2.size());
    
    // ScriptIDs should match
    CScriptID id1 = script1.GetID();
    CScriptID id2 = script2.GetID();
    BOOST_CHECK(id1 == id2);
    
    printf("✅ Scripts are reproducible\n");
}

BOOST_AUTO_TEST_SUITE_END()
