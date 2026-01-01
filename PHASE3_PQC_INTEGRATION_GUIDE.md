# Phase 3: Post-Quantum Cryptography Integration Guide

**Adding ML-DSA to AumCoin using @smartledger/keys**

---

## 🎯 **Goal**

Add **optional** ML-DSA (post-quantum) signature support to AumCoin while maintaining backward compatibility with ECDSA.

**Timeline:** Q2 2026 (after Phase 2 security complete)  
**Effort:** 3-4 weeks  
**Risk:** Medium (new crypto primitive)

---

## 🤔 **Why Add Post-Quantum Cryptography?**

### **The Threat**
- **2030-2035:** Large-scale quantum computers expected
- **Store Now, Decrypt Later:** Adversaries capturing encrypted data today to decrypt with future quantum computers
- **ECDSA Vulnerability:** Shor's algorithm can break ECDSA in polynomial time on quantum computers

### **The Solution**
- **ML-DSA (Dilithium):** NIST-standardized post-quantum signature algorithm
- **Quantum-resistant:** Based on lattice mathematics, secure against quantum attacks
- **@smartledger/keys:** Production-ready implementation already exists!

### **Why Now?**
- ✅ Standard is finalized (NIST FIPS 204)
- ✅ Implementation is tested (@smartledger/keys v2.0.0)
- ✅ **3.6x faster than ECDSA** (unexpected bonus!)
- ✅ Early adopter advantage

---

## 🏗️ **Three Implementation Options**

### **Option A: Node.js Addon (Fastest to Prototype)**
**Approach:** Call @smartledger/keys from C++ via Node.js native addons

**Pros:**
- ✅ Fastest development (use existing SDK)
- ✅ No need to reimplement crypto
- ✅ Can prototype in 1-2 weeks

**Cons:**
- ❌ Requires Node.js runtime
- ❌ Not suitable for consensus layer
- ❌ Performance overhead of FFI

**Use Case:** Wallet GUI, RPC commands, off-chain signing

**Implementation:**
```cpp
// src/wallet.cpp - Wallet signing with ML-DSA via Node.js

#include <node_api.h>

bool SignML DSA(const uint256& hash, std::vector<unsigned char>& vchSig) {
    // Call @smartledger/keys via N-API
    napi_env env;
    napi_create_environment(&env);
    
    // Load @smartledger/keys module
    napi_value module = LoadNodeModule(env, "@smartledger/keys");
    
    // Call signWithKey() function
    napi_value result = CallFunction(env, module, "signWithKey", 
                                     keyId, hash.begin(), hash.size());
    
    // Convert result to vchSig
    ConvertToVector(env, result, vchSig);
    return true;
}
```

**Timeline:** 1-2 weeks for wallet integration

---

### **Option B: C++ Reimplementation (Best for Consensus)**
**Approach:** Port @noble/post-quantum (used by @smartledger/keys) to C++

**Pros:**
- ✅ No external dependencies
- ✅ Suitable for consensus layer
- ✅ Maximum performance
- ✅ Full control over implementation

**Cons:**
- ❌ 3-4 weeks development time
- ❌ Need to maintain crypto code
- ❌ Security audit required

**Use Case:** Consensus validation, OP_CHECKMLDSASIG

**Implementation:**
```cpp
// src/crypto/mldsa.h - Native C++ ML-DSA implementation

#ifndef AUMCOIN_CRYPTO_MLDSA_H
#define AUMCOIN_CRYPTO_MLDSA_H

#include <vector>
#include <cstdint>

namespace MLDSA {
    // ML-DSA-65 parameters (NIST Level 3, recommended)
    constexpr size_t PUBLIC_KEY_BYTES = 1952;
    constexpr size_t PRIVATE_KEY_BYTES = 4032;
    constexpr size_t SIGNATURE_BYTES = 3309;
    
    // Key generation
    bool GenerateKeypair(std::vector<uint8_t>& pubkey, 
                        std::vector<uint8_t>& privkey);
    
    // Signing
    bool Sign(const std::vector<uint8_t>& privkey,
             const uint8_t* message, size_t message_len,
             std::vector<uint8_t>& signature);
    
    // Verification (most important for consensus!)
    bool Verify(const std::vector<uint8_t>& pubkey,
               const uint8_t* message, size_t message_len,
               const std::vector<uint8_t>& signature);
}

#endif // AUMCOIN_CRYPTO_MLDSA_H
```

