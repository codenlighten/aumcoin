#!/bin/bash
# AumCoin Docker Build & Genesis Mining Script
# This script builds AumCoin in a Docker container and mines the genesis block

set -e

echo "=========================================="
echo "   AumCoin Docker Build & Genesis Miner"
echo "=========================================="
echo ""

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Check if Docker is installed
if ! command -v docker &> /dev/null; then
    echo -e "${RED}Error: Docker is not installed${NC}"
    echo "Please install Docker: https://docs.docker.com/get-docker/"
    exit 1
fi

echo -e "${BLUE}Step 1: Building Docker image (Ubuntu 18.04 + OpenSSL 1.1.1)...${NC}"
docker build -t aumcoin:latest .

if [ $? -eq 0 ]; then
    echo -e "${GREEN}✓ Docker image built successfully${NC}"
else
    echo -e "${RED}✗ Docker build failed${NC}"
    exit 1
fi

echo ""
echo -e "${BLUE}Step 2: Extracting binary from Docker container...${NC}"

# Create a temporary container and copy the binary
docker create --name aumcoin-temp aumcoin:latest
docker cp aumcoin-temp:/aumcoin/src/aumcoind ./aumcoind
docker rm aumcoin-temp

if [ -f ./aumcoind ]; then
    chmod +x ./aumcoind
    echo -e "${GREEN}✓ Binary extracted: ./aumcoind${NC}"
else
    echo -e "${RED}✗ Failed to extract binary${NC}"
    exit 1
fi

echo ""
echo -e "${YELLOW}=========================================="
echo "   Genesis Block Mining"
echo "==========================================${NC}"
echo ""
echo "The genesis block mining code is enabled in src/main.cpp"
echo "When you start aumcoind, it will mine the genesis block."
echo ""
echo -e "${BLUE}To mine the genesis block:${NC}"
echo "  ./aumcoind -printtoconsole"
echo ""
echo -e "${BLUE}Watch for output like:${NC}"
echo "  GENESIS BLOCK MINED!"
echo "  genesis hash: 0x[hash]"
echo "  genesis merkle root: 0x[merkle]"
echo "  genesis nonce: [nonce]"
echo ""
echo -e "${YELLOW}After mining, you must update src/main.cpp with these values!${NC}"
echo ""
echo -e "${GREEN}Build complete!${NC}"
echo ""
echo "Options:"
echo "  1. Mine genesis block now: ./aumcoind -printtoconsole"
echo "  2. Run as daemon: ./aumcoind -daemon"
echo "  3. Run in Docker: docker-compose up"
echo ""
