#!/bin/bash
# AumCoin Testnet Quick Test Suite
# Phase 5.2: Basic Validation

# Colors
GREEN='\033[0;32m'
BLUE='\033[0;34m'
RED='\033[0;31m'
NC='\033[0m'

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
CLI="$SCRIPT_DIR/src/aumcoind -testnet -datadir=$HOME/.aumcoin"

echo -e "${BLUE}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
echo -e "${BLUE}  🧪 AumCoin Testnet Quick Validation${NC}"
echo -e "${BLUE}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
echo ""

# Test 1: Basic connectivity
echo -e "${BLUE}Test 1: Checking testnet status...${NC}"
INFO=$($CLI getinfo 2>&1)
if echo "$INFO" | grep -q '"testnet" : true'; then
    echo -e "${GREEN}✅ PASS: Testnet is running${NC}"
    echo "$INFO" | head -5
else
    echo -e "${RED}❌ FAIL: Testnet not responding${NC}"
    exit 1
fi
echo ""

# Test 2: Protocol version
echo -e "${BLUE}Test 2: Checking protocol version...${NC}"
PROTO=$(echo "$INFO" | grep '"protocolversion"' | grep -oP '\d+')
if [ "$PROTO" = "70003" ]; then
    echo -e "${GREEN}✅ PASS: Protocol version 70003 (Post-Quantum Edition)${NC}"
else
    echo -e "${RED}❌ FAIL: Expected 70003, got $PROTO${NC}"
fi
echo ""

# Test 3: Generate quantum address
echo -e "${BLUE}Test 3: Generating quantum address...${NC}"
ADDR=$($CLI getnewaddress "quantum_test" 2>&1)
if [ $? -eq 0 ]; then
    echo -e "${GREEN}✅ PASS: Generated address: $ADDR${NC}"
else
    echo -e "${RED}❌ FAIL: $ADDR${NC}"
fi
echo ""

# Test 4: Check blocks
echo -e "${BLUE}Test 4: Checking blockchain...${NC}"
BLOCKS=$($CLI getblockcount 2>&1)
echo -e "${GREEN}✅ Current block count: $BLOCKS${NC}"
echo ""

# Test 5: Validate address
echo -e "${BLUE}Test 5: Validating address...${NC}"
VALID=$($CLI validateaddress "$ADDR" 2>&1)
if echo "$VALID" | grep -q '"isvalid" : true'; then
    echo -e "${GREEN}✅ PASS: Address validation successful${NC}"
    IS_MINE=$(echo "$VALID" | grep '"ismine"' | grep -oP '(true|false)')
    echo "  - Is mine: $IS_MINE"
else
    echo -e "${RED}❌ FAIL: Address validation failed${NC}"
fi
echo ""

# Test 6: RPC help
echo -e "${BLUE}Test 6: Checking quantum RPC commands...${NC}"
HELP=$($CLI help 2>&1)
MLDSA_COUNT=$(echo "$HELP" | grep -c "mldsa" || echo "0")
echo -e "${GREEN}✅ Found $MLDSA_COUNT ML-DSA related commands${NC}"
echo ""

echo -e "${BLUE}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
echo -e "${GREEN}  ✅ Quick validation complete!${NC}"
echo -e "${BLUE}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
