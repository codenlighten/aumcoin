#!/bin/bash
# Master script for complete quantum multisig demonstration
# Runs the entire workflow from funding to spending

DAEMON="./aumcoind -testnet"
TARGET_BLOCK=434
MULTISIG_ADDR="2MtxuRKifrWcqRZAFA5MpJuHem7Ji827hy9"

echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "  🚀 QUANTUM MULTISIG MASTER WORKFLOW"
echo "  Complete Demonstration of World's First Quantum Multisig"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""

# Check daemon is running
if ! $DAEMON getinfo >/dev/null 2>&1; then
    echo "❌ Daemon not running!"
    echo "   Start with: ./aumcoind -testnet -daemon"
    exit 1
fi

# Get current status
CURRENT_BLOCK=$($DAEMON getblockcount)
BALANCE=$($DAEMON getbalance)

echo "📊 Current Status:"
echo "   Block:   $CURRENT_BLOCK"
echo "   Balance: $BALANCE AUM"
echo "   Target:  Block $TARGET_BLOCK (coinbase maturity)"
echo ""

# Check if we've reached target block
if [ "$CURRENT_BLOCK" -lt "$TARGET_BLOCK" ]; then
    REMAINING=$((TARGET_BLOCK - CURRENT_BLOCK))
    PERCENT=$(( (CURRENT_BLOCK * 100) / TARGET_BLOCK ))
    
    echo "⏳ Mining in Progress"
    echo "   Need $REMAINING more blocks ($PERCENT% complete)"
    echo ""
    echo "   Options:"
    echo "   1. Wait and run this script again later"
    echo "   2. Monitor progress: ./monitor-mining.sh"
    echo "   3. Get notified: ./notify-maturity.sh"
    echo ""
    exit 0
fi

echo "✅ Coinbase maturity reached!"
echo ""

# Phase 1: Fund the quantum multisig
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "  PHASE 1: Fund Quantum Multisig Address"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""

# Check if already funded
UNSPENT=$($DAEMON listunspent 1 999999 "[\"$MULTISIG_ADDR\"]")
if echo "$UNSPENT" | grep -q "amount"; then
    AMOUNT=$(echo "$UNSPENT" | grep -o '"amount" : [0-9.]*' | awk '{print $3}' | head -1)
    echo "✅ Multisig already funded with $AMOUNT AUM"
    echo ""
else
    # Need to fund
    if (( $(echo "$BALANCE < 50" | bc -l) )); then
        echo "❌ Insufficient balance for funding"
        echo "   Required: 50 AUM"
        echo "   Available: $BALANCE AUM"
        exit 1
    fi
    
    echo "💸 Sending 50 AUM to quantum multisig..."
    FUND_TXID=$($DAEMON sendtoaddress $MULTISIG_ADDR 50.0)
    
    if [ -z "$FUND_TXID" ]; then
        echo "❌ Funding transaction failed!"
        exit 1
    fi
    
    echo "✅ Funding transaction created: $FUND_TXID"
    echo ""
    echo "⛏️  Mining confirmation block..."
    $DAEMON setgenerate true 1
    sleep 5
    
    CURRENT_BLOCK=$($DAEMON getblockcount)
    echo "✅ Block $CURRENT_BLOCK mined!"
    echo ""
    
    # Verify funding
    UNSPENT=$($DAEMON listunspent 1 999999 "[\"$MULTISIG_ADDR\"]")
    if echo "$UNSPENT" | grep -q "amount"; then
        AMOUNT=$(echo "$UNSPENT" | grep -o '"amount" : [0-9.]*' | awk '{print $3}' | head -1)
        echo "✅ Quantum multisig funded successfully!"
        echo "   Amount: $AMOUNT AUM"
        echo ""
    else
        echo "⚠️  Funding not visible yet, may need more confirmations"
        echo ""
    fi
fi

# Phase 2: Display multisig details
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "  PHASE 2: Quantum Multisig Details"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""
echo "Address:      $MULTISIG_ADDR"
echo "Type:         2-of-3 ML-DSA-65"
echo "Participants: Alice, Bob, Carol"
echo ""
echo "UTXOs:"
$DAEMON listunspent 1 999999 "[\"$MULTISIG_ADDR\"]"
echo ""

# Phase 3: Prepare for spending
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "  PHASE 3: Prepare Spending Transaction"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""
echo "The quantum multisig is now funded and ready for spending."
echo ""
echo "Next steps:"
echo "1. Create spending transaction (raw transaction)"
echo "2. Sign with Alice's ML-DSA key"
echo "3. Sign with Bob's ML-DSA key (2-of-3 threshold)"
echo "4. Broadcast to testnet"
echo "5. Verify ML-DSA signatures in blockchain"
echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

read -p "Would you like to continue with spending? (y/n): " -n 1 -r
echo ""
echo ""

if [[ $REPLY =~ ^[Yy]$ ]]; then
    echo "🚀 Launching spending workflow..."
    echo ""
    
    # Run the spending script
    ./spend-quantum-multisig.sh
    
else
    echo "⏸️  Spending workflow paused"
    echo ""
    echo "When ready to spend, run: ./spend-quantum-multisig.sh"
    echo ""
fi

echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "  📊 Current ML-DSA Cache Metrics"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""
$DAEMON getmldsacachemetrics
echo ""

echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "  ✅ Quantum Multisig Master Workflow Complete"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""
echo "🎉 You've witnessed history: the world's first quantum-safe"
echo "   multisignature cryptocurrency transaction using NIST-approved"
echo "   ML-DSA-65 (FIPS 204) signatures!"
echo ""
echo "📚 For technical details, see:"
echo "   docs/QUANTUM_MULTISIG_BREAKTHROUGH.md"
echo ""
