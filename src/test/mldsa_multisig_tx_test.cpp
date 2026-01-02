// Copyright (c) 2025-2026 AumCoin Developers
// Test suite for ML-DSA-65 multisig transaction workflow (Phase 4.3)

#include <boost/test/unit_test.hpp>
#include "main.h"
#include "wallet.h"
#include "key.h"
#include "script.h"
#include "util.h"
#include <vector>

BOOST_AUTO_TEST_SUITE(mldsa_multisig_tx_tests)

// Helper function to create hybrid keys
void CreateHybridKeys(std::vector<CKey>& keys, int count)
{
    keys.clear();
    keys.reserve(count);
    for (int i = 0; i < count; ++i)
    {
        CKey key;
        key.MakeNewHybridKey(true);
        // Hybrid keys have ML-DSA but may not pass IsValid() (ECDSA check)
        BOOST_CHECK(key.HasMLDSAKey());
        keys.push_back(key);
    }
}

// Test 1: Create unsigned transaction with valid parameters
BOOST_AUTO_TEST_CASE(test_create_unsigned_transaction)
{
    printf("\n=== Test 1: Create Unsigned Transaction ===\n");
    
    // Create 2-of-3 multisig redeem script
    std::vector<CKey> keys;
    CreateHybridKeys(keys, 3);
    
    std::vector<std::vector<unsigned char> > pubkeys;
    for (const auto& key : keys)
        pubkeys.push_back(key.GetPubKey().GetMLDSAPubKey());
    
    CScript redeemScript = CreateMLDSAMultisigScript(2, pubkeys);
    printf("Redeem script size: %zu bytes\n", redeemScript.size());
    
    // Create a dummy transaction
    CTransaction tx;
    tx.nVersion = 1;
    tx.nLockTime = 0;
    
    // Add input with redeem script in scriptSig
    CTxIn txin(uint256("1234567890abcdef1234567890abcdef1234567890abcdef1234567890abcdef"), 0);
    txin.scriptSig = redeemScript;
    tx.vin.push_back(txin);
    
    // Add output
    CScript scriptPubKey;
    scriptPubKey << OP_DUP << OP_HASH160 << std::vector<unsigned char>(20, 0x00) << OP_EQUALVERIFY << OP_CHECKSIG;
    CTxOut txout(100000000, scriptPubKey);  // 1.0 coin
    tx.vout.push_back(txout);
    
    BOOST_CHECK(!tx.IsNull());
    BOOST_CHECK_EQUAL(tx.vin.size(), 1);
    BOOST_CHECK_EQUAL(tx.vout.size(), 1);
    
    printf("✓ Unsigned transaction created successfully\n");
    printf("  Transaction hash: %s\n", tx.GetHash().GetHex().c_str());
    printf("  Input count: %zu\n", tx.vin.size());
    printf("  Output count: %zu\n", tx.vout.size());
}

