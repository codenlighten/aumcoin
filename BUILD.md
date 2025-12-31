# AumCoin Build Guide

## Table of Contents
1. [Quick Start (Docker)](#quick-start-docker)
2. [Genesis Block Mining](#genesis-block-mining)
3. [Native Build Instructions](#native-build-instructions)
4. [Platform-Specific Guides](#platform-specific-guides)
5. [Troubleshooting](#troubleshooting)

---

## Quick Start (Docker)

The easiest way to build AumCoin is using Docker with Ubuntu 20.04 (OpenSSL 1.1.x):

```bash
# Pull Ubuntu 20.04 image
docker pull ubuntu:20.04

# Run container with workspace mounted
docker run -it -v $(pwd):/aumcoin ubuntu:20.04 bash

# Inside container: Install dependencies
apt-get update
apt-get install -y build-essential \
    libssl-dev \
    libboost-all-dev \
    libdb++-dev \
    libminiupnpc-dev

# Build AumCoin
cd /aumcoin/src
make -f makefile.unix

# Binary will be created: ./aumcoind
./aumcoind --version
```

---

## Genesis Block Mining

AumCoin requires a unique genesis block. Follow these steps:

### Step 1: Enable Genesis Mining (Already Done)

The source code has genesis mining enabled at `src/main.cpp` line ~2030:
```cpp
if(true)  // Genesis block mining enabled
{
    uint256 hashTarget = CBigNum().SetCompact(block.nBits).getuint256();
    while(block.GetHash() > hashTarget)
    {
        ++block.nNonce;
        if (block.nNonce == 0)
        {
            printf("NONCE WRAPPED, incrementing time");
            ++block.nTime;
        }
    }
}
```

### Step 2: Build and Mine

```bash
# Using Docker (recommended)
docker run -it -v $(pwd):/aumcoin ubuntu:20.04 bash

# Install dependencies
apt-get update && apt-get install -y build-essential libssl-dev libboost-all-dev libdb++-dev

# Build
cd /aumcoin/src
make -f makefile.unix

# Run to mine genesis (will take a few minutes)
./aumcoind -printtoconsole

# Watch for output:
# "GENESIS BLOCK MINED!"
# "genesis hash: 0x[hash]"
# "genesis merkle root: 0x[merkle]"
# "genesis nonce: [nonce]"
```

### Step 3: Update Source with Genesis Values

Once mining completes, update `src/main.cpp`:

```cpp
// Around line 32 - Update genesis hash
static const uint256 hashGenesisBlock("0x[YOUR_MINED_HASH]");

// Around line 2020 - Update merkle root assertion
assert(block.hashMerkleRoot == uint256("0x[YOUR_MERKLE_ROOT]"));

// Around line 2030 - DISABLE genesis mining
if(false)  // Change from if(true) to if(false)
{
    // Mining code...
}
```

### Step 4: Rebuild Final Version

```bash
# Rebuild with genesis values
cd src
make clean
make -f makefile.unix

# Test
./aumcoind --version
./aumcoind -daemon

# Verify genesis block is accepted
./aumcoind getinfo
```

---

## Native Build Instructions

### Dependencies

**Required:**
- GCC 4.8+ or Clang 3.3+
- OpenSSL 1.1.x (not 3.x - see compatibility notes)
- Boost 1.48+
- Berkeley DB 4.8
- make

**Optional:**
- miniupnpc (for UPnP support)
- Qt 4.8+ (for GUI)

### Ubuntu/Debian (18.04, 20.04)

```bash
# Install dependencies
sudo apt-get update
sudo apt-get install -y \
    build-essential \
    libssl-dev \
    libboost-all-dev \
    libdb++-dev \
    libdb-dev \
    libminiupnpc-dev

# Build daemon
cd src
make -f makefile.unix

# Build GUI (optional)
cd ..
qmake aumcoin-qt.pro
make
```

### Ubuntu/Debian (22.04+ with OpenSSL 3.x)

OpenSSL 3.x requires compatibility layer or using older OpenSSL:

**Option 1: Use Docker (Recommended)**
```bash
docker run -it -v $(pwd):/aumcoin ubuntu:20.04 bash
# Follow Quick Start instructions above
```

**Option 2: Install OpenSSL 1.1 Compatibility**
```bash
# Some distributions provide libssl1.1
sudo apt-get install libssl1.1-dev

# Or compile OpenSSL 1.1.1 from source
wget https://www.openssl.org/source/openssl-1.1.1w.tar.gz
tar xzf openssl-1.1.1w.tar.gz
cd openssl-1.1.1w
./config --prefix=/usr/local/openssl-1.1.1
make && sudo make install

# Build with custom OpenSSL path
cd /path/to/aumcoin/src
make -f makefile.unix \
    OPENSSL_INCLUDE_PATH=/usr/local/openssl-1.1.1/include \
    OPENSSL_LIB_PATH=/usr/local/openssl-1.1.1/lib
```

### Fedora/RHEL/CentOS

```bash
# Install dependencies
sudo dnf install -y \
    gcc-c++ \
    make \
    openssl-devel \
    boost-devel \
    libdb-cxx-devel \
    miniupnpc-devel

# Build
cd src
make -f makefile.unix
```

### Arch Linux

```bash
# Install dependencies
sudo pacman -S base-devel openssl boost boost-libs db miniupnpc

# Build
cd src
make -f makefile.unix
```

---

## Platform-Specific Guides

### macOS

See `doc/build-osx.txt` for detailed macOS build instructions.

**Quick start with Homebrew:**
```bash
# Install dependencies
brew install boost berkeley-db@4 openssl@1.1 miniupnpc

# Build
cd src
make -f makefile.osx \
    OPENSSL_INCLUDE_PATH=$(brew --prefix openssl@1.1)/include \
    OPENSSL_LIB_PATH=$(brew --prefix openssl@1.1)/lib
```

### Windows

See `doc/build-msw.txt` for Windows build instructions using MinGW.

**Cross-compile from Linux:**
```bash
# Install MinGW
sudo apt-get install -y mingw-w64

# Build
cd src
make -f makefile.linux-mingw
```

---

## Build Options

### Optimization Flags

**Release build (maximum performance):**
```bash
make -f makefile.unix RELEASE=1
```

**Debug build:**
```bash
make -f makefile.unix DEBUG=1
```

### UPnP Support

**Enable UPnP:**
```bash
make -f makefile.unix USE_UPNP=1
```

**Disable UPnP:**
```bash
make -f makefile.unix USE_UPNP=-
```

### Static Build

For redistribution:
```bash
make -f makefile.unix STATIC=1
```

---

## Troubleshooting

### OpenSSL 3.x Compatibility Issues

**Problem:** Build fails with BIGNUM errors on modern systems

**Solution:** Use Docker with Ubuntu 20.04 or install OpenSSL 1.1.x

**Error Example:**
```
error: invalid use of incomplete type 'BIGNUM' {aka 'struct bignum_st'}
```

**Fix:** See [Ubuntu 22.04+ section](#ubuntudebian-2204-with-openssl-3x) above

### Berkeley DB Version Issues

**Problem:** `fatal error: db_cxx.h: No such file or directory`

**Solution:** Install Berkeley DB 4.8 development package
```bash
# Ubuntu/Debian
sudo apt-get install libdb++-dev

# If not available, install db4.8 specifically
sudo apt-get install libdb4.8-dev libdb4.8++-dev
```

### Boost Thread Linking Issues

**Problem:** Undefined reference to boost::thread functions

**Solution:** Link boost_thread explicitly
```bash
make -f makefile.unix BOOST_LIB_SUFFIX=-mt
```

### Missing miniupnpc

**Problem:** Cannot find miniupnpc headers

**Solution:** Either install miniupnpc or disable UPnP
```bash
# Install
sudo apt-get install libminiupnpc-dev

# Or disable
make -f makefile.unix USE_UPNP=-
```

### Permission Denied on aumcoind

**Problem:** Cannot execute `./aumcoind`

**Solution:** Make binary executable
```bash
chmod +x aumcoind
```

---

## Verification

After building, verify the binary:

```bash
# Check version
./aumcoind --version

# Expected output:
# AumCoin version v1.0.0-alpha

# Run daemon
./aumcoind -daemon

# Check status
./aumcoind getinfo

# Stop daemon
./aumcoind stop
```

---

## Next Steps

After successful build:

1. **Mine Genesis Block** (if not done already)
2. **Configure** - Edit `~/.aumcoin/aumcoin.conf`
3. **Run Network** - Start aumcoind daemon
4. **Test OP_CODES** - See `TESTING.md`
5. **Contribute** - Submit pull requests to improve AumCoin

---

## Additional Resources

- **Testing Guide**: `TESTING.md` - OP_CODE test scripts
- **Configuration**: `doc/` - Detailed configuration options  
- **Development**: `ROADMAP.md` - Future development plans
- **Support**: Open an issue on GitHub

---

## License

Build documentation: MIT License  
See main LICENSE file for AumCoin code licensing.
