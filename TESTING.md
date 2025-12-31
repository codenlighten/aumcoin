# AumCoin Testing Guide

## Build Requirements

### OpenSSL Compatibility
AumCoin v0.6.3c uses OpenSSL 1.x BIGNUM API. Modern systems with OpenSSL 3.x require compatibility:

**Option 1: Docker Build (Recommended)**
```bash
# Use Ubuntu 18.04 or 20.04 with OpenSSL 1.1.x
docker run -it -v $(pwd):/aumcoin ubuntu:20.04 bash
cd /aumcoin/src
apt-get update && apt-get install -y build-essential libssl-dev libboost-all-dev libdb++-dev
make -f makefile.unix
```

**Option 2: OpenSSL 1.x on Host**
```bash
# Install OpenSSL 1.1.x compatibility (varies by distro)
# Debian/Ubuntu:
sudo apt-get install libssl1.1-dev
# Arch: 
# yay -S openssl-1.1
```

**Option 3: Modify Build Flags**
```bash
# Link against OpenSSL 1.1 if available
cd src
make -f makefile.unix OPENSSL=/usr/lib/openssl-1.1
```

### Genesis Block Mining

After building, the genesis block needs to be mined:

```bash
# Enable genesis mining mode (already done in source)
# File: src/main.cpp line ~2030
# Changed: if(false) -> if(true)

# Run aumcoind briefly to mine genesis
./aumcoind -testnet

# Watch for console output:
# "GENESIS BLOCK MINED!"
# "genesis hash: [hash]"
# "genesis merkle root: [merkle]"  
# "genesis nonce: [nonce]"

# Copy these values and update src/main.cpp:
# hashGenesisBlock = uint256("0x[hash]")
# Re-enable assertion with correct merkle root
# Change genesis mining: if(true) -> if(false)

# Rebuild and push final version
```

## Testing Restored OP_CODES

### Prerequisites
```bash
# Build the client (see above for OpenSSL compatibility)
cd src
make -f makefile.unix

# Start in test mode
./aumcoind -testnet -daemon
```

## OP_CODE Test Scripts

### 1. OP_CAT Test (String Concatenation)
```
# Script: Concatenate "Hello" and "World"
OP_PUSHDATA "Hello"
OP_PUSHDATA "World"
OP_CAT
# Result should be: "HelloWorld"
```

**Hex Script:**
```
0x05 48656c6c6f  // Push "Hello"
0x05 576f726c64  // Push "World"  
0x7e              // OP_CAT
```

**Expected Behavior:** 
- Stack top contains "HelloWorld"
- No script failure

### 2. OP_SUBSTR Test (Substring Extraction)
```
# Script: Extract "low" from "ellow" (begin=2, length=3)
OP_PUSHDATA "ellow"
OP_PUSHDATA 2
OP_PUSHDATA 3
OP_SUBSTR
# Result should be: "low"
```

**Hex Script:**
```
0x06 656c6c6f7720  // Push "yellow"
0x01 02             // Push 2 (begin)
0x01 03             // Push 3 (length)
0x7f                // OP_SUBSTR
```

### 3. OP_AND Test (Bitwise AND)
```
# Script: 0xFF AND 0x0F = 0x0F
OP_PUSHDATA 0xFF
OP_PUSHDATA 0x0F
OP_AND
# Result should be: 0x0F
```

**Hex Script:**
```
0x01 ff      // Push 0xFF
0x01 0f      // Push 0x0F
0x84         // OP_AND
```

### 4. OP_MUL Test (Multiplication)
```
# Script: 6 * 7 = 42
OP_6
OP_7
OP_MUL
OP_PUSHDATA 42
OP_EQUAL
# Result should be: TRUE
```

**Hex Script:**
```
0x56         // OP_6
0x57         // OP_7
0x95         // OP_MUL
0x01 2a      // Push 42
0x87         // OP_EQUAL
```

### 5. OP_LSHIFT Test (Left Bit Shift)
```
# Script: 8 << 2 = 32
OP_8
OP_2
OP_LSHIFT
OP_PUSHDATA 32
OP_EQUAL
# Result should be: TRUE
```