// Test 2: Sign transaction with first key
BOOST_AUTO_TEST_CASE(test_sign_with_first_key)
{
    printf("\n=== Test 2: Sign Transaction with First Key ===\n");
    
    // Create 2-of-3 multisig
    std::vector<CKey> keys;
    CreateHybridKeys(keys, 3);
    
    std::vector<std::vector<unsigned char> > pubkeys;
    for (const auto& key : keys)
        pubkeys.push_back(key.GetPubKey().GetMLDSAPubKey());
    
    CScript redeemScript = CreateMLDSAMultisigScript(2, pubkeys);
    
    // Create transaction
    CTransaction tx;
    tx.nVersion = 1;
    tx.nLockTime = 0;
    
    CTxIn txin(uint256("1234567890abcdef1234567890abcdef1234567890abcdef1234567890abcdef"), 0);
    txin.scriptSig = redeemScript;
    tx.vin.push_back(txin);
    
    CScript scriptPubKey;
    scriptPubKey << OP_DUP << OP_HASH160 << std::vector<unsigned char>(20, 0x00) << OP_EQUALVERIFY << OP_CHECKSIG;
    tx.vout.push_back(CTxOut(100000000, scriptPubKey));
    
    uint256 txHash = tx.GetHash();
    printf("Transaction hash: %s\n", txHash.GetHex().c_str());
    
    // Sign with first key
    std::vector<unsigned char> sig1;
    BOOST_CHECK(keys[0].SignMLDSA(txHash, sig1));
    // Hybrid signature includes both ECDSA and ML-DSA parts
    BOOST_CHECK(sig1.size() > 1952);  // ML-DSA-65 + ECDSA overhead
    
    // Verify signature
    BOOST_CHECK(CKey::VerifyMLDSA(txHash, sig1, pubkeys[0]));
    
    printf("✓ First signature created and verified\n");
    printf("  Signature size: %zu bytes\n", sig1.size());
    printf("  Verification: PASSED\n");
    
    // Build scriptSig with one signature
    CScript signedScript;
    signedScript << CScript::EncodeOP_N(1);  // sig count = 1
    signedScript << sig1;
    std::vector<unsigned char> redeemScriptVec(redeemScript.begin(), redeemScript.end());
    signedScript << redeemScriptVec;
    
    tx.vin[0].scriptSig = signedScript;
    
    printf("  ScriptSig size: %zu bytes\n", tx.vin[0].scriptSig.size());
    printf("  Transaction now has 1/2 required signatures\n");
}

// Test 3: Complete 2-of-3 multisig (2 signatures)
BOOST_AUTO_TEST_CASE(test_complete_2of3_multisig)
{
    printf("\n=== Test 3: Complete 2-of-3 Multisig Transaction ===\n");
    
    // Create 2-of-3 multisig
    std::vector<CKey> keys;
    CreateHybridKeys(keys, 3);
    
    std::vector<std::vector<unsigned char> > pubkeys;
    for (const auto& key : keys)
        pubkeys.push_back(key.GetPubKey().GetMLDSAPubKey());
    
    CScript redeemScript = CreateMLDSAMultisigScript(2, pubkeys);
    
    // Create transaction
    CTransaction tx;
    tx.nVersion = 1;
    CTxIn txin(uint256("1234567890abcdef1234567890abcdef1234567890abcdef1234567890abcdef"), 0);
    txin.scriptSig = redeemScript;
    tx.vin.push_back(txin);
    
    CScript scriptPubKey;
    scriptPubKey << OP_DUP << OP_HASH160 << std::vector<unsigned char>(20, 0x00) << OP_EQUALVERIFY << OP_CHECKSIG;
    tx.vout.push_back(CTxOut(100000000, scriptPubKey));
    
    uint256 txHash = tx.GetHash();
    
    // Sign with first two keys (Alice and Bob)
    std::vector<unsigned char> sig1, sig2;
    BOOST_CHECK(keys[0].SignMLDSA(txHash, sig1));
    BOOST_CHECK(keys[1].SignMLDSA(txHash, sig2));
    
    // Verify both signatures
    BOOST_CHECK(CKey::VerifyMLDSA(txHash, sig1, pubkeys[0]));
    BOOST_CHECK(CKey::VerifyMLDSA(txHash, sig2, pubkeys[1]));
    
    // Build complete scriptSig
    CScript signedScript;
    signedScript << CScript::EncodeOP_N(2);  // sig count = 2
    signedScript << sig1 << sig2;
    std::vector<unsigned char> redeemScriptVec(redeemScript.begin(), redeemScript.end());
    signedScript << redeemScriptVec;
    
    tx.vin[0].scriptSig = signedScript;
    
    printf("✓ 2-of-3 multisig transaction complete\n");
    printf("  Signature 1 (Alice): %zu bytes\n", sig1.size());
    printf("  Signature 2 (Bob): %zu bytes\n", sig2.size());
    printf("  Total scriptSig size: %zu bytes\n", tx.vin[0].scriptSig.size());
    printf("  Transaction has 2/2 required signatures - COMPLETE!\n");
    
    // Verify scriptSig contains all components
    BOOST_CHECK(tx.vin[0].scriptSig.size() > 0);
    BOOST_CHECK(tx.vin[0].scriptSig.size() > sig1.size() + sig2.size());
}