```cpp
// src/crypto/mldsa.cpp - Implementation

#include "mldsa.h"
#include <noble/post-quantum/ml-dsa-65.hpp> // Port from JS to C++

namespace MLDSA {
    bool Verify(const std::vector<uint8_t>& pubkey,
               const uint8_t* message, size_t message_len,
               const std::vector<uint8_t>& signature) {
        
        // Validate input sizes
        if (pubkey.size() != PUBLIC_KEY_BYTES) return false;
        if (signature.size() != SIGNATURE_BYTES) return false;
        
        // ML-DSA-65 verification (lattice-based crypto)
        // Port from @noble/post-quantum implementation
        return ml_dsa_65_verify(pubkey.data(), message, message_len, 
                               signature.data());
    }
}
```

**Timeline:** 3-4 weeks for full implementation + testing

---

### **Option C: Hybrid Approach (Recommended)**
**Approach:** Use Option B for consensus, Option A for wallet

**Pros:**
- ✅ Fast wallet development (Node.js addon)
- ✅ Secure consensus (native C++)
- ✅ Best of both worlds

**Cons:**
- ⚠️ Two implementations to maintain
- ⚠️ Must ensure compatibility

**Architecture:**
```
┌─────────────────────────────────────────┐
│           AumCoin Node                   │
│                                          │
│  ┌────────────────────────────────┐    │
│  │  Consensus Layer (C++)          │    │
│  │  - OP_CHECKMLDSASIG             │    │
│  │  - Block validation             │    │
│  │  - Native ML-DSA-65             │    │
│  └────────────────────────────────┘    │
│              ▲                           │
│              │                           │
│  ┌───────────┴──────────────────┐      │
│  │  Wallet Layer (C++ + Node.js) │      │
│  │  - Key generation             │      │
│  │  - Transaction signing        │      │
│  │  - Uses @smartledger/keys     │      │
│  └────────────────────────────────┘    │
│                                          │
└─────────────────────────────────────────┘
```

**Timeline:** 2 weeks (wallet) + 3 weeks (consensus) = 5 weeks total

---

## 🔧 **Implementation Plan (Option C - Hybrid)**

### **Phase 3.1: Add OP_CHECKMLDSASIG (Week 1-3)**

**Step 1: Define Opcode**
```cpp
// src/script.h - Add new opcode

enum opcodetype {
    // ... existing opcodes
    OP_CHECKSIG = 0xac,
    OP_CHECKSIGVERIFY = 0xad,
    OP_CHECKMULTISIG = 0xae,
    OP_CHECKMULTISIGVERIFY = 0xaf,
    
    // NEW: Post-quantum signature verification
    OP_CHECKMLDSASIG = 0xb8,        // ML-DSA-65 signature check
    OP_CHECKMLDSASIGVERIFY = 0xb9,  // ML-DSA-65 with VERIFY
    
    // ... rest
};
```

**Step 2: Implement Verification**
```cpp
// src/script.cpp - Add ML-DSA verification

case OP_CHECKMLDSASIG:
{
    if (stack.size() < 2)
        return false;
    
    // Pop signature and public key from stack
    valtype& vchSig = stacktop(-2);
    valtype& vchPubKey = stacktop(-1);
    
    // Validate sizes
    if (vchPubKey.size() != 1952) { // ML-DSA-65 public key
        return false;
    }
    if (vchSig.size() != 3309) { // ML-DSA-65 signature
        return false;
    }
    
    // Get message to verify (transaction hash)
    uint256 sighash = SignatureHash(scriptCode, txTo, nIn, nHashType);
    
    // Verify ML-DSA signature
    bool fSuccess = MLDSA::Verify(vchPubKey, 
                                  sighash.begin(), 32,
                                  vchSig);
    
    // Remove signature and pubkey from stack
    popstack(stack);
    popstack(stack);
    
    // Push result
    stack.push_back(fSuccess ? vchTrue : vchFalse);
    
    if (opcode == OP_CHECKMLDSASIGVERIFY) {
        if (fSuccess)
            popstack(stack);
        else
            return false;
    }
}
break;
```

