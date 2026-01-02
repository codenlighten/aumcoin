#!/bin/bash
# AumCoin Testnet Stop Script
# Phase 5.2: Quantum-Safe Test Network

set -e

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
BLUE='\033[0;34m'
NC='\033[0m'

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
AUMCOIN_BIN="$SCRIPT_DIR/src/aumcoind"

echo -e "${BLUE}🛑 Stopping AumCoin Testnet...${NC}"

if ! pgrep -f "aumcoind.*testnet" > /dev/null; then
    echo -e "${RED}⚠️  Testnet is not running${NC}"
    exit 0
fi

# Stop gracefully
"$AUMCOIN_BIN" -testnet stop

# Wait for shutdown
echo -n "Waiting for shutdown"
for i in {1..30}; do
    if ! pgrep -f "aumcoind.*testnet" > /dev/null; then
        echo ""
        echo -e "${GREEN}✅ Testnet stopped successfully${NC}"
        exit 0
    fi
    sleep 1
    echo -n "."
done

echo ""
echo -e "${RED}⚠️  Testnet did not stop gracefully, forcing...${NC}"
pkill -9 -f "aumcoind.*testnet"
echo -e "${GREEN}✅ Testnet stopped${NC}"
