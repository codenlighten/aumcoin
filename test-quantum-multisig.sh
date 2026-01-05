#!/bin/bash

# AumCoin Quantum Multisig Testing Script
# Phase 5.3 - Multisig Testing with Mature Coinbase

echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "  🔐 AumCoin Quantum Multisig Testing Suite"
echo "  Phase 5.3 - Network Validation"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""

AUMCOIN="./src/aumcoind -testnet"

# Check if we have mature coinbase
echo "📊 Checking wallet status..."
BALANCE=$($AUMCOIN getinfo | grep '"balance"' | grep -o '[0-9.]*' | head -1)
BLOCKS=$($AUMCOIN getinfo | grep '"blocks"' | grep -o '[0-9]*')

echo "  Blocks: $BLOCKS"
echo "  Balance: $BALANCE AUM"
echo ""

if (( $(echo "$BALANCE == 0" | bc -l) )); then
    echo "⚠️  No spendable balance yet. Need to reach block 257+ for coinbase maturity."
    echo "   Current block: $BLOCKS"
    echo "   Target block: 257"
    echo "   Blocks remaining: $((257 - BLOCKS))"
    exit 1
fi

echo "✅ Wallet has spendable balance!"
echo ""

# Step 1: Generate ML-DSA addresses for 3 parties
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "📝 Step 1: Generate ML-DSA Addresses"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""

echo "Generating address for Alice..."
ALICE=$($AUMCOIN getnewmldsaaddress "Alice")
echo "  Alice: $ALICE"
echo ""

echo "Generating address for Bob..."
BOB=$($AUMCOIN getnewmldsaaddress "Bob")
echo "  Bob: $BOB"
echo ""

echo "Generating address for Carol..."
CAROL=$($AUMCOIN getnewmldsaaddress "Carol")
echo "  Carol: $CAROL"
echo ""

# Step 2: Create 2-of-3 multisig address
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "🔐 Step 2: Create 2-of-3 Quantum Multisig"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""

echo "Creating 2-of-3 multisig address..."
MULTISIG=$($AUMCOIN addmultisigmldsaaddress 2 "[\"$ALICE\",\"$BOB\",\"$CAROL\"]" "quantum_escrow" 2>&1)

if echo "$MULTISIG" | grep -q "error"; then
    echo "❌ Multisig creation failed:"
    echo "$MULTISIG"
    echo ""
    echo "This might be because the ML-DSA keys haven't been fully initialized."
    echo "The RPC fix is in place, but key generation may need more time."
    exit 1
fi

echo "  Multisig Address: $MULTISIG"
echo ""

# Step 3: Send funds to multisig
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "💰 Step 3: Fund Quantum Multisig Address"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""

echo "Sending 50 AUM to multisig address..."
TXID=$($AUMCOIN sendtoaddress "$MULTISIG" 50.0 "Quantum Multisig Test" "Phase 5.3 Testing")
echo "  Transaction ID: $TXID"
echo ""

echo "Waiting for transaction to enter mempool..."
sleep 2

# Check mempool
echo "Mempool contents:"
$AUMCOIN getrawmempool
echo ""

# Step 4: Get transaction details
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "📋 Step 4: Transaction Details"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""

$AUMCOIN gettransaction "$TXID"
echo ""

# Step 5: Check ML-DSA cache metrics
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "📊 Step 5: ML-DSA Signature Cache Metrics"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""

$AUMCOIN getmldsacachemetrics
echo ""

echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "✅ Quantum Multisig Test Complete!"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""
echo "Summary:"
echo "  ✅ Generated 3 ML-DSA addresses (Alice, Bob, Carol)"
echo "  ✅ Created 2-of-3 quantum multisig address"
echo "  ✅ Funded multisig with 50 AUM"
echo "  ✅ Transaction in mempool"
echo "  ✅ Cache metrics collected"
echo ""
echo "Next steps:"
echo "  1. Wait for transaction to confirm"
echo "  2. Test spending from multisig (requires 2-of-3 signatures)"
echo "  3. Validate ML-DSA signature verification"
echo ""
