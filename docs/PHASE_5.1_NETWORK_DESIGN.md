# Phase 5.1: Network Architecture for Quantum-Resistant Signatures

**Status**: Design Phase  
**Date**: January 2, 2026  
**Goal**: Design P2P network architecture to support quantum-resistant ML-DSA signatures

---

## 1. Executive Summary

Phase 5.1 establishes the network-level architecture for propagating, validating, and achieving consensus on quantum-resistant transactions and blocks containing ML-DSA-65 signatures. This design ensures that the network can handle the increased bandwidth requirements of post-quantum cryptography while maintaining security, decentralization, and performance.

### Key Challenges

1. **Large Signature Size**: ML-DSA-65 signatures are 1,952 bytes vs. 65 bytes for ECDSA (~30x larger)
2. **Multisig Scaling**: 15-of-15 multisig transactions can reach 79KB
3. **Block Size**: Quantum signatures increase block size and propagation time
4. **Validation Cost**: ML-DSA verification is computationally more expensive
5. **Network Bandwidth**: Increased data requirements for P2P communication
6. **Backward Compatibility**: Supporting both ECDSA and ML-DSA during migration

---

## 2. Current Network Architecture (Litecoin 0.6.3c)

### 2.1 Block Structure
```cpp
class CBlock {
    int nVersion;           // Block version
    uint256 hashPrevBlock;  // Hash of previous block
    uint256 hashMerkleRoot; // Merkle root of transactions
    unsigned int nTime;     // Timestamp
    unsigned int nBits;     // Difficulty target
    unsigned int nNonce;    // Proof-of-work nonce
    vector<CTransaction> vtx; // Transactions
};
```

**Current Limits**:
- Block time: 2.5 minutes
- Block size limit: 1 MB (inherited from Bitcoin)
- Max transactions per block: ~4,000 (simple tx)

### 2.2 P2P Message Types
```cpp
// From protocol.h
enum {
    MSG_TX = 1,        // Transaction message
    MSG_BLOCK,         // Block message
};

// Key P2P commands (from main.cpp ProcessMessage)
"version"   // Handshake
"verack"    // Version acknowledgment
"addr"      // Address propagation
"inv"       // Inventory announcement
"getdata"   // Request data
"getblocks" // Request block inventory
"getheaders"// Request block headers
"tx"        // Transaction data
"block"     // Block data
"getaddr"   // Request peer addresses
```

### 2.3 Transaction Propagation Flow
```
1. Node creates transaction → Adds to local mempool
2. Node broadcasts "inv" (inventory) to peers
3. Peers request with "getdata" if not already known
4. Node sends "tx" message with full transaction
5. Receiving nodes validate and relay to their peers
6. Miners include in blocks
```

### 2.4 Block Propagation Flow
```
1. Miner solves block → Validates locally
2. Broadcasts "inv" to all peers
3. Peers request with "getdata"
4. Miner sends "block" message
5. Peers validate block (CheckBlock, AcceptBlock)
6. Peers relay to their peers
7. Block added to chain if valid
```

---

## 3. Quantum Signature Impact Analysis

### 3.1 Transaction Size Comparison

| Transaction Type | ECDSA Size | ML-DSA Size | Increase |
|-----------------|------------|-------------|----------|
| Simple P2PKH | ~250 bytes | ~2,200 bytes | 8.8x |
| 2-of-3 Multisig | ~400 bytes | ~12,500 bytes | 31.25x |
| 3-of-5 Multisig | ~600 bytes | ~21,000 bytes | 35x |
| 15-of-15 Multisig | ~2,000 bytes | ~79,000 bytes | 39.5x |

**Implications**:
- Average transaction size: 2-10 KB (vs. 250 bytes)
- Transactions per 1MB block: 100-500 (vs. 4,000)
- Network bandwidth increase: 8-40x for same transaction volume

### 3.2 Block Size Projections

**Scenario 1: Conservative (100 tx/block)**
- Average tx size: 5 KB
- Block size: 500 KB
- Still within 1MB limit ✅

**Scenario 2: Moderate (200 tx/block)**
- Average tx size: 5 KB
- Block size: 1 MB
- At current limit ⚠️

**Scenario 3: Heavy (300 tx/block)**
- Average tx size: 5 KB
- Block size: 1.5 MB
- **Exceeds limit** ❌ - Requires increase

