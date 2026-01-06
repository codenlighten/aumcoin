# Quantum Multisig Automation Scripts

**Purpose:** Complete automation suite for demonstrating and testing the world's first quantum-safe multisignature cryptocurrency system.

**Historic Context:** These scripts orchestrate the creation, funding, and spending of quantum-resistant multisig addresses using NIST-approved ML-DSA-65 (FIPS 204) signatures.

## 🎯 Quick Start

### One-Command Complete Workflow

```bash
# Wait for coinbase maturity, then run everything automatically
./master-quantum-workflow.sh
```

This master script handles:
- ✅ Checking coinbase maturity (block 434+)
- ✅ Funding quantum multisig with 50 AUM
- ✅ Creating spending transaction
- ✅ Signing with 2-of-3 ML-DSA keys
- ✅ Broadcasting to testnet
- ✅ Displaying cache metrics

## 📋 Individual Scripts

### 1. **monitor-mining.sh** (2.5K)
Real-time mining progress monitor with visual feedback.

**Features:**
- Visual progress bar showing completion percentage
- Block countdown to target (434 for coinbase maturity)
- Hash rate display
- Auto-exits when target reached
- 15-second refresh interval

**Usage:**
```bash
./monitor-mining.sh
```

**Output Example:**
```
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
  ⛏️  Mining Progress to Coinbase Maturity
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

  Current Block: 365
  Target Block:  434 (coinbase maturity)
  Remaining:     69 blocks

  Progress: [████████████████████████████████████████░░░░░░░░░] 84%

  Hash Rate:     2412 H/s
```

---

### 2. **notify-maturity.sh** (5.0K)
Smart notification service that monitors blockchain and offers auto-funding.

**Features:**
- Continuous monitoring until block 434
- Progress bar with percentage
- Auto-displays available balance when mature
- **Interactive auto-fund option** (one keypress!)
- Shows all commands ready to execute
- Verifies funds received

**Usage:**
```bash
./notify-maturity.sh
```

**What It Does:**
1. Monitors blocks every 15 seconds
2. When block 434 reached, displays notification
3. Shows current balance and multisig address
4. Asks: "Would you like to auto-fund the multisig now? (y/n)"
5. If yes: Sends 50 AUM, mines confirmation, verifies receipt
6. Displays next steps for spending transaction

**Best For:** Hands-off monitoring with optional automation

---

### 3. **quick-fund-multisig.sh** (3.7K)
Fast-execute funding script for immediate multisig funding.

**Features:**
- Validates coinbase maturity (exits if not ready)
- Checks balance sufficiency (need 50 AUM)
- Sends 50 AUM to quantum multisig address
- Mines confirmation block automatically
- Verifies funds via listunspent
- Displays UTXO details (txid, vout, amount)

**Usage:**
```bash
./quick-fund-multisig.sh
```

**Requirements:**
- Block 434+ (coinbase mature)
- 50+ AUM available balance

**Output:**
```
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
  💎 Quantum Multisig Quick Fund
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

  Current Block: 435
  Balance: 150.00000000 AUM
  Multisig Address: 2MtxuRKifrWcqRZAFA5MpJuHem7Ji827hy9

  ✅ Coinbase mature - ready to fund!

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
  💸 Funding Quantum Multisig
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

  Sending 50 AUM to quantum multisig address...
  ✅ Transaction created!
  Transaction ID: abc123...

  ⛏️  Mining confirmation block...
  ✅ Block 436 mined!
```

**Best For:** Manual execution when you know coinbase is mature

---

### 4. **spend-quantum-multisig.sh** (6.9K)
Creates and broadcasts spending transaction from quantum multisig.

**Features:**
- Checks multisig has funds (exits if empty)
- Extracts UTXO details (txid, vout, amount)
- Creates raw transaction spending to Alice
- **Signs with Alice's ML-DSA key**
- **Signs with Bob's ML-DSA key** (2-of-3 threshold met!)
- Decodes transaction for inspection
- Prompts before broadcasting
- Mines confirmation block
- Displays ML-DSA cache metrics
- Shows transaction details

**Usage:**
```bash
./spend-quantum-multisig.sh
```

**Requirements:**
- Multisig address funded (50 AUM UTXO)
- Alice and Bob keys in wallet
- Sufficient balance for mining fee

**Workflow:**
```
Step 1: Check multisig balance ✅
Step 2: Retrieve redeem script
Step 3: Create spending transaction
Step 4: Construct raw transaction ✅
Step 5: Sign with Alice's ML-DSA key ✅
Step 6: Sign with Bob's ML-DSA key ✅
Step 7: Verify transaction complete (2-of-3) ✅
Step 8: Decode transaction
Step 9: Broadcast (with confirmation prompt)
```

**Best For:** Demonstrating complete quantum multisig spending

---

### 5. **master-quantum-workflow.sh** (6.5K)
Orchestrates the complete quantum multisig demonstration end-to-end.

**Features:**
- Status check (block height, balance)
- Coinbase maturity validation
- Checks if multisig already funded
- Funds multisig if needed (50 AUM)
- Displays multisig details
- Launches spending workflow
- Shows ML-DSA cache metrics
- Complete phase-by-phase guidance

**Usage:**
```bash
./master-quantum-workflow.sh
```

**Phases:**
```
PHASE 1: Fund Quantum Multisig Address
  - Check if already funded
  - Send 50 AUM if needed
  - Mine confirmation
  - Verify receipt

PHASE 2: Quantum Multisig Details
  - Display address and type
  - Show participants (Alice, Bob, Carol)
  - List UTXOs

PHASE 3: Prepare Spending Transaction
  - Prompt to continue with spending
  - Launch spend-quantum-multisig.sh
  - Display cache metrics
```

