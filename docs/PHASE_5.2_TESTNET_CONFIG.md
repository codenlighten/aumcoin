# Phase 5.2: Quantum-Safe Testnet Configuration

**Status**: Implementation Phase  
**Date**: January 2, 2026  
**Goal**: Deploy dedicated testnet for quantum-resistant signature validation

---

## 1. Executive Summary

Phase 5.2 establishes "AumTestNet" - a dedicated test network for validating quantum-resistant ML-DSA-65 signatures in a realistic P2P environment. This testnet allows safe testing of post-quantum cryptography features before mainnet deployment.

### Key Features

1. **Separate Network**: Independent from mainnet with unique magic bytes
2. **Quantum Genesis Block**: First block contains ML-DSA signature
3. **Fast Block Time**: 30-second blocks for rapid testing
4. **Low Difficulty**: CPU-mineable for easy testing
5. **Test Coins**: No real value, freely distributed

---

## 2. Network Parameters

### 2.1 Basic Configuration

```cpp
// Network identification
const char* pchMessageStart[4] = { 0xfc, 0xc1, 0xb7, 0xdc };  // AumTestNet magic bytes
const int DEFAULT_PORT = 19335;  // Testnet P2P port (mainnet: 9335)
const int RPC_PORT = 19332;      // Testnet RPC port (mainnet: 9332)

// Chain parameters
const int TESTNET_PROTOCOL_VERSION = 70003;  // Post-quantum protocol
const uint256 hashGenesisBlockTestNet = 
    uint256("0x000000001234567890abcdef..."); // Quantum genesis block

// Block timing
const int64 TESTNET_TARGET_SPACING = 30;  // 30 seconds (vs. 2.5 min mainnet)
const int64 TESTNET_TARGET_TIMESPAN = 3.5 * 24 * 60 * 60;  // 3.5 days

// Difficulty
const CBigNum bnProofOfWorkLimitTestNet(~uint256(0) >> 12);  // Very easy (1/2^12)
```

### 2.2 Network Seeds

```cpp
// DNS seeds for peer discovery
static const char *strTestNetDNSSeed[] = {
    "testnet-seed.aumcoin.org",
    "testnet-dnsseed.aumcoin.io",
    NULL
};

// Hardcoded fallback nodes
static const char *pnSeedTestNet[] = {
    "127.0.0.1:19335",  // Local testing
    NULL
};
```

### 2.3 Checkpoints

```cpp
// Testnet checkpoints (quantum signature verification)
static MapCheckpoints mapCheckpointsTestnet =
    boost::assign::map_list_of
    ( 0, uint256("0x..."))  // Genesis block
    ;
```

---

## 3. Genesis Block Creation

### 3.1 Quantum Genesis Block

The testnet genesis block will contain the **first-ever ML-DSA-65 signature** in a cryptocurrency genesis block!

```cpp
/**
 * CreateQuantumGenesisBlock
 * 
 * Creates genesis block with ML-DSA signature instead of ECDSA
 * This is a historic first - quantum-resistant from block 0!
 */
CBlock CreateQuantumGenesisBlock(
    const char* pszTimestamp,
    const CPubKey& genesisOutputKey,
    uint32_t nTime,
    uint32_t nNonce,
    uint32_t nBits)
{
    CMutableTransaction txNew;
    txNew.nVersion = 1;
    txNew.vin.resize(1);
    txNew.vout.resize(1);
    
    // Coinbase input with timestamp
    txNew.vin[0].scriptSig = CScript() 
        << 486604799 
        << CScriptNum(4) 
        << vector<unsigned char>((const unsigned char*)pszTimestamp, 
                                  (const unsigned char*)pszTimestamp + strlen(pszTimestamp));
    
    // Output: 50 AUM to genesis key (with ML-DSA pubkey)
    txNew.vout[0].nValue = 50 * COIN;
    txNew.vout[0].scriptPubKey = CScript() 
        << genesisOutputKey 
        << OP_CHECKMLDSASIG;  // Quantum-resistant from day one!
    
    CBlock genesis;
    genesis.vtx.push_back(txNew);
    genesis.hashPrevBlock = 0;
    genesis.hashMerkleRoot = genesis.BuildMerkleTree();
    genesis.nVersion = 1;
    genesis.nTime = nTime;
    genesis.nBits = nBits;
    genesis.nNonce = nNonce;
    
    return genesis;
}
```