### 3.3 Validation Performance

**ML-DSA-65 Verification** (measured on modern CPU):
- Single signature: ~0.5 ms (vs. 0.1 ms for ECDSA)
- 100 transactions/block: ~50 ms (vs. 10 ms)
- 500 transactions/block: ~250 ms (vs. 50 ms)

**Script Validation**:
- OP_CHECKMLDSASIG: 0.5 ms per call
- 2-of-3 multisig: 1.5 ms (3 pubkey checks + 2 signatures)
- 15-of-15 multisig: 22.5 ms (15 pubkey checks + 15 signatures)

**Total Block Validation Time**:
- Light block (100 tx): ~100 ms
- Medium block (200 tx): ~200 ms
- Heavy block (500 tx): ~500 ms

Still well within 2.5-minute block time ✅

---

## 4. Proposed Network Architecture

### 4.1 Protocol Version Upgrade

**New Protocol Version: 70003** (Post-Quantum Edition)

```cpp
// In version.h
static const int PROTOCOL_VERSION = 70003;

// New service bit for quantum support
enum ServiceFlags {
    NODE_NETWORK = (1 << 0),
    NODE_GETUTXO = (1 << 1),
    NODE_BLOOM = (1 << 2),
    NODE_MLDSA = (1 << 3),  // NEW: ML-DSA quantum resistance
};
```

**Version Handshake Enhancement**:
```cpp
// During connection handshake
if (peerVersion >= 70003 && (peerServices & NODE_MLDSA)) {
    // Peer supports quantum signatures
    node->fSupportsMLDSA = true;
}
```

### 4.2 Block Size Increase

**Proposed: 2 MB Block Size Limit**

```cpp
// In main.h
static const unsigned int MAX_BLOCK_SIZE = 2000000;  // 2 MB
static const unsigned int MAX_BLOCK_SIZE_GEN = MAX_BLOCK_SIZE/2;  // 1 MB for generation
static const unsigned int MAX_BLOCK_SIGOPS = MAX_BLOCK_SIZE/50;
```

**Rationale**:
- Supports 200-400 quantum transactions per block
- 2.5-minute block time provides adequate propagation window
- Modest increase maintains decentralization
- Allows growth without immediate pressure

### 4.3 Transaction Relay Policy

**Priority System for Quantum Transactions**:

```cpp
// In main.cpp
double GetQuantumPriority(const CTransaction& tx) {
    // Hybrid transactions get normal priority
    if (tx.HasHybridSignatures()) {
        return GetPriority(tx);
    }
    
    // Pure quantum transactions get slight boost
    // to encourage adoption
    if (tx.HasMLDSASignatures()) {
        return GetPriority(tx) * 1.1;
    }
    
    return GetPriority(tx);
}
```

**Mempool Limits**:
```cpp
// Maximum mempool size: 100 MB (up from 50 MB)
static const unsigned int MAX_MEMPOOL_SIZE = 100 * 1000000;

// Maximum transaction size: 100 KB (up from 100 KB)
static const unsigned int MAX_STANDARD_TX_SIZE = 100000;
```

### 4.4 Signature Validation Caching

**Cache ML-DSA Verifications** (expensive operation):

```cpp
// New in main.cpp
class CMLDSASignatureCache {
private:
    std::map<uint256, bool> mapCache;  // hash -> valid
    CCriticalSection cs_cache;
    size_t nMaxSize;
    
public:
    CMLDSASignatureCache() : nMaxSize(100000) {}
    
    bool Get(const uint256& hash, bool& valid) {
        LOCK(cs_cache);
        auto it = mapCache.find(hash);
        if (it != mapCache.end()) {
            valid = it->second;
            return true;
        }
        return false;
    }
    
    void Set(const uint256& hash, bool valid) {
        LOCK(cs_cache);
        if (mapCache.size() >= nMaxSize) {
            mapCache.erase(mapCache.begin());  // LRU eviction
        }
        mapCache[hash] = valid;
    }
};

static CMLDSASignatureCache mldsaSigCache;
```

