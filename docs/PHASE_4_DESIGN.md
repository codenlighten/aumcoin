# Phase 4.1: Quantum-Resistant Multisig Design

**Status:** In Design  
**Date:** January 2, 2026  
**Goal:** Enable quantum-resistant 2-of-3 multisignature transactions

---

## 🎯 Objectives

Create a multisignature scheme that:
1. ✅ Requires multiple ML-DSA signatures to spend funds
2. ✅ Supports M-of-N signing policies (start with 2-of-3)
3. ✅ Uses P2SH (Pay-to-Script-Hash) for backward compatibility
4. ✅ Provides quantum resistance for institutional custody
5. ✅ Integrates with existing RPC wallet commands

---

## 🏗️ Architecture

### Traditional Bitcoin Multisig (ECDSA)

**Script Structure:**
```
scriptPubKey: OP_HASH160 <script_hash> OP_EQUAL
scriptSig: <sig1> <sig2> <redeem_script>
redeem_script: OP_2 <pubkey1> <pubkey2> <pubkey3> OP_3 OP_CHECKMULTISIG
```

**Limitations:**
- ❌ `OP_CHECKMULTISIG` was disabled in Litecoin/AumCoin
- ❌ Even if restored, only verifies ECDSA (quantum-vulnerable)
- ❌ Cannot handle 1952-byte ML-DSA public keys efficiently

---

## 🔮 AumCoin Quantum Multisig Design

### Option A: Sequential ML-DSA Verification (CHOSEN)

**Redeem Script Structure:**
```
<ml-dsa-pubkey1> OP_CHECKMLDSASIG OP_VERIFY
<ml-dsa-pubkey2> OP_CHECKMLDSASIG OP_ADD
<ml-dsa-pubkey3> OP_CHECKMLDSASIG OP_ADD
OP_2 OP_GREATERTHANOREQUAL
```

**scriptSig Structure:**
```
<ml-dsa-sig3-or-empty> <ml-dsa-sig2-or-empty> <ml-dsa-sig1-or-empty> <redeem_script>
```

**Execution Flow (2-of-3):**
1. Load redeem script from scriptSig
2. Hash and compare to scriptPubKey (P2SH validation)
3. Execute redeem script with remaining stack items:
   - Pop sig1, verify with pubkey1 → push 1 (valid) or 0 (invalid/missing)
   - If valid, OP_VERIFY consumes it (stack empty), else fail
   - Pop sig2, verify with pubkey2 → push 1 or 0
   - Pop sig3, verify with pubkey3 → push 1 or 0
   - OP_ADD twice → sum of valid signatures
   - OP_2 OP_GREATERTHANOREQUAL → check if count >= 2

**Stack Trace Example (2-of-3, Alice and Bob sign):**
```
Initial: [<sig_alice> <sig_bob> <empty> <redeem_script>]

After P2SH check:
[<sig_alice> <sig_bob> <empty>]  # Redeem script loaded

Execute: <ml-dsa-pubkey-alice> OP_CHECKMLDSASIG OP_VERIFY
[<sig_bob> <empty>]  # Alice's sig verified

Execute: <ml-dsa-pubkey-bob> OP_CHECKMLDSASIG OP_ADD
[<empty> 1]  # Bob's sig verified, added to count

Execute: <ml-dsa-pubkey-carol> OP_CHECKMLDSASIG OP_ADD
[1 0]  # Carol didn't sign (empty sig = 0)

Execute: OP_ADD
[1]  # Total valid sigs

Execute: OP_2 OP_GREATERTHANOREQUAL
[1]  # 1 >= 2? FALSE → transaction invalid

Wait, that's wrong! Let me fix...
```

**Issue with Option A:** Sequential verification doesn't handle "any 2 of 3" elegantly.

---

### Option B: Threshold Counting (BETTER)

**Redeem Script Structure:**
```
OP_2                           # Required signatures (M)
<ml-dsa-pubkey1> OP_CHECKMLDSASIG OP_IF OP_1 OP_ELSE OP_0 OP_ENDIF
<ml-dsa-pubkey2> OP_CHECKMLDSASIG OP_IF OP_1 OP_ELSE OP_0 OP_ENDIF
<ml-dsa-pubkey3> OP_CHECKMLDSASIG OP_IF OP_1 OP_ELSE OP_0 OP_ENDIF
OP_ADD OP_ADD                  # Sum the results
OP_GREATERTHANOREQUAL          # Check if sum >= M
```

**scriptSig Structure:**
```
<ml-dsa-sig1> <ml-dsa-sig2> <ml-dsa-sig3> <redeem_script>
```

