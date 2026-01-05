#!/bin/bash

# AumCoin Testnet Node 2 Launcher
# Second testnet node for P2P testing

echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "  🚀 AumCoin Testnet Node 2 Launcher"
echo "  Multi-Node P2P Testing Configuration"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""

# Configuration
NODE2_DIR="$HOME/.aumcoin-node2"
NODE2_PORT=19336          # P2P port (Node 1 uses 19335)
NODE2_RPC_PORT=19333      # RPC port (Node 1 uses 19332)
NODE1_CONNECT="127.0.0.1:19335"  # Connect to Node 1

# Create data directory
mkdir -p "$NODE2_DIR"
mkdir -p "$NODE2_DIR/testnet3"

# Create configuration file
cat > "$NODE2_DIR/aumcoin.conf" << EOF
# AumCoin Testnet Node 2 Configuration
# Multi-Node P2P Testing

# Network
testnet=1
port=$NODE2_PORT
rpcport=$NODE2_RPC_PORT

# Connect to Node 1
addnode=$NODE1_CONNECT

# RPC
rpcuser=aumcoinrpc
rpcpassword=testnet_rpc_password_change_in_production
rpcallowip=127.0.0.1

# Mining (disabled initially - will enable via RPC)
gen=0

# Logging
debug=net
debug=bench

# Performance
maxconnections=20
EOF

echo "✅ Configuration created at $NODE2_DIR/aumcoin.conf"
echo ""
echo "🚀 Starting AumCoin Testnet Node 2..."
echo ""

# Start the daemon
./src/aumcoind -testnet \
    -datadir="$NODE2_DIR" \
    -port=$NODE2_PORT \
    -rpcport=$NODE2_RPC_PORT \
    -daemon

# Wait for startup
echo -n "Waiting for startup"
for i in {1..10}; do
    sleep 1
    echo -n "."
    if ./src/aumcoind -testnet -datadir="$NODE2_DIR" -rpcport=$NODE2_RPC_PORT getinfo &>/dev/null; then
        break
    fi
done
echo ""

# Check if started successfully
if ./src/aumcoind -testnet -datadir="$NODE2_DIR" -rpcport=$NODE2_RPC_PORT getinfo &>/dev/null; then
    echo ""
    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    echo "  ✅ AumCoin Testnet Node 2 Started Successfully!"
    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    echo ""
    echo "📊 Network Information:"
    ./src/aumcoind -testnet -datadir="$NODE2_DIR" -rpcport=$NODE2_RPC_PORT getinfo
    echo ""
    echo "🔧 Node 2 Commands:"
    echo "  Check status:   ./src/aumcoind -testnet -datadir=\"$NODE2_DIR\" -rpcport=$NODE2_RPC_PORT getinfo"
    echo "  Get peers:      ./src/aumcoind -testnet -datadir=\"$NODE2_DIR\" -rpcport=$NODE2_RPC_PORT getpeerinfo"
    echo "  Enable mining:  ./src/aumcoind -testnet -datadir=\"$NODE2_DIR\" -rpcport=$NODE2_RPC_PORT setgenerate true 1"
    echo "  Stop node:      ./src/aumcoind -testnet -datadir=\"$NODE2_DIR\" -rpcport=$NODE2_RPC_PORT stop"
    echo ""
    echo "📡 P2P Connection:"
    echo "  Node 1: 127.0.0.1:19335 (RPC: 19332)"
    echo "  Node 2: 127.0.0.1:$NODE2_PORT (RPC: $NODE2_RPC_PORT)"
    echo ""
    echo "🎉 Ready for multi-node P2P testing!"
else
    echo ""
    echo "⚠️  Failed to start. Check logs:"
    echo "    tail -f $NODE2_DIR/testnet3/debug.log"
    exit 1
fi
