# AumCoin ↔ Boundless Bridge Design

**Cross-Chain Atomic Swaps with Post-Quantum Security**

---

## 🎯 **Executive Summary**

This document specifies a **trustless bridge** between AumCoin (privacy-focused P2P layer) and Boundless Trust (compliance-focused enterprise layer), enabling atomic swaps with zero counterparty risk.

**Key Innovation:** Quantum-resistant from day one using ML-DSA signatures on both chains.

**Timeline:** Q3 2026 (after AumCoin Phase 3 PQC integration)

---

## 📐 **Architecture Overview**

### **The Problem**

Users need to move value between:
- **AumCoin**: Anonymous, high-throughput, script-based smart contracts
- **Boundless**: Compliant, identity-linked, purpose-built transactions

**Traditional bridges are:**
- ❌ Custodial (trust required)
- ❌ Centralized (single point of failure)
- ❌ Vulnerable to quantum attacks (use ECDSA)

### **Our Solution: Hash Time-Locked Contracts (HTLCs)**

**HTLCs** enable atomic swaps:
1. Alice locks AUM on AumCoin chain
2. Bob locks BND on Boundless chain
3. Alice reveals secret to claim BND
4. Bob uses same secret to claim AUM

**Result:** Either both transactions happen, or neither. No custodian needed.

### **Post-Quantum Enhancement**

Both chains use **ML-DSA signatures** for:
- ✅ Quantum-resistant locking scripts
- ✅ Quantum-resistant claim transactions
- ✅ Future-proof against quantum computers

---

## 🔗 **Three-Layer Architecture**

```
┌─────────────────────────────────────────────────────────────┐
│                      Bridge Interface                        │
│                  (User-Facing Layer)                        │
│                                                              │
│  ┌──────────────┐     ┌──────────────┐     ┌─────────────┐│
│  │ Web UI       │     │ CLI Tool     │     │ RPC API     ││
│  │ (React/Next) │     │ (Node.js)    │     │ (REST)      ││
│  └──────────────┘     └──────────────┘     └─────────────┘│
└────────────────────────────┬─────────────────────────────────┘
                             │
        ┌────────────────────┴────────────────────┐
        │                                          │
        ▼                                          ▼
┌──────────────────┐                     ┌──────────────────┐
│  AumCoin Node    │                     │  Boundless Node  │
│  (Privacy Layer) │◄────[HTLC]──────────►│ (Compliance)    │
│                  │                     │                  │
│  Features:       │                     │  Features:       │
│  • 128MB blocks  │                     │  • E2 Identity   │
│  • OP_CAT/etc    │                     │  • KYC 0-4       │
│  • ML-DSA sigs   │                     │  • ML-DSA-44     │
│  • Anonymous     │                     │  • 39 tx types   │
└──────────────────┘                     └──────────────────┘

Legend:
  [HTLC] = Hash Time-Locked Contract (trustless swap)
```

---

## 🔐 **HTLC Protocol Specification**

### **Step-by-Step Atomic Swap**

**Participants:**
- **Alice**: Has 100 AUM, wants 100 BND
- **Bob**: Has 100 BND, wants 100 AUM

**Prerequisites:**
- Alice controls AumCoin address `AUM1alice...` (ML-DSA public key)
- Bob controls Boundless address `BND1bob...` (ML-DSA-44 public key)

---

### **Phase 1: Setup (Off-Chain)**

**1.1 Alice generates secret**
```javascript
// Alice generates random secret (pre-image)
const secret = crypto.randomBytes(32); // 256-bit secret
const secretHash = sha256(secret);     // Hash of secret

console.log("Secret:", secret.toString('hex'));
console.log("Secret Hash:", secretHash.toString('hex'));
```

**Output:**
```
Secret: a1b2c3d4e5f6...  (32 bytes, known only to Alice)
Secret Hash: 5f8a7b3c...  (32 bytes, known to both parties)
```

**1.2 Exchange details**
Alice and Bob exchange:
- Amounts: 100 AUM ↔ 100 BND
- Secret hash: `5f8a7b3c...`
- Timelock: 24 hours (Alice), 12 hours (Bob)
- Addresses: `AUM1alice...`, `BND1bob...`