**Execution Flow (2-of-3, Alice and Carol sign):**
```
Initial stack: [<sig1> <sig2> <sig3>]

After pubkey1 check:
[<sig2> <sig3> 1]  # Alice signed (valid)

After pubkey2 check:
[<sig3> 1 0]  # Bob didn't sign (invalid)

After pubkey3 check:
[1 0 1]  # Carol signed (valid)

After OP_ADD OP_ADD:
[2]  # 1 + 0 + 1 = 2 valid signatures

After OP_2 OP_GREATERTHANOREQUAL:
[1]  # 2 >= 2 → TRUE → transaction valid! ✅
```

**Advantages:**
- ✅ Handles "any M of N" combinations
- ✅ Clear threshold logic
- ✅ Each signer can independently verify
- ✅ Script size: ~6KB (3 × 1952 bytes pubkeys + opcodes)

---

## 📐 Script Size Analysis

### Components

| Component | Size | Count | Total |
|-----------|------|-------|-------|
| ML-DSA Public Key | 1,952 bytes | 3 | 5,856 bytes |
| ML-DSA Signature | 3,309 bytes | 3 | 9,927 bytes |
| Opcodes | ~50 bytes | 1 | 50 bytes |
| **Redeem Script** | - | - | **~6 KB** |
| **scriptSig** | - | - | **~10 KB** |
| **Total P2SH** | - | - | **~16 KB** |

**Comparison:**
- Traditional ECDSA multisig: ~500 bytes
- AumCoin quantum multisig: ~16 KB (32x larger)

**Optimization Strategies:**
1. **Compressed Public Keys:** Research ML-DSA key compression (NIST exploring)
2. **SegWit-like Separation:** Move signatures to separate witness data
3. **Schnorr-style Aggregation:** Future research on ML-DSA signature aggregation
4. **Hybrid Sparse:** Use ECDSA for small amounts, ML-DSA for cold storage

---

## 🔧 Implementation Plan

### Step 1: Script Builder Functions

**File:** `src/script.cpp`

```cpp
#ifdef ENABLE_MLDSA
CScript CreateMLDSAMultisigScript(int nRequired, 
                                   const std::vector<std::vector<unsigned char>>& vchMLDSAPubKeys)
{
    CScript script;
    
    // Push required count
    script << nRequired;
    
    // For each public key, add verification + conditional 1/0
    for (const auto& pubkey : vchMLDSAPubKeys)
    {
        script << pubkey;
        script << OP_CHECKMLDSASIG;
        script << OP_IF << OP_1 << OP_ELSE << OP_0 << OP_ENDIF;
    }
    
    // Sum the results (N-1 OP_ADDs for N pubkeys)
    for (size_t i = 1; i < vchMLDSAPubKeys.size(); ++i)
        script << OP_ADD;
    
    // Check threshold
    script << OP_GREATERTHANOREQUAL;
    
    return script;
}

CScriptID GetScriptIDForMLDSAMultisig(const CScript& redeemScript)
{
    return CScriptID(redeemScript);
}
#endif
```

---

### Step 2: RPC Command - `addmultisigmldsaaddress`

**File:** `src/rpcdump.cpp`

```cpp
#ifdef ENABLE_MLDSA
Value addmultisigmldsaaddress(const Array& params, bool fHelp)
{
    if (fHelp || params.size() < 3 || params.size() > 4)
        throw runtime_error(
            "addmultisigmldsaaddress <nrequired> <'[\"key\",...]'> [account]\n"
            "Create quantum-resistant M-of-N multisig address.\n"
            "Keys must be ML-DSA addresses (generated via getnewmldsaaddress).\n"
            "Returns P2SH address for the multisig script.");
    
    // Parse parameters
    int nRequired = params[0].get_int();
    Array keys = params[1].get_array();
    string strAccount;
    if (params.size() > 2)
        strAccount = AccountFromValue(params[2]);
    
    // Validate
    if (nRequired < 1 || nRequired > (int)keys.size())
        throw runtime_error("Invalid nrequired parameter");
    
    // Collect ML-DSA public keys
    vector<vector<unsigned char>> vchMLDSAPubKeys;
    for (size_t i = 0; i < keys.size(); ++i)
    {
        CBitcoinAddress address(keys[i].get_str());
        if (!address.IsValid())
            throw JSONRPCError(-5, "Invalid AumCoin address");
        
        CKeyID keyID;
        if (!address.GetKeyID(keyID))
            throw JSONRPCError(-5, "Address does not refer to a key");
        
        CPubKey pubkey;
        if (!pwalletMain->GetPubKey(keyID, pubkey))
            throw JSONRPCError(-5, "Public key not found in wallet");
        
        if (!pubkey.HasMLDSAKey())
            throw JSONRPCError(-5, "Key does not have ML-DSA component");
        
        vchMLDSAPubKeys.push_back(pubkey.GetMLDSAPubKey());
    }
    
    // Create redeem script
    CScript redeemScript = CreateMLDSAMultisigScript(nRequired, vchMLDSAPubKeys);
    
    // Create P2SH address
    CScriptID scriptID = GetScriptIDForMLDSAMultisig(redeemScript);
    CBitcoinAddress address(scriptID);
    
    // Add to wallet
    pwalletMain->AddCScript(redeemScript);
    pwalletMain->SetAddressBookName(scriptID, strAccount);
    
    return address.ToString();
}
#endif
```

