#!/bin/bash

# AumCoin Network Launcher
# One-command network initialization and launch

set -e

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
PURPLE='\033[0;35m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

BOLD='\033[1m'

print_banner() {
    echo -e "${CYAN}"
    cat << "EOF"
    ___                  ______      _       
   / _ \                 / ___\ \   / /      
  / /_\ \_   _ _ __ ___ | |    | | | |_ __  
  |  _  | | | | '_ ` _ \| |    | | | | '_ \ 
  | | | | |_| | | | | | | |____| |_| | | | |
  \_| |_/\__,_|_| |_| |_|\_____|_____/_| |_|
                                            
  Satoshi Vision Restored • Post-Quantum Ready
  
EOF
    echo -e "${NC}"
}

print_header() {
    echo -e "${BLUE}${BOLD}========================================${NC}"
    echo -e "${BLUE}${BOLD}  $1${NC}"
    echo -e "${BLUE}${BOLD}========================================${NC}"
}

print_success() {
    echo -e "${GREEN}✓ $1${NC}"
}

print_error() {
    echo -e "${RED}✗ $1${NC}"
}

print_info() {
    echo -e "${YELLOW}ℹ $1${NC}"
}

print_step() {
    echo -e "${PURPLE}▶ $1${NC}"
}

# Check prerequisites
check_prerequisites() {
    print_header "Checking Prerequisites"
    
    local all_good=true
    
    # Check Docker
    if command -v docker &> /dev/null; then
        print_success "Docker installed: $(docker --version | cut -d' ' -f3)"
    else
        print_error "Docker not found. Install from: https://docs.docker.com/get-docker/"
        all_good=false
    fi
    
    # Check if aumcoind binary exists
    if [ -f "./aumcoind" ]; then
        print_success "AumCoin binary found"
    else
        print_info "Binary not found. Will build using Docker..."
    fi
    
    # Check disk space (need at least 2GB free)
    local free_space=$(df -BG . | awk 'NR==2 {print $4}' | sed 's/G//')
    if [ "$free_space" -gt 2 ]; then
        print_success "Disk space: ${free_space}GB available"
    else
        print_error "Low disk space: ${free_space}GB (need at least 2GB)"
        all_good=false
    fi
    
    if [ "$all_good" = false ]; then
        echo ""
        print_error "Prerequisites not met. Please install missing dependencies."
        exit 1
    fi
    
    echo ""
}

# Build if needed
build_if_needed() {
    if [ ! -f "./aumcoind" ]; then
        print_header "Building AumCoin"
        print_step "Running Docker build..."
        ./build-docker.sh
        print_success "Build complete!"
        echo ""
    fi
}

# Setup configuration
setup_config() {
    print_header "Setting Up Configuration"
    
    local config_dir="$HOME/.aumcoin"
    local config_file="$config_dir/aumcoin.conf"
    
    # Create directory
    if [ ! -d "$config_dir" ]; then
        mkdir -p "$config_dir"
        print_success "Created data directory: $config_dir"
    else
        print_info "Data directory exists: $config_dir"
    fi
    
    # Create or update config
    if [ ! -f "$config_file" ]; then
        print_step "Generating configuration..."
        
        local rpc_password=$(openssl rand -hex 32 2>/dev/null || cat /dev/urandom | tr -dc 'a-zA-Z0-9' | fold -w 32 | head -n 1)
        
        cat > "$config_file" << EOF
# AumCoin Configuration
# Generated: $(date)

# RPC Settings
rpcuser=aumcoinrpc
rpcpassword=$rpc_password
rpcport=9442
rpcallowip=127.0.0.1

# Network Settings
port=9443
listen=1
server=1
daemon=1

# Blockchain Settings
txindex=1

# Mining (disabled by default)
gen=0

# Connection Settings
maxconnections=125

# Logging
debug=0
EOF
        
        chmod 600 "$config_file"
        print_success "Configuration created: $config_file"
        print_info "RPC password: $rpc_password"
    else
        print_info "Configuration exists: $config_file"
    fi
    
    echo ""
}

