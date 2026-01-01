# 🍎 AumCoin Low-Hanging Fruit Roadmap

**Philosophy:** Quick wins first. Build momentum. Ship often.

**Date:** January 1, 2026  
**Status:** Phase 1 Security Complete ✅ | Ready for Phase 2

---

## 🎯 **Guiding Principles**

1. **Ship > Perfect** - Done is better than perfect
2. **Measure Progress** - Track every win
3. **Build Momentum** - Start easy, get harder
4. **Focus** - One task at a time
5. **Document** - Use City of Boxes knowledge graph

---

## 🟢 **TIER 1: Quick Wins (1-2 days each)**

These are **low effort, high impact** tasks that will give us immediate progress.

---

### ✅ **Task 1.1: Create Project Comparison Matrix** (2 hours)
**Status:** 🔄 Next Up  
**Effort:** 🟢 Low  
**Impact:** 🔥 High  

**Goal:** Document how AumCoin, @smartledger/keys, and Boundless fit together.

**Action Items:**
```bash
cd /mnt/storage/dev/dev/lightcoin-pq
touch ECOSYSTEM_COMPARISON.md
```

**Content:**
- Feature comparison table
- Use case mapping
- Technology stack comparison
- When to use which project

**Deliverable:** `ECOSYSTEM_COMPARISON.md` (1 page, clear table)

**Success Metric:** ✅ Team can explain differences in 30 seconds

---

### ✅ **Task 1.2: Apply City of Boxes to @smartledger/keys** (4 hours)
**Status:** ⏳ Queued  
**Effort:** 🟢 Low (we already have the tool!)  
**Impact:** 🔥 High (easier contributor onboarding)  

**Goal:** Create knowledge graph for lumenkeys repository.

**Action Items:**
```bash
# Clone lumenkeys repo
cd /tmp
git clone https://github.com/codenlighten/lumenkeys.git
cd lumenkeys

# Copy our knowledge graph generator
cp /mnt/storage/dev/dev/lightcoin-pq/create-knowledge-graph.js .
cp /mnt/storage/dev/dev/lightcoin-pq/query-knowledge-graph.js .

# Generate knowledge graph
node create-knowledge-graph.js

# Query examples
node query-knowledge-graph.js stats
node query-knowledge-graph.js category cryptography
node query-knowledge-graph.js box MlDsa65SuiteBox
```

**Deliverable:** 
- `lumenkeys/project-knowledge/` directory
- Master legend with all ML-DSA suites mapped
- Query tool for contributors

**Success Metric:** ✅ Contributor can find ML-DSA implementation in <30 seconds

---

### ✅ **Task 1.3: Write @smartledger/keys → AumCoin Integration Guide** (3 hours)
**Status:** ⏳ Queued  
**Effort:** 🟢 Low  
**Impact:** 🔥 High  

**Goal:** Document HOW to add ML-DSA to AumCoin using @smartledger/keys.

**Action Items:**
```bash
cd /mnt/storage/dev/dev/lightcoin-pq
touch PHASE3_PQC_INTEGRATION_GUIDE.md
```

**Content:**
1. **Option A:** Node.js addon (call @smartledger/keys from C++)
2. **Option B:** C++ reimplementation (use @noble/post-quantum)
3. **Option C:** Hybrid (JavaScript for wallet, C++ for consensus)

**Include:**
- Pros/cons of each approach
- Code snippets
- Performance estimates
- Timeline estimates

**Deliverable:** `PHASE3_PQC_INTEGRATION_GUIDE.md` (3-4 pages)

**Success Metric:** ✅ Developer can choose implementation strategy in <10 minutes

---

### ✅ **Task 1.4: Update README with Ecosystem Links** (1 hour)
**Status:** ⏳ Queued  
**Effort:** 🟢 Very Low  
**Impact:** 🔥 Medium  

**Goal:** Cross-link all three projects in READMEs.

**Action Items:**
1. AumCoin README → Add section "Related Projects"
2. @smartledger/keys README → Add "Used By" section
3. Boundless → (if you control it) add AumCoin mention