**Hex Script:**
```
0x58         // OP_8
0x52         // OP_2
0x98         // OP_LSHIFT
0x01 20      // Push 32
0x87         // OP_EQUAL
```

## Large Block Tests

### Test 1: Create Large Transaction
```bash
# Create a transaction with 10KB of data
./litecoin-cli createrawtransaction '[...]' '{"data": "..."}'
```

### Test 2: Fill Block to 10MB
```bash
# Generate many transactions
for i in {1..1000}; do
  ./litecoin-cli sendtoaddress <address> 0.01
done

# Mine block
./litecoin-cli generate 1

# Check block size
./litecoin-cli getblock <blockhash> | grep size
```

### Test 3: Maximum Block Size
```bash
# Attempt to create 128MB block
# Monitor with:
tail -f ~/.litecoin/debug.log | grep "block size"
```

## Complex Script Tests

### Test: Nested Operations
```
# Calculate: ((10 * 5) + 20) / 7 = 10
OP_10
OP_5
OP_MUL        # Stack: 50
OP_PUSHDATA 20
OP_ADD        # Stack: 70
OP_7
OP_DIV        # Stack: 10
OP_10
OP_EQUAL      # Should be TRUE
```

### Test: String Manipulation
```
# Concatenate and extract
OP_PUSHDATA "Hello"
OP_PUSHDATA "World"
OP_CAT              # Stack: "HelloWorld"
OP_PUSHDATA 0       # Begin at 0
OP_PUSHDATA 5       # Length 5
OP_SUBSTR           # Stack: "Hello"
OP_PUSHDATA "Hello"
OP_EQUAL            # Should be TRUE
```

## Performance Benchmarks

### Benchmark 1: Script Execution Time
```bash
# Time 1000 OP_MUL operations
time ./benchmark_mul_1000

# Expected: < 100ms on modern hardware
```

### Benchmark 2: Large Block Processing
```bash
# Time to validate 128MB block
time ./litecoin-cli verifyblock <128mb_block_hash>

# Expected: < 30 seconds on modern hardware
```

### Benchmark 3: OP_CAT Performance
```bash
# Concatenate to maximum 10KB limit
time ./benchmark_cat_max

# Expected: < 10ms
```

## Unit Tests

### Build Tests
```bash
cd /mnt/storage/dev/dev/lightcoin-pq/src
make -f makefile.unix test_litecoin

# Run tests
./test_litecoin
```

### Expected Test Results
```
✓ OP_CAT basic concatenation
✓ OP_CAT maximum size
✓ OP_SUBSTR boundary conditions
✓ OP_LEFT/RIGHT edge cases
✓ OP_INVERT bitwise inversion
✓ OP_AND/OR/XOR operations
✓ OP_MUL overflow handling
✓ OP_DIV division by zero prevention
✓ OP_MOD modulo operations
✓ OP_LSHIFT/RSHIFT bit operations
✓ Large block validation
✓ Script size limits removed
```

## Integration Tests

### Test Network Setup
```bash
# Terminal 1: Start node 1
./litecoind -testnet -port=19335 -rpcport=19334 -datadir=/tmp/node1

# Terminal 2: Start node 2
./litecoind -testnet -port=19336 -rpcport=19337 -datadir=/tmp/node2 -connect=127.0.0.1:19335

# Terminal 3: Send test transactions
./litecoin-cli -datadir=/tmp/node1 sendtoaddress <address> 1.0
```

### Verify OP_CODE Propagation
```bash
# Create transaction with OP_CAT script
TX=$(./litecoin-cli -datadir=/tmp/node1 createrawtransaction '[...]' '{"scriptPubKey": "0548656c6c6f05576f726c647e87"}')

# Sign and send
SIGNED=$(./litecoin-cli -datadir=/tmp/node1 signrawtransaction $TX)
TXID=$(./litecoin-cli -datadir=/tmp/node1 sendrawtransaction $SIGNED)

# Verify on node2
./litecoin-cli -datadir=/tmp/node2 getrawtransaction $TXID 1
```