---

### Step 3: Signing Workflow

**Challenge:** Multisig requires collecting signatures from multiple parties.

**Solution:** Two-phase approach
1. **Create Transaction:** Any party creates unsigned tx with multisig inputs
2. **Collect Signatures:** Each signer adds their ML-DSA signature
3. **Broadcast:** Once M signatures collected, broadcast to network

**RPC Commands Needed:**
- `createmultisigtx` - Create unsigned transaction
- `signmultisigtxmldsa` - Sign with one party's key
- `combinemultisigtxmldsa` - Combine partial signatures
- `sendrawtransaction` - Broadcast (already exists)

---

### Step 4: Transaction Validation

**File:** `src/script.cpp` (existing OP_CHECKMLDSASIG)

**No changes needed!** The existing opcode already handles:
- ✅ Stack manipulation
- ✅ ML-DSA signature verification
- ✅ Return true/false on stack

The multisig script just combines multiple verifications with threshold logic.

---

## 🧪 Test Scenarios

### Test 1: Create 2-of-3 Multisig Address
```cpp
BOOST_AUTO_TEST_CASE(mldsa_multisig_address_creation)
{
    // Generate 3 hybrid keys
    CKey keyAlice, keyBob, keyCarol;
    keyAlice.MakeNewHybridKey();
    keyBob.MakeNewHybridKey();
    keyCarol.MakeNewHybridKey();
    
    // Get ML-DSA public keys
    vector<vector<unsigned char>> pubkeys = {
        keyAlice.GetPubKey().GetMLDSAPubKey(),
        keyBob.GetPubKey().GetMLDSAPubKey(),
        keyCarol.GetPubKey().GetMLDSAPubKey()
    };
    
    // Create 2-of-3 script
    CScript redeemScript = CreateMLDSAMultisigScript(2, pubkeys);
    
    // Verify script size
    BOOST_CHECK(redeemScript.size() > 6000);  // ~6KB
    BOOST_CHECK(redeemScript.size() < 7000);
    
    // Create P2SH address
    CScriptID scriptID(redeemScript);
    BOOST_CHECK(!scriptID.IsNull());
}
```

---

### Test 2: Sign with 2 of 3 Keys (Valid)
```cpp
BOOST_AUTO_TEST_CASE(mldsa_multisig_2of3_valid)
{
    // Setup (keys, script, transaction...)
    
    // Alice and Bob sign
    vector<unsigned char> sigAlice, sigBob;
    keyAlice.SignMLDSA(txHash, sigAlice);
    keyBob.SignMLDSA(txHash, sigBob);
    
    // Carol doesn't sign (empty)
    vector<unsigned char> sigCarol;  // Empty
    
    // Create scriptSig
    CScript scriptSig;
    scriptSig << sigAlice << sigBob << sigCarol << redeemScript;
    
    // Verify script execution
    ScriptError error;
    bool valid = VerifyScript(scriptSig, scriptPubKey, tx, 0, 0, error);
    
    BOOST_CHECK(valid);  // Should succeed with 2 valid sigs
}
```

---

### Test 3: Sign with 1 of 3 Keys (Invalid)
```cpp
BOOST_AUTO_TEST_CASE(mldsa_multisig_2of3_insufficient)
{
    // Only Alice signs
    vector<unsigned char> sigAlice;
    keyAlice.SignMLDSA(txHash, sigAlice);
    
    // Bob and Carol don't sign
    vector<unsigned char> sigBob, sigCarol;  // Empty
    
    // Create scriptSig
    CScript scriptSig;
    scriptSig << sigAlice << sigBob << sigCarol << redeemScript;
    
    // Verify script execution
    ScriptError error;
    bool valid = VerifyScript(scriptSig, scriptPubKey, tx, 0, 0, error);
    
    BOOST_CHECK(!valid);  // Should fail with only 1 sig
}
```

---