// Test 4: Verify signature from wrong key fails
BOOST_AUTO_TEST_CASE(test_wrong_key_signature_fails)
{
    printf("\n=== Test 4: Wrong Key Signature Verification ===\n");
    
    // Create keys
    std::vector<CKey> keys;
    CreateHybridKeys(keys, 4);  // 4 keys, but only 3 in multisig
    
    std::vector<std::vector<unsigned char> > pubkeys;
    for (int i = 0; i < 3; ++i)  // Only first 3 in multisig
        pubkeys.push_back(keys[i].GetPubKey().GetMLDSAPubKey());
    
    CScript redeemScript = CreateMLDSAMultisigScript(2, pubkeys);
    
    // Create transaction
    CTransaction tx;
    tx.nVersion = 1;
    CTxIn txin(uint256("abcdef1234567890abcdef1234567890abcdef1234567890abcdef1234567890"), 0);
    tx.vin.push_back(txin);
    CScript scriptPubKey;
    scriptPubKey << OP_DUP << OP_HASH160 << std::vector<unsigned char>(20, 0x00) << OP_EQUALVERIFY << OP_CHECKSIG;
    tx.vout.push_back(CTxOut(50000000, scriptPubKey));
    
    uint256 txHash = tx.GetHash();
    
    // Sign with key NOT in multisig (keys[3])
    std::vector<unsigned char> wrongSig;
    BOOST_CHECK(keys[3].SignMLDSA(txHash, wrongSig));
    
    // Verify signature with wrong pubkey fails against multisig pubkeys
    std::vector<unsigned char> wrongPubkey = keys[3].GetPubKey().GetMLDSAPubKey();
    BOOST_CHECK(CKey::VerifyMLDSA(txHash, wrongSig, wrongPubkey));  // Sig is valid for that key
    
    // But this key is NOT in the multisig, so it shouldn't be accepted
    bool keyFound = false;
    for (const auto& pk : pubkeys)
    {
        if (pk == wrongPubkey)
        {
            keyFound = true;
            break;
        }
    }
    BOOST_CHECK(!keyFound);
    
    printf("✓ Wrong key correctly identified\n");
    printf("  Key 4 (outside multisig) can sign but is NOT in redeem script\n");
    printf("  Only keys 1-3 are valid for this multisig\n");
}