**Step 3: Write Tests**
```cpp
// src/test/script_tests.cpp - ML-DSA tests

BOOST_AUTO_TEST_CASE(op_checkmldsasig_test)
{
    // Generate ML-DSA-65 keypair
    std::vector<uint8_t> pubkey, privkey;
    MLDSA::GenerateKeypair(pubkey, privkey);
    
    // Create test transaction
    CTransaction txFrom, txTo;
    // ... setup transaction
    
    // Sign with ML-DSA
    uint256 hash = SignatureHash(scriptPubKey, txTo, 0, SIGHASH_ALL);
    std::vector<uint8_t> signature;
    BOOST_CHECK(MLDSA::Sign(privkey, hash.begin(), 32, signature));
    
    // Create script: <sig> <pubkey> OP_CHECKMLDSASIG
    CScript script;
    script << signature << pubkey << OP_CHECKMLDSASIG;
    
    // Verify
    std::vector<std::vector<unsigned char>> stack;
    BOOST_CHECK(EvalScript(stack, script, txTo, 0, 0));
    BOOST_CHECK(CastToBool(stack.back()));
}
```

**Deliverable:** OP_CHECKMLDSASIG working, tests passing

---

### **Phase 3.2: Wallet Integration (Week 4-5)**

**Step 1: Add ML-DSA Key Type**
```cpp
// src/wallet.h - New key type

enum KeyType {
    KEY_TYPE_ECDSA,      // Traditional
    KEY_TYPE_MLDSA,      // Post-quantum
    KEY_TYPE_HYBRID      // Both (dual-signature)
};

class CWallet {
public:
    // Generate ML-DSA address
    std::string GetMLDSAAddress();
    
    // Sign transaction with ML-DSA
    bool SignMLDSA(const CTransaction& tx, std::vector<unsigned char>& vchSig);
    
    // Dual signing (ECDSA + ML-DSA)
    bool SignHybrid(const CTransaction& tx, 
                   std::vector<unsigned char>& vchSigECDSA,
                   std::vector<unsigned char>& vchSigMLDSA);
};
```

**Step 2: Integrate @smartledger/keys**
```cpp
// src/wallet.cpp - Use @smartledger/keys for wallet operations

#include "node_addon_api.h"
#include <smartledger/keys>

bool CWallet::SignMLDSA(const CTransaction& tx, 
                       std::vector<unsigned char>& vchSig) {
    // Get key ID
    std::string keyId = GetMLDSAKeyId();
    
    // Serialize transaction
    CDataStream ss(SER_NETWORK, PROTOCOL_VERSION);
    ss << tx;
    
    // Call @smartledger/keys
    auto sdk = SmartLedger::createKeySDK();
    vchSig = sdk->signWithKey(keyId, ss.data(), ss.size());
    
    return vchSig.size() == 3309; // ML-DSA-65 signature size
}
```

**Deliverable:** Wallet can generate ML-DSA addresses and sign transactions

---

### **Phase 3.3: RPC Commands (Week 5)**

**New RPC Commands:**
```cpp
// src/rpcwallet.cpp - New RPC commands

// Create ML-DSA address
Value createmldsaaddress(const Array& params, bool fHelp) {
    if (fHelp || params.size() > 1)
        throw runtime_error(
            "createmldsaaddress [label]\n"
            "Creates a new post-quantum (ML-DSA-65) address");
    
    string strLabel = params.size() > 0 ? params[0].get_str() : "";
    
    // Generate ML-DSA keypair via @smartledger/keys
    auto sdk = SmartLedger::createKeySDK();
    auto key = sdk->createKey("aumcoin-wallet", {
        primarySignatureSuite: "ml-dsa-65"
    });
    
    // Store in wallet
    pwalletMain->AddMLDSAKey(key);
    
    // Return address
    return pwalletMain->GetMLDSAAddress(key.keyId);
}

// Sign raw transaction with ML-DSA
Value signmldsatransaction(const Array& params, bool fHelp) {
    // ... implementation
}

// Verify signature (works for both ECDSA and ML-DSA)
Value verifysignature(const Array& params, bool fHelp) {
    // ... implementation
}
```

**Deliverable:** Full RPC support for ML-DSA operations

---

## 📐 **Address Format**

### **Option 1: New Address Version (Recommended)**
```
ECDSA Address:  AUM1abcd...  (version 48, starts with 'A')
ML-DSA Address: QUM1abcd...  (version 85, starts with 'Q' for Quantum)
```