**Example:**
```markdown
## 🌐 Ecosystem

AumCoin is part of the SmartLedger quantum-resistant blockchain ecosystem:

- **[@smartledger/keys](https://npmjs.com/package/@smartledger/keys)** - Post-quantum cryptography SDK (ML-DSA-44/65/87)
- **[AumCoin](https://github.com/codenlighten/aumcoin)** - Satoshi Vision P2P cryptocurrency with restored OP_CODES
- **[Boundless Trust](https://boundlesstrust.org)** - Enterprise blockchain with native compliance (ML-DSA-44)

**Use AumCoin for:** Censorship-resistant P2P payments, privacy, freedom  
**Use Boundless for:** Tokenized securities, KYC/compliance, enterprise
```

**Deliverable:** Updated READMEs across projects

**Success Metric:** ✅ Users can discover related projects easily

---

### ✅ **Task 1.5: Create AumCoin → Boundless Bridge Whitepaper** (6 hours)
**Status:** ⏳ Queued  
**Effort:** 🟢 Low (just design, no code)  
**Impact:** 🔥 High (future vision)  

**Goal:** Document how cross-chain atomic swaps could work.

**Action Items:**
```bash
cd /mnt/storage/dev/dev/lightcoin-pq
touch BRIDGE_DESIGN.md
```

**Content:**
1. **Motivation:** Why bridge AumCoin ↔ Boundless?
2. **Architecture:** Hash time-locked contracts (HTLCs)
3. **Cryptography:** ML-DSA signatures for trustless swaps
4. **Security:** Quantum-resistant from day one
5. **Example Flow:** Alice (AumCoin) swaps with Bob (Boundless)

**Deliverable:** `BRIDGE_DESIGN.md` (5-6 pages)

**Success Metric:** ✅ External developer can understand bridge design in <15 minutes

---

## 🟡 **TIER 2: Medium Wins (3-5 days each)**

These require more effort but are still achievable quickly.

---

### ✅ **Task 2.1: Phase 2 Security - OpenSSL Upgrade** (3-5 days)
**Status:** ⏳ Queued  
**Effort:** 🟡 Medium  
**Impact:** 🔥 Critical  

**Goal:** Upgrade OpenSSL 1.0.2g → 3.3.x (50+ CVEs fixed).

**Strategy:** Use City of Boxes knowledge graph!

**Action Items:**
```bash
# Step 1: Find all OpenSSL dependencies
node query-knowledge-graph.js keyword openssl

# Step 2: List affected boxes
# Expected: KeyBox, CrypterBox, WalletBox, NetBox

# Step 3: Update one box at a time
# Box 1: KeyBox (src/key.cpp)
#   - Replace EVP_MD_CTX_create → EVP_MD_CTX_new
#   - Replace ECDSA_size → EVP_PKEY_get_size
#   - Test: npm run test:key

# Box 2: CrypterBox (src/crypter.cpp)
#   - Update AES APIs
#   - Test: npm run test:crypter

# Repeat for all boxes...
```

**Tools:**
- Use knowledge graph to track progress
- Test each box independently
- Regenerate knowledge graph after changes

**Deliverable:** 
- OpenSSL 3.3.x fully integrated
- All tests passing
- Updated knowledge graph

**Success Metric:** ✅ Zero OpenSSL CVEs remaining

---

### ✅ **Task 2.2: Phase 2 Security - Boost Upgrade** (3-4 days)
**Status:** ⏳ Queued  
**Effort:** 🟡 Medium  
**Impact:** 🔥 High  

**Goal:** Upgrade Boost 1.58 → 1.84+ (multiple CVEs fixed).

**Action Items:**
```bash
# Step 1: Find Boost dependencies
node query-knowledge-graph.js keyword boost

# Step 2: Update Boost headers
# - Update boost::filesystem → std::filesystem (C++17)
# - Update boost::thread → std::thread
# - Update boost::shared_ptr → std::shared_ptr

# Step 3: Test incrementally
```

**Deliverable:** Boost 1.84+ integrated, tests passing

**Success Metric:** ✅ Modern C++17 standard library used where possible

---