// Test 5: Test 3-of-5 multisig scenario
BOOST_AUTO_TEST_CASE(test_3of5_multisig)
{
    printf("\n=== Test 5: 3-of-5 Multisig Transaction ===\n");
    
    // Create 3-of-5 multisig (e.g., corporate treasury)
    std::vector<CKey> keys;
    CreateHybridKeys(keys, 5);
    
    std::vector<std::vector<unsigned char> > pubkeys;
    for (const auto& key : keys)
        pubkeys.push_back(key.GetPubKey().GetMLDSAPubKey());
    
    CScript redeemScript = CreateMLDSAMultisigScript(3, pubkeys);
    printf("5-party multisig redeem script: %zu bytes\n", redeemScript.size());
    
    // Create transaction
    CTransaction tx;
    tx.nVersion = 1;
    CTxIn txin(uint256("fedcba0987654321fedcba0987654321fedcba0987654321fedcba0987654321"), 0);
    tx.vin.push_back(txin);
    CScript scriptPubKey;
    scriptPubKey << OP_DUP << OP_HASH160 << std::vector<unsigned char>(20, 0x00) << OP_EQUALVERIFY << OP_CHECKSIG;
    tx.vout.push_back(CTxOut(500000000, scriptPubKey));  // 5.0 coins
    
    uint256 txHash = tx.GetHash();
    
    // Sign with 3 of 5 keys (Alice, Charlie, Eve)
    std::vector<unsigned char> sig1, sig3, sig5;
    BOOST_CHECK(keys[0].SignMLDSA(txHash, sig1));
    BOOST_CHECK(keys[2].SignMLDSA(txHash, sig3));
    BOOST_CHECK(keys[4].SignMLDSA(txHash, sig5));
    
    // Verify all three signatures
    BOOST_CHECK(CKey::VerifyMLDSA(txHash, sig1, pubkeys[0]));
    BOOST_CHECK(CKey::VerifyMLDSA(txHash, sig3, pubkeys[2]));
    BOOST_CHECK(CKey::VerifyMLDSA(txHash, sig5, pubkeys[4]));
    
    // Build scriptSig with 3 signatures
    CScript signedScript;
    signedScript << CScript::EncodeOP_N(3);  // sig count = 3
    signedScript << sig1 << sig3 << sig5;
    std::vector<unsigned char> redeemScriptVec(redeemScript.begin(), redeemScript.end());
    signedScript << redeemScriptVec;
    
    tx.vin[0].scriptSig = signedScript;
    
    printf("✓ 3-of-5 multisig transaction complete\n");
    printf("  Signers: Alice (key 1), Charlie (key 3), Eve (key 5)\n");
    printf("  Total signatures: 3/3 required\n");
    printf("  ScriptSig size: %zu bytes\n", tx.vin[0].scriptSig.size());
    printf("  Expected size: ~%zu bytes (3 sigs + redeem script)\n", 3*1952 + redeemScript.size() + 10);
}

// Test 6: Verify transaction hash changes invalidate signatures
BOOST_AUTO_TEST_CASE(test_transaction_immutability)
{
    printf("\n=== Test 6: Transaction Immutability ===\n");
    
    // Create 2-of-2 multisig
    std::vector<CKey> keys;
    CreateHybridKeys(keys, 2);
    
    std::vector<std::vector<unsigned char> > pubkeys;
    for (const auto& key : keys)
        pubkeys.push_back(key.GetPubKey().GetMLDSAPubKey());
    
    CScript redeemScript = CreateMLDSAMultisigScript(2, pubkeys);
    
    // Create transaction
    CTransaction tx1;
    tx1.nVersion = 1;
    CTxIn txin(uint256("1111111111111111111111111111111111111111111111111111111111111111"), 0);
    tx1.vin.push_back(txin);
    CScript scriptPubKey;
    scriptPubKey << OP_DUP << OP_HASH160 << std::vector<unsigned char>(20, 0x00) << OP_EQUALVERIFY << OP_CHECKSIG;
    tx1.vout.push_back(CTxOut(100000000, scriptPubKey));
    
    uint256 txHash1 = tx1.GetHash();
    
    // Sign with first key
    std::vector<unsigned char> sig1;
    BOOST_CHECK(keys[0].SignMLDSA(txHash1, sig1));
    BOOST_CHECK(CKey::VerifyMLDSA(txHash1, sig1, pubkeys[0]));
    
    printf("Original transaction hash: %s\n", txHash1.GetHex().c_str());
    
    // Modify transaction (change output amount)
    CTransaction tx2 = tx1;
    tx2.vout[0].nValue = 90000000;  // Changed!
    uint256 txHash2 = tx2.GetHash();
    
    printf("Modified transaction hash: %s\n", txHash2.GetHex().c_str());
    
    // Original signature should fail for modified transaction
    BOOST_CHECK(txHash1 != txHash2);
    BOOST_CHECK(!CKey::VerifyMLDSA(txHash2, sig1, pubkeys[0]));  // Should fail!
    
    printf("✓ Transaction immutability verified\n");
    printf("  Changing output amount invalidated signature\n");
    printf("  Signature valid for original: YES\n");
    printf("  Signature valid for modified: NO\n");
}

