// Copyright (c) 2025 The AumCoin Developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "mldsa.h"
#include <oqs/oqs.h>
#include <cstring>

namespace MLDSA {

// Algorithm name constant
static const char* ALGORITHM_NAME = "ML-DSA-65";

bool GenerateKeypair(std::vector<uint8_t>& pubkey, 
                    std::vector<uint8_t>& privkey) {
    // Initialize OQS signature object
    OQS_SIG *sig = OQS_SIG_new(ALGORITHM_NAME);
    if (sig == NULL) {
        return false;
    }
    
    // Resize output vectors
    pubkey.resize(PUBLIC_KEY_BYTES);
    privkey.resize(PRIVATE_KEY_BYTES);
    
    // Generate keypair
    OQS_STATUS rc = OQS_SIG_keypair(sig, pubkey.data(), privkey.data());
    
    OQS_SIG_free(sig);
    return (rc == OQS_SUCCESS);
}

bool Sign(const std::vector<uint8_t>& privkey,
         const uint8_t* message, size_t message_len,
         std::vector<uint8_t>& signature) {
    // Validate input
    if (privkey.size() != PRIVATE_KEY_BYTES) {
        return false;
    }
    
    if (message == NULL || message_len == 0) {
        return false;
    }
    
    // Initialize OQS signature object
    OQS_SIG *sig = OQS_SIG_new(ALGORITHM_NAME);
    if (sig == NULL) {
        return false;
    }
    
    // Allocate signature buffer
    signature.resize(SIGNATURE_BYTES);
    size_t signature_len;
    
    // Sign the message
    OQS_STATUS rc = OQS_SIG_sign(sig, signature.data(), &signature_len,
                                 message, message_len,
                                 privkey.data());
    
    OQS_SIG_free(sig);
    
    if (rc != OQS_SUCCESS) {
        return false;
    }
    
    // Resize to actual signature length (should be SIGNATURE_BYTES)
    signature.resize(signature_len);
    return true;
}

bool Verify(const std::vector<uint8_t>& pubkey,
           const uint8_t* message, size_t message_len,
           const std::vector<uint8_t>& signature) {
    // Validate inputs
    if (pubkey.size() != PUBLIC_KEY_BYTES) {
        return false;
    }
    
    if (signature.size() != SIGNATURE_BYTES) {
        return false;
    }
    
    if (message == NULL || message_len == 0) {
        return false;
    }
    
    // Initialize OQS signature object
    OQS_SIG *sig = OQS_SIG_new(ALGORITHM_NAME);
    if (sig == NULL) {
        return false;
    }
    
    // Verify the signature
    OQS_STATUS rc = OQS_SIG_verify(sig, message, message_len,
                                   signature.data(), signature.size(),
                                   pubkey.data());
    
    OQS_SIG_free(sig);
    
    return (rc == OQS_SUCCESS);
}

const char* GetAlgorithmName() {
    return ALGORITHM_NAME;
}

bool IsAvailable() {
    return (OQS_SIG_alg_is_enabled(ALGORITHM_NAME) == 1);
}

} // namespace MLDSA