---

### **Phase 2: Lock on AumCoin (Alice → Bob)**

**2.1 Alice creates HTLC on AumCoin**

```javascript
// AumCoin HTLC script (using restored OP_CODES)
const htlcScript = `
  OP_IF
    // Bob can claim with secret (after Alice reveals it)
    OP_SHA256
    <secretHash>
    OP_EQUALVERIFY
    <bobPublicKeyMLDSA>
    OP_CHECKMLDSASIG
  OP_ELSE
    // Alice can refund after 24 hours
    <locktime>
    OP_CHECKLOCKTIMEVERIFY
    OP_DROP
    <alicePublicKeyMLDSA>
    OP_CHECKMLDSASIG
  OP_ENDIF
`;
```

**Breakdown:**
- **Path 1 (Claim)**: Bob provides `secret`, proves it hashes to `secretHash`, signs with his ML-DSA key
- **Path 2 (Refund)**: After 24 hours, Alice can get her AUM back by signing with her ML-DSA key

**2.2 Alice broadcasts lock transaction**

```json
{
  "txid": "aumcoin_tx_1234...",
  "vin": [
    {
      "txid": "alice_input...",
      "vout": 0,
      "scriptSig": "..."
    }
  ],
  "vout": [
    {
      "value": 100.0,
      "scriptPubKey": "<htlcScript>"
    }
  ],
  "locktime": 1735689600  // Jan 1, 2026 + 24 hours
}
```

**Status:** ✅ 100 AUM locked, Bob can claim with secret

---

### **Phase 3: Lock on Boundless (Bob → Alice)**

**3.1 Bob watches AumCoin chain**

Bob's node detects Alice's lock transaction:
```bash
$ aumcoin-cli gettransaction aumcoin_tx_1234...

{
  "amount": 100.0,
  "confirmations": 6,
  "scriptPubKey": {
    "type": "htlc",
    "secretHash": "5f8a7b3c...",
    "recipient": "AUM1bob...",
    "refundAddress": "AUM1alice...",
    "timelock": 1735689600
  }
}
```

**Verification:**
- ✅ Amount is correct (100 AUM)
- ✅ Secret hash matches (`5f8a7b3c...`)
- ✅ Timelock is 24 hours
- ✅ Bob is recipient

**3.2 Bob creates HTLC on Boundless**

Boundless has **native HTLC transaction type** (no script needed):

```json
{
  "type": "htlc_lock",
  "version": 1,
  "sender": "BND1bob...",
  "amount": 100.0,
  "recipient": "BND1alice...",
  "secretHash": "5f8a7b3c...",
  "timelock": 1735646400,  // 12 hours (less than Alice's)
  "signatureSuite": "ml-dsa-44",
  "signature": "<bobMLDSASignature>"
}
```

**Key difference:** Bob's timelock is **12 hours** (shorter than Alice's 24 hours). This ensures Bob has time to claim on AumCoin after Alice claims on Boundless.

**3.3 Bob broadcasts lock transaction**

```bash
$ boundless-cli sendhtlc BND1alice... 100 5f8a7b3c... 43200

{
  "txid": "boundless_tx_5678...",
  "status": "confirmed",
  "confirmations": 3
}
```

**Status:** ✅ 100 BND locked, Alice can claim with secret

---

### **Phase 4: Claim on Boundless (Alice)**

**4.1 Alice watches Boundless chain**

Alice's node detects Bob's lock:
```bash
$ boundless-cli gettransaction boundless_tx_5678...

{
  "type": "htlc_lock",
  "amount": 100.0,
  "recipient": "BND1alice...",
  "secretHash": "5f8a7b3c...",
  "timelock": 1735646400,
  "confirmations": 6
}
```

**Verification:**
- ✅ Amount is correct (100 BND)
- ✅ Secret hash matches
- ✅ Alice is recipient
- ✅ Sufficient confirmations

**4.2 Alice claims BND by revealing secret**

