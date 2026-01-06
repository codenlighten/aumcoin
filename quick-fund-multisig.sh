#!/bin/bash
# Quick command script for funding quantum multisig
# Run this immediately when coinbase is mature

DAEMON="./aumcoind -testnet"
MULTISIG_ADDR="2MtxuRKifrWcqRZAFA5MpJuHem7Ji827hy9"

echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "  💎 Quantum Multisig Quick Fund"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""

# Check current status
BLOCKCOUNT=$($DAEMON getblockcount)
BALANCE=$($DAEMON getbalance)

echo "  Current Block: $BLOCKCOUNT"
echo "  Balance: $BALANCE AUM"
echo "  Multisig Address: $MULTISIG_ADDR"
echo ""

# Verify maturity
if [ "$BLOCKCOUNT" -lt 434 ]; then
    REMAINING=$((434 - BLOCKCOUNT))
    echo "  ⚠️  Coinbase not yet mature!"
    echo "  Need $REMAINING more blocks."
    echo "  Current block: $BLOCKCOUNT / 434"
    exit 1
fi

if (( $(echo "$BALANCE < 50" | bc -l) )); then
    echo "  ⚠️  Insufficient balance for 50 AUM funding."
    echo "  Available: $BALANCE AUM"
    exit 1
fi

echo "  ✅ Coinbase mature - ready to fund!"
echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "  💸 Funding Quantum Multisig"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""

# Send funds
echo "  Sending 50 AUM to quantum multisig address..."
TXID=$($DAEMON sendtoaddress $MULTISIG_ADDR 50.0)

if [ -z "$TXID" ]; then
    echo "  ❌ Transaction failed!"
    exit 1
fi

echo "  ✅ Transaction created!"
echo "  Transaction ID: $TXID"
echo ""

# Mine confirmation
echo "  ⛏️  Mining confirmation block..."
$DAEMON setgenerate true 1
sleep 5

NEW_BLOCK=$($DAEMON getblockcount)
echo "  ✅ Block $NEW_BLOCK mined!"
echo ""

# Verify funds received
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "  📊 Verifying Multisig Balance"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""

UNSPENT=$($DAEMON listunspent 1 999999 "[\"$MULTISIG_ADDR\"]")
echo "$UNSPENT"
echo ""

# Check if funds present
if echo "$UNSPENT" | grep -q "amount"; then
    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    echo "  ✅ SUCCESS! Quantum Multisig Funded!"
    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    echo ""
    echo "  🎯 Next Steps:"
    echo "  1. Note the transaction ID and vout for spending"
    echo "  2. Create raw transaction spending from multisig"
    echo "  3. Sign with Alice's ML-DSA key"
    echo "  4. Sign with Bob's ML-DSA key (2-of-3 satisfied)"
    echo "  5. Broadcast the signed transaction"
    echo ""
    echo "  💡 Quick Command:"
    echo "     ./test-quantum-multisig-complete.sh"
    echo ""
    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
else
    echo "  ⚠️  Funds not visible yet, may need more confirmations"
fi