**Pros:**
- ✅ Clear distinction
- ✅ No confusion
- ✅ Easy to identify quantum-safe addresses

**Cons:**
- ❌ Requires changes to address validation

### **Option 2: Same Address Format**
Use same version, detect key type from public key size:
- ECDSA: 33 bytes compressed
- ML-DSA-65: 1,952 bytes

**Pros:**
- ✅ No address format changes

**Cons:**
- ❌ Ambiguity in some cases
- ❌ Larger addresses (QR codes harder)

**Recommendation:** Use Option 1 (new address version)

---

## 💾 **Storage Considerations**

### **Signature Size Impact**

| Type | Signature Size | Increase |
|------|----------------|----------|
| ECDSA | 64-73 bytes | Baseline |
| ML-DSA-44 | ~2,420 bytes | 37x larger |
| ML-DSA-65 | ~3,309 bytes | 51x larger |
| ML-DSA-87 | ~4,627 bytes | 71x larger |

**Recommendation:** Use ML-DSA-65 (balanced security/size)

### **Block Size Impact**

**Current:**
- Max block size: 1 MB
- Average tx size: ~250 bytes
- Transactions per block: ~4,000

**With ML-DSA-65:**
- Average tx size: ~3,500 bytes (14x larger)
- Transactions per block: ~285 (14x fewer)

**Solution: Increase Max Block Size**
```cpp
// src/main.h - Increase block size for PQ sigs

static const unsigned int MAX_BLOCK_SIZE = 10000000; // 10 MB (from 1 MB)
```

**Impact:**
- Allows ~2,850 ML-DSA txs per block
- Similar throughput to current ECDSA
- Still reasonable for full nodes

---

## 🔄 **Migration Path**

### **Phase 1: Optional ML-DSA (Q2 2026)**
- ECDSA remains default
- ML-DSA available for early adopters
- Hybrid addresses for power users

### **Phase 2: Encourage Migration (Q3-Q4 2026)**
- Wallet defaults to ML-DSA for new addresses
- ECDSA still fully supported
- Educational campaign about quantum threat

### **Phase 3: Quantum Threat Response (2030+)**
- If quantum threat becomes real, disable ECDSA
- All transactions must use ML-DSA
- Users have 4+ years to migrate

---

## 🧪 **Testing Strategy**

### **Unit Tests**
```bash
# Test ML-DSA crypto primitives
src/test/mldsa_tests.cpp

# Test OP_CHECKMLDSASIG
src/test/script_tests.cpp

# Test wallet operations
src/test/wallet_tests.cpp
```

### **Integration Tests**
```bash
# Test full transaction flow
qa/rpc-tests/mldsa_transactions.py

# Test hybrid signing
qa/rpc-tests/hybrid_signatures.py
```

### **Performance Benchmarks**
```bash
# Benchmark signature verification
src/test/bench/verify_signature.cpp

# Expected results (from @smartledger/keys):
# - Key generation: 3.2x faster than ECDSA
# - Signing: 1.3x faster than ECDSA
# - Verification: 9.4x faster than ECDSA
```

---

## 📊 **Performance Expectations**

Based on @smartledger/keys benchmarks:

| Operation | ECDSA | ML-DSA-65 | Speedup |
|-----------|-------|-----------|---------|
| Keygen | 2.5ms | 0.8ms | **3.2x faster** |
| Sign | 1.2ms | 0.9ms | **1.3x faster** |
| Verify | 2.8ms | 0.3ms | **9.4x faster** |

**Conclusion:** ML-DSA-65 is **faster** than ECDSA for all operations! 🚀

Only downside is signature size (51x larger), but block size increase solves this.

---

## 🛡️ **Security Considerations**

### **Audit Requirements**
- [ ] External security audit of ML-DSA implementation
- [ ] Formal verification of OP_CHECKMLDSASIG
- [ ] Side-channel analysis (timing attacks)
- [ ] Test vectors from NIST

### **Risk Mitigation**
- Use battle-tested @smartledger/keys for wallet (low risk)
- Reimplement from audited @noble/post-quantum for consensus
- Extensive testing period on testnet
- Bug bounty program

---

## 📅 **Timeline & Milestones**