Alice creates claim transaction:
```json
{
  "type": "htlc_claim",
  "version": 1,
  "htlcTxId": "boundless_tx_5678...",
  "secret": "a1b2c3d4e5f6...",  // ⚠️ SECRET REVEALED ON-CHAIN
  "claimant": "BND1alice...",
  "signatureSuite": "ml-dsa-44",
  "signature": "<aliceMLDSASignature>"
}
```

**4.3 Alice broadcasts claim**

```bash
$ boundless-cli claimhtlc boundless_tx_5678... a1b2c3d4e5f6...

{
  "txid": "boundless_claim_9012...",
  "status": "confirmed",
  "result": "100 BND transferred to BND1alice..."
}
```

**Status:** ✅ Alice receives 100 BND, **secret is now public**

---

### **Phase 5: Claim on AumCoin (Bob)**

**5.1 Bob watches Boundless chain**

Bob's node detects Alice's claim and **extracts the secret**:

```bash
$ boundless-cli gettransaction boundless_claim_9012...

{
  "type": "htlc_claim",
  "secret": "a1b2c3d4e5f6...",  // 🔓 SECRET REVEALED
  "claimant": "BND1alice...",
  "confirmations": 1
}
```

**5.2 Bob claims AUM using revealed secret**

Bob creates claim transaction on AumCoin:

```javascript
// AumCoin claim transaction
const claimTx = {
  vin: [
    {
      txid: "aumcoin_tx_1234...",  // Alice's HTLC
      vout: 0,
      scriptSig: [
        // Push secret (pre-image)
        secret,
        
        // Push Bob's ML-DSA signature
        bobMLDSASignature,
        
        // Select claim path (OP_IF branch = true)
        OP_TRUE
      ]
    }
  ],
  vout: [
    {
      value: 100.0,
      scriptPubKey: "OP_DUP OP_HASH160 <bobPubKeyHash> OP_EQUALVERIFY OP_CHECKSIG"
    }
  ]
};
```

**Script execution:**
```
Stack (bottom to top):
  secret                    // Bob's input
  bobMLDSASignature         // Bob's input
  OP_TRUE                   // Bob's input (selects claim path)

HTLC script executes:
  OP_IF                     // TRUE, take claim path
    OP_SHA256               // Hash the secret
    <secretHash>            // Push expected hash
    OP_EQUALVERIFY          // Verify hash matches ✅
    <bobPublicKeyMLDSA>     // Push Bob's public key
    OP_CHECKMLDSASIG        // Verify Bob's signature ✅
  // ... claim path succeeds
```

**5.3 Bob broadcasts claim**

```bash
$ aumcoin-cli sendrawtransaction <claimTx>

aumcoin_claim_3456...
```

**Status:** ✅ Bob receives 100 AUM

---

### **Result: Atomic Swap Complete! 🎉**

**Final state:**
- Alice: Started with 100 AUM → Now has 100 BND
- Bob: Started with 100 BND → Now has 100 AUM
- ✅ No custodian
- ✅ No counterparty risk
- ✅ Quantum-resistant (ML-DSA signatures)

**Atomicity guaranteed:**
- If Alice doesn't claim BND, secret is never revealed → Bob never learns secret → Alice gets refund after 24h
- If Alice claims BND, secret is revealed → Bob can claim AUM → Both succeed

---

## ⏱️ **Timelock Safety Analysis**

### **Why Different Timelocks?**

- **Alice's timelock: 24 hours** (AumCoin)
- **Bob's timelock: 12 hours** (Boundless)

**Scenario 1: Happy path**
1. **T+0h:** Alice locks AUM (24h timelock)
2. **T+1h:** Bob locks BND (12h timelock)
3. **T+2h:** Alice claims BND (reveals secret)
4. **T+3h:** Bob claims AUM (using revealed secret)
5. **Result:** ✅ Swap complete

**Scenario 2: Alice doesn't claim**
1. **T+0h:** Alice locks AUM (24h timelock)
2. **T+1h:** Bob locks BND (12h timelock)
3. **T+12h:** Bob's timelock expires → Bob gets refund of BND
4. **T+24h:** Alice's timelock expires → Alice gets refund of AUM
5. **Result:** ✅ Both parties refunded, no loss

