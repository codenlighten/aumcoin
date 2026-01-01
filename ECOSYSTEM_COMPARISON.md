# SmartLedger Ecosystem Comparison

**The Quantum-Resistant Blockchain Stack**

---

## 🎯 **Quick Decision Guide**

**Need censorship-resistant P2P payments?** → Use **AumCoin**  
**Need post-quantum crypto library?** → Use **@smartledger/keys**  
**Need compliant tokenized securities?** → Use **Boundless Trust**

---

## 📊 **Feature Comparison**

| Feature | AumCoin | @smartledger/keys | Boundless Trust |
|---------|---------|-------------------|-----------------|
| **Type** | Layer 1 Blockchain | Cryptography SDK | Layer 1 Blockchain |
| **Purpose** | P2P Cash | Developer Tool | Enterprise Finance |
| **Status** | Testnet (Phase 1 Complete) | Production (v2.0.0) | Mainnet Live (Dec 5, 2025) |
| | | | |
| **Consensus** | PoW (SHA256d) | N/A | PoW (SHA3-256) |
| **Block Time** | 2.5 minutes | N/A | 3 minutes |
| **Total Supply** | 84 million | N/A | 21 million BLS |
| | | | |
| **Signatures** | ECDSA (secp256k1) | ECDSA + ML-DSA-44/65/87 | ML-DSA-44 + Falcon-512 |
| **Quantum-Resistant** | ⚠️ Planned (Phase 3) | ✅ Yes (NIST FIPS 204) | ✅ Yes (Day 1) |
| **Signature Size** | 64 bytes (ECDSA) | 64B - 4.6KB (hybrid) | ~2.4KB (ML-DSA-44) |
| | | | |
| **Smart Contracts** | Script-based (restored OP_CODES) | N/A | 39 native tx types |
| **OP_CODES** | 15 restored (CAT, MUL, DIV, MOD, etc.) | N/A | None (purpose-built) |
| **Scripting** | ✅ Turing-complete | N/A | ❌ (by design) |
| | | | |
| **Identity** | Pseudonymous (Bitcoin model) | N/A | E2 Identity (KYC 0-4) |
| **Compliance** | ❌ (censorship-resistant) | N/A | ✅ SEC Reg D/S |
| **Privacy** | Pseudonymous addresses | N/A | Selective disclosure (ZK) |
| | | | |
| **Use Cases** | P2P payments, DeFi, privacy | Key management, signing | Securities, bonds, governance |
| **Target Users** | Individuals, freedom advocates | Developers, enterprises | Institutions, accredited investors |
| **Philosophy** | Satoshi Vision, decentralization | Quantum-safe foundation | Regulatory compliance |
| | | | |
| **Language** | C++ | TypeScript/JavaScript | Rust |
| **Platforms** | Linux, macOS, Windows | Node.js, Browser, Edge | Linux, macOS, Windows |
| **License** | MIT | MIT | Proprietary |
| | | | |
| **Repository** | github.com/codenlighten/aumcoin | github.com/codenlighten/lumenkeys | Closed source |
| **Package** | N/A | npm: @smartledger/keys | N/A |
| **Website** | TBD | npmjs.com/package/@smartledger/keys | boundlesstrust.org |

---

## 🏗️ **Architecture Comparison**

### **AumCoin - Script-Based Flexibility**
```
Transaction → Script Evaluation → OP_CODES → Result
                     ↓
        Restored: CAT, MUL, DIV, MOD,
                  LSHIFT, RSHIFT, etc.
                     ↓
        Enables: Token issuance, DeFi,
                 complex contracts
```

**Philosophy:** Maximum flexibility through scripting  
**Trade-off:** More complex, potential attack surface  
**Best for:** Developers who need custom logic

---

### **@smartledger/keys - Unified Crypto API**
```
Application → SDK → Suite Selection → Sign/Verify
                        ↓
           ECDSA (secp256k1) or
           ML-DSA-44/65/87 (post-quantum) or
           Hybrid (both)
                        ↓
           Same API, different security levels
```

**Philosophy:** Crypto agility through abstraction  
**Trade-off:** Abstraction overhead  
**Best for:** Apps needing flexible cryptography

---

### **Boundless - Purpose-Built Transactions**
```
Transaction → Type Selection (1 of 39) → Validation → Execution
                     ↓
        Bond Issuance, Identity Anchoring,
        Governance Vote, Asset Transfer,
        Coupon Payment, etc.
                     ↓
        No smart contracts = Less attack surface
```

**Philosophy:** Simplicity through purpose-built operations  
**Trade-off:** Less flexible, can't do arbitrary logic  
**Best for:** Enterprises needing compliance

---

## 🎭 **Use Case Matrix**