### 3.2 Genesis Block Parameters

```cpp
// Timestamp: Phase 5.2 completion date
const char* pszTimestamp = "AumCoin 2026-01-02 - World's First Quantum-Safe Cryptocurrency Testnet";

// Genesis block time: January 2, 2026, 16:00:00 GMT
const uint32_t nTimeGenesisTestNet = 1735833600;

// Nonce: Will be mined to satisfy difficulty
uint32_t nNonceGenesisTestNet = 0;  // To be determined

// Difficulty: 1/2^12 (very easy)
const uint32_t nBitsGenesisTestNet = 0x1e0fffff;
```

---

## 4. Configuration Files

### 4.1 aumcoin.conf (Testnet)

```ini
# AumCoin Testnet Configuration
# Place in ~/.aumcoin/testnet/aumcoin.conf

# Network
testnet=1
listen=1
server=1

# P2P
port=19335
maxconnections=125

# RPC
rpcuser=aumcoin_test
rpcpassword=CHANGEME_SECURE_PASSWORD
rpcport=19332
rpcallowip=127.0.0.1

# Mining (for testing)
gen=0  # Enable with gen=1 to mine test blocks

# Quantum features
enablemldsa=1

# Debug (verbose logging for testing)
debug=1
printtoconsole=1

# DNS seeds
dnsseed=1
```

### 4.2 Launch Script

```bash
#!/bin/bash
# start-testnet.sh

# Create testnet data directory
mkdir -p ~/.aumcoin/testnet

# Generate test configuration if not exists
if [ ! -f ~/.aumcoin/testnet/aumcoin.conf ]; then
    cat > ~/.aumcoin/testnet/aumcoin.conf <<EOF
testnet=1
server=1
rpcuser=aumcoin_test
rpcpassword=$(openssl rand -hex 32)
rpcport=19332
enablemldsa=1
debug=1
EOF
    echo "✅ Created testnet configuration"
fi

# Start aumcoind in testnet mode
./aumcoind -testnet -daemon

echo "🚀 AumCoin Testnet started!"
echo "📊 Check status: ./aumcoind -testnet getinfo"
echo "🔍 View debug log: tail -f ~/.aumcoin/testnet/debug.log"
```

---

## 5. Testing Scenarios

### 5.1 Basic Connectivity Test

```bash
# Node 1 (testnet seed)
./aumcoind -testnet -port=19335 -rpcport=19332 -daemon

# Node 2 (connect to seed)
./aumcoind -testnet -port=19336 -rpcport=19333 \
    -connect=127.0.0.1:19335 -daemon

# Check connection
./aumcoind -testnet -rpcport=19332 getpeerinfo
```

### 5.2 Quantum Transaction Test

```bash
# Generate quantum address
ADDR=$(./aumcoind -testnet getnewaddress "quantum_test")

# Mine some test coins
./aumcoind -testnet setgenerate true 1
./aumcoind -testnet generate 101  # 101 blocks for maturity

# Send quantum transaction
./aumcoind -testnet sendtoaddress $ADDR 10

# Verify ML-DSA signature
./aumcoind -testnet getrawtransaction <txid> 1
```

### 5.3 Multisig Test

```bash
# Create 2-of-3 quantum multisig
ADDR1=$(./aumcoind -testnet getnewaddress "alice")
ADDR2=$(./aumcoind -testnet getnewaddress "bob")
ADDR3=$(./aumcoind -testnet getnewaddress "carol")

MULTISIG=$(./aumcoind -testnet addmultisigmldsaaddress 2 \
    "[\"$ADDR1\",\"$ADDR2\",\"$ADDR3\"]" "escrow_test")

# Fund multisig
./aumcoind -testnet sendtoaddress $MULTISIG 50

# Create and sign quantum multisig transaction
# (using Phase 4.3 RPC commands)
```