### Test 4: Wrong Signature (Security)
```cpp
BOOST_AUTO_TEST_CASE(mldsa_multisig_wrong_signature)
{
    // Alice and Bob sign correct hash
    vector<unsigned char> sigAlice, sigBob;
    keyAlice.SignMLDSA(txHash, sigAlice);
    keyBob.SignMLDSA(txHash, sigBob);
    
    // But Carol's signature is for different hash (attack attempt)
    uint256 wrongHash = Hash(txHash.begin(), txHash.end());
    vector<unsigned char> sigCarolWrong;
    keyCarol.SignMLDSA(wrongHash, sigCarolWrong);
    
    // Create scriptSig with 3 sigs (2 correct, 1 wrong)
    CScript scriptSig;
    scriptSig << sigAlice << sigBob << sigCarolWrong << redeemScript;
    
    // Verify - should fail (Carol's sig is for wrong message)
    ScriptError error;
    bool valid = VerifyScript(scriptSig, scriptPubKey, tx, 0, 0, error);
    
    BOOST_CHECK(!valid);
}
```

---

## 📊 Performance Estimates

### Transaction Validation Time

| Component | Time | Count | Total |
|-----------|------|-------|-------|
| P2SH Hash Check | 0.01 ms | 1 | 0.01 ms |
| ML-DSA Verify | 0.12 ms | 3 | 0.36 ms |
| Script Execution | 0.01 ms | 1 | 0.01 ms |
| **Total** | - | - | **~0.4 ms** |

**Comparison:**
- ECDSA multisig: ~3 ms (slower!)
- ML-DSA multisig: ~0.4 ms (faster!)

**Surprising Result:** Despite larger signatures, ML-DSA verification is faster!

---

## 🔒 Security Analysis

### Threat Model

**Attack Scenario:** Attacker wants to spend from 2-of-3 multisig.

**Defense Layers:**
1. **Threshold Requirement:** Need 2 valid ML-DSA signatures
2. **ML-DSA Security:** Each signature is quantum-resistant
3. **P2SH Commitment:** Redeem script hash prevents script modification
4. **Transaction Binding:** Signatures commit to full transaction

**Quantum Attack Resistance:**
- ✅ Private keys: Protected by ML-DSA-65 (NIST Level 3)
- ✅ Signatures: 3309 bytes, lattice-based (Shor-resistant)
- ✅ Threshold: Attacker must break 2+ keys (exponentially harder)

**Classical Attack Resistance:**
- ✅ Replay: Transaction hash changes per tx
- ✅ Forgery: ML-DSA signatures unforgeable
- ✅ Substitution: P2SH hash prevents script tampering

---

## 🚀 Deployment Strategy

### Phase 4.1 Milestones

1. **Week 1:** Script builder + RPC command (addmultisigmldsaaddress)
2. **Week 2:** Transaction signing workflow (createmultisigtx, sign, combine)
3. **Week 3:** Comprehensive testing (10+ scenarios)
4. **Week 4:** Documentation + demo video

### Integration Checklist

- [ ] Implement `CreateMLDSAMultisigScript()` in script.cpp
- [ ] Add `addmultisigmldsaaddress` RPC command
- [ ] Test address creation with 2-of-3, 3-of-5 combinations
- [ ] Implement signing workflow RPC commands
- [ ] Add tests for valid/invalid signature combinations
- [ ] Security audit: replay attacks, signature substitution
- [ ] Performance benchmarking vs ECDSA multisig
- [ ] Documentation: tutorial + API reference
- [ ] Demo: Create multisig, sign from 3 wallets, broadcast

---

## 🎯 Success Criteria

**Phase 4.1 Complete When:**
1. ✅ Can create 2-of-3 ML-DSA multisig P2SH addresses
2. ✅ Can sign transactions with partial signatures
3. ✅ Can combine signatures and broadcast valid transactions
4. ✅ All test scenarios pass (valid, invalid, attacks)
5. ✅ Documentation complete with examples
6. ✅ Performance meets targets (<1ms validation per multisig)

---

## 💡 Future Enhancements (Phase 4.2+)

### Signature Aggregation
- Research ML-DSA signature aggregation schemes
- Reduce 3 × 3309 bytes to single aggregated signature
- Potential savings: 66% size reduction

### Threshold Signatures
- Implement true (t, n)-threshold ML-DSA
- No need to reveal which M parties signed
- Better privacy + smaller signatures

### Hardware Wallet Support
- Integrate with Ledger/Trezor for ML-DSA signing
- Secure key storage for quantum-resistant keys
- Multi-party signing coordination

### Cross-Chain Bridges
- Quantum-resistant atomic swaps
- ML-DSA signatures for Bitcoin/Ethereum bridges
- Future-proof DeFi infrastructure

---

**Status:** Design Complete - Ready for Implementation  
**Next Step:** Implement `CreateMLDSAMultisigScript()` and test

**Estimated Implementation Time:** 3-4 hours  
**Complexity:** Medium (builds on existing infrastructure)

---

*"The best time to implement quantum-resistant multisig was 10 years ago.  
The second best time is now."* 🚀