### ✅ **Task 2.3: Add OP_CHECKMLDSASIG Prototype** (4-5 days)
**Status:** ⏳ Queued  
**Effort:** 🟡 Medium  
**Impact:** 🔥 High (Phase 3 foundation)  

**Goal:** Proof-of-concept ML-DSA signature verification opcode.

**Action Items:**
```bash
# Step 1: Find where OP_CHECKSIG is implemented
node query-knowledge-graph.js keyword OP_CHECKSIG

# Step 2: Add new opcode next to it
# File: src/script.cpp
# Add: case OP_CHECKMLDSASIG:

# Step 3: Implement verification
# - Load @smartledger/keys via Node.js addon (quick prototype)
# - Or use @noble/post-quantum directly in C++

# Step 4: Write test
# File: src/test/script_tests.cpp
# Test: ML-DSA-65 signature verification
```

**Deliverable:** 
- `OP_CHECKMLDSASIG` opcode working in test branch
- Test suite passing
- Performance benchmarks

**Success Metric:** ✅ Can verify ML-DSA-65 signature in <10ms

---

### ✅ **Task 2.4: Create Unified Documentation Site** (3-4 days)
**Status:** ⏳ Queued  
**Effort:** 🟡 Medium  
**Impact:** 🔥 Medium  

**Goal:** Single documentation hub for entire ecosystem.

**Action Items:**
```bash
# Use GitHub Pages + Jekyll or Docusaurus
mkdir -p /tmp/smartledger-docs
cd /tmp/smartledger-docs

# Structure:
# /
#   /aumcoin/        - AumCoin docs
#   /keys/           - @smartledger/keys docs
#   /boundless/      - Boundless integration guide
#   /bridge/         - Cross-chain bridge design
#   /city-of-boxes/  - Development methodology
```

**Content:**
- Getting started guides
- API documentation
- Tutorial videos (optional)
- Knowledge graph explorer (interactive!)

**Deliverable:** `smartledger.tech` documentation site (or similar)

**Success Metric:** ✅ Developer can find any API in <2 minutes

---

## 🔴 **TIER 3: Long-Term Wins (2-4 weeks each)**

These are important but require significant effort. Save for later.

---

### ✅ **Task 3.1: BDB → LevelDB Migration** (2-3 weeks)
**Status:** 🔮 Future  
**Effort:** 🔴 High  
**Impact:** 🔥 Critical  

**Goal:** Replace Berkeley DB 5.3 (EOL) with LevelDB.

**Why Later:** This is a major change affecting database layer. Do after Phase 2 security is complete.

**Strategy:** Use knowledge graph to identify all database touches.

---

### ✅ **Task 3.2: Full ML-DSA Integration** (3-4 weeks)
**Status:** 🔮 Future  
**Effort:** 🔴 High  
**Impact:** 🔥 Critical  

**Goal:** Production-ready ML-DSA support in AumCoin.

**Includes:**
- OP_CHECKMLDSASIG finalized
- Wallet GUI support
- RPC commands (createmldsaaddress, etc.)
- Hybrid addresses (ECDSA + ML-DSA)
- Block size increase
- Testnet deployment

**Why Later:** Need OP_CHECKMLDSASIG prototype working first (Task 2.3).

---

### ✅ **Task 3.3: Cross-Chain Bridge Implementation** (4+ weeks)
**Status:** 🔮 Future  
**Effort:** 🔴 Very High  
**Impact:** 🔥 High  

**Goal:** Working atomic swaps between AumCoin and Boundless.

**Prerequisites:**
- AumCoin ML-DSA support (Task 3.2)
- Bridge design finalized (Task 1.5)
- Boundless team coordination

**Why Later:** Requires both chains to be quantum-ready.

---

## 📊 **Sprint Planning**

### **Sprint 1: Documentation & Discovery** (Week 1)
**Focus:** Quick wins, no code changes

- ✅ Task 1.1: Comparison matrix (2h)
- ✅ Task 1.2: City of Boxes for @smartledger/keys (4h)
- ✅ Task 1.3: PQC integration guide (3h)
- ✅ Task 1.4: Update READMEs (1h)
- ✅ Task 1.5: Bridge whitepaper (6h)

