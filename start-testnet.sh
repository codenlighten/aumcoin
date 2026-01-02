#!/bin/bash
# AumCoin Testnet Launcher
# Phase 5.2: Quantum-Safe Test Network
# Date: January 2, 2026

set -e

# Colors for output
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${BLUE}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
echo -e "${BLUE}  🚀 AumCoin Testnet Launcher - Phase 5.2                           ${NC}"
echo -e "${BLUE}  World's First Quantum-Safe Cryptocurrency Test Network           ${NC}"
echo -e "${BLUE}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
echo ""

# Get the directory where this script is located
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
AUMCOIN_BIN="$SCRIPT_DIR/src/aumcoind"

# Check if binary exists
if [ ! -f "$AUMCOIN_BIN" ]; then
    echo -e "${YELLOW}⚠️  aumcoind binary not found at: $AUMCOIN_BIN${NC}"
    echo "Building aumcoind..."
    cd "$SCRIPT_DIR/src"
    make -f makefile.unix USE_UPNP=1 ENABLE_MLDSA=1 aumcoind
    cd "$SCRIPT_DIR"
    echo -e "${GREEN}✅ Build complete!${NC}"
fi

# Create testnet data directory
TESTNET_DIR="$HOME/.aumcoin/testnet"
mkdir -p "$TESTNET_DIR"

# Generate configuration if not exists
CONFIG_FILE="$TESTNET_DIR/aumcoin.conf"
if [ ! -f "$CONFIG_FILE" ]; then
    echo -e "${BLUE}📝 Creating testnet configuration...${NC}"
    
    # Generate secure random password
    RPC_PASSWORD=$(openssl rand -hex 32)
    
    cat > "$CONFIG_FILE" <<EOF
# AumCoin Testnet Configuration
# Phase 5.2: Quantum-Safe Test Network
# Generated: $(date)

# Network Settings
testnet=1
listen=1
server=1

# P2P Configuration
port=19335
maxconnections=125

# RPC Configuration
rpcuser=aumcoin_testnet
rpcpassword=$RPC_PASSWORD
rpcport=19332
rpcallowip=127.0.0.1

# Quantum Features
enablemldsa=1

# Debug Settings (verbose logging for testing)
debug=1
printtoconsole=1

# Mining (disabled by default, enable with gen=1)
gen=0

# DNS Seeds (for peer discovery)
dnsseed=1

# Block Size (128 MB for quantum transactions)
blockmaxsize=128000000
EOF
    
    echo -e "${GREEN}✅ Configuration created at: $CONFIG_FILE${NC}"
    echo -e "${YELLOW}🔐 RPC Password: $RPC_PASSWORD${NC}"
    echo ""
fi

# Check if already running
if pgrep -f "aumcoind.*testnet" > /dev/null; then
    echo -e "${YELLOW}⚠️  Testnet already running!${NC}"
    echo ""
    echo "Commands:"
    echo "  Status:  $AUMCOIN_BIN -testnet getinfo"
    echo "  Stop:    $AUMCOIN_BIN -testnet stop"
    echo "  Logs:    tail -f $TESTNET_DIR/debug.log"
    exit 0
fi

# Start aumcoind in testnet mode
echo -e "${BLUE}🚀 Starting AumCoin Testnet...${NC}"
echo ""

# Launch in daemon mode
"$AUMCOIN_BIN" -testnet -daemon

# Wait for startup
echo -n "Waiting for startup"
for i in {1..10}; do
    sleep 1
    echo -n "."
    if "$AUMCOIN_BIN" -testnet getinfo &>/dev/null; then
        break
    fi
done
echo ""

# Check if started successfully
if "$AUMCOIN_BIN" -testnet getinfo &>/dev/null; then
    echo ""
    echo -e "${GREEN}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
    echo -e "${GREEN}  ✅ AumCoin Testnet Started Successfully!                          ${NC}"
    echo -e "${GREEN}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
    echo ""
    echo -e "${BLUE}📊 Network Information:${NC}"
    "$AUMCOIN_BIN" -testnet getinfo
    echo ""
    echo -e "${BLUE}🔧 Useful Commands:${NC}"
    echo "  Check status:         $AUMCOIN_BIN -testnet getinfo"
    echo "  Get new address:      $AUMCOIN_BIN -testnet getnewaddress"
    echo "  Mine test blocks:     $AUMCOIN_BIN -testnet setgenerate true 1"
    echo "  Check blockchain:     $AUMCOIN_BIN -testnet getblockchaininfo"
    echo "  View mempool:         $AUMCOIN_BIN -testnet getrawmempool"
    echo "  Stop testnet:         $AUMCOIN_BIN -testnet stop"
    echo "  View logs:            tail -f $TESTNET_DIR/debug.log"
    echo ""
    echo -e "${BLUE}🧪 Quantum Testing:${NC}"
    echo "  Create quantum addr:  $AUMCOIN_BIN -testnet getnewaddress 'quantum_test'"
    echo "  2-of-3 multisig:      $AUMCOIN_BIN -testnet addmultisigmldsaaddress 2 '[...]'"
    echo "  Cache stats:          grep 'cache' $TESTNET_DIR/debug.log"
    echo ""
    echo -e "${GREEN}🎉 Ready for quantum-safe testing!${NC}"
    echo ""
else
    echo -e "${YELLOW}⚠️  Failed to start. Check logs:${NC}"
    echo "    tail -f $TESTNET_DIR/debug.log"
    exit 1
fi
