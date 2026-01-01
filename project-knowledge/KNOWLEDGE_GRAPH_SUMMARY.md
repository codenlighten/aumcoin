# AumCoin Knowledge Graph Summary

**Generated:** 2026-01-01T22:27:08.829Z

**Protocol:** City of Boxes v1.0

---

## 📊 Statistics

- **Total Boxes:** 209
- **Categories:** 13
- **Embeddings:** 209
- **Error Templates:** 209

## 📂 Categories

### documentation
Boxes: 12
- BUILDBox, CHANGELOGBox, LAUNCHBox, LIGHTCOINSATOSHIVISIONBox, PROJECTSUMMARYBox, ... (7 more)

### infrastructure
Boxes: 2
- DockerfileBox, DockerComposeBox

### testing
Boxes: 18
- TESTINGBox, UritestsBox, UritestsBox, CheckpointsTestsBox, DoSTestsBox, ... (13 more)

### other
Boxes: 207
- AumcoinQtBox, BitcoindBox, BitcoinQtBox, BitcoindBox, ReleaseProcessBox, ... (202 more)

### network
Boxes: 9
- LaunchNetworkBox, NetBox, NetBox, NetbaseBox, NetbaseBox, ... (4 more)

### build-system
Boxes: 2
- LumenBridgeIntegrationBox, MakeWindowsIconBox

### api
Boxes: 7
- BitcoinrpcBox, BitcoinrpcBox, RpcconsoleBox, RpcconsoleBox, RpcconsoleBox, ... (2 more)

### storage
Boxes: 2
- DbBox, DbBox

### utilities
Boxes: 6
- JsonSpiritUtilsBox, GuiutilBox, GuiutilBox, UtilTestsBox, UtilBox, ... (1 more)

### cryptography
Boxes: 6
- KeyBox, KeyBox, KeystoreBox, KeystoreBox, KeyBox, ... (1 more)

### core-consensus
Boxes: 3
- MainBox, MainBox, TestMainBox

### wallet
Boxes: 7
- WalletmodelBox, WalletmodelBox, WalletTestsBox, WalletBox, WalletBox, ... (2 more)

### script-engine
Boxes: 6
- ScriptBox, ScriptBox, ScriptInvalidBox, ScriptValidBox, ScriptP2SHTestsBox, ... (1 more)

## 🔑 Key Boxes

### ScriptBox
**Path:** `src/script.h`

**Description:** Copyright (c) 2009-2010 Satoshi Nakamoto Copyright (c) 2009-2012 The Bitcoin developers Copyright (c) 2011-2012 Litecoin Developers Distributed under the MIT/X11 software license, see the accompanying file COPYING or http://www.opensource.org/licenses/mit-license.php.

**Functions:** 50
**Classes:** 3

### MainBox
**Path:** `src/main.h`

**Description:** Copyright (c) 2009-2010 Satoshi Nakamoto Copyright (c) 2009-2012 The Bitcoin developers Copyright (c) 2011-2012 Litecoin Developers Copyright (c) 2025 AumCoin Developers Distributed under the MIT/X11 software license, see the accompanying

**Functions:** 50
**Classes:** 27

### InitBox
**Path:** `src/init.h`

**Description:** Copyright (c) 2009-2010 Satoshi Nakamoto Copyright (c) 2009-2012 The Bitcoin developers Copyright (c) 2011-2012 Litecoin Developers Distributed under the MIT/X11 software license, see the accompanying file COPYING or http://www.opensource.org/licenses/mit-license.php.

**Functions:** 3
**Classes:** 0

### BitcoinrpcBox
**Path:** `src/bitcoinrpc.h`

**Description:** Copyright (c) 2010 Satoshi Nakamoto Copyright (c) 2009-2012 The Bitcoin developers Copyright (c) 2011-2012 Litecoin Developers Distributed under the MIT/X11 software license, see the accompanying file COPYING or http://www.opensource.org/licenses/mit-license.php.

**Functions:** 2
**Classes:** 2

### WalletBox
**Path:** `src/wallet.h`

**Description:** Copyright (c) 2009-2010 Satoshi Nakamoto Copyright (c) 2009-2012 The Bitcoin developers Copyright (c) 2011-2012 Litecoin Developers Distributed under the MIT/X11 software license, see the accompanying file COPYING or http://www.opensource.org/licenses/mit-license.php.

**Functions:** 50
**Classes:** 12

## 🔗 Usage

```javascript
// Load the knowledge graph
const legend = require("./project-knowledge/master-legend.json");
const searchIndex = require("./project-knowledge/search-index.json");

// Find a box
const scriptBox = legend.boxes["ScriptBox"];
console.log(scriptBox.description);
```

## 🏗️ City of Boxes Protocol

This knowledge graph follows the **City of Boxes** protocol:

1. **Boxes** - Discrete modules with clear boundaries
2. **Contracts** - Defined inputs/outputs/errors
3. **Context-Rich Errors** - Errors contain repair instructions
4. **Semantic Search** - Find boxes by meaning, not just name
5. **AI-Native** - Designed for LLM consumption

## 🚀 Next Steps

1. Use `search-index.json` for semantic queries
2. Reference `error-templates.json` when implementing error handling
3. Keep `master-legend.json` updated as code evolves
4. Use Lumen Bridge API for embedding generation in production