**Total:** ~16 hours (2 days)  
**Deliverables:** 5 documents, 0 code changes, 100% documentation

---

### **Sprint 2: Phase 2 Security** (Weeks 2-3)
**Focus:** OpenSSL + Boost upgrades

- ✅ Task 2.1: OpenSSL 3.3.x upgrade (5 days)
- ✅ Task 2.2: Boost 1.84+ upgrade (4 days)

**Total:** 9 days  
**Deliverables:** Zero critical CVEs, modern dependencies

---

### **Sprint 3: PQC Prototype** (Week 4)
**Focus:** Prove ML-DSA works in AumCoin

- ✅ Task 2.3: OP_CHECKMLDSASIG prototype (5 days)

**Total:** 5 days  
**Deliverable:** Working proof-of-concept, performance data

---

### **Sprint 4: Polish & Publish** (Week 5)
**Focus:** Documentation site + testing

- ✅ Task 2.4: Unified documentation site (4 days)
- 🧪 Testing, bug fixes, polish (1 day)

**Total:** 5 days  
**Deliverable:** Public documentation hub

---

## 🎯 **Success Metrics**

### **Week 1 (Documentation Sprint)**
- [ ] 5 documents published
- [ ] @smartledger/keys knowledge graph live
- [ ] Cross-project links in all READMEs
- [ ] Bridge design documented

### **Week 3 (Security Sprint)**
- [ ] OpenSSL 3.3.x integrated
- [ ] Boost 1.84+ integrated
- [ ] 0 critical CVEs remaining
- [ ] All tests passing

### **Week 4 (PQC Sprint)**
- [ ] OP_CHECKMLDSASIG working
- [ ] ML-DSA-65 verification <10ms
- [ ] Test suite passing
- [ ] Performance benchmarks published

### **Week 5 (Launch)**
- [ ] Documentation site live
- [ ] Blog post: "AumCoin Q1 2026 Progress Report"
- [ ] External security audit scheduled
- [ ] Phase 3 roadmap finalized

---

## 🚀 **Next Action**

**RIGHT NOW:** Start with Task 1.1 (Comparison Matrix)

```bash
cd /mnt/storage/dev/dev/lightcoin-pq
touch ECOSYSTEM_COMPARISON.md
```

**Contents should include:**
```markdown
# SmartLedger Ecosystem Comparison

## Quick Reference

| Feature | AumCoin | @smartledger/keys | Boundless |
|---------|---------|-------------------|-----------|
| Purpose | P2P Cash | Crypto SDK | Enterprise |
| Consensus | PoW (SHA256d) | N/A | PoW (SHA3-256) |
| Signatures | ECDSA (secp256k1) | ECDSA + ML-DSA | ML-DSA-44 |
| ...
```

---

## 📝 **Notes**

### **Why This Order?**

1. **Documentation first** - Clarify vision before coding
2. **Security second** - Fix known vulnerabilities 
3. **Innovation third** - Add PQC after foundation is solid
4. **Polish fourth** - Ship when ready

### **Risk Mitigation**

- **Each task is independent** - Can skip/reorder if needed
- **Quick feedback loops** - Ship something every few days
- **Use knowledge graph** - Never lose context
- **Measure progress** - Check off boxes, stay motivated

### **Team Velocity**

Assuming **1 developer full-time:**
- Week 1: ✅ All documentation tasks
- Week 2-3: ✅ Phase 2 security
- Week 4: ✅ PQC prototype
- Week 5: ✅ Launch documentation site

**Total: 5 weeks to Phase 2 complete + PQC prototype**

---

## 🎊 **Celebration Points**

Ship something every week, celebrate each milestone:

- 🎉 **Week 1:** Documentation sprint complete → Share on Twitter
- 🎉 **Week 3:** Phase 2 security complete → Blog post
- 🎉 **Week 4:** PQC prototype working → Demo video
- 🎉 **Week 5:** Documentation site live → Major announcement

---

**Let's start with Task 1.1 right now!** 🚀