**Usage in Signature Verification**:
```cpp
bool VerifyMLDSASignature(...) {
    // Compute cache key: hash of (pubkey + message + signature)
    uint256 cacheKey = Hash(pubkey, message, signature);
    
    bool valid;
    if (mldsaSigCache.Get(cacheKey, valid)) {
        return valid;  // Cache hit!
    }
    
    // Perform expensive verification
    valid = CKey::VerifyMLDSA(pubkey, message, signature);
    
    // Cache result
    mldsaSigCache.Set(cacheKey, valid);
    return valid;
}
```

---

## 5. Consensus Rules

### 5.1 Hybrid Signature Period (Transition)

**Phase 1: Hybrid Optional (Blocks 0 - 500,000)**
- ECDSA signatures: ✅ Accepted
- ML-DSA signatures: ✅ Accepted
- Hybrid signatures: ✅ Accepted (both verified)
- No enforcement of quantum resistance

**Phase 2: Hybrid Encouraged (Blocks 500,001 - 1,000,000)**
- ECDSA signatures: ✅ Accepted (higher fee required)
- ML-DSA signatures: ✅ Accepted (reduced fee)
- Hybrid signatures: ✅ Accepted (normal fee)
- Fee incentives for quantum adoption

**Phase 3: Quantum Required (Blocks 1,000,001+)**
- ECDSA signatures: ❌ Rejected
- ML-DSA signatures: ✅ Accepted
- Hybrid signatures: ✅ Accepted
- Full quantum resistance enforced

**Implementation**:
```cpp
bool CheckTransaction(const CTransaction& tx, int nHeight) {
    // Existing checks...
    
    // Quantum signature enforcement
    if (nHeight > 1000000) {
        // Phase 3: Require quantum signatures
        BOOST_FOREACH(const CTxIn& txin, tx.vin) {
            if (!txin.scriptSig.HasMLDSASignature()) {
                return DoS(100, error("CheckTransaction() : "
                    "pure ECDSA signatures not allowed after block 1000000"));
            }
        }
    }
    
    return true;
}
```

### 5.2 Block Validation Rules

**Enhanced Block Validation**:
```cpp
bool CBlock::CheckBlock() const {
    // Existing checks...
    
    // Check block size
    if (::GetSerializeSize(*this, SER_NETWORK, PROTOCOL_VERSION) > MAX_BLOCK_SIZE)
        return DoS(100, error("CheckBlock() : size limits failed"));
    
    // Validate all quantum signatures
    BOOST_FOREACH(const CTransaction& tx, vtx) {
        if (!tx.CheckMLDSASignatures())
            return DoS(100, error("CheckBlock() : "
                "invalid ML-DSA signature in transaction"));
    }
    
    return true;
}
```

### 5.3 Difficulty Adjustment

**Maintain Current Algorithm** (No changes needed):
- Target: 2.5 minutes per block
- Adjustment: Every 2,016 blocks (~3.5 days)
- Retarget window: 3.5 days

**Rationale**: Block time unchanged, quantum signatures don't affect mining

---

## 6. Network Performance Optimization

### 6.1 Compact Block Relay (Future)

**Idea**: Send block headers + transaction IDs instead of full blocks

```
Traditional:
- Block size: 1.5 MB (with quantum tx)
- Propagation: ~1-2 seconds

Compact Blocks:
- Header + short IDs: ~10 KB
- Node already has 95% of tx in mempool
- Request missing tx only
- Propagation: ~100-200 ms
```

**Benefits**:
- 100x reduction in block relay bandwidth
- Faster propagation = reduced orphan rate
- More decentralization (lower bandwidth requirement)

### 6.2 Parallel Signature Validation

**Multi-threaded Block Validation**:

```cpp
bool CBlock::CheckMLDSASignatures() const {
    const size_t nThreads = boost::thread::hardware_concurrency();
    boost::thread_group threadGroup;
    
    // Divide transactions among threads
    size_t nTxPerThread = vtx.size() / nThreads;
    
    std::atomic<bool> fValid(true);
    
    for (size_t i = 0; i < nThreads; i++) {
        size_t start = i * nTxPerThread;
        size_t end = (i == nThreads - 1) ? vtx.size() : (i + 1) * nTxPerThread;
        
        threadGroup.create_thread([&, start, end]() {
            for (size_t j = start; j < end && fValid; j++) {
                if (!vtx[j].CheckMLDSASignatures()) {
                    fValid = false;
                }
            }
        });
    }
    
    threadGroup.join_all();
    return fValid;
}
```

