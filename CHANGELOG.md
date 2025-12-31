# LightCoin-PQ Changelog

## Version 0.6.3c-SV (Satoshi Vision) - December 30, 2025

### Major Changes: Satoshi Vision Restoration

#### ✅ OP_CODE Restoration (Phase 1 - COMPLETE)
Re-enabled all 15 original Bitcoin OP_CODES that were disabled in Bitcoin 0.3.x:

**Splice Operations:**
- `OP_CAT` (0x7e) - Concatenate two values
- `OP_SUBSTR` (0x7f) - Extract substring 
- `OP_LEFT` (0x80) - Keep left portion of value
- `OP_RIGHT` (0x81) - Keep right portion of value

**Bitwise Logic:**
- `OP_INVERT` (0x83) - Bitwise NOT operation
- `OP_AND` (0x84) - Bitwise AND operation
- `OP_OR` (0x85) - Bitwise OR operation  
- `OP_XOR` (0x86) - Bitwise XOR operation

**Arithmetic Operations:**
- `OP_2MUL` (0x8d) - Multiply by 2 (left shift)
- `OP_2DIV` (0x8e) - Divide by 2 (right shift)
- `OP_MUL` (0x95) - Multiplication
- `OP_DIV` (0x96) - Division
- `OP_MOD` (0x97) - Modulo operation
- `OP_LSHIFT` (0x98) - Left bit shift
- `OP_RSHIFT` (0x99) - Right bit shift

**Technical Details:**
- Removed OP_CODE blocking check in `EvalScript()` (script.cpp:287-301)
- All implementations were already present and functional
- Added explanatory comments documenting the restoration

#### ✅ Block Size Liberation (Phase 2 - COMPLETE)

**Massive Block Size Increase:**
- `MAX_BLOCK_SIZE`: 1 MB → **128 MB** (128x increase)
- Follows BSV philosophy of unlimited scaling
- Enables thousands of transactions per block
- Supports future growth without hard forks

**Script Size Limits Removed:**
- Removed 10 KB script size limit
- Increased push value size: 520 bytes → **10 KB**
- Increased OP_CAT result size: 520 bytes → **10 KB**
- Enables complex smart contracts and data storage

**Benefits:**
- True peer-to-peer electronic cash scalability
- Support for sophisticated smart contracts
- On-chain data storage capabilities
- Low fees due to abundant block space

### Files Modified

1. **src/script.cpp**
   - Lines 245-253: Removed 10KB script size limit
   - Lines 266-269: Increased push value size to 10KB
   - Lines 273-278: Removed OP_CODE blocking (enabled 15 OP_CODES)
   - Lines 589-592: Increased OP_CAT concatenation limit to 10KB

2. **src/main.h**
   - Lines 29-32: Increased MAX_BLOCK_SIZE to 128MB

3. **LIGHTCOIN_SATOSHI_VISION.md**
   - New file: Comprehensive project documentation and roadmap

### Philosophy

This fork restores Bitcoin's original vision as outlined by Satoshi Nakamoto:
- Rich scripting language for programmable money
- No artificial limits on growth or capability
- True peer-to-peer electronic cash system
- Built on Litecoin's proven foundation (Scrypt POW, 2.5min blocks)

### Why Litecoin v0.6.3c?

- **Simple codebase**: ~30,000 lines vs modern 100,000+
- **OP_CODES intact**: Implementations never removed, just disabled
- **Pre-modern complexity**: No SegWit, no MWEB, no technical debt
- **MIT License**: Clean fork-friendly licensing
- **Proven technology**: Litecoin's stability and Scrypt ASIC resistance

### Comparison with Other Projects

| Feature | LightCoin-PQ | Bitcoin | Litecoin | BSV |
|---------|--------------|---------|----------|-----|
| All OP_CODES | ✅ Yes | ❌ No | ❌ No | ✅ Yes |
| Large Blocks | ✅ 128MB+ | ❌ 1-4MB | ❌ 1MB | ✅ 4GB+ |
| License | ✅ MIT | ✅ MIT | ✅ MIT | ⚠️ Mixed |
| POW Algorithm | ✅ Scrypt | SHA-256 | ✅ Scrypt | SHA-256 |
| Block Time | ✅ 2.5min | 10min | ✅ 2.5min | 10min |
| Simple Codebase | ✅ Yes | ❌ No | ❌ No | ❌ No |
| PQ Ready | 🔄 Planned | ❌ No | ❌ No | ❌ No |

### What's Next?

**Phase 3: Rebranding (Next Steps)**
- [ ] Rename from Litecoin to LightCoin-PQ throughout codebase
- [ ] Create new genesis block
- [ ] Set up dedicated network parameters
- [ ] Update all version strings and client identification
- [ ] Design new logo and branding

**Phase 4: Post-Quantum Cryptography (Future)**
- [ ] Research PQ signature schemes (NIST standards)
- [ ] Design hybrid classical + PQ address system
- [ ] Implement PQ signature verification
- [ ] Add quantum-resistant key generation
- [ ] Create migration path for existing users

### Build Instructions

```bash
# Dependencies (Ubuntu/Debian)
sudo apt-get install build-essential libboost-all-dev libssl-dev libdb++-dev

# Build
cd /mnt/storage/dev/dev/lightcoin-pq/src
make -f makefile.unix

# Run
./litecoind
```

### Testing

See `TESTING.md` for comprehensive test cases covering:
- OP_CODE functionality tests
- Large block handling
- Complex script execution
- Performance benchmarks

### Security Considerations

**Enabled Features Considerations:**
- OP_MUL/OP_DIV: BigNum operations are computationally expensive
- Large blocks: Network bandwidth and storage requirements increase
- Complex scripts: CPU validation time increases with script complexity

**Mitigations:**
- Operation count limits still enforced (201 ops per script)
- Push value size limited to 10KB (reasonable for most use cases)
- Block size limit prevents single-block DOS attacks
- Scrypt POW provides natural rate limiting

### Community

- **Repository**: Fork of Litecoin v0.6.3c
- **License**: MIT/X11
- **Original Litecoin**: Copyright (c) 2011-2012 Litecoin Developers
- **Original Bitcoin**: Copyright (c) 2009-2010 Satoshi Nakamoto

### Acknowledgments

- Satoshi Nakamoto - Original Bitcoin vision and codebase
- Litecoin Developers - Scrypt POW and proven stability
- Bitcoin SV Team - Inspiration for OP_CODE restoration philosophy
- Post-Quantum Cryptography Researchers - Future-proofing guidance

---

**This is experimental software. Use at your own risk.**
**Not for production use without thorough testing and security audit.**