## Security Tests

### Test 1: OP_COUNT Limit Still Enforced
```bash
# Try to execute 202 operations (should fail)
# Limit is 201 operations per script
./test_op_count_limit
# Expected: Script validation fails
```

### Test 2: Push Value Size Limit
```bash
# Try to push 11KB (should fail, limit is 10KB)
./test_push_size_limit
# Expected: Script validation fails
```

### Test 3: Stack Depth
```bash
# Try to exceed stack depth
./test_stack_depth
# Expected: Script validation fails appropriately
```

## Regression Tests

### Verify Existing Functionality
```bash
# Standard P2PKH transactions still work
./litecoin-cli sendtoaddress <address> 1.0

# Multisig still works
./litecoin-cli createmultisig 2 '["pubkey1", "pubkey2", "pubkey3"]'

# Time-locked transactions still work
./litecoin-cli createrawtransaction '[...]' '{"address": 1.0, "nLockTime": 500000}'
```

## Stress Tests

### Test 1: Maximum OP_CAT Usage
```python
# Python script to generate script with many OP_CAT operations
script = ""
for i in range(100):
    script += "OP_PUSHDATA 'A' "
    if i > 0:
        script += "OP_CAT "

# Result: 100-character string
# Tests: OP_CAT reliability and performance
```

### Test 2: Deep Arithmetic
```python
# Nested arithmetic operations
# ((((10 + 5) * 2) - 3) / 4) ^ 2
script = """
OP_10
OP_5 OP_ADD
OP_2 OP_MUL
OP_3 OP_SUB
OP_4 OP_DIV
OP_2 OP_XOR
"""
# Expected: Complex calculations work correctly
```

### Test 3: Large Block Propagation
```bash
# Create 100MB block
# Measure propagation time across network
# Expected: < 60 seconds for 3-node network
```

## Automated Test Suite

### Run All Tests
```bash
cd /mnt/storage/dev/dev/lightcoin-pq/src/test
./run_all_tests.sh
```

### Expected Output
```
==========================================
LightCoin-PQ Test Suite v0.6.3c-SV
==========================================

[PASS] OP_CODE Restoration Tests: 15/15
[PASS] Block Size Tests: 5/5
[PASS] Script Size Tests: 3/3
[PASS] Performance Tests: 8/8
[PASS] Security Tests: 6/6
[PASS] Regression Tests: 12/12
[PASS] Integration Tests: 4/4

==========================================
Total: 53/53 tests passed (100%)
==========================================
```

## Manual Verification Checklist

- [ ] All 15 OP_CODES execute without error
- [ ] OP_CAT concatenates correctly up to 10KB
- [ ] OP_MUL/DIV/MOD handle BigNum operations
- [ ] Bitwise operations produce correct results
- [ ] Blocks up to 128MB validate successfully
- [ ] Scripts larger than 10KB execute
- [ ] Operation count limit (201) still enforced
- [ ] Push value limit (10KB) enforced
- [ ] Existing P2PKH transactions work
- [ ] Network propagation functions normally
- [ ] No memory leaks under stress
- [ ] Client remains stable after 24 hours

## Troubleshooting

### OP_CODE Not Working
```bash
# Check if OP_CODE is actually enabled
grep -n "opcode == OP_CAT" src/script.cpp
# Should NOT find the blocking check
```

### Block Size Issues
```bash
# Verify MAX_BLOCK_SIZE
grep MAX_BLOCK_SIZE src/main.h
# Should show: 128000000 (128MB)
```

### Script Size Issues
```bash
# Verify script size check is removed
grep "script.size() > 10000" src/script.cpp
# Should be commented out
```

## Reporting Issues

If you find bugs or unexpected behavior:
1. Check debug.log: `tail -f ~/.litecoin/debug.log`
2. Enable debug mode: `./litecoind -debug=script`
3. Document exact steps to reproduce
4. Include script hex and expected vs actual behavior

---

**Note**: These tests assume a working build. If compilation fails, check dependencies and build instructions in CHANGELOG.md.
