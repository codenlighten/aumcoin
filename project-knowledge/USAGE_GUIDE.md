# AumCoin Knowledge Graph - Usage Guide

**City of Boxes Protocol Implementation**

This guide explains how to use the AumCoin knowledge graph for methodical AI-assisted development.

---

## 📚 What is This?

The AumCoin knowledge graph is a complete semantic map of the entire codebase, following the **City of Boxes** protocol. It enables:

1. **AI Context Management** - Work on specific modules without loading entire codebase
2. **Semantic Search** - Find code by meaning, not just filename
3. **Dependency Tracking** - Understand how modules connect
4. **Context-Rich Errors** - Errors that contain repair instructions
5. **Methodical Development** - Structured approach to complex changes

---

## 🗂️ File Structure

```
project-knowledge/
├── master-legend.json           # Complete registry of all "boxes" (modules)
├── embeddings.json              # Lumen Bridge embeddings for semantic search
├── search-index.json            # Optimized search index
├── error-templates.json         # Context-rich error templates
├── KNOWLEDGE_GRAPH_SUMMARY.md   # Human-readable overview
└── USAGE_GUIDE.md              # This file
```

---

## 🔍 Using the Query Tool

### Basic Commands

```bash
# Show project statistics
node query-knowledge-graph.js stats

# List all categories
node query-knowledge-graph.js categories

# Search by category
node query-knowledge-graph.js category script-engine
node query-knowledge-graph.js category core-consensus
node query-knowledge-graph.js category wallet

# Search by keyword
node query-knowledge-graph.js keyword transaction
node query-knowledge-graph.js keyword RPC
node query-knowledge-graph.js keyword OP_CAT

# Find dependencies
node query-knowledge-graph.js depends script.h
node query-knowledge-graph.js depends wallet.h

# Get box details
node query-knowledge-graph.js box ScriptBox
node query-knowledge-graph.js box MainBox
node query-knowledge-graph.js box WalletBox
```

---

## 🏗️ City of Boxes Architecture

### What is a "Box"?

A **Box** is a self-contained module with:

- **Identity** - Unique ID and path
- **Contract** - Defined inputs/outputs/errors
- **Interface** - Functions, classes, OP_CODES exposed
- **Dependencies** - What it requires
- **Embedding** - Semantic vector for AI understanding

### Example Box Structure

```json
{
  "id": "ScriptBox",
  "path": "src/script.cpp",
  "category": "script-engine",
  "description": "Core script evaluation engine...",
  "aiContext": "Handles all OP_CODES including restored operations...",
  
  "interface": {
    "functions": ["EvalScript", "Solver", "SignSignature"],
    "classes": ["CScript", "CScriptCheck"],
    "opcodes": ["OP_CAT", "OP_MUL", "OP_DIV", ...]
  },
  
  "dependencies": ["bignum.h", "keystore.h", "main.h"],
  
  "contract": {
    "inputs": { "script": "CScript", "txTo": "CTransaction" },
    "outputs": { "result": "bool", "stack": "vector<vector<byte>>" },
    "errors": ["SCRIPT_ERR_INVALID_STACK_OPERATION", ...],
    "guarantees": ["Thread-safe", "No global state"]
  },
  
  "embedding": {
    "available": true,
    "model": "lumen-bridge-v1",
    "dimensions": 1536
  }
}
```

---

## 🤖 AI Development Workflow

### 1. **Context Discovery**

Instead of: *"Load entire 30,000 line codebase"*

Do:
```bash
# Find relevant modules
node query-knowledge-graph.js keyword transaction

# Get specific box details
node query-knowledge-graph.js box TransactionBox

# Check dependencies
node query-knowledge-graph.js depends main.h
```

**Result:** Load only 2-3 relevant files (~2000 lines) instead of entire codebase

---

### 2. **Methodical Changes**

For complex changes (e.g., Phase 2 security upgrades):

**Step 1:** Identify affected boxes
```bash
node query-knowledge-graph.js category cryptography
node query-knowledge-graph.js keyword openssl
```

**Step 2:** Create change plan
```json
{
  "task": "Upgrade OpenSSL 1.0.2g → 3.3",
  "affected_boxes": [
    "KeyBox (src/key.cpp)",
    "CrypterBox (src/crypter.cpp)", 
    "WalletBox (src/wallet.cpp)"
  ],
  "steps": [
    "Update KeyBox: Replace EVP_MD_CTX_create → EVP_MD_CTX_new",
    "Update CrypterBox: Replace AES_set_encrypt_key API",
    "Test: Run src/test/key_tests.cpp"
  ]
}
```

**Step 3:** Execute changes box-by-box
- Load one box at a time
- Make changes
- Update knowledge graph
- Test
- Move to next box

---

### 3. **Error Handling**

Use context-rich error templates:

```javascript
// Load error template
const errorTemplate = require('./project-knowledge/error-templates.json');
const scriptBoxError = errorTemplate.templates.ScriptBox;

// When error occurs, construct context-rich error
const error = {
  // Layer 1: Identity
  boxId: "ScriptBox",
  boxPath: "src/script.cpp",
  
  // Layer 2: Definition
  definition: scriptBoxError.definition,
  purpose: scriptBoxError.purpose,
  
  // Layer 3: Contract
  expectedInputs: scriptBoxError.contract.inputs,
  actualInputs: actualInputs,
  
  // Layer 4: Runtime
  timestamp: Date.now(),
  stackTrace: new Error().stack,
  systemState: {
    memory: process.memoryUsage(),
    uptime: process.uptime()
  }
};

// AI can now self-repair using complete context
```

