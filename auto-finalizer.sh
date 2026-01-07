#!/bin/bash
# auto-finalizer.sh
# Waits for coinbase maturity (block 434) then funds the multisig and executes spending workflow.
# Logs to /tmp/auto-finalizer.log

DAEMON="./aumcoind -testnet"
TARGET=434
LOGFILE="/tmp/auto-finalizer.log"
QUANTUM_MULTISIG="2MtxuRKifrWcqRZAFA5MpJuHem7Ji827hy9"

exec > >(tee -a "$LOGFILE") 2>&1

echo "[auto-finalizer] Started at $(date)" 

while true; do
    CURRENT=$($DAEMON getblockcount 2>/dev/null || echo "")
    if [ -z "$CURRENT" ]; then
        echo "[auto-finalizer] Daemon not responding, retrying in 30s..."
        sleep 30
        continue
    fi
    echo "[auto-finalizer] Current block: $CURRENT (target: $TARGET)"
    if [ "$CURRENT" -ge "$TARGET" ]; then
        echo "[auto-finalizer] Target reached: $CURRENT >= $TARGET"
        break
    fi
    REMAINING=$((TARGET - CURRENT))
    echo "[auto-finalizer] Waiting for maturity: $REMAINING blocks left"
    sleep 20
done

# Once mature, attempt to fund (quick-fund script does checks)
echo "[auto-finalizer] Attempting to fund quantum multisig: $QUANTUM_MULTISIG"
./quick-fund-multisig.sh || { echo "[auto-finalizer] quick-fund-multisig.sh failed"; }

# Give the daemon a few seconds to update
sleep 5

# Run the spending script and automatically confirm the broadcast prompt
# spend-quantum-multisig.sh prompts once to confirm broadcast; feed 'y' to it
if [ -x ./spend-quantum-multisig.sh ]; then
    echo "[auto-finalizer] Running spend-quantum-multisig.sh and auto-confirming broadcast"
    printf "y\n" | ./spend-quantum-multisig.sh || { echo "[auto-finalizer] spend script failed"; }
else
    echo "[auto-finalizer] spend-quantum-multisig.sh not found or not executable"
fi

# Final status
echo "[auto-finalizer] Completed at $(date)"

echo "[auto-finalizer] Final wallet balance:" 
$DAEMON getbalance

echo "[auto-finalizer] UTXOs at multisig address:" 
$DAEMON listunspent 1 999999 "[\"$QUANTUM_MULTISIG\"]"