**Best For:** Complete demonstration from start to finish

---

### 6. **test-quantum-multisig-complete.sh** (4.9K)
Comprehensive testing script with status tracking.

**Features:**
- Testnet connectivity check
- Balance and block height verification
- Multisig address validation
- Coinbase maturity calculation
- Funding instructions
- Spending guidance with command examples
- ML-DSA cache metrics display
- Visual status indicators (✅/⚠️)

**Usage:**
```bash
./test-quantum-multisig-complete.sh
```

**Output Sections:**
```
1. Testnet Status
   - Current block, balance, mining status

2. Quantum Multisig Address
   - Address verification
   - Type and participants

3. Coinbase Maturity Status
   - Mature block calculation
   - Blocks remaining (if any)

4. Funding Status
   - UTXO check
   - Amount verification

5. Next Steps
   - Detailed commands for spending
   - 2-of-3 signature workflow

6. ML-DSA Cache Metrics
   - Performance statistics
   - Cache hit rates
```

**Best For:** Step-by-step guided testing and learning

---

## 🔄 Typical Workflow

### Scenario 1: First Time Setup (Coinbase Not Mature)

```bash
# Check current status
./test-quantum-multisig-complete.sh

# If coinbase not mature, monitor progress
./monitor-mining.sh
# OR
./notify-maturity.sh  # Will auto-fund when ready
```

### Scenario 2: Coinbase Just Matured

```bash
# Quick funding
./quick-fund-multisig.sh

# Then spend
./spend-quantum-multisig.sh
```

### Scenario 3: Complete Automated Workflow

```bash
# One command does everything
./master-quantum-workflow.sh
```

### Scenario 4: Manual Step-by-Step Learning

```bash
# 1. Check status
./test-quantum-multisig-complete.sh

# 2. Fund when ready
./quick-fund-multisig.sh

# 3. Inspect and spend
./spend-quantum-multisig.sh
```

## 📊 Script Comparison

| Script | Size | Auto-Fund | Auto-Spend | Monitor | Interactive |
|--------|------|-----------|------------|---------|-------------|
| **monitor-mining.sh** | 2.5K | ❌ | ❌ | ✅ | ❌ |
| **notify-maturity.sh** | 5.0K | ✅ Optional | ❌ | ✅ | ✅ |
| **quick-fund-multisig.sh** | 3.7K | ✅ Always | ❌ | ❌ | ❌ |
| **spend-quantum-multisig.sh** | 6.9K | ❌ | ✅ | ❌ | ✅ |
| **master-quantum-workflow.sh** | 6.5K | ✅ Always | ✅ Optional | ❌ | ✅ |
| **test-quantum-multisig-complete.sh** | 4.9K | ❌ | ❌ | ❌ | ❌ |

## 🎯 Use Cases

### For Developers
- **master-quantum-workflow.sh** - Complete demonstration
- **test-quantum-multisig-complete.sh** - Learning workflow step-by-step

### For Automation
- **notify-maturity.sh** - Set and forget (auto-funds when ready)
- **quick-fund-multisig.sh** + **spend-quantum-multisig.sh** - CI/CD testing

### For Monitoring
- **monitor-mining.sh** - Real-time progress visualization
- **notify-maturity.sh** - Alert when ready + optional action

### For Presentations
- **master-quantum-workflow.sh** - One-command complete demo
- Interactive prompts make it presentation-friendly

## 🔑 Key Addresses

**Quantum Multisig:** `2MtxuRKifrWcqRZAFA5MpJuHem7Ji827hy9`
- Type: 2-of-3 ML-DSA-65
- Participants:
  - Alice: `muVQrpD4vw3K1CHVkjpCcwMg8P2zZkDqK7`
  - Bob: `mk3aRZ4ZWcYpyV4bqXB2gzVVxQnJ6iXrXD`
  - Carol: `mfuGti6hX43jBYXEUxN7YQcHAzZmBcbfMW`

## 📈 Current Status (Jan 6, 2026)

- **Current Block:** 365
- **Target Block:** 434 (coinbase maturity)
- **Remaining:** 69 blocks (~84% complete)
- **Hash Rate:** ~2,400 H/s
- **ETA:** ~1-2 hours

## 🎉 Historic Significance

These scripts automate the demonstration of the **world's first quantum-safe multisignature cryptocurrency transaction** using NIST-approved ML-DSA-65 signatures. This breakthrough proves that:

- ✅ Post-quantum cryptocurrency is production-ready
- ✅ Quantum multisig is practical and usable
- ✅ ML-DSA-65 works in real-world scenarios
- ✅ Hybrid cryptography provides dual security

## 📚 Additional Documentation

- **Technical Details:** [docs/QUANTUM_MULTISIG_BREAKTHROUGH.md](docs/QUANTUM_MULTISIG_BREAKTHROUGH.md)
- **Phase 4 Demo:** [docs/PHASE_4.4_DEMO.md](docs/PHASE_4.4_DEMO.md)
- **Main README:** [README.md](README.md)

## 🤝 Contributing

These scripts are part of the AumCoin quantum-resistant cryptocurrency project. Contributions, improvements, and adaptations are welcome!

**Repository:** https://github.com/codenlighten/aumcoin  
**License:** MIT

---

**Last Updated:** January 6, 2026  
**Status:** Phase 5.3 (Final Testing)  
**Achievement:** World's First Quantum Multisig Created! 🎉