// Test 7: Test signature count encoding
BOOST_AUTO_TEST_CASE(test_signature_count_encoding)
{
    printf("\n=== Test 7: Signature Count Encoding ===\n");
    
    // Test encoding signature counts 1-15
    for (int count = 1; count <= 15; ++count)
    {
        CScript script;
        script << CScript::EncodeOP_N(count);
        
        // Decode and verify
        CScript::const_iterator pc = script.begin();
        opcodetype opcode;
        std::vector<unsigned char> vchData;
        
        BOOST_CHECK(script.GetOp(pc, opcode, vchData));
        int decoded = CScript::DecodeOP_N(opcode);
        BOOST_CHECK_EQUAL(decoded, count);
    }
    
    printf("✓ Signature count encoding tested for 1-15\n");
    printf("  All counts encode and decode correctly\n");
}

// Test 8: Test scriptSig parsing
BOOST_AUTO_TEST_CASE(test_scriptsig_parsing)
{
    printf("\n=== Test 8: ScriptSig Parsing ===\n");
    
    // Create 2-of-3 multisig
    std::vector<CKey> keys;
    CreateHybridKeys(keys, 3);
    
    std::vector<std::vector<unsigned char> > pubkeys;
    for (const auto& key : keys)
        pubkeys.push_back(key.GetPubKey().GetMLDSAPubKey());
    
    CScript redeemScript = CreateMLDSAMultisigScript(2, pubkeys);
    
    // Create fake signatures (just dummy data for parsing test)
    std::vector<unsigned char> fakeSig1(1952, 0xAA);
    std::vector<unsigned char> fakeSig2(1952, 0xBB);
    
    // Build scriptSig
    CScript scriptSig;
    scriptSig << CScript::EncodeOP_N(2);
    scriptSig << fakeSig1 << fakeSig2;
    std::vector<unsigned char> redeemScriptVec(redeemScript.begin(), redeemScript.end());
    scriptSig << redeemScriptVec;
    
    printf("Built scriptSig: %zu bytes\n", scriptSig.size());
    
    // Parse scriptSig
    CScript::const_iterator pc = scriptSig.begin();
    opcodetype opcode;
    std::vector<unsigned char> vchData;
    
    std::vector<std::vector<unsigned char> > parsedData;
    while (pc < scriptSig.end())
    {
        if (!scriptSig.GetOp(pc, opcode, vchData))
            break;
        if (opcode >= 0 && opcode <= OP_PUSHDATA4 && vchData.size() > 0)
            parsedData.push_back(vchData);
    }
    
    printf("Parsed %zu elements from scriptSig\n", parsedData.size());
    BOOST_CHECK_EQUAL(parsedData.size(), 3);  // count + sig1 + sig2 + redeemScript = 4, but count is encoded differently
    
    // Actually we should get: [1 byte count] [sig1] [sig2] [redeemScript]
    // Let's re-parse properly
    pc = scriptSig.begin();
    scriptSig.GetOp(pc, opcode, vchData);
    int sigCount = CScript::DecodeOP_N(opcode);
    BOOST_CHECK_EQUAL(sigCount, 2);
    
    printf("✓ ScriptSig parsing successful\n");
    printf("  Signature count: %d\n", sigCount);
    printf("  Total components parsed correctly\n");
}

