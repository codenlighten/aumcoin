// Copyright (c) 2026 AumCoin Developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef AUMCOIN_MLDSA_CACHE_H
#define AUMCOIN_MLDSA_CACHE_H

#include "uint256.h"
#include "sync.h"
#include "util.h"  // For Hash() function
#include <map>
#include <list>

/**
 * CMLDSASignatureCache - Cache for expensive ML-DSA-65 signature verifications
 * 
 * ML-DSA signature verification takes ~0.5 ms per signature, which can add up
 * to significant time when validating blocks with hundreds of quantum transactions.
 * This cache stores verification results keyed by hash of (pubkey + message + signature).
 * 
 * Performance Impact:
 * - Without cache: 500 signatures = 250 ms
 * - With 80% hit rate: 100 verifications + 400 lookups = 50 ms + ~1 ms = 51 ms
 * - Speedup: ~5x for blocks with repeated signatures
 * 
 * Cache Eviction:
 * - LRU (Least Recently Used) eviction when cache exceeds nMaxSize
 * - Default: 100,000 entries (~40 MB memory for cache keys)
 * 
 * Thread Safety:
 * - All operations protected by CCriticalSection
 * - Safe for concurrent access from multiple validation threads
 */
class CMLDSASignatureCache
{
private:
    std::map<uint256, bool> mapCache;  // hash -> verification result
    mutable CCriticalSection cs_cache;  // mutable for const GetStats()
    size_t nMaxSize;
    
    // LRU tracking (simple implementation)
    std::list<uint256> lruList;
    std::map<uint256, std::list<uint256>::iterator> lruMap;
    
    // Simple hit/miss tracking (mutable for const GetStats())
    mutable size_t nHits;
    mutable size_t nMisses;
    
public:
    CMLDSASignatureCache() : nMaxSize(100000), nHits(0), nMisses(0) {}
    
    explicit CMLDSASignatureCache(size_t nMaxSizeIn) : nMaxSize(nMaxSizeIn), nHits(0), nMisses(0) {}
    
    /**
     * Get a cached verification result
     * @param hash Cache key (hash of pubkey + message + signature)
     * @param valid Output: verification result if found
     * @return true if cache hit, false if cache miss
     */
    bool Get(const uint256& hash, bool& valid)
    {
        LOCK(cs_cache);
        
        auto it = mapCache.find(hash);
        if (it != mapCache.end()) {
            // Cache hit! Update LRU
            valid = it->second;
            UpdateLRU(hash);
            nHits++;
            return true;
        }
        
        // Cache miss
        nMisses++;
        return false;
    }
    
    /**
     * Store a verification result in the cache
     * @param hash Cache key
     * @param valid Verification result (true = valid signature)
     */
    void Set(const uint256& hash, bool valid)
    {
        LOCK(cs_cache);
        
        // Check if we need to evict
        if (mapCache.size() >= nMaxSize && mapCache.find(hash) == mapCache.end()) {
            // Evict LRU entry
            if (!lruList.empty()) {
                uint256 evictHash = lruList.back();
                lruList.pop_back();
                lruMap.erase(evictHash);
                mapCache.erase(evictHash);
            }
        }
        
        // Insert new entry
        mapCache[hash] = valid;
        UpdateLRU(hash);
    }
    
    /**
     * Clear all cached entries
     * Useful for testing or if memory pressure requires it
     */
    void Clear()
    {
        LOCK(cs_cache);
        mapCache.clear();
        lruList.clear();
        lruMap.clear();
    }
    
    /**
     * Get cache statistics
     */
    struct CacheStats {
        size_t nSize;
        size_t nMaxSize;
        size_t nHits;
        size_t nMisses;
        double hitRate;
    };
    
    CacheStats GetStats() const
    {
        LOCK(cs_cache);
        
        CacheStats stats;
        stats.nSize = mapCache.size();
        stats.nMaxSize = nMaxSize;
        stats.nHits = nHits;
        stats.nMisses = nMisses;
        
        size_t total = nHits + nMisses;
        stats.hitRate = (total > 0) ? ((double)nHits / total) : 0.0;
        
        return stats;
    }
    
    /**
     * Compute cache key for a signature verification
     * @param pubkey ML-DSA-65 public key (1952 bytes)
     * @param message Message that was signed
     * @param signature ML-DSA-65 signature (1952 bytes)
     * @return Hash to use as cache key
     */
    static uint256 ComputeKey(const std::vector<unsigned char>& pubkey,
                              const uint256& message,
                              const std::vector<unsigned char>& signature)
    {
        // Concatenate all components and hash
        std::vector<unsigned char> vData;
        vData.insert(vData.end(), pubkey.begin(), pubkey.end());
        
        // Add message bytes (uint256 is 32 bytes)
        const unsigned char* pMessage = (const unsigned char*)&message;
        vData.insert(vData.end(), pMessage, pMessage + sizeof(message));
        
        vData.insert(vData.end(), signature.begin(), signature.end());
        
        // Hash the concatenated data using util.h Hash() function
        return Hash(vData.begin(), vData.end());
    }
    
private:
    void UpdateLRU(const uint256& hash)
    {
        // Remove from current position if exists
        auto lruIt = lruMap.find(hash);
        if (lruIt != lruMap.end()) {
            lruList.erase(lruIt->second);
        }
        
        // Add to front of LRU list (most recently used)
        lruList.push_front(hash);
        lruMap[hash] = lruList.begin();
    }
};

// Global signature cache instance
extern CMLDSASignatureCache mldsaSigCache;

#endif // AUMCOIN_MLDSA_CACHE_H
