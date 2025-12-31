# Quick Start Guide - AumCoin

## 🚀 Fastest Way to Build & Run

### Option 1: Docker Build (Recommended)

```bash
# Build and extract binary in one command
./build-docker.sh

# Mine the genesis block
./aumcoind -printtoconsole
```

**What happens:**
1. Docker builds AumCoin with OpenSSL 1.1.x (compatible)
2. Binary is extracted to current directory
3. Genesis block mining starts automatically
4. Watch for "GENESIS BLOCK MINED!" message with hash/nonce values

### Option 2: Docker Compose

```bash
# Build and run as service
docker-compose build
docker-compose up -d

# View logs
docker-compose logs -f aumcoin

# Stop
docker-compose down
```

### Option 3: Native Build

**If you have OpenSSL 1.1.x:**
```bash
cd src
make -f makefile.unix
./aumcoind -printtoconsole
```

**If you have OpenSSL 3.x:**
See [BUILD.md](BUILD.md) for detailed instructions.

---

## 📋 After Genesis Block is Mined

1. **Copy the output values:**
   ```
   genesis hash: 0x[hash]
   genesis merkle root: 0x[merkle]
   genesis nonce: [nonce]
   ```

2. **Update `src/main.cpp`:**
   - Line ~32: Set `hashGenesisBlock = uint256("0x[hash]")`
   - Line ~2030: Change `if (true)` back to `if (false)` to disable mining
   - Line ~2025: Re-enable merkle root assertion with correct value

3. **Rebuild:**
   ```bash
   ./build-docker.sh  # or native build
   ```

4. **Start the network:**
   ```bash
   ./aumcoind -daemon
   ./aumcoind getinfo
   ```

---

## 🔌 RPC Commands

```bash
# Get blockchain info
./aumcoind getinfo
./aumcoind getblockcount
./aumcoind getdifficulty

# Get mining info
./aumcoind getmininginfo

# Generate blocks (testnet/regtest)
./aumcoind setgenerate true 1

# Wallet operations
./aumcoind getnewaddress
./aumcoind getbalance
./aumcoind listreceivedbyaddress

# Stop daemon
./aumcoind stop
```

---

## 🌐 Network Configuration

**Mainnet:**
- P2P Port: 9443
- RPC Port: 9442
- Data Dir: `~/.aumcoin/`
- Config: `~/.aumcoin/aumcoin.conf`

**Testnet:**
- P2P Port: 19443
- RPC Port: 19442
- Data Dir: `~/.aumcoin/testnet/`
- Start: `./aumcoind -testnet`

---

## ⚙️ Configuration File

Create `~/.aumcoin/aumcoin.conf`:

```conf
# RPC credentials
rpcuser=aumcoinrpc
rpcpassword=your_secure_password_here

# Network
server=1
daemon=1
listen=1

# RPC access (be careful with 0.0.0.0!)
rpcallowip=127.0.0.1
rpcport=9442

# Logging
debug=0
printtoconsole=0
```

---

## 🧪 Testing OP_CODES

After genesis block is set:

```bash
# Start in testnet
./aumcoind -testnet -daemon

# Use RPC to create transactions with OP_CAT, OP_MUL, etc.
# See TESTING.md for detailed OP_CODE test scripts
```

---

## 🐛 Troubleshooting

**"Cannot bind to port":**
- Check if another instance is running: `ps aux | grep aumcoind`
- Kill old processes: `killall aumcoind`

**"OpenSSL errors during build":**
- Use Docker build: `./build-docker.sh`
- See BUILD.md for OpenSSL 1.1.x installation

**"Genesis mining taking forever":**
- Normal for Scrypt POW (5-30 minutes)
- Monitor CPU usage to confirm mining is active
- Press Ctrl+C to cancel if needed

**"Wallet locked errors":**
- Unlock: `./aumcoind walletpassphrase "password" 600`
- Encrypt: `./aumcoind encryptwallet "password"`

---

## 📚 More Information

- **Full Build Guide:** [BUILD.md](BUILD.md)
- **Testing Guide:** [TESTING.md](TESTING.md)
- **Development Roadmap:** [ROADMAP.md](ROADMAP.md)
- **Project Overview:** [README.md](README.md)

---

## 🎯 Next Steps

1. ✅ Build AumCoin
2. ⛏️ Mine genesis block
3. 📝 Update genesis values in source
4. 🔄 Rebuild with final genesis
5. 🚀 Start your node
6. 🌐 Connect with peers
7. 💰 Start mining blocks!

**Welcome to the AumCoin network!** 🎉
