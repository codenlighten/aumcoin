#!/bin/bash
# AumCoin Quantum Multisig Dashboard
# Real-time comprehensive status display

DAEMON="./aumcoind -testnet"
MULTISIG_ADDR="2MtxuRKifrWcqRZAFA5MpJuHem7Ji827hy9"
TARGET_BLOCK=434

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
PURPLE='\033[0;35m'
CYAN='\033[0;36m'
WHITE='\033[1;37m'
NC='\033[0m' # No Color

while true; do
    clear
    
    # Get current data
    BLOCK=$($DAEMON getblockcount 2>/dev/null)
    BALANCE=$($DAEMON getbalance 2>/dev/null)
    MINING_INFO=$($DAEMON getmininginfo 2>/dev/null)
    HASH_RATE=$(echo "$MINING_INFO" | grep hashespersec | awk '{print $3}' | tr -d ',')
    DIFFICULTY=$(echo "$MINING_INFO" | grep difficulty | awk '{print $3}' | tr -d ',')
    CONNECTIONS=$($DAEMON getconnectioncount 2>/dev/null)
    
    # Check if daemon is running
    if [ -z "$BLOCK" ]; then
        echo -e "${RED}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
        echo -e "${RED}  ⚠️  DAEMON NOT RUNNING${NC}"
        echo -e "${RED}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
        echo ""
        echo "  Start with: ./aumcoind -testnet -daemon"
        exit 1
    fi
    
    # Calculate progress
    REMAINING=$((TARGET_BLOCK - BLOCK))
    PERCENT=$(( (BLOCK * 100) / TARGET_BLOCK ))
    FILLED=$(( PERCENT / 2 ))
    EMPTY=$(( 50 - FILLED ))
    
    # Create progress bar
    BAR=$(printf '█%.0s' $(seq 1 $FILLED))
    SPACE=$(printf '░%.0s' $(seq 1 $EMPTY))
    
    # Header
    echo -e "${PURPLE}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
    echo -e "${PURPLE}  ⚛️  AUMCOIN QUANTUM MULTISIG DASHBOARD${NC}"
    echo -e "${PURPLE}  🌟 World's First Post-Quantum Cryptocurrency${NC}"
    echo -e "${PURPLE}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
    echo ""
    
    # Blockchain Status
    echo -e "${CYAN}┌─────────────────────────────────────────────────────────────────────────┐${NC}"
    echo -e "${CYAN}│${NC} ${WHITE}BLOCKCHAIN STATUS${NC}                                                      ${CYAN}│${NC}"
    echo -e "${CYAN}├─────────────────────────────────────────────────────────────────────────┤${NC}"
    printf "${CYAN}│${NC}   Current Block:  ${GREEN}%-50s${NC}  ${CYAN}│${NC}\n" "$BLOCK"
    printf "${CYAN}│${NC}   Target Block:   ${YELLOW}%-50s${NC}  ${CYAN}│${NC}\n" "$TARGET_BLOCK (coinbase maturity)"
    
    if [ "$BLOCK" -ge "$TARGET_BLOCK" ]; then
        printf "${CYAN}│${NC}   Status:         ${GREEN}%-50s${NC}  ${CYAN}│${NC}\n" "✅ COINBASE MATURE - READY TO FUND!"
    else
        printf "${CYAN}│${NC}   Remaining:      ${YELLOW}%-50s${NC}  ${CYAN}│${NC}\n" "$REMAINING blocks"
    fi
    
    printf "${CYAN}│${NC}   Progress:       ${WHITE}%-50s${NC}  ${CYAN}│${NC}\n" "[$BAR$SPACE] $PERCENT%"
    echo -e "${CYAN}└─────────────────────────────────────────────────────────────────────────┘${NC}"
    echo ""
    
    # Mining Status
    echo -e "${CYAN}┌─────────────────────────────────────────────────────────────────────────┐${NC}"
    echo -e "${CYAN}│${NC} ${WHITE}MINING STATUS${NC}                                                          ${CYAN}│${NC}"
    echo -e "${CYAN}├─────────────────────────────────────────────────────────────────────────┤${NC}"
    printf "${CYAN}│${NC}   Hash Rate:      ${GREEN}%-50s${NC}  ${CYAN}│${NC}\n" "$HASH_RATE H/s"
    printf "${CYAN}│${NC}   Difficulty:     ${WHITE}%-50s${NC}  ${CYAN}│${NC}\n" "$DIFFICULTY"
    printf "${CYAN}│${NC}   Connections:    ${BLUE}%-50s${NC}  ${CYAN}│${NC}\n" "$CONNECTIONS"
    echo -e "${CYAN}└─────────────────────────────────────────────────────────────────────────┘${NC}"
    echo ""
    
    # Wallet Status
    echo -e "${CYAN}┌─────────────────────────────────────────────────────────────────────────┐${NC}"
    echo -e "${CYAN}│${NC} ${WHITE}WALLET STATUS${NC}                                                          ${CYAN}│${NC}"
    echo -e "${CYAN}├─────────────────────────────────────────────────────────────────────────┤${NC}"
    printf "${CYAN}│${NC}   Balance:        ${GREEN}%-50s${NC}  ${CYAN}│${NC}\n" "$BALANCE AUM"
    
    # Check immature transactions
    IMMATURE_COUNT=$($DAEMON listtransactions "*" 999 | grep -c "immature")
    MATURE_COUNT=$($DAEMON listtransactions "*" 999 | grep -c '"generate"')
    
    if [ "$IMMATURE_COUNT" -gt 0 ]; then
        IMMATURE_AMOUNT=$(echo "$IMMATURE_COUNT * 50" | bc)
        printf "${CYAN}│${NC}   Immature:       ${YELLOW}%-50s${NC}  ${CYAN}│${NC}\n" "$IMMATURE_COUNT coinbase ($IMMATURE_AMOUNT AUM pending)"
    fi
    
    if [ "$MATURE_COUNT" -gt 0 ]; then
        printf "${CYAN}│${NC}   Mature Coinbase:${GREEN}%-50s${NC}  ${CYAN}│${NC}\n" "$MATURE_COUNT transactions available"
    fi
    
    echo -e "${CYAN}└─────────────────────────────────────────────────────────────────────────┘${NC}"
    echo ""
    
    # Quantum Multisig Status
    echo -e "${CYAN}┌─────────────────────────────────────────────────────────────────────────┐${NC}"
    echo -e "${CYAN}│${NC} ${WHITE}QUANTUM MULTISIG STATUS${NC}                                                ${CYAN}│${NC}"
    echo -e "${CYAN}├─────────────────────────────────────────────────────────────────────────┤${NC}"
    printf "${CYAN}│${NC}   Address:        ${PURPLE}%-50s${NC}  ${CYAN}│${NC}\n" "$MULTISIG_ADDR"
    printf "${CYAN}│${NC}   Type:           ${WHITE}%-50s${NC}  ${CYAN}│${NC}\n" "2-of-3 ML-DSA-65 (NIST FIPS 204)"
    printf "${CYAN}│${NC}   Participants:   ${WHITE}%-50s${NC}  ${CYAN}│${NC}\n" "Alice, Bob, Carol"
    
    # Check multisig balance
    MULTISIG_UNSPENT=$($DAEMON listunspent 1 999999 "[\"$MULTISIG_ADDR\"]" 2>/dev/null)
    if echo "$MULTISIG_UNSPENT" | grep -q "amount"; then
        MULTISIG_AMOUNT=$(echo "$MULTISIG_UNSPENT" | grep -o '"amount" : [0-9.]*' | awk '{print $3}' | head -1)
        printf "${CYAN}│${NC}   Funded:         ${GREEN}%-50s${NC}  ${CYAN}│${NC}\n" "✅ YES ($MULTISIG_AMOUNT AUM)"
    else
        printf "${CYAN}│${NC}   Funded:         ${YELLOW}%-50s${NC}  ${CYAN}│${NC}\n" "⏳ NO (waiting for funds)"
    fi
    
    echo -e "${CYAN}└─────────────────────────────────────────────────────────────────────────┘${NC}"
    echo ""
    
    # ML-DSA Cache Metrics
    CACHE_METRICS=$($DAEMON getmldsacachemetrics 2>/dev/null)
    if [ ! -z "$CACHE_METRICS" ]; then
        echo -e "${CYAN}┌─────────────────────────────────────────────────────────────────────────┐${NC}"
        echo -e "${CYAN}│${NC} ${WHITE}ML-DSA SIGNATURE CACHE${NC}                                                ${CYAN}│${NC}"
        echo -e "${CYAN}├─────────────────────────────────────────────────────────────────────────┤${NC}"
        
        SIZE=$(echo "$CACHE_METRICS" | grep cachesize | awk '{print $3}' | tr -d ',')
        HITS=$(echo "$CACHE_METRICS" | grep cachehits | awk '{print $3}' | tr -d ',')
        MISSES=$(echo "$CACHE_METRICS" | grep cachemisses | awk '{print $3}' | tr -d ',')
        
        printf "${CYAN}│${NC}   Cache Size:     ${WHITE}%-50s${NC}  ${CYAN}│${NC}\n" "$SIZE entries"
        printf "${CYAN}│${NC}   Cache Hits:     ${GREEN}%-50s${NC}  ${CYAN}│${NC}\n" "$HITS"
        printf "${CYAN}│${NC}   Cache Misses:   ${YELLOW}%-50s${NC}  ${CYAN}│${NC}\n" "$MISSES"
        
        if [ "$HITS" -gt 0 ] || [ "$MISSES" -gt 0 ]; then
            TOTAL=$((HITS + MISSES))
            HIT_RATE=$(( (HITS * 100) / TOTAL ))
            printf "${CYAN}│${NC}   Hit Rate:       ${GREEN}%-50s${NC}  ${CYAN}│${NC}\n" "$HIT_RATE%"
        fi
        
        echo -e "${CYAN}└─────────────────────────────────────────────────────────────────────────┘${NC}"
        echo ""
    fi
    
    # Action Items
    echo -e "${CYAN}┌─────────────────────────────────────────────────────────────────────────┐${NC}"
    echo -e "${CYAN}│${NC} ${WHITE}NEXT ACTIONS${NC}                                                           ${CYAN}│${NC}"
    echo -e "${CYAN}├─────────────────────────────────────────────────────────────────────────┤${NC}"
    
    if [ "$BLOCK" -ge "$TARGET_BLOCK" ]; then
        if echo "$MULTISIG_UNSPENT" | grep -q "amount"; then
            printf "${CYAN}│${NC}   ${GREEN}✅ Ready for spending transaction!${NC}                                ${CYAN}│${NC}\n"
            printf "${CYAN}│${NC}   ${WHITE}Run: ./spend-quantum-multisig.sh${NC}                                  ${CYAN}│${NC}\n"
        else
            printf "${CYAN}│${NC}   ${GREEN}✅ Ready to fund quantum multisig!${NC}                                ${CYAN}│${NC}\n"
            printf "${CYAN}│${NC}   ${WHITE}Run: ./quick-fund-multisig.sh${NC}                                     ${CYAN}│${NC}\n"
        fi
    else
        ETA_MINUTES=$(( (REMAINING * 150) / 60 ))  # ~2.5 min per block
        printf "${CYAN}│${NC}   ${YELLOW}⏳ Mining in progress...${NC}                                            ${CYAN}│${NC}\n"
        printf "${CYAN}│${NC}   ${WHITE}ETA: ~$ETA_MINUTES minutes to coinbase maturity${NC}                    ${CYAN}│${NC}\n"
    fi
    
    echo -e "${CYAN}└─────────────────────────────────────────────────────────────────────────┘${NC}"
    echo ""
    
    # Footer
    echo -e "${PURPLE}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
    echo -e "${WHITE}  Press Ctrl+C to exit | Refreshing every 15 seconds...${NC}"
    echo -e "${PURPLE}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
    
    sleep 15
done