**Scenario 3: Alice claims late**
1. **T+0h:** Alice locks AUM (24h timelock)
2. **T+1h:** Bob locks BND (12h timelock)
3. **T+13h:** Bob's timelock expired → Bob already got BND refund
4. **T+14h:** Alice tries to claim BND → **FAILS** (already refunded)
5. **T+24h:** Alice gets AUM refund
6. **Result:** ✅ Alice gets her AUM back, no loss

**Scenario 4: Network congestion**
1. **T+0h:** Alice locks AUM (24h timelock)
2. **T+1h:** Bob locks BND (12h timelock)
3. **T+2h:** Alice broadcasts claim, but **transaction delayed**
4. **T+11h:** Alice's claim confirms on Boundless (secret revealed)
5. **T+12h:** Bob sees secret, claims AUM on AumCoin
6. **Result:** ✅ Swap completes even with delays

**Safety Margin:**
- **12-hour gap** ensures Bob has time to claim after Alice
- Even with network congestion, 12 hours is ample time
- If Alice waits too long (>12h), Bob's refund protects him

---

## 🛡️ **Security Analysis**

### **Attack Vectors**

#### **Attack 1: Alice tries to steal both**
**Scenario:** Alice claims BND, then tries to refund AUM before Bob can claim

**Defense:**
1. Alice claims BND → Secret revealed on-chain
2. Bob's software auto-detects secret immediately
3. Bob claims AUM using secret (well before 24h timelock)
4. Alice's refund fails (HTLC already spent by Bob)

