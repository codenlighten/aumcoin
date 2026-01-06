#!/bin/bash
# Quantum Multisig Spending Script
# Creates and signs a transaction spending from the world's first quantum multisig

DAEMON="./aumcoind -testnet"
MULTISIG_ADDR="2MtxuRKifrWcqRZAFA5MpJuHem7Ji827hy9"

# Participant addresses
ALICE="muVQrpD4vw3K1CHVkjpCcwMg8P2zZkDqK7"
BOB="mk3aRZ4ZWcYpyV4bqXB2gzVVxQnJ6iXrXD"
CAROL="mfuGti6hX43jBYXEUxN7YQcHAzZmBcbfMW"

echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "  🌟 Quantum Multisig Spending Demonstration"
echo "  World's First Post-Quantum Multisig Transaction"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""

# Step 1: Verify multisig has funds
echo "Step 1: Checking multisig balance..."
UNSPENT=$($DAEMON listunspent 1 999999 "[\"$MULTISIG_ADDR\"]")
echo "$UNSPENT"
echo ""

if ! echo "$UNSPENT" | grep -q "amount"; then
    echo "❌ No funds found in multisig address!"
    echo "   Please fund the address first: ./quick-fund-multisig.sh"
    exit 1
fi

# Extract UTXO details
TXID=$(echo "$UNSPENT" | grep -o '"txid" : "[^"]*"' | cut -d'"' -f4 | head -1)
VOUT=$(echo "$UNSPENT" | grep -o '"vout" : [0-9]*' | awk '{print $3}' | head -1)
AMOUNT=$(echo "$UNSPENT" | grep -o '"amount" : [0-9.]*' | awk '{print $3}' | head -1)

if [ -z "$TXID" ] || [ -z "$VOUT" ]; then
    echo "❌ Could not extract UTXO details!"
    exit 1
fi

echo "✅ Found UTXO in quantum multisig:"
echo "   Transaction ID: $TXID"
echo "   Output Index:   $VOUT"
echo "   Amount:         $AMOUNT AUM"
echo ""

# Step 2: Get redeem script
echo "Step 2: Retrieving multisig redeem script..."

# Recreate multisig to get redeem script
MULTISIG_INFO=$($DAEMON addmultisigmldsaaddress 2 "[\"$ALICE\",\"$BOB\",\"$CAROL\"]")
echo "   Multisig Address: $MULTISIG_INFO"
echo ""

# The redeem script is needed for signing but may not be directly available
# In a real scenario, you'd get this from the wallet or transaction
echo "⚠️  Note: Redeem script retrieval depends on RPC support"
echo "   For manual signing, use: validateaddress $MULTISIG_ADDR"
echo ""

# Step 3: Choose destination
echo "Step 3: Creating spending transaction..."
echo "   We'll send funds back to Alice (demonstrating 2-of-3 approval)"
echo ""

# Calculate amount minus fee (0.001 AUM fee)
SEND_AMOUNT=$(echo "$AMOUNT - 0.001" | bc)
echo "   Send Amount: $SEND_AMOUNT AUM (fee: 0.001 AUM)"
echo ""

# Step 4: Create raw transaction
echo "Step 4: Constructing raw transaction..."
RAW_TX=$($DAEMON createrawtransaction \
    "[{\"txid\":\"$TXID\",\"vout\":$VOUT}]" \
    "{\"$ALICE\":$SEND_AMOUNT}")

if [ -z "$RAW_TX" ]; then
    echo "❌ Failed to create raw transaction!"
    exit 1
fi

echo "✅ Raw transaction created:"
echo "   $RAW_TX"
echo ""

# Step 5: Sign with Alice's key
echo "Step 5: Signing with Alice's ML-DSA key..."
SIGNED_ALICE=$($DAEMON signrawtransaction "$RAW_TX" \
    "[{\"txid\":\"$TXID\",\"vout\":$VOUT,\"scriptPubKey\":\"\",\"redeemScript\":\"\"}]")

if [ $? -ne 0 ]; then
    echo "❌ Alice's signature failed!"
    echo "   This may require enhanced RPC support for ML-DSA multisig"
    exit 1
fi

echo "✅ Alice signed the transaction"
echo ""

# Extract partially signed transaction
PARTIAL_TX=$(echo "$SIGNED_ALICE" | grep -o '"hex" : "[^"]*"' | cut -d'"' -f4)

# Step 6: Sign with Bob's key
echo "Step 6: Signing with Bob's ML-DSA key (2-of-3 threshold)..."
SIGNED_BOB=$($DAEMON signrawtransaction "$PARTIAL_TX" \
    "[{\"txid\":\"$TXID\",\"vout\":$VOUT,\"scriptPubKey\":\"\",\"redeemScript\":\"\"}]")

if [ $? -ne 0 ]; then
    echo "❌ Bob's signature failed!"
    exit 1
fi

echo "✅ Bob signed the transaction"
echo ""

# Extract fully signed transaction
FINAL_TX=$(echo "$SIGNED_BOB" | grep -o '"hex" : "[^"]*"' | cut -d'"' -f4)
COMPLETE=$(echo "$SIGNED_BOB" | grep -o '"complete" : [^,}]*' | awk '{print $3}')

# Step 7: Verify transaction is complete
echo "Step 7: Verifying transaction completeness..."
if [ "$COMPLETE" = "true" ]; then
    echo "✅ Transaction fully signed (2-of-3 threshold met!)"
    echo ""
else
    echo "⚠️  Transaction may need additional signatures"
    echo "   Complete: $COMPLETE"
    echo ""
fi

# Step 8: Decode transaction
echo "Step 8: Decoding final transaction..."
$DAEMON decoderawtransaction "$FINAL_TX"
echo ""

# Step 9: Broadcast transaction
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "  🚀 Ready to Broadcast World's First Quantum Multisig Tx"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""
read -p "  Broadcast this historic transaction? (y/n): " -n 1 -r
echo ""

if [[ $REPLY =~ ^[Yy]$ ]]; then
    echo ""
    echo "  📡 Broadcasting quantum multisig transaction..."
    BROADCAST_TXID=$($DAEMON sendrawtransaction "$FINAL_TX")
    
    if [ $? -eq 0 ]; then
        echo "  ✅ SUCCESS! Transaction broadcast to testnet!"
        echo "  Transaction ID: $BROADCAST_TXID"
        echo ""
        echo "  ⛏️  Mining confirmation block..."
        $DAEMON setgenerate true 1
        sleep 5
        
        BLOCK=$($DAEMON getblockcount)
        echo "  ✅ Block $BLOCK mined!"
        echo ""
        echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
        echo "  🎉 HISTORIC ACHIEVEMENT COMPLETE!"
        echo "  🌟 World's First Quantum-Safe Multisig Transaction"
        echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
        echo ""
        echo "  📊 ML-DSA Signature Cache Metrics:"
        $DAEMON getmldsacachemetrics
        echo ""
        echo "  🔍 Transaction Details:"
        $DAEMON gettransaction "$BROADCAST_TXID"
        echo ""
        echo "  ✅ Phase 5.3 Testing: COMPLETE!"
        echo "  ✅ Quantum Multisig: VALIDATED!"
        echo "  ✅ ML-DSA-65 Signatures: WORKING!"
        echo ""
    else
        echo "  ❌ Broadcast failed!"
        echo "  Error: $BROADCAST_TXID"
    fi
else
    echo ""
    echo "  ⏸️  Broadcast cancelled"
    echo "  Transaction hex saved for manual broadcast:"
    echo "  $FINAL_TX"
    echo ""
fi

echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "  Script complete"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
