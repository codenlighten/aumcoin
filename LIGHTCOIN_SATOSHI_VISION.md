# LightCoin-PQ: Satoshi Vision + Post-Quantum Fork

## Project Overview
LightCoin-PQ is a fork of Litecoin v0.6.3c that restores Satoshi's original vision with all OP_CODES enabled (BSV-style) while avoiding BSV's licensing issues. Built on the proven Litecoin codebase with plans for post-quantum cryptography additions.

## Base Version
- **Source**: Litecoin v0.6.3c (October 2011)
- **License**: MIT/X11 (fork-friendly)
- **Base Protocol**: Pre-modern complexity, close to original Bitcoin design

## Satoshi Vision Restoration

### Phase 1: Re-enable Disabled OP_CODES ✅ IN PROGRESS

#### Splice Operations (String manipulation)
- ✅ `OP_CAT` (0x7e) - Concatenate two strings
- ✅ `OP_SUBSTR` (0x7f) - Extract substring
- ✅ `OP_LEFT` (0x80) - Extract left portion
- ✅ `OP_RIGHT` (0x81) - Extract right portion

#### Bitwise Logic Operations
- ✅ `OP_INVERT` (0x83) - Bitwise NOT
- ✅ `OP_AND` (0x84) - Bitwise AND
- ✅ `OP_OR` (0x85) - Bitwise OR
- ✅ `OP_XOR` (0x86) - Bitwise XOR

#### Arithmetic Operations
- ✅ `OP_2MUL` (0x8d) - Multiply by 2
- ✅ `OP_2DIV` (0x8e) - Divide by 2
- ✅ `OP_MUL` (0x95) - Multiplication
- ✅ `OP_DIV` (0x96) - Division
- ✅ `OP_MOD` (0x97) - Modulo
- ✅ `OP_LSHIFT` (0x98) - Left bit shift
- ✅ `OP_RSHIFT` (0x99) - Right bit shift

**Implementation Status**: These OP_CODES are ALREADY IMPLEMENTED in the code (lines 587-720 of script.cpp) but are blocked in the EvalScript function (lines 287-301). Simple removal of the blocking check will restore them.

### Phase 2: Remove Block Size Limits
- [ ] Remove MAX_BLOCK_SIZE limit or set to very high value (128MB+)
- [ ] Remove script size restrictions
- [ ] Remove stack size limits
- [ ] Enable large transactions

### Phase 3: Branding & Identity
- [ ] Rename from Litecoin to LightCoin-PQ
- [ ] Update version strings
- [ ] Create new genesis block
- [ ] Set network parameters
- [ ] Update RPC commands
- [ ] Modify GUI branding

## Litecoin Core Features (Retained)
- **Block Time**: 2.5 minutes
- **Hashing Algorithm**: Scrypt (ASIC-resistant)
- **Total Supply**: 84 million coins
- **Block Reward**: 50 coins per block
- **Halving**: Every 840,000 blocks (~4 years)
- **Difficulty Adjustment**: Every 2016 blocks

## Future: Post-Quantum Cryptography (Phase 4)
- [ ] Research PQ signature schemes (Dilithium, Falcon, SPHINCS+)
- [ ] Design hybrid classical + PQ address system
- [ ] Implement PQ signature verification
- [ ] Add PQ key generation to wallet
- [ ] Create migration path for existing addresses

## Why This Approach?

### Advantages over BSV:
1. **Clean MIT License** - No legal/licensing concerns
2. **Proven Scrypt POW** - Different mining ecosystem from BTC/BSV
3. **Simple Codebase** - Easy to audit and modify (~30k lines vs 100k+)
4. **Active Community** - Litecoin's longevity and stability

### Advantages over Modern Litecoin:
1. **Pre-SegWit** - Simpler transaction structure
2. **Pre-MWEB** - No complex privacy features to conflict with
3. **Minimal Abstraction** - Direct, understandable code
4. **Easy PQ Integration** - Less technical debt

## Development Timeline

### Week 1: ✅ Foundation Setup (CURRENT)
- [x] Clone Litecoin v0.6.3c
- [x] Create project branch
- [x] Analyze codebase
- [x] Identify disabled OP_CODES
- [ ] Re-enable all OP_CODES
- [ ] Test OP_CODE functionality

### Week 2: Block Size Liberation
- [ ] Remove block size limits
- [ ] Test large blocks
- [ ] Benchmark performance

### Week 3: Rebranding
- [ ] Rename to LightCoin-PQ
- [ ] New genesis block
- [ ] Network parameters
- [ ] Update documentation

### Week 4+: Testing & PQ Research
- [ ] Comprehensive testing
- [ ] Security audit
- [ ] PQ cryptography research
- [ ] Community building

## Technical Notes

### Current Code Status:
- **Location**: `/mnt/storage/dev/dev/lightcoin-pq`
- **Branch**: `lightcoin-pq-satoshi-vision`
- **Base Commit**: 8ad55e476 (Litecoin v0.6.3c)

### Key Files:
- `src/script.cpp` - Script evaluation (OP_CODE blocking at lines 287-301)
- `src/script.h` - OP_CODE definitions (lines 119-151)
- `src/main.h` - Block size limits
- `src/main.cpp` - Consensus rules

### Build Requirements:
- GCC/G++ 4.x or later
- Boost libraries
- OpenSSL
- Berkeley DB 4.8
- Qt 4.x (for GUI)

## Philosophy

**Satoshi's Vision**: Bitcoin was designed as programmable electronic cash with a rich scripting language. Many OP_CODES were disabled due to security concerns in the early days, but they enable powerful smart contract capabilities.

**LightCoin-PQ Mission**: Restore Bitcoin's original computational power, leverage Litecoin's proven stability, and prepare for the post-quantum future.

---

**Status**: Phase 1 in progress - Re-enabling OP_CODES
**Next Action**: Remove OP_CODE blocking in script.cpp