---

### 4. **Semantic Search** (with Lumen Bridge)

```javascript
// Production: Use Lumen Bridge API
const response = await fetch('https://lumenbridge.xyz/api/search', {
  method: 'POST',
  headers: { 'Content-Type': 'application/json' },
  body: JSON.stringify({
    query: "Where is transaction validation logic?",
    embeddings: searchIndex.vectors,
    topK: 5
  })
});

const results = await response.json();
// Results: [MainBox, ScriptBox, TransactionBox, ...]

// Fallback: Keyword search
node query-knowledge-graph.js keyword "transaction validation"
```

---

## 📝 Common Workflows

### A. Adding New OP_CODE

1. **Discover related boxes**
   ```bash
   node query-knowledge-graph.js keyword OP_CODE
   # Result: ScriptBox, Script.cppBox
   ```

2. **Load only needed files**
   - `src/script.h` (OP_CODE definitions)
   - `src/script.cpp` (evaluation logic)
   - `src/test/script_tests.cpp` (tests)

3. **Make changes**
   - Add OP_CODE constant
   - Implement evaluation
   - Add tests

4. **Update knowledge graph**
   ```bash
   node create-knowledge-graph.js
   ```

### B. Security Audit

1. **Find all crypto boxes**
   ```bash
   node query-knowledge-graph.js category cryptography
   ```

2. **Check OpenSSL usage**
   ```bash
   node query-knowledge-graph.js keyword openssl
   ```

3. **Audit box-by-box**
   ```bash
   node query-knowledge-graph.js box KeyBox
   # Review functions, dependencies, security contract
   ```

### C. Building New Feature

1. **Semantic discovery**
   ```bash
   node query-knowledge-graph.js semantic "where are transactions signed"
   # Result: KeyBox, WalletBox, ScriptBox
   ```

2. **Review interfaces**
   ```bash
   node query-knowledge-graph.js box WalletBox
   # Shows: functions, classes, dependencies
   ```

3. **Check dependencies**
   ```bash
   node query-knowledge-graph.js depends wallet.h
   # Shows: what will break if you change wallet.h
   ```

---

## 🔄 Keeping Knowledge Graph Updated

Regenerate after significant changes:

```bash
# Full regeneration
node create-knowledge-graph.js

# Verify
node query-knowledge-graph.js stats
```

**When to regenerate:**
- After adding/removing files
- After major refactoring
- Weekly during active development
- Before starting Phase 2 security work

---

## 🎯 Benefits

### Traditional Approach
- Load entire codebase: 30,000+ lines
- Burn context window on irrelevant code
- Hard to track dependencies
- Errors lack context for repair
- Sequential, ad-hoc development

### City of Boxes Approach
- Load only relevant boxes: 2,000-3,000 lines
- Semantic search finds exact code needed
- Dependency tracking automatic
- Errors contain repair instructions
- Methodical, structured development

**Context Savings:** 90%+ reduction in tokens per task

---

## 🚀 Next Steps

1. **Phase 2 Security** - Use knowledge graph to methodically upgrade:
   - Query: `node query-knowledge-graph.js keyword openssl`
   - Load only affected boxes
   - Update box-by-box
   - Regenerate graph

2. **External Audit** - Share knowledge graph with auditors:
   - `KNOWLEDGE_GRAPH_SUMMARY.md` - Overview
   - `master-legend.json` - Complete technical map
   - Auditors can query specific areas efficiently

3. **Lumen Bridge Integration** - Connect to production API:
   - Replace mock embeddings with real semantic vectors
   - Enable true natural language queries
   - "Find all timing attack vulnerabilities"
   - "Where is ECDSA signature verification?"

4. **Self-Registration** - New modules auto-register:
   - Template: `templates/box-template.cpp`
   - On file creation → auto-add to master legend
   - Maintain graph without manual updates

---

## 📚 Resources

- **City of Boxes Protocol:** `city-of-boxes.md`
- **Security Analysis:** `SECURITY_ANALYSIS.md`
- **Project Status:** `STATUS.md`
- **Testing Guide:** `TESTING.md`

---

## 🤝 Contributing

When adding code:

1. Follow Box encapsulation principles
2. Document inputs/outputs clearly
3. Register new boxes in legend
4. Update embeddings if significant changes

```bash
# After changes
node create-knowledge-graph.js
git add project-knowledge/
git commit -m "Update knowledge graph"
```

---

## 💡 Tips

1. **Start Small** - Query before loading
2. **Think in Boxes** - One module at a time
3. **Use Categories** - Group related work
4. **Check Dependencies** - Know what breaks
5. **Update Often** - Keep graph current

---

**Version:** 1.0.0  
**Protocol:** City of Boxes v1.0  
**Generated:** 2026-01-01  
**Project:** AumCoin (Satoshi Vision)
