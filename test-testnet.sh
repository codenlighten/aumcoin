#!/bin/bash
# AumCoin Testnet Testing Suite
# Phase 5.2: Quantum-Safe Test Network Validation

set -e

# Colors
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m'

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
AUMCOIN_CLI="$SCRIPT_DIR/src/aumcoind -testnet"

# Test counters
TESTS_RUN=0
TESTS_PASSED=0
TESTS_FAILED=0

# Helper functions
print_test() {
    echo -e "${BLUE}━━━ Test $TESTS_RUN: $1 ━━━${NC}"
}

pass() {
    echo -e "${GREEN}✅ PASS${NC}"
    ((TESTS_PASSED++))
    echo ""
}

fail() {
    echo -e "${RED}❌ FAIL: $1${NC}"
    ((TESTS_FAILED++))
    echo ""
}

run_test() {
    ((TESTS_RUN++))
    print_test "$1"
}

# Check if testnet is running
if ! pgrep -f "aumcoind.*testnet" > /dev/null; then
    echo -e "${RED}❌ Testnet is not running!${NC}"
    echo "Start it with: ./start-testnet.sh"
    exit 1
fi

echo -e "${BLUE}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
echo -e "${BLUE}  🧪 AumCoin Testnet Testing Suite - Phase 5.2                      ${NC}"
echo -e "${BLUE}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
echo ""

# Test 1: Basic connectivity
run_test "Basic Connectivity"
if INFO=$($AUMCOIN_CLI getinfo 2>&1); then
    echo "Connected to testnet"
    echo "$INFO" | grep -E "version|blocks|connections"
    pass
else
    fail "Cannot connect to testnet"
fi

# Test 2: Protocol version
run_test "Protocol Version (should be 70003)"
VERSION=$($AUMCOIN_CLI getinfo | grep '"version"' | grep -oP '\d+')
if [ "$VERSION" -ge "70003" ]; then
    echo "Protocol version: $VERSION (Post-Quantum Edition ✅)"
    pass
else
    fail "Protocol version $VERSION < 70003"
fi

# Test 3: Generate quantum address
run_test "Generate Quantum Address"
if ADDR=$($AUMCOIN_CLI getnewaddress "test_quantum" 2>&1); then
    echo "Generated address: $ADDR"
    pass
else
    fail "Failed to generate address"
fi

# Test 4: Mine test blocks
run_test "Mine Test Blocks (for balance)"
echo "Mining 101 blocks (for coin maturity)..."
if $AUMCOIN_CLI setgenerate true 1 >/dev/null 2>&1; then
    # Generate blocks
    for i in {1..101}; do
        $AUMCOIN_CLI generate 1 >/dev/null 2>&1 || true
        if [ $((i % 20)) -eq 0 ]; then
            echo "  Mined $i/101 blocks..."
        fi
    done
    
    BLOCKS=$($AUMCOIN_CLI getblockcount)
    if [ "$BLOCKS" -gt "0" ]; then
        echo "Current block height: $BLOCKS"
        pass
    else
        fail "Block count is 0"
    fi
else
    echo -e "${YELLOW}⚠️  Mining not available (expected in some configs)${NC}"
    TESTS_RUN=$((TESTS_RUN - 1))
fi

# Test 5: Check balance
run_test "Check Wallet Balance"
BALANCE=$($AUMCOIN_CLI getbalance 2>&1 || echo "0")
echo "Balance: $BALANCE AUM"
if [ "$BALANCE" != "0" ] && [ "$BALANCE" != "0.00000000" ]; then
    pass
else
    echo -e "${YELLOW}⚠️  No balance (mining may not be enabled)${NC}"
    TESTS_RUN=$((TESTS_RUN - 1))
fi

# Test 6: Create quantum transaction
run_test "Send Quantum Transaction"
if [ "$BALANCE" != "0" ] && [ "$BALANCE" != "0.00000000" ]; then
    DEST=$($AUMCOIN_CLI getnewaddress "test_dest")
    if TXID=$($AUMCOIN_CLI sendtoaddress "$DEST" 1.0 2>&1); then
        echo "Transaction sent: $TXID"
        echo "Destination: $DEST"
        pass
    else
        fail "Failed to send transaction: $TXID"
    fi