**Result:** ❌ Attack impossible (secret can't be hidden once revealed)

---

#### **Attack 2: Bob tries to front-run Alice**
**Scenario:** Bob sees Alice's claim transaction in mempool, tries to extract secret and claim AUM before Alice's transaction confirms

**Defense:**
1. Both Boundless and AumCoin use **ML-DSA signatures** (quantum-resistant)
2. Bob can't forge Alice's signature to claim BND early
3. Alice's claim must confirm first for secret to be publicly visible
4. Once Alice's claim confirms, Bob is **supposed to** claim AUM (this is desired behavior)

**Result:** ❌ Not actually an attack (Bob claiming after Alice is the expected flow)

---

#### **Attack 3: Quantum computer breaks ECDSA**
**Scenario:** Attacker with quantum computer tries to forge signatures

**Defense:**
1. Both chains use **ML-DSA signatures** (resistant to Shor's algorithm)
2. No ECDSA in critical path
3. Quantum attacker gains no advantage

**Result:** ✅ Bridge is quantum-resistant from day one

---

#### **Attack 4: Blockchain reorganization**
**Scenario:** Deep reorg on one chain invalidates HTLC locks

**Defense:**
1. **Wait for sufficient confirmations** before proceeding
   - AumCoin: 6 confirmations (~15 minutes)
   - Boundless: 6 confirmations (~15 minutes)
2. **Longer timelocks** (12-24 hours) exceed reorg risk window
3. **Monitoring software** alerts users to reorgs

**Mitigation:**
- Conservative confirmation requirements (10+ for large swaps)
- Increase timelocks for high-value swaps (48-72 hours)

**Result:** ⚠️ Low risk with proper confirmations

---

#### **Attack 5: Malleability attack**
**Scenario:** Attacker modifies transaction before confirmation to change txid

**Defense:**
1. **AumCoin:** All signatures are ML-DSA (non-malleable by design)
2. **Boundless:** Native HTLC transactions have strict validation
3. **Secret hash** is binding (can't be changed without invalidating)

**Result:** ✅ Both chains resistant to malleability

---

### **Trust Assumptions**

**What users MUST trust:**
- ✅ Cryptography is sound (ML-DSA, SHA256)
- ✅ Both blockchains are secure (51% attack prevention)
- ✅ Software correctly implements HTLC protocol

**What users DON'T need to trust:**
- ❌ Any third party
- ❌ Any centralized bridge operator
- ❌ Any custodian
- ❌ Any relayer or oracle

**Conclusion:** Minimal trust assumptions (only trust the blockchains themselves)

---

## 💻 **Implementation**

### **Component 1: Bridge SDK (JavaScript)**

**Purpose:** Library for developers to build bridge UIs

```javascript
// @aumcoin/bridge-sdk

import { BridgeSDK } from '@aumcoin/bridge-sdk';

// Initialize SDK
const bridge = new BridgeSDK({
  aumcoinRPC: 'http://localhost:9332',
  boundlessRPC: 'http://localhost:8332',
  networks: ['mainnet']
});

// Initiate swap (Alice's perspective)
const swap = await bridge.initiateSwap({
  fromChain: 'aumcoin',
  toChain: 'boundless',
  amount: 100,
  recipientAddress: 'BND1alice...',
  
  // Automated parameters
  generateSecret: true,       // SDK generates secret
  timelock: 24 * 60 * 60,    // 24 hours
  requiredConfirmations: 6
});

console.log('Swap ID:', swap.id);
console.log('Secret Hash:', swap.secretHash);
console.log('Lock Transaction:', swap.lockTxId);

// Monitor swap progress
swap.on('locked', (txid) => {
  console.log('Funds locked:', txid);
});

swap.on('counterparty_locked', (txid) => {
  console.log('Counterparty locked funds:', txid);
  // SDK automatically claims when safe
});

swap.on('completed', (result) => {
  console.log('Swap complete!', result);
  console.log('You received:', result.amountReceived, result.toChain);
});

swap.on('refunded', (txid) => {
  console.log('Swap timed out, funds refunded:', txid);
});
```

---

### **Component 2: Bridge CLI**

**Purpose:** Command-line tool for power users

```bash
# Install CLI
npm install -g @aumcoin/bridge-cli

# Configure networks
bridge config set aumcoin-rpc http://localhost:9332
bridge config set boundless-rpc http://localhost:8332

# Initiate swap (Alice wants 100 BND for 100 AUM)
bridge swap create \
  --from aumcoin \
  --to boundless \
  --amount 100 \
  --recipient BND1alice... \
  --timelock 24h

# Output:
Swap ID: swap_1234abcd
Secret: a1b2c3d4e5f6... (SAVE THIS SECURELY!)
Secret Hash: 5f8a7b3c...
Lock Transaction: aumcoin_tx_1234...

Share secret hash with counterparty: 5f8a7b3c...

# Monitor swap status
bridge swap status swap_1234abcd

Status: WAITING_FOR_COUNTERPARTY
Your lock: aumcoin_tx_1234... (6 confirmations)
Counterparty lock: (not detected)
Time remaining: 23h 45m

# When counterparty locks, SDK auto-claims
bridge swap status swap_1234abcd

Status: CLAIMING
Your lock: aumcoin_tx_1234... (confirmed)
Counterparty lock: boundless_tx_5678... (confirmed)
Claim transaction: boundless_claim_9012... (broadcasting)

# Final status
bridge swap status swap_1234abcd

Status: COMPLETED ✅
You sent: 100 AUM
You received: 100 BND
Secret revealed: a1b2c3d4e5f6...
```

---

### **Component 3: Bridge UI (Web)**

**Purpose:** User-friendly web interface

**Tech Stack:**
- **Frontend:** Next.js 14 + React
- **Wallet:** WalletConnect / MetaMask support
- **Styling:** Tailwind CSS

**Key Features:**
1. **Swap Interface**
   - Select chains (AumCoin ↔ Boundless)
   - Enter amounts
   - One-click swap initiation

2. **Swap Monitoring**
   - Real-time progress updates
   - Transaction links for both chains
   - Estimated time remaining

3. **Swap History**
   - Past swaps
   - Export CSV
   - Tax reporting data

**Screenshot (mockup):**
```
┌────────────────────────────────────────┐
│  AumCoin ↔ Boundless Bridge            │
├────────────────────────────────────────┤
│                                        │
│  From: AumCoin                      ▼ │
│  Amount: [100] AUM                    │
│                                        │
│  To: Boundless                      ▼ │
│  You receive: [100] BND               │
│                                        │
│  Recipient: BND1alice...              │
│                                        │
│  Timelock: 24 hours                 ▼ │
│  Required confirmations: 6            │
│                                        │
│  [Initiate Swap]                      │
│                                        │
└────────────────────────────────────────┘
```

---

### **Component 4: Monitoring Software**

**Purpose:** Auto-detect and claim swaps

```javascript
// bridge-monitor.js

import { BridgeMonitor } from '@aumcoin/bridge-sdk';

const monitor = new BridgeMonitor({
  aumcoinRPC: 'http://localhost:9332',
  boundlessRPC: 'http://localhost:8332',
  
  // Bob's configuration
  wallet: {
    aumcoin: 'AUM1bob...',
    boundless: 'BND1bob...'
  },
  
  // Auto-claim settings
  autoClaim: true,
  confirmations: 6
});

// Start monitoring
monitor.start();

monitor.on('htlc_detected', (swap) => {
  console.log('New HTLC detected:', swap);
  console.log('Secret hash:', swap.secretHash);
  
  // If we're the recipient, auto-lock counterparty funds
  if (swap.recipient === monitor.wallet.aumcoin) {
    console.log('We are recipient, locking counterparty funds...');
    // Bob's software auto-creates lock on Boundless
  }
});

monitor.on('secret_revealed', (swap) => {
  console.log('Secret revealed:', swap.secret);
  
  // Auto-claim if we're recipient
  if (swap.recipient === monitor.wallet.aumcoin) {
    console.log('Claiming funds...');
    // Bob's software auto-claims on AumCoin
  }
});

monitor.on('swap_completed', (swap) => {
  console.log('Swap completed successfully:', swap.id);
});

console.log('Bridge monitor running...');
```

**Deployment:**
- Run on VPS / cloud server
- Monitor both chains 24/7
- Auto-respond to HTLC events

---

## 📊 **Performance Specifications**

### **Swap Latency**

| Phase | Time | Notes |
|-------|------|-------|
| Setup | 1 min | Generate secret, exchange details |
| Alice locks AUM | 2.5 min | 1 block confirmation |
| Bob locks BND | 2.5 min | 1 block confirmation |
| Wait for confirmations | 15 min | 6 confirmations on each chain |
| Alice claims BND | 2.5 min | 1 block |
| Bob claims AUM | 2.5 min | 1 block |
| **Total** | **~25 minutes** | End-to-end swap time |

**Optimization:**
- Reduce confirmations for small swaps (3 confirmations = 7.5 min → **15 min total**)
- Parallel locking (Alice and Bob lock simultaneously → save 2.5 min)

---

### **Throughput**

**Per-chain limits:**
- **AumCoin:** 128 MB blocks = ~50,000 simple transactions per block
- **Boundless:** ~2,000 transactions per block

**Bridge throughput:**
- Limited by Boundless (lower throughput)
- **~2,000 swaps per 2.5 minutes = 800 swaps/min = 48,000 swaps/hour**

**Practical throughput:**
- With monitoring delays, realistic: **10,000 swaps/hour**
- More than sufficient for foreseeable demand

---

### **Costs**

**Transaction fees:**
- **AumCoin:** ~0.001 AUM per tx (2 tx: lock + claim = 0.002 AUM)
- **Boundless:** ~0.01 BND per tx (2 tx: lock + claim = 0.02 BND)

**Total cost per swap:**
- **0.002 AUM + 0.02 BND** (~$0.05-0.10 at expected prices)
- Far cheaper than custodial bridges (0.1-1% fees)

---

## 🧪 **Testing Plan**

### **Phase 1: Unit Tests**

**HTLC Script Tests:**
```javascript
// test/htlc.test.js

describe('HTLC Scripts', () => {
  it('should allow claim with valid secret', async () => {
    const secret = randomBytes(32);
    const secretHash = sha256(secret);
    const htlc = createHTLC(secretHash, bobPubKey, alicePubKey, locktime);
    
    // Bob claims with secret
    const claimTx = createClaimTx(htlc, secret, bobSignature);
    const result = await aumcoin.sendrawtransaction(claimTx);
    
    expect(result).to.be.a('string'); // txid
  });
  
  it('should allow refund after timelock', async () => {
    const htlc = createHTLC(secretHash, bobPubKey, alicePubKey, locktime);
    
    // Advance time past locktime
    await aumcoin.setmocktime(locktime + 1);
    
    // Alice refunds
    const refundTx = createRefundTx(htlc, aliceSignature);
    const result = await aumcoin.sendrawtransaction(refundTx);
    
    expect(result).to.be.a('string'); // txid
  });
  
  it('should reject claim with wrong secret', async () => {
    const wrongSecret = randomBytes(32);
    const claimTx = createClaimTx(htlc, wrongSecret, bobSignature);
    
    await expect(
      aumcoin.sendrawtransaction(claimTx)
    ).to.be.rejectedWith('Script failed');
  });
  
  it('should reject refund before timelock', async () => {
    const refundTx = createRefundTx(htlc, aliceSignature);
    
    await expect(
      aumcoin.sendrawtransaction(refundTx)
    ).to.be.rejectedWith('Timelock not expired');
  });
});
```

---

### **Phase 2: Integration Tests**

**Full Swap Tests:**
```javascript
// test/integration/swap.test.js

describe('Full Atomic Swap', () => {
  it('should complete swap successfully', async () => {
    // Setup
    const alice = await createWallet('aumcoin');
    const bob = await createWallet('boundless');
    
    // Alice initiates swap
    const swap = await bridge.initiateSwap({
      from: 'aumcoin',
      to: 'boundless',
      amount: 100,
      alice: alice.address,
      bob: bob.address
    });
    
    // Bob locks counterparty funds
    await bob.lockHTLC(swap.secretHash, 100, alice.address);
    
    // Alice claims on Boundless (reveals secret)
    const claimTx = await alice.claimHTLC(swap.htlcBoundless, swap.secret);
    expect(claimTx).to.be.a('string');
    
    // Bob extracts secret and claims on AumCoin
    const secret = await boundless.getSecret(claimTx);
    const bobClaimTx = await bob.claimHTLC(swap.htlcAumCoin, secret);
    expect(bobClaimTx).to.be.a('string');
    
    // Verify final balances
    expect(await alice.getBalance('boundless')).to.equal(100);
    expect(await bob.getBalance('aumcoin')).to.equal(100);
  });
  
  it('should refund if Alice doesnt claim', async () => {
    // Setup swap
    const swap = await bridge.initiateSwap({...});
    await bob.lockHTLC(...);
    
    // Wait for timelocks to expire (simulate with mocktime)
    await aumcoin.setmocktime(swap.locktime + 1);
    await boundless.setmocktime(swap.locktime + 1);
    
    // Alice refunds
    const aliceRefund = await alice.refundHTLC(swap.htlcAumCoin);
    expect(aliceRefund).to.be.a('string');
    
    // Bob refunds
    const bobRefund = await bob.refundHTLC(swap.htlcBoundless);
    expect(bobRefund).to.be.a('string');
    
    // Verify original balances restored
    expect(await alice.getBalance('aumcoin')).to.equal(100);
    expect(await bob.getBalance('boundless')).to.equal(100);
  });
});
```

---

### **Phase 3: Testnet Deployment**

**Testnet Swap:**
1. Deploy bridge software on both testnets
2. Perform 100+ test swaps
3. Test failure scenarios (timeouts, invalid secrets, etc.)
4. Monitor performance (latency, throughput)

**Success Criteria:**
- [ ] 100% success rate for valid swaps
- [ ] 100% refund rate for timed-out swaps
- [ ] Average swap time < 30 minutes
- [ ] Zero loss of funds in all scenarios

---

### **Phase 4: Security Audit**

**Scope:**
- HTLC script implementations
- Bridge SDK code
- Monitoring software
- Timelock safety analysis

**Auditors:**
- External security firm (e.g., Trail of Bits, Cure53)
- Bug bounty program

**Budget:** $30-50K

---

## 🚀 **Deployment Timeline**

### **Q2 2026: Development**
- **Week 1-2:** HTLC script implementation
- **Week 3-4:** Bridge SDK development
- **Week 5-6:** CLI and UI development
- **Week 7-8:** Monitoring software

**Deliverables:**
- HTLC scripts operational on both chains
- Bridge SDK (npm package)
- CLI tool
- Web UI (alpha)

---

### **Q3 2026: Testing & Audit**
- **Week 1-2:** Unit and integration tests
- **Week 3-4:** Testnet deployment
- **Week 5-8:** Security audit

**Deliverables:**
- 100+ successful test swaps
- Security audit report
- Bug fixes

---

### **Q4 2026: Mainnet Launch**
- **Week 1-2:** Mainnet deployment
- **Week 3-4:** Limited beta (whitelisted users)
- **Week 5-8:** Public launch

**Deliverables:**
- Bridge live on mainnet
- Public documentation
- User guides

---

## 💡 **Future Enhancements**

### **Enhancement 1: Lightning Network Integration**

**Idea:** Use Lightning-style payment channels for faster swaps

**Benefits:**
- Instant swaps (<1 second)
- Lower fees
- Higher throughput

**Timeline:** 2027+

---

### **Enhancement 2: Multi-Asset Swaps**

**Idea:** Swap AUM for Boundless assets (not just BND)

**Example:**
- Alice swaps 100 AUM for 10 Boundless MuniBond tokens
- Same HTLC protocol, different asset types

**Timeline:** 2027+

---

### **Enhancement 3: Decentralized Order Book**

**Idea:** P2P marketplace for atomic swaps

**Features:**
- Post swap offers (I want to trade X AUM for Y BND)
- Match orders automatically
- No centralized exchange

**Timeline:** 2027+

---

## 📚 **References**

### **Academic Papers**
- "Bitcoin: A Peer-to-Peer Electronic Cash System" (Satoshi Nakamoto, 2008)
- "Atomic Cross-Chain Swaps" (Tier Nolan, 2013)
- "Hashed Timelock Contracts" (Lightning Network Paper, 2016)

### **Implementations**
- Bitcoin HTLC: BIP 199
- Lightning Network: BOLT #3
- Ethereum HTLC: ERC-1630

### **Related Projects**
- Atomic Swaps: https://en.bitcoin.it/wiki/Atomic_swap
- Lightning Network: https://lightning.network
- THORChain: https://thorchain.org (different architecture, but similar goal)

---

## ✅ **Checklist**

**Before implementation starts:**
- [ ] AumCoin Phase 3 complete (ML-DSA support)
- [ ] Boundless mainnet stable (>6 months uptime)
- [ ] Development team assigned (2 developers)
- [ ] Budget approved ($100K: dev $50K + audit $50K)

**Phase 1 (Development):**
- [ ] HTLC scripts written and tested
- [ ] Bridge SDK published to npm
- [ ] CLI tool functional
- [ ] Web UI alpha deployed

**Phase 2 (Testing):**
- [ ] Unit tests passing (100% coverage)
- [ ] Integration tests passing
- [ ] Testnet deployment successful
- [ ] Security audit complete

**Phase 3 (Launch):**
- [ ] Mainnet deployment
- [ ] User documentation published
- [ ] Bug bounty program launched
- [ ] Public announcement

---

## 🎯 **Success Metrics**

**Launch Goals (Q4 2026):**
- 100+ swaps per day
- <0.1% failure rate
- <30 minute average swap time
- Zero loss of funds

**1-Year Goals (Q4 2027):**
- 10,000+ swaps per day
- $1M+ daily volume
- Lightning-style channels operational
- Multi-asset swaps live

---

## 🤝 **Get Involved**

**Developers:**
- Contribute to bridge SDK: github.com/codenlighten/aumcoin-bridge
- Build UIs on top of SDK
- Run monitoring nodes

**Users:**
- Test on testnet
- Provide feedback
- Report bugs

**Investors:**
- Support development
- Sponsor security audits
- Build on top of bridge

---

**Let's connect AumCoin and Boundless - trustless, quantum-resistant, unstoppable!** 🌉🔐

---

**Last Updated:** January 1, 2026  
**Version:** 1.0  
**Status:** Design Complete - Implementation Q3 2026  
**Authors:** AumCoin & Boundless Development Teams