| Use Case | AumCoin | @smartledger/keys | Boundless |
|----------|---------|-------------------|-----------|
| **P2P Payments** | ✅ Primary | ❌ N/A | ✅ Supported |
| **Privacy** | ✅ Pseudonymous | ❌ N/A | ⚠️ Optional (ZK) |
| **Censorship Resistance** | ✅ Core value | ❌ N/A | ❌ By design |
| | | | |
| **Token Issuance** | ✅ Via OP_CAT | ❌ N/A | ✅ Native tx type |
| **DeFi** | ✅ Via scripts | ❌ N/A | ⚠️ Limited |
| **NFTs** | ✅ Via scripts | ❌ N/A | ✅ Native support |
| | | | |
| **Tokenized Securities** | ⚠️ Possible but not compliant | ❌ N/A | ✅ Primary use case |
| **Municipal Bonds** | ❌ No compliance | ❌ N/A | ✅ MuniBonds platform |
| **KYC/AML** | ❌ Pseudonymous | ❌ N/A | ✅ E2 Identity required |
| **Accredited Investor Verification** | ❌ Not supported | ❌ N/A | ✅ Built-in (Levels 0-4) |
| | | | |
| **Library Integration** | ❌ Full node only | ✅ Primary purpose | ⚠️ Via SDK |
| **App Development** | ⚠️ Via RPC | ✅ Direct integration | ✅ Via API |
| **Post-Quantum Signatures** | ⚠️ Planned (Phase 3) | ✅ Production ready | ✅ Default |

---

## 🔐 **Cryptography Comparison**

### **Signature Algorithms**

| Algorithm | AumCoin | @smartledger/keys | Boundless |
|-----------|---------|-------------------|-----------|
| **ECDSA (secp256k1)** | ✅ Current | ✅ Supported | ❌ Not used |
| **ML-DSA-44** | ⚠️ Planned | ✅ Supported | ✅ Primary |
| **ML-DSA-65** | ⚠️ Planned | ✅ Supported | ❌ Not used |
| **ML-DSA-87** | ⚠️ Planned | ✅ Supported | ❌ Not used |
| **Falcon-512** | ❌ Not planned | ❌ Not supported | ✅ Alternate |
| **Hybrid (ECDSA + ML-DSA)** | ⚠️ Future | ✅ Supported | ❌ Not needed |

### **Security Levels**

| Level | Security | AumCoin | @smartledger/keys | Boundless |
|-------|----------|---------|-------------------|-----------|
| **Classical** | ECDSA (secp256k1) | ✅ Current | ✅ Available | ❌ |
| **Post-Quantum Level 2** | ML-DSA-44 (AES-128 equiv) | ⚠️ Planned | ✅ Available | ✅ Default |
| **Post-Quantum Level 3** | ML-DSA-65 (AES-192 equiv) | ⚠️ Planned | ✅ Available | ❌ |
| **Post-Quantum Level 5** | ML-DSA-87 (AES-256 equiv) | ⚠️ Planned | ✅ Available | ❌ |
| **Compact PQ** | Falcon-512 | ❌ | ❌ | ✅ Available |

### **Quantum Threat Timeline**

- **Today:** ECDSA secure, quantum computers not threat
- **2030-2035:** Large-scale quantum computers possible
- **2035+:** ECDSA potentially broken

**Recommendation:**
- **AumCoin:** Add ML-DSA in Phase 3, keep ECDSA default for compatibility
- **@smartledger/keys:** Already quantum-safe, use ML-DSA-65 as default
- **Boundless:** Already quantum-safe from day one (ML-DSA-44)

---

## 💰 **Economic Model**

| Feature | AumCoin | @smartledger/keys | Boundless |
|---------|---------|-------------------|-----------|
| **Native Token** | AUM | N/A | BLS |
| **Total Supply** | 84 million | N/A | 21 million |
| **Block Reward** | Halving every 840K blocks | N/A | Halving schedule |
| **Transaction Fees** | Market-based (like Bitcoin) | N/A | Type-based (predictable) |
| **Mining** | CPU/GPU (SHA256d) | N/A | ASIC-resistant (SHA3-256) |

---

## 🌉 **Interoperability**

### **Current State**
- **AumCoin ↔ @smartledger/keys:** ❌ Not yet integrated
- **AumCoin ↔ Boundless:** ❌ No bridge exists
- **@smartledger/keys ↔ Boundless:** ✅ Boundless uses ML-DSA (compatible!)

### **Future Vision (2026+)**
```
┌─────────────────┐
│    AumCoin      │  (Privacy, P2P cash)
│  ECDSA/ML-DSA   │
└────────┬────────┘
         │
         │  Cross-chain bridge
         │  (ML-DSA signatures)
         │
         ▼
┌─────────────────┐
│  @smartledger   │  (Crypto foundation)
│     /keys       │
└────────┬────────┘
         │
         │  Used by both
         │
         ▼
┌─────────────────┐
│   Boundless     │  (Compliance, securities)
│    ML-DSA-44    │
└─────────────────┘
```