# Display network info
show_network_info() {
    print_header "Network Information"
    
    echo -e "${BOLD}Genesis Block:${NC}"
    echo "  Hash:      5828800007714e96f32995e76076b990a1211cf264f2eae74b5ac8be32222950"
    echo "  Nonce:     73563"
    echo "  Timestamp: December 30, 2025 00:00:00 UTC"
    echo ""
    
    echo -e "${BOLD}Network Parameters:${NC}"
    echo "  P2P Port:  9443"
    echo "  RPC Port:  9442"
    echo "  Block Time: 2.5 minutes"
    echo "  Block Size: 128 MB max"
    echo "  Algorithm:  Scrypt POW"
    echo ""
    
    echo -e "${BOLD}Features:${NC}"
    echo "  ✓ All 15 original Bitcoin OP_CODES enabled"
    echo "  ✓ 128MB blocks (128x larger than BTC)"
    echo "  ✓ Unlimited script size and operations"
    echo "  ✓ Post-quantum ready architecture"
    echo ""
}

# Start node
start_node() {
    print_header "Starting AumCoin Node"
    
    # Use Docker if binary has library issues
    if docker image inspect aumcoin:latest >/dev/null 2>&1; then
        print_step "Starting node in Docker container..."
        
        docker run -d \
            --name aumcoin-node \
            --restart unless-stopped \
            -v "$HOME/.aumcoin:/root/.aumcoin" \
            -p 9443:9443 \
            -p 127.0.0.1:9442:9442 \
            aumcoin:latest \
            /aumcoin/src/aumcoind -daemon -printtoconsole
        
        print_success "Node started in Docker container 'aumcoin-node'"
        print_info "View logs: docker logs -f aumcoin-node"
    else
        print_step "Starting node locally..."
        ./aumcoind -daemon
        print_success "Node started as daemon"
        print_info "View debug.log: tail -f ~/.aumcoin/debug.log"
    fi
    
    echo ""
    print_info "Waiting for node to initialize..."
    sleep 5
}

# Check node status
check_status() {
    print_header "Node Status"
    
    local max_attempts=10
    local attempt=0
    local node_ready=false
    
    while [ $attempt -lt $max_attempts ]; do
        if docker ps | grep -q aumcoin-node; then
            # Try Docker RPC
            if docker exec aumcoin-node /aumcoin/src/aumcoind getinfo >/dev/null 2>&1; then
                node_ready=true
                break
            fi
        elif ./aumcoind getinfo >/dev/null 2>&1; then
            # Try local RPC
            node_ready=true
            break
        fi
        
        attempt=$((attempt + 1))
        sleep 2
    done
    
    if [ "$node_ready" = true ]; then
        print_success "Node is running!"
        echo ""
        
        if docker ps | grep -q aumcoin-node; then
            docker exec aumcoin-node /aumcoin/src/aumcoind getinfo
        else
            ./aumcoind getinfo
        fi
    else
        print_error "Node failed to start. Check logs for details."
        exit 1
    fi
    
    echo ""
}

# Show next steps
show_next_steps() {
    print_header "Next Steps"
    
    echo -e "${BOLD}Basic Commands:${NC}"
    echo ""
    echo "  # Get node information"
    if docker ps | grep -q aumcoin-node; then
        echo "  docker exec aumcoin-node /aumcoin/src/aumcoind getinfo"
        echo ""
        echo "  # Check block count"
        echo "  docker exec aumcoin-node /aumcoin/src/aumcoind getblockcount"
        echo ""
        echo "  # Enable mining (1 thread)"
        echo "  docker exec aumcoin-node /aumcoin/src/aumcoind setgenerate true 1"
        echo ""
        echo "  # Get mining info"
        echo "  docker exec aumcoin-node /aumcoin/src/aumcoind getmininginfo"
        echo ""
        echo "  # View logs"
        echo "  docker logs -f aumcoin-node"
        echo ""
        echo "  # Stop node"
        echo "  docker stop aumcoin-node && docker rm aumcoin-node"
    else
        echo "  ./aumcoind getinfo"
        echo ""
        echo "  # Check block count"
        echo "  ./aumcoind getblockcount"
        echo ""
        echo "  # Enable mining (1 thread)"
        echo "  ./aumcoind setgenerate true 1"
        echo ""
        echo "  # Get mining info"
        echo "  ./aumcoind getmininginfo"
        echo ""
        echo "  # Stop node"
        echo "  ./aumcoind stop"
    fi
    
    echo ""
    echo -e "${BOLD}Lumen Bridge Integration:${NC}"
    echo "  ./lumen-bridge-integration.sh"
    echo ""
    
    echo -e "${BOLD}Documentation:${NC}"
    echo "  • Launch Guide:    cat LAUNCH.md"
    echo "  • Status Report:   cat STATUS.md"
    echo "  • Testing Guide:   cat TESTING.md"
    echo ""
    
    echo -e "${GREEN}${BOLD}🎊 AumCoin Network Launched Successfully! 🎊${NC}"
    echo ""
}

