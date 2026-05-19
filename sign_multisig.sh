#!/bin/bash
# Script to sign quantum multisig transaction
# Usage: ./sign_multisig.sh <tx_hex> <address>

set -e

DAEMON_CMD="./aumcoind -testnet"
TIMEOUT_SECONDS=${TIMEOUT_SECONDS:-30}
RETRIES=${RETRIES:-5}
RETRY_DELAY=${RETRY_DELAY:-3}

if [ $# -ne 2 ]; then
    echo "Usage: $0 <transaction_hex> <address>"
    echo "Example: $0 \"\$TX_HEX\" muVQrpDASvce2yTUXxtNZ3VdcAULt1AFnb"
    exit 1
fi

TX_HEX="$1"
ADDRESS="$2"

echo "🔐 Signing transaction with address: $ADDRESS"
echo "📏 Input TX length: ${#TX_HEX} chars"
echo "Timeout: ${TIMEOUT_SECONDS}s, Retries: ${RETRIES}"
echo ""

# Wait for daemon RPC to be available
echo "⏳ Waiting for daemon RPC to be ready..."
for i in $(seq 1 12); do
    if $DAEMON_CMD getblockchaininfo >/dev/null 2>&1; then
        echo "✅ Daemon RPC is ready"
        break
    fi
    echo "   daemon not ready yet (attempt $i), retrying in 1s..."
    sleep 1
done

attempt=1
while [ $attempt -le $RETRIES ]; do
    echo "Attempt $attempt/$RETRIES: signing (timeout ${TIMEOUT_SECONDS}s)..."
    # Use timeout to avoid hanging indefinitely
    RESULT=$(timeout "${TIMEOUT_SECONDS}s" $DAEMON_CMD signmldsatx "$TX_HEX" "$ADDRESS" 2>&1 || true)

    if [ -z "$RESULT" ]; then
        echo "⚠️  No response (empty)."
    fi

    if echo "$RESULT" | grep -q '"hex"'; then
        echo "✅ Signing successful on attempt $attempt"

        COMPLETE=$(echo "$RESULT" | grep -o '"complete" : [^,}]*' | cut -d':' -f2 | tr -d ' ')
        SIGNATURES=$(echo "$RESULT" | grep -o '"signatures" : [^,}]*' | cut -d':' -f2 | tr -d ' ')
        REQUIRED=$(echo "$RESULT" | grep -o '"required" : [^,}]*' | cut -d':' -f2 | tr -d ' ')
        NEW_HEX=$(echo "$RESULT" | grep -o '"hex" : "[^"]*"' | cut -d'"' -f4)

        echo "📊 Transaction Status:"
        echo "   Signatures: $SIGNATURES / $REQUIRED"
        echo "   Complete: $COMPLETE"
        echo "   Output length: ${#NEW_HEX} chars"
        echo ""

        echo "$NEW_HEX" > /tmp/signed_tx.hex
        echo "💾 Signed transaction saved to: /tmp/signed_tx.hex"
        echo ""
        echo "📋 Full output:"
        echo "$RESULT"
        exit 0
    fi

    # If timeout triggered, timeout returns non-zero and RESULT may include 'Command terminated'
    if echo "$RESULT" | grep -qi -e 'timed out' -e 'command terminated' -e 'TERM'; then
        echo "⚠️  Command timed out."
    else
        echo "⚠️  Signing failed (daemon returned error). See output below:"
        echo "$RESULT"
    fi

    attempt=$((attempt+1))
    echo "Retrying in ${RETRY_DELAY}s..."
    sleep $RETRY_DELAY
done

echo "❌ All $RETRIES attempts failed. Check daemon logs (~/.aumcoin/debug.log or ~/.aumcoin/testnet3/debug.log) for details."
exit 1