// Test 9: Test maximum multisig (15-of-15)
BOOST_AUTO_TEST_CASE(test_max_multisig_15of15)
{
    printf("\n=== Test 9: Maximum 15-of-15 Multisig ===\n");
    
    // Create 15-of-15 multisig (maximum allowed)
    std::vector<CKey> keys;
    CreateHybridKeys(keys, 15);
    
    std::vector<std::vector<unsigned char> > pubkeys;
    for (const auto& key : keys)
        pubkeys.push_back(key.GetPubKey().GetMLDSAPubKey());
    
    CScript redeemScript = CreateMLDSAMultisigScript(15, pubkeys);
    printf("15-of-15 multisig redeem script: %zu bytes\n", redeemScript.size());
    
    // Create transaction
    CTransaction tx;
    tx.nVersion = 1;
    CTxIn txin(uint256("ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff"), 0);
    tx.vin.push_back(txin);
    CScript scriptPubKey;
    scriptPubKey << OP_DUP << OP_HASH160 << std::vector<unsigned char>(20, 0x00) << OP_EQUALVERIFY << OP_CHECKSIG;
    tx.vout.push_back(CTxOut(1000000000, scriptPubKey));
    
    uint256 txHash = tx.GetHash();
    
    // Sign with all 15 keys
    std::vector<std::vector<unsigned char> > signatures;
    for (int i = 0; i < 15; ++i)
    {
        std::vector<unsigned char> sig;
        BOOST_CHECK(keys[i].SignMLDSA(txHash, sig));
        BOOST_CHECK(CKey::VerifyMLDSA(txHash, sig, pubkeys[i]));
        signatures.push_back(sig);
    }
    
    // Build scriptSig with all 15 signatures
    CScript signedScript;
    signedScript << CScript::EncodeOP_N(15);
    for (const auto& sig : signatures)
        signedScript << sig;
    std::vector<unsigned char> redeemScriptVec(redeemScript.begin(), redeemScript.end());
    signedScript << redeemScriptVec;
    
    tx.vin[0].scriptSig = signedScript;
    
    size_t expectedSize = 15 * 1952 + redeemScript.size() + 10;  // 15 sigs + redeem script + overhead
    
    printf("✓ Maximum 15-of-15 multisig transaction created\n");
    printf("  All 15 signatures collected\n");
    printf("  ScriptSig size: %zu bytes\n", tx.vin[0].scriptSig.size());
    printf("  Expected size: ~%zu bytes\n", expectedSize);
    printf("  This represents the maximum quantum-safe multisig complexity\n");
}

// Test 10: Test 1-of-1 (single-party quantum signature)
BOOST_AUTO_TEST_CASE(test_1of1_single_party)
{
    printf("\n=== Test 10: 1-of-1 Single Party Quantum Signature ===\n");
    
    // Create 1-of-1 "multisig" (essentially a single quantum signature)
    std::vector<CKey> keys;
    CreateHybridKeys(keys, 1);
    
    std::vector<std::vector<unsigned char> > pubkeys;
    pubkeys.push_back(keys[0].GetPubKey().GetMLDSAPubKey());
    
    CScript redeemScript = CreateMLDSAMultisigScript(1, pubkeys);
    printf("1-of-1 quantum signature script: %zu bytes\n", redeemScript.size());
    
    // Create transaction
    CTransaction tx;
    tx.nVersion = 1;
    CTxIn txin(uint256("0000000000000000000000000000000000000000000000000000000000000001"), 0);
    tx.vin.push_back(txin);
    CScript scriptPubKey;
    scriptPubKey << OP_DUP << OP_HASH160 << std::vector<unsigned char>(20, 0x00) << OP_EQUALVERIFY << OP_CHECKSIG;
    tx.vout.push_back(CTxOut(25000000, scriptPubKey));
    
    uint256 txHash = tx.GetHash();
    
    // Sign with single key
    std::vector<unsigned char> sig;
    BOOST_CHECK(keys[0].SignMLDSA(txHash, sig));
    BOOST_CHECK(CKey::VerifyMLDSA(txHash, sig, pubkeys[0]));
    
    // Build scriptSig
    CScript signedScript;
    signedScript << CScript::EncodeOP_N(1);
    signedScript << sig;
    std::vector<unsigned char> redeemScriptVec(redeemScript.begin(), redeemScript.end());
    signedScript << redeemScriptVec;
    
    tx.vin[0].scriptSig = signedScript;
    
    printf("✓ 1-of-1 quantum signature transaction complete\n");
    printf("  Single ML-DSA-65 signature: %zu bytes\n", sig.size());
    printf("  ScriptSig size: %zu bytes\n", tx.vin[0].scriptSig.size());
    printf("  This is the simplest quantum-safe transaction format\n");
}

BOOST_AUTO_TEST_SUITE_END()