### 5.4 Network Stress Test

```bash
# Spam quantum transactions
for i in {1..100}; do
    ADDR=$(./aumcoind -testnet getnewaddress "stress_$i")
    ./aumcoind -testnet sendtoaddress $ADDR 0.1
done

# Monitor mempool and block propagation
watch -n 1 './aumcoind -testnet getmempoolinfo'
```

---

## 6. Implementation Checklist

### 6.1 Code Changes

- [ ] Add testnet network parameters to `main.cpp`
- [ ] Create quantum genesis block function
- [ ] Mine testnet genesis block
- [ ] Add testnet checkpoints to `checkpoints.cpp`
- [ ] Update `protocol.h` with testnet magic bytes
- [ ] Add testnet DNS seeds to `net.cpp`
- [ ] Update `init.cpp` for testnet data directory

### 6.2 Configuration

- [ ] Create testnet configuration template
- [ ] Write testnet launch scripts
- [ ] Document testnet RPC ports
- [ ] Set up testnet block explorer (optional)

### 6.3 Testing

- [ ] Verify genesis block creation
- [ ] Test single-node mining
- [ ] Test multi-node P2P
- [ ] Validate quantum transactions
- [ ] Test multisig on testnet
- [ ] Measure block propagation times
- [ ] Verify signature cache performance

---

## 7. Testnet vs Mainnet Comparison

| Feature | Mainnet | Testnet |
|---------|---------|---------|
| **Network ID** | 0xfb, 0xc0, 0xb6, 0xdb | 0xfc, 0xc1, 0xb7, 0xdc |
| **P2P Port** | 9335 | 19335 |
| **RPC Port** | 9332 | 19332 |
| **Block Time** | 2.5 minutes | 30 seconds |
| **Difficulty** | 1/2^20 | 1/2^12 (easier) |
| **Genesis Block** | TBD | Quantum genesis |
| **Coins** | Real value | No value (test) |
| **Reset** | Never | Anytime |
| **Purpose** | Production | Testing |

---

## 8. Quantum Features Testing

### 8.1 ML-DSA Signature Validation

```bash
# Create quantum transaction
TX=$(./aumcoind -testnet createrawtransaction \
    '[{"txid":"...", "vout":0}]' \
    '{"<addr>": 1.0}')

# Sign with ML-DSA
SIGNED=$(./aumcoind -testnet signrawtransaction $TX)

# Verify signature (should use cache)
./aumcoind -testnet decoderawtransaction $SIGNED
```

### 8.2 Signature Cache Testing

```bash
# Send same transaction multiple times to different outputs
# Cache should speed up validation on subsequent blocks

# Before cache warmup
time ./aumcoind -testnet submitblock <block_hex>

# After cache warmup (should be faster)
time ./aumcoind -testnet submitblock <block_hex>
```

### 8.3 Protocol Version Detection

```bash
# Check peer quantum support
./aumcoind -testnet getpeerinfo | grep -A5 "NODE_MLDSA"

# Should show:
# "services": "0000000000000009"  (NODE_NETWORK | NODE_MLDSA)
```

---

## 9. Monitoring and Debugging

### 9.1 Key Metrics

```bash
# Network health
./aumcoind -testnet getnetworkinfo

# Blockchain status
./aumcoind -testnet getblockchaininfo

# Mempool quantum transactions
./aumcoind -testnet getrawmempool true | grep -A10 "quantum"

# Signature cache statistics (via debug log)
tail -f ~/.aumcoin/testnet/debug.log | grep "cache"
```

### 9.2 Debug Commands

```bash
# Enable debug categories
./aumcoind -testnet -debug=net -debug=mldsa -debug=mempool

# Dump mempool
./aumcoind -testnet savemempool

# Invalidate block (for reorg testing)
./aumcoind -testnet invalidateblock <blockhash>

# Reconsider block
./aumcoind -testnet reconsiderblock <blockhash>
```

---

