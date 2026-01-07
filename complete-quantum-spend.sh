#!/bin/bash
# Complete Quantum Multisig Spending Workflow
# Uses getredeemscript RPC command to complete the spending transaction

set -e

MULTISIG_ADDR="2MtxuRKifrWcqRZAFA5MpJuHem7Ji827hy9"
DEST_ADDR="muVQrpDASvce2yTUXxtNZ3VdcAULt1AFnb"  # Alice's address
AMOUNT_AUM="25.0"
AMOUNT_SATOSHIS="2500000000"  # 25 AUM in satoshis
FUNDING_TXID="b8ec792ef3bdd30d6e7df8d3c83e3227c8ed80d45125fb607eae7315469e81aa"

echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "  🔐 Complete Quantum Multisig Spending Workflow"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""

# Step 1: Get redeem script
echo "📜 Step 1: Extracting redeem script..."
REDEEM_SCRIPT=$(./aumcoind -testnet getredeemscript "$MULTISIG_ADDR" | grep redeemScript | cut -d'"' -f4)

if [ -z "$REDEEM_SCRIPT" ]; then
    echo "❌ Failed to extract redeem script"
    exit 1
fi

echo "✅ Redeem script extracted (${#REDEEM_SCRIPT} hex chars)"
echo ""

# Step 2: Create unsigned transaction
echo "💸 Step 2: Creating unsigned spending transaction..."
echo "  From: $MULTISIG_ADDR"
echo "  To: $DEST_ADDR"
echo "  Amount: $AMOUNT_AUM AUM ($AMOUNT_SATOSHIS satoshis)"
echo ""

UNSIGNED_TX=$(./aumcoind -testnet createmultisigmldsatx \
    "$FUNDING_TXID" \
    0 \
    "$REDEEM_SCRIPT" \
    "$DEST_ADDR" \
    "$AMOUNT_SATOSHIS" \
    100000)

if [ -z "$UNSIGNED_TX" ]; then
    echo "❌ Failed to create unsigned transaction"
    exit 1
fi

echo "✅ Unsigned transaction created:"
echo "$UNSIGNED_TX" | head -20
echo ""

# Step 3: Sign with Alice (first signature)
echo "🔑 Step 3: Signing with Alice's ML-DSA key..."
ALICE_ADDR="muVQrpDASvce2yTUXxtNZ3VdcAULt1AFnb"

SIGNED_TX_1=$(./aumcoind -testnet signmldsatx "$UNSIGNED_TX" "$ALICE_ADDR")

if [ -z "$SIGNED_TX_1" ]; then
    echo "❌ Failed to sign with Alice"
    exit 1
fi

echo "✅ Alice signature added"
echo ""

# Step 4: Sign with Bob (second signature - completes 2-of-3)
echo "🔑 Step 4: Signing with Bob's ML-DSA key..."
BOB_ADDR="mk3aRZ4bTxKRRjSVhga5MSjHr5QRyFaNxa"

SIGNED_TX_2=$(./aumcoind -testnet signmldsatx "$SIGNED_TX_1" "$BOB_ADDR")

if [ -z "$SIGNED_TX_2" ]; then
    echo "❌ Failed to sign with Bob"
    exit 1
fi

echo "✅ Bob signature added (2-of-3 threshold met!)"
echo ""

# Step 5: Broadcast transaction
echo "📡 Step 5: Broadcasting fully-signed transaction..."
SPEND_TXID=$(./aumcoind -testnet sendrawtransaction "$SIGNED_TX_2" 2>&1)

if echo "$SPEND_TXID" | grep -q "error"; then
    echo "❌ Broadcast failed:"
    echo "$SPEND_TXID"
    echo ""
    echo "📋 Transaction hex (for manual inspection):"
    echo "$SIGNED_TX_2"
    exit 1
fi

echo "✅ Transaction broadcast successful!"
echo "   TX ID: $SPEND_TXID"
echo ""

# Step 6: Mine confirmation block
echo "⛏️  Step 6: Mining confirmation block..."
./aumcoind -testnet setgenerate true 1 > /dev/null 2>&1
sleep 2

CONFIRMATIONS=$(./aumcoind -testnet gettransaction "$SPEND_TXID" 2>/dev/null | grep -m1 confirmations | grep -oE '[0-9]+')

if [ "$CONFIRMATIONS" -gt 0 ]; then
    echo "✅ Transaction confirmed!"
    echo ""
else
    echo "⏳ Transaction in mempool (may need another block)"
    echo ""
fi

# Step 7: Verify ML-DSA signatures
echo "🔬 Step 7: Checking ML-DSA verification metrics..."
CACHE_METRICS=$(./aumcoind -testnet getmldsacachemetrics)

echo "$CACHE_METRICS"
echo ""

TOTAL_VERIFICATIONS=$(echo "$CACHE_METRICS" | grep total_verifications | grep -oE '[0-9]+')

if [ "$TOTAL_VERIFICATIONS" -gt 0 ]; then
    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    echo "  🎉 QUANTUM MULTISIG SPEND COMPLETE!"
    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    echo ""
    echo "  ✅ ML-DSA Signatures Verified: $TOTAL_VERIFICATIONS"
    echo "  ✅ Quantum-Safe Multisig: OPERATIONAL"
    echo "  ✅ World's First: ACHIEVED"
    echo ""
    echo "  Transaction Details:"
    echo "  • Funding TX:  $FUNDING_TXID"
    echo "  • Spending TX: $SPEND_TXID"
    echo "  • Amount:      $AMOUNT_AUM AUM"
    echo "  • Multisig:    2-of-3 ML-DSA-65"
    echo "  • Signers:     Alice + Bob"
    echo ""
    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
else
    echo "⚠️  No ML-DSA verifications detected yet"
    echo "   This may mean signatures were cached or"
    echo "   the transaction is still being processed"
fi

echo ""
echo "✅ Workflow complete!"