# Main menu for interactive mode
interactive_menu() {
    print_banner
    show_network_info
    
    echo -e "${BOLD}Choose an action:${NC}"
    echo ""
    echo "  1) Full Launch (recommended)"
    echo "  2) Build Only"
    echo "  3) Start Node Only"
    echo "  4) Check Node Status"
    echo "  5) Enable Mining"
    echo "  6) Disable Mining"
    echo "  7) View Node Info"
    echo "  8) Stop Node"
    echo "  9) Lumen Bridge Integration"
    echo "  q) Quit"
    echo ""
    read -p "Enter choice [1-9, q]: " choice
    
    case $choice in
        1)
            check_prerequisites
            build_if_needed
            setup_config
            start_node
            check_status
            show_next_steps
            ;;
        2)
            check_prerequisites
            ./build-docker.sh
            ;;
        3)
            setup_config
            start_node
            check_status
            ;;
        4)
            check_status
            ;;
        5)
            if docker ps | grep -q aumcoin-node; then
                docker exec aumcoin-node /aumcoin/src/aumcoind setgenerate true 1
            else
                ./aumcoind setgenerate true 1
            fi
            print_success "Mining enabled (1 thread)"
            ;;
        6)
            if docker ps | grep -q aumcoin-node; then
                docker exec aumcoin-node /aumcoin/src/aumcoind setgenerate false
            else
                ./aumcoind setgenerate false
            fi
            print_success "Mining disabled"
            ;;
        7)
            if docker ps | grep -q aumcoin-node; then
                docker exec aumcoin-node /aumcoin/src/aumcoind getinfo
            else
                ./aumcoind getinfo
            fi
            ;;
        8)
            if docker ps | grep -q aumcoin-node; then
                docker stop aumcoin-node && docker rm aumcoin-node
                print_success "Node stopped"
            else
                ./aumcoind stop
                print_success "Node stopped"
            fi
            ;;
        9)
            ./lumen-bridge-integration.sh
            ;;
        q|Q)
            exit 0
            ;;
        *)
            print_error "Invalid choice"
            interactive_menu
            ;;
    esac
}

# Main execution
main() {
    if [ "$1" == "--full" ] || [ "$1" == "-f" ]; then
        print_banner
        check_prerequisites
        build_if_needed
        setup_config
        show_network_info
        start_node
        check_status
        show_next_steps
    elif [ "$1" == "--build" ]; then
        print_banner
        check_prerequisites
        ./build-docker.sh
    elif [ "$1" == "--start" ]; then
        print_banner
        setup_config
        start_node
        check_status
    elif [ "$1" == "--status" ]; then
        check_status
    elif [ "$1" == "--help" ] || [ "$1" == "-h" ]; then
        print_banner
        echo "Usage: $0 [OPTIONS]"
        echo ""
        echo "Options:"
        echo "  --full, -f     Complete setup and launch (recommended)"
        echo "  --build        Build binary only"
        echo "  --start        Start node only"
        echo "  --status       Check node status"
        echo "  --help, -h     Show this help message"
        echo "  (no args)      Interactive menu"
        echo ""
        echo "Examples:"
        echo "  $0 --full      # First time launch"
        echo "  $0 --start     # Start existing node"
        echo "  $0             # Interactive mode"
    else
        interactive_menu
    fi
}

main "$@"