**Planned:**
- AumCoin Phase 3: Add ML-DSA via @smartledger/keys
- Cross-chain atomic swaps: AumCoin ↔ Boundless
- Unified wallet: Hold AUM + BLS in one app

---

## 👥 **Target Audience**

### **AumCoin**
- **Primary:** Individuals seeking financial freedom
- **Secondary:** DeFi developers, privacy advocates
- **Geographic:** Global (censorship-resistant)
- **Technical:** Medium (wallet GUI available)

**User Persona:** "Alex" - Tech-savvy individual who wants control over their money without banks or government interference.

---

### **@smartledger/keys**
- **Primary:** Application developers
- **Secondary:** Blockchain projects, security teams
- **Geographic:** Global
- **Technical:** High (requires programming)

**User Persona:** "Jordan" - Full-stack developer building a dApp that needs quantum-resistant signatures.

---

### **Boundless Trust**
- **Primary:** Accredited investors, institutions
- **Secondary:** Municipal governments, VC funds
- **Geographic:** US + International (Reg D/S)
- **Technical:** Low (user-friendly platforms)

**User Persona:** "Morgan" - Accredited investor looking to diversify into tokenized municipal bonds with compliance built-in.

---

## 🎯 **When to Use What**

### **Use AumCoin If:**
- ✅ You need censorship-resistant payments
- ✅ You value privacy and pseudonymity
- ✅ You want to build DeFi apps with restored OP_CODES
- ✅ You need token issuance without smart contracts
- ✅ You prioritize decentralization over compliance
- ❌ You DON'T need regulatory compliance

### **Use @smartledger/keys If:**
- ✅ You're building an application that needs signatures
- ✅ You want quantum-resistant cryptography now
- ✅ You need hybrid ECDSA + ML-DSA signing
- ✅ You want a simple, unified crypto API
- ✅ You work in Node.js, browser, or edge environments
- ❌ You DON'T need a full blockchain

### **Use Boundless If:**
- ✅ You need SEC-compliant tokenized securities
- ✅ You're issuing municipal bonds, real estate, or VC funds
- ✅ You require KYC/AML verification
- ✅ You want automated coupon payments
- ✅ You need quantum-resistant signatures today
- ❌ You DON'T need scripting flexibility or privacy

---

## 🔗 **Integration Paths**

### **Scenario 1: DeFi App with Privacy**
```
Your App
   ↓
AumCoin (for transactions)
   +
@smartledger/keys (for off-chain signing)
   ↓
Result: Privacy-preserving DeFi with quantum-ready signatures
```

### **Scenario 2: Compliant Asset Management**
```
Your App
   ↓
Boundless (for tokenized securities)
   +
@smartledger/keys (if building custom integrations)
   ↓
Result: SEC-compliant tokenization with quantum resistance
```

### **Scenario 3: Hybrid Finance Platform**
```
Your App
   ↓
AumCoin (for private transactions)
   +
Boundless (for compliant offramp)
   +
@smartledger/keys (unified crypto layer)
   ↓
Result: Privacy when needed, compliance when required
```

---

## 📚 **Resources**

### **AumCoin**
- Repository: https://github.com/codenlighten/aumcoin
- Documentation: See `/docs` folder in repo
- Knowledge Graph: `node query-knowledge-graph.js`
- Status: Testnet ready, Phase 2 security in progress

### **@smartledger/keys**
- Package: https://npmjs.com/package/@smartledger/keys
- Repository: https://github.com/codenlighten/lumenkeys
- Documentation: See package README
- Status: Production ready (v2.0.0)

### **Boundless Trust**
- Website: https://boundlesstrust.org
- Explorer: https://traceboundless.com
- Apps: https://e2multipass.com
- Status: Mainnet live (launched Dec 5, 2025)

---

## 🤝 **Contributing**

All three projects share the same vision: **Quantum-resistant blockchain infrastructure for everyone.**

- **AumCoin:** Accepts contributions (MIT license)
- **@smartledger/keys:** Accepts contributions (MIT license)
- **Boundless:** Contact team for collaboration

---

## 📈 **Roadmap Alignment**

### **Q1 2026**
- **AumCoin:** Phase 2 security complete, PQC prototype
- **@smartledger/keys:** Expand documentation, browser demos
- **Boundless:** Ecosystem expansion, new dApps

### **Q2-Q3 2026**
- **AumCoin:** Phase 3 PQC integration (using @smartledger/keys)
- **Cross-chain:** Bridge design and prototype
- **Unified:** Documentation site spanning all projects

### **2027+**
- **Full ecosystem:** AumCoin ↔ Boundless bridge live
- **Quantum-ready:** All projects using ML-DSA by default
- **Mass adoption:** Millions of users across privacy and compliance use cases

---

**Last Updated:** January 1, 2026  
**Version:** 1.0  
**Maintained by:** SmartLedger Solutions / Codenlighten