## 10. Testnet Faucet (Future)

### 10.1 Concept

Automated faucet for distributing test AUM:

```javascript
// Pseudocode for testnet faucet
app.post('/faucet/request', async (req, res) => {
    const address = req.body.address;
    
    // Validate quantum address
    if (!isValidQuantumAddress(address)) {
        return res.status(400).json({ error: "Invalid address" });
    }
    
    // Send 100 test AUM
    const txid = await aumcoin.sendToAddress(address, 100);
    
    res.json({ 
        success: true, 
        txid: txid,
        amount: 100,
        message: "100 test AUM sent! Happy quantum testing! 🎉"
    });
});
```

---

## 11. Success Criteria

### 11.1 Network Stability

- ✅ Multiple nodes can connect and sync
- ✅ Blocks propagate within 5 seconds
- ✅ No network splits or forks
- ✅ Consistent block height across nodes

### 11.2 Quantum Transactions

- ✅ ML-DSA signatures validate correctly
- ✅ Quantum multisig works (M-of-N)
- ✅ Transactions propagate through mempool
- ✅ Miners include quantum tx in blocks

### 11.3 Performance

- ✅ Signature cache hit rate >80%
- ✅ Block validation time <1 second
- ✅ Mempool handles 1000+ quantum tx
- ✅ No memory leaks after 24h operation

---

## 12. Known Issues & Workarounds

### 12.1 Testnet Reset

If testnet becomes corrupted:

```bash
# Stop daemon
./aumcoind -testnet stop

# Remove blockchain data (keeps wallet!)
rm -rf ~/.aumcoin/testnet/blocks
rm -rf ~/.aumcoin/testnet/chainstate
rm ~/.aumcoin/testnet/peers.dat

# Restart
./aumcoind -testnet -daemon
```

### 12.2 Port Conflicts

If ports are in use:

```bash
# Use custom ports
./aumcoind -testnet -port=19340 -rpcport=19341
```

### 12.3 Slow Sync

If initial sync is slow:

```bash
# Add bootstrap nodes
./aumcoind -testnet -addnode=<ip>:19335 -addnode=<ip2>:19335
```

---

## 13. Future Enhancements

### 13.1 Testnet v2 Features

- **Quantum Lightning**: Test payment channels
- **Cross-chain**: Atomic swaps with other quantum chains
- **Smart Contracts**: Quantum-safe script templates
- **Privacy**: Quantum-resistant ring signatures

### 13.2 Automated Testing

- CI/CD integration for quantum regression tests
- Nightly testnet stress tests
- Automated fuzz testing of ML-DSA edge cases
- Performance benchmarking suite

---

## 14. Documentation

### 14.1 User Guide

Document for developers wanting to use testnet:

- How to get test coins
- How to create quantum addresses
- How to test multisig scenarios
- Troubleshooting common issues

### 14.2 API Reference

RPC commands specific to testnet:

- `gettestnetinfo` - Testnet-specific status
- `generatequantumblock` - Mine block with quantum tx
- `verifysignaturecache` - Check cache statistics

---

## 15. Timeline

**Week 1:**
- [ ] Implement testnet parameters
- [ ] Create quantum genesis block
- [ ] Basic single-node testing

**Week 2:**
- [ ] Multi-node P2P testing
- [ ] Quantum transaction validation
- [ ] Performance benchmarking

**Week 3:**
- [ ] Multisig testing
- [ ] Stress testing
- [ ] Documentation finalization

---

## 16. Conclusion

Phase 5.2 establishes a robust testnet environment for validating AumCoin's quantum-resistant features. This dedicated network allows safe experimentation with ML-DSA-65 signatures, M-of-N multisig, and post-quantum network protocols before mainnet deployment.

**Key Deliverables:**
- ✅ Quantum genesis block
- ✅ Testnet configuration
- ✅ Testing infrastructure
- ✅ Performance validation

**Next:** Phase 5.3 - Network Validation Tests

---

**Status**: Ready for Implementation  
**Risk Level**: Low (testnet environment)  
**Timeline**: 3 weeks
