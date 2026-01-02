// Copyright (c) 2026 AumCoin Developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "mldsa_cache.h"

// Global ML-DSA signature verification cache
// Size: 100,000 entries (configurable)
// Memory usage: ~40 MB for cache keys + metadata
// Expected hit rate: 80%+ on block validation
CMLDSASignatureCache mldsaSigCache;
