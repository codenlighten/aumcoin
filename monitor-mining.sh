#!/bin/bash

# Monitor mining progress toward coinbase maturity
# Shows countdown to first spendable coinbase

AUMCOIN="./src/aumcoind -testnet"

echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "  ⛏️  Mining Progress Monitor"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""

while true; do
    BLOCKS=$($AUMCOIN getblockcount 2>/dev/null)
    MININGINFO=$($AUMCOIN getmininginfo 2>/dev/null)
    HASHRATE=$(echo "$MININGINFO" | grep -o '"hashespersec" : [0-9]*' | grep -o '[0-9]*')
    
    if [ -z "$BLOCKS" ]; then
        echo "⚠️  Daemon not responding..."
        sleep 5
        continue
    fi
    
    TARGET=434
    REMAINING=$((TARGET - BLOCKS))
    PROGRESS=$((BLOCKS * 100 / TARGET))
    
    # Create progress bar
    FILLED=$((PROGRESS / 2))
    EMPTY=$((50 - FILLED))
    BAR=$(printf "%${FILLED}s" | tr ' ' '█')
    BAR="${BAR}$(printf "%${EMPTY}s" | tr ' ' '░')"
    
    clear
    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    echo "  ⛏️  Mining Progress to Coinbase Maturity"
    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    echo ""
    echo "  Current Block: $BLOCKS"
    echo "  Target Block:  $TARGET (coinbase maturity)"
    echo "  Remaining:     $REMAINING blocks"
    echo ""
    echo "  Progress: [$BAR] ${PROGRESS}%"
    echo ""
    echo "  Hash Rate:     $HASHRATE H/s"
    echo ""
    
    if [ $REMAINING -le 0 ]; then
        echo "  🎉 COINBASE MATURE! Ready to test quantum multisig!"
        echo ""
        echo "  Run: ./test-quantum-multisig-complete.sh"
        echo ""
        break
    else
        echo "  ⏳ Mining in progress..."
        if [ $REMAINING -le 20 ]; then
            echo "  🔥 Almost there! Only $REMAINING blocks left!"
        fi
    fi
    
    echo ""
    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    echo "  Press Ctrl+C to exit"
    echo ""
    
    sleep 15
done