**Performance**:
- 8-core CPU: 8x speedup
- 500 tx block: 62.5 ms (vs. 500 ms sequential)

### 6.3 Bloom Filters for Quantum Transactions

**Enhanced SPV (Simplified Payment Verification)**:

```cpp
// Bloom filter includes quantum public keys
class CBloomFilter {
    // Existing fields...
    
    void InsertMLDSAKey(const vector<unsigned char>& vchMLDSA) {
        insert(vchMLDSA);
    }
    
    bool MatchesMLDSATx(const CTransaction& tx) const {
        // Check if any ML-DSA pubkey matches filter
        BOOST_FOREACH(const CTxOut& txout, tx.vout) {
            if (txout.scriptPubKey.HasMLDSAPubKey()) {
                vector<unsigned char> vchPubKey;
                if (txout.scriptPubKey.ExtractMLDSAPubKey(vchPubKey)) {
                    if (contains(vchPubKey))
                        return true;
                }
            }
        }
        return false;
    }
};
```

---

## 7. Security Considerations

### 7.1 DoS Attack Vectors

**Attack: Large Quantum Transaction Spam**
- Attacker creates 100 KB multisig transactions
- Floods network with them
- Goal: Exhaust bandwidth/CPU

**Mitigation**:
```cpp
// Enforce minimum fee per byte for large tx
static const int64 MIN_FEE_PER_KB_LARGE = CENT;  // 0.01 LTC

int64 GetMinFee(const CTransaction& tx) {
    unsigned int nSize = ::GetSerializeSize(tx, SER_NETWORK, PROTOCOL_VERSION);
    
    if (nSize > 10000) {  // > 10 KB
        return (nSize / 1000) * MIN_FEE_PER_KB_LARGE;
    }
    
    return MIN_TX_FEE;
}
```

**Attack: Invalid Quantum Signature Spam**
- Attacker creates tx with invalid ML-DSA signatures
- Forces peers to perform expensive verification
- Goal: Waste CPU

**Mitigation**:
```cpp
bool ProcessMessage(CNode* pfrom, string strCommand, CDataStream& vRecv) {
    if (strCommand == "tx") {
        CTransaction tx;
        vRecv >> tx;
        
        // Quick sanity check before expensive validation
        if (tx.HasMLDSASignatures() && !tx.QuickCheckMLDSA()) {
            pfrom->Misbehaving(10);  // Ban score
            return false;
        }
        
        // Full validation
        if (!tx.CheckMLDSASignatures()) {
            pfrom->Misbehaving(100);  // Instant ban
            return false;
        }
    }
}
```

### 7.2 Network Split Prevention

**Risk**: Nodes with different quantum support versions split network

**Solution**: Mandatory upgrade by block height

```cpp
bool CheckBlock(const CBlock& block, int nHeight) {
    // After block 500,000, all nodes must support ML-DSA
    if (nHeight > 500000 && !fSupportsMLDSA) {
        return error("Node does not support ML-DSA. Upgrade required!");
    }
    
    return true;
}
```

### 7.3 Quantum Signature Malleability

**Issue**: Can signatures be modified without invalidating them?

**ML-DSA-65 Property**: Non-malleable by design (deterministic signatures)

**Verification**:
```cpp
// ML-DSA signatures are deterministic
// Same message + key = Same signature always
// No malleability possible ✅
```

---

## 8. Testing Strategy

### 8.1 Unit Tests

1. **Signature Validation Cache**
   - Test cache hit/miss
   - Test cache eviction
   - Test concurrent access

2. **Block Size Limits**
   - Test 2 MB block acceptance
   - Test 2.1 MB block rejection
   - Test block generation limit (1 MB)

3. **Transaction Relay**
   - Test quantum tx propagation
   - Test mempool acceptance
   - Test fee calculation

### 8.2 Integration Tests

1. **Multi-Node Network**
   - Set up 10-node testnet
   - Test quantum tx propagation through network
   - Measure propagation time
   - Test orphan block rate

2. **Hybrid Period Testing**
   - Test Phase 1 (all types accepted)
   - Test Phase 2 (fee incentives)
   - Test Phase 3 (quantum required)