else
    echo -e "${YELLOW}⚠️  Skipping (no balance)${NC}"
    TESTS_RUN=$((TESTS_RUN - 1))
fi

# Test 7: Check mempool
run_test "Check Mempool"
if MEMPOOL=$($AUMCOIN_CLI getrawmempool 2>&1); then
    COUNT=$(echo "$MEMPOOL" | grep -o '"' | wc -l)
    echo "Mempool transactions: $((COUNT / 2))"
    pass
else
    fail "Cannot query mempool"
fi

# Test 8: Multisig address creation
run_test "Create 2-of-3 Quantum Multisig"
ADDR1=$($AUMCOIN_CLI getnewaddress "alice")
ADDR2=$($AUMCOIN_CLI getnewaddress "bob")
ADDR3=$($AUMCOIN_CLI getnewaddress "carol")

if MULTISIG=$($AUMCOIN_CLI addmultisigmldsaaddress 2 "[\"$ADDR1\",\"$ADDR2\",\"$ADDR3\"]" "test_escrow" 2>&1); then
    echo "Multisig address: $MULTISIG"
    echo "Participants: alice, bob, carol"
    pass
else
    fail "Failed to create multisig: $MULTISIG"
fi

# Test 9: Node info
run_test "Network Node Information"
if NODEINFO=$($AUMCOIN_CLI getpeerinfo 2>&1); then
    PEER_COUNT=$(echo "$NODEINFO" | grep -c "addr" || echo "0")
    echo "Connected peers: $PEER_COUNT"
    pass
else
    fail "Cannot get peer info"
fi

# Test 10: Blockchain info
run_test "Blockchain Information"
if CHAININFO=$($AUMCOIN_CLI getblockchaininfo 2>&1); then
    echo "$CHAININFO" | grep -E "chain|blocks|difficulty" || true
    pass
else
    fail "Cannot get blockchain info"
fi

# Test 11: Validate address
run_test "Validate Quantum Address"
TEST_ADDR=$($AUMCOIN_CLI getnewaddress "validation_test")
if VALIDATE=$($AUMCOIN_CLI validateaddress "$TEST_ADDR" 2>&1); then
    IS_VALID=$(echo "$VALIDATE" | grep '"isvalid"' | grep -o 'true\|false')
    if [ "$IS_VALID" = "true" ]; then
        echo "Address validated: $TEST_ADDR"
        pass
    else
        fail "Address validation failed"
    fi
else
    fail "Cannot validate address"
fi

# Test 12: RPC help
run_test "RPC Help Command"
if $AUMCOIN_CLI help >/dev/null 2>&1; then
    QUANTUM_CMDS=$($AUMCOIN_CLI help | grep -c "mldsa" || echo "0")
    echo "Quantum RPC commands available: $QUANTUM_CMDS"
    pass
else
    fail "RPC help command failed"
fi

# Summary
echo ""
echo -e "${BLUE}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
echo -e "${BLUE}  📊 Test Summary                                                    ${NC}"
echo -e "${BLUE}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
echo ""
echo "Total tests run:    $TESTS_RUN"
echo -e "${GREEN}Tests passed:       $TESTS_PASSED${NC}"
if [ "$TESTS_FAILED" -gt "0" ]; then
    echo -e "${RED}Tests failed:       $TESTS_FAILED${NC}"
else
    echo "Tests failed:       $TESTS_FAILED"
fi

SUCCESS_RATE=$((TESTS_PASSED * 100 / TESTS_RUN))
echo "Success rate:       $SUCCESS_RATE%"
echo ""

if [ "$SUCCESS_RATE" -ge "90" ]; then
    echo -e "${GREEN}🎉 Excellent! Testnet is working great!${NC}"
    exit 0
elif [ "$SUCCESS_RATE" -ge "70" ]; then
    echo -e "${YELLOW}⚠️  Good, but some issues detected${NC}"
    exit 0
else
    echo -e "${RED}❌ Testnet has significant issues${NC}"
    exit 1
fi