### **Week 1-3: Consensus Layer**
- [ ] Port ML-DSA-65 to C++
- [ ] Implement OP_CHECKMLDSASIG
- [ ] Write test suite
- [ ] Performance benchmarks

### **Week 4-5: Wallet Layer**
- [ ] Integrate @smartledger/keys
- [ ] Add ML-DSA key generation
- [ ] Implement signing functions
- [ ] RPC commands

### **Week 6: Testing**
- [ ] Integration tests
- [ ] Testnet deployment
- [ ] Community testing

### **Week 7-8: Polish**
- [ ] Bug fixes
- [ ] Documentation
- [ ] Security audit preparation

**Total: 8 weeks for production-ready ML-DSA support**

---

## 💰 **Cost/Benefit Analysis**

### **Costs**
- **Development:** 8 weeks (~$20-40K if contracting)
- **Audit:** ~$15-30K for external security review
- **Risk:** Potential bugs in new crypto code

### **Benefits**
- ✅ **Quantum-resistant** - Future-proof against quantum computers
- ✅ **Faster verification** - 9.4x speedup improves node performance
- ✅ **Marketing** - First Bitcoin fork with post-quantum crypto
- ✅ **Differentiation** - Unique value proposition vs other cryptocurrencies
- ✅ **Ecosystem alignment** - Interoperability with Boundless (both use ML-DSA)

**ROI:** High - future-proofs entire network for 10+ years

---

## 🔗 **Dependencies**

### **Required Before Starting**
- [x] Phase 1 security complete (OP_CAT fix, etc.)
- [ ] Phase 2 security complete (OpenSSL, Boost, BDB)
- [ ] @smartledger/keys v2.0+ available
- [ ] Testnet infrastructure ready

### **External Dependencies**
- @smartledger/keys npm package
- @noble/post-quantum (for C++ port)
- Node.js native addon build tools

---

## 📚 **Resources**

### **Reference Implementations**
- @smartledger/keys: https://npmjs.com/package/@smartledger/keys
- @noble/post-quantum: https://github.com/paulmillr/noble-post-quantum
- Boundless (ML-DSA-44): https://boundlesstrust.org

### **Standards**
- NIST FIPS 204: ML-DSA Standard
- NIST FIPS 206: SLH-DSA (hash-based signatures, future consideration)

### **Papers**
- CRYSTALS-Dilithium: https://pq-crystals.org/dilithium/
- "Quantum-Safe Blockchain" (research literature)

---

## 🎯 **Success Criteria**

**Phase 3 is complete when:**
- [ ] OP_CHECKMLDSASIG implemented and tested
- [ ] Wallet can generate ML-DSA addresses
- [ ] RPC commands working
- [ ] Performance meets expectations (9x faster verification)
- [ ] Testnet running with ML-DSA transactions
- [ ] External security audit passed
- [ ] Documentation complete

---

## 🤔 **Open Questions**

1. **Which ML-DSA variant?**
   - Recommendation: ML-DSA-65 (balanced)
   - Alternative: ML-DSA-44 (smaller, faster)

2. **Block size increase?**
   - Recommendation: 10 MB (10x current)
   - Allows similar tx throughput with ML-DSA

3. **Migration timeline?**
   - Recommendation: Optional in 2026, encouraged in 2027, required only if quantum threat emerges (2030+)

4. **Hybrid signing default?**
   - Recommendation: No (too large)
   - Make available for backward compatibility but don't default

---

## 🚀 **Recommended Path Forward**

**My recommendation: Option C (Hybrid Approach)**

1. **Start with wallet integration** (Weeks 1-2)
   - Use @smartledger/keys via Node.js addon
   - Fast development, low risk
   - Users can generate ML-DSA addresses immediately

2. **Then add consensus** (Weeks 3-5)
   - Port @noble/post-quantum to C++
   - Implement OP_CHECKMLDSASIG
   - Higher effort but necessary for validation

3. **Test extensively** (Weeks 6-8)
   - Testnet deployment
   - Performance benchmarking
   - Security audit

**This balances speed (fast wallet) with security (audited consensus).**

---

**Ready to start Phase 3? Let's make AumCoin quantum-resistant! 🛡️**

---

**Last Updated:** January 1, 2026  
**Version:** 1.0  
**Status:** Planning Document - Implementation starts Q2 2026
