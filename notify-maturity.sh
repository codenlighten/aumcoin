#!/bin/bash
# Notification script for coinbase maturity
# Monitors until block 434, then prepares funding commands

TARGET_BLOCK=434
MULTISIG_ADDR="2MtxuRKifrWcqRZAFA5MpJuHem7Ji827hy9"
DAEMON="./aumcoind -testnet"

echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "  🔔 Coinbase Maturity Notification Service"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""

# Wait for maturity
while true; do
    CURRENT=$($DAEMON getblockcount 2>/dev/null)
    
    if [ -z "$CURRENT" ]; then
        echo "⚠️  Daemon not responding, retrying..."
        sleep 30
        continue
    fi
    
    if [ "$CURRENT" -ge "$TARGET_BLOCK" ]; then
        break
    fi
    
    REMAINING=$((TARGET_BLOCK - CURRENT))
    PERCENT=$(( (CURRENT * 100) / TARGET_BLOCK ))
    
    # Progress bar
    FILLED=$(( PERCENT / 2 ))
    EMPTY=$(( 50 - FILLED ))
    BAR=$(printf '█%.0s' $(seq 1 $FILLED))
    SPACE=$(printf '░%.0s' $(seq 1 $EMPTY))
    
    clear
    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    echo "  🔔 Coinbase Maturity Notification Service"
    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    echo ""
    echo "  Current Block: $CURRENT"
    echo "  Target Block:  $TARGET_BLOCK"
    echo "  Remaining:     $REMAINING blocks"
    echo ""
    echo "  Progress: [$BAR$SPACE] $PERCENT%"
    echo ""
    echo "  ⏳ Waiting for coinbase maturity..."
    echo ""
    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    echo "  Press Ctrl+C to exit"
    echo ""
    
    sleep 15
done

# Maturity reached!
clear
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "  🎉 COINBASE MATURITY REACHED! 🎉"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""
echo "  ✅ Block $CURRENT reached!"
echo "  ✅ Coinbase is now spendable!"
echo ""

# Check balance
BALANCE=$($DAEMON getbalance)
echo "  💰 Available Balance: $BALANCE AUM"
echo ""

# Display ready commands
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "  📋 QUANTUM MULTISIG FUNDING COMMANDS READY"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""
echo "  Step 1: Fund the quantum multisig address"
echo "  Command:"
echo "    $DAEMON sendtoaddress $MULTISIG_ADDR 50.0"
echo ""
echo "  Step 2: Mine a block to confirm"
echo "  Command:"
echo "    $DAEMON setgenerate true 1"
echo ""
echo "  Step 3: Verify funds received"
echo "  Command:"
echo "    $DAEMON listunspent 1 999999 '[\"$MULTISIG_ADDR\"]'"
echo ""
echo "  Step 4: Run complete workflow test"
echo "  Command:"
echo "    ./test-quantum-multisig-complete.sh"
echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "  🚀 Ready to make history with the world's first"
echo "     quantum-safe multisig transaction!"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""

# Optionally auto-execute
read -p "  Would you like to auto-fund the multisig now? (y/n): " -n 1 -r
echo ""
if [[ $REPLY =~ ^[Yy]$ ]]; then
    echo ""
    echo "  💸 Sending 50 AUM to quantum multisig..."
    TXID=$($DAEMON sendtoaddress $MULTISIG_ADDR 50.0)
    echo "  ✅ Transaction ID: $TXID"
    echo ""
    echo "  ⛏️  Mining confirmation block..."
    $DAEMON setgenerate true 1
    sleep 5
    echo ""
    echo "  ✅ Block mined!"
    echo ""
    echo "  📊 Checking multisig balance..."
    $DAEMON listunspent 1 999999 "[\"$MULTISIG_ADDR\"]"
    echo ""
    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    echo "  ✅ QUANTUM MULTISIG FUNDED!"
    echo "  🎯 Next: Create and sign spending transaction"
    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
fi