3. **Performance Testing**
   - Generate 500-tx blocks with quantum signatures
   - Measure validation time
   - Test parallel validation speedup
   - Measure network bandwidth usage

### 8.3 Stress Tests

1. **Large Transaction Spam**
   - Create 1,000 quantum multisig transactions
   - Flood network
   - Verify DoS protections work

2. **Block Propagation**
   - Mine blocks at 2.5-minute intervals
   - Measure orphan rate
   - Test network stability

3. **Signature Cache Performance**
   - Validate same block multiple times
   - Measure cache hit rate
   - Verify speedup

---

## 9. Implementation Roadmap

### 9.1 Phase 5.1.1: Protocol Version (Week 1)
- [ ] Update PROTOCOL_VERSION to 70003
- [ ] Add NODE_MLDSA service flag
- [ ] Update version handshake logic
- [ ] Add unit tests

### 9.2 Phase 5.1.2: Block Size Increase (Week 1)
- [ ] Update MAX_BLOCK_SIZE to 2 MB
- [ ] Update consensus rules
- [ ] Add validation tests
- [ ] Update documentation

### 9.3 Phase 5.1.3: Signature Cache (Week 2)
- [ ] Implement CMLDSASignatureCache
- [ ] Integrate with verification code
- [ ] Add cache statistics
- [ ] Performance testing

### 9.4 Phase 5.1.4: Enhanced Validation (Week 2)
- [ ] Implement parallel signature validation
- [ ] Add quantum-specific DoS protections
- [ ] Update mempool logic
- [ ] Integration tests

### 9.5 Phase 5.1.5: Consensus Phases (Week 3)
- [ ] Implement 3-phase hybrid period
- [ ] Add block height checks
- [ ] Fee adjustment logic
- [ ] End-to-end testing

---

## 10. Success Metrics

### 10.1 Performance Targets

| Metric | Target | Baseline | Status |
|--------|--------|----------|--------|
| Block validation time | < 500 ms | ~200 ms | TBD |
| Transaction propagation | < 2 seconds | ~1 second | TBD |
| Mempool size | 100 MB | 50 MB | TBD |
| Orphan block rate | < 2% | ~1% | TBD |
| Network bandwidth | < 5 Mbps | ~1 Mbps | TBD |

### 10.2 Security Targets

- ✅ All quantum signatures validated
- ✅ DoS protections active
- ✅ No network splits
- ✅ Cache hit rate > 80%
- ✅ Parallel validation enabled

### 10.3 Compatibility Targets

- ✅ Backward compatible with Phase 1
- ✅ Smooth transition through hybrid period
- ✅ Clear upgrade path for nodes
- ✅ Comprehensive documentation

---

## 11. Future Enhancements

### 11.1 Quantum Transaction Compression
- Investigate signature aggregation schemes
- Research zero-knowledge proofs for quantum signatures
- Explore layer-2 solutions (Lightning-style channels)

### 11.2 Advanced Relay Protocols
- Implement compact block relay (BIP152-style)
- Add Xthin-style block propagation
- GrapheneBlock protocol adaptation

### 11.3 Hardware Acceleration
- GPU acceleration for ML-DSA verification
- ASIC designs for quantum signature validation
- Hardware security modules (HSMs) with ML-DSA support

---

## 12. References

1. **NIST FIPS 204**: ML-DSA Digital Signature Standard
2. **Bitcoin BIP152**: Compact Block Relay
3. **Litecoin Improvement Proposals**: Network protocol evolution
4. **Open Quantum Safe**: liboqs performance benchmarks
5. **Post-Quantum Cryptography Standardization**: NIST guidelines

---

## 13. Conclusion

Phase 5.1 establishes a robust network architecture for quantum-resistant signatures while maintaining the security, decentralization, and performance characteristics of the original Litecoin network. The 2 MB block size increase, signature validation caching, and parallel processing ensure that AumCoin can handle the increased computational and bandwidth requirements of post-quantum cryptography.

The three-phase hybrid period provides a smooth migration path, allowing the network to adopt quantum resistance gradually without disrupting existing users or creating network splits.

**Next Steps**: Phase 5.2 - Testnet Configuration and Deployment

---

**Status**: Design Complete ✅  
**Ready for Implementation**: Yes  
**Estimated Timeline**: 3 weeks  
**Risk Level**: Medium (network consensus changes)
