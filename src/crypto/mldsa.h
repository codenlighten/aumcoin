// Copyright (c) 2025 The AumCoin Developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef AUMCOIN_CRYPTO_MLDSA_H
#define AUMCOIN_CRYPTO_MLDSA_H

#include <vector>
#include <cstdint>
#include <cstddef>  // for size_t

/**
 * ML-DSA-65 Post-Quantum Signature Support
 * 
 * Implementation of FIPS 204 (ML-DSA) using liboqs.
 * ML-DSA-65 provides NIST Security Level 3 (equivalent to AES-192).
 * 
 * Key sizes:
 * - Public key: 1952 bytes
 * - Private key: 4032 bytes
 * - Signature: 3309 bytes
 * 
 * Performance: ~3.6x faster than ECDSA (sign + verify)
 * Security: Quantum-resistant (lattice-based cryptography)
 */

namespace MLDSA {

// ML-DSA-65 parameter sizes (from FIPS 204)
constexpr size_t PUBLIC_KEY_BYTES = 1952;
constexpr size_t PRIVATE_KEY_BYTES = 4032;
constexpr size_t SIGNATURE_BYTES = 3309;

/**
 * Generate a new ML-DSA-65 keypair
 * 
 * @param pubkey Output buffer for public key (1952 bytes)
 * @param privkey Output buffer for private key (4032 bytes)
 * @return true if successful, false on error
 */
bool GenerateKeypair(std::vector<uint8_t>& pubkey, 
                    std::vector<uint8_t>& privkey);

/**
 * Sign a message with ML-DSA-65
 * 
 * @param privkey Private key (4032 bytes)
 * @param message Message to sign
 * @param message_len Length of message in bytes
 * @param signature Output buffer for signature (3309 bytes)
 * @return true if successful, false on error
 * 
 * Note: For Bitcoin-style signing, message should be a 32-byte hash
 */
bool Sign(const std::vector<uint8_t>& privkey,
         const uint8_t* message, size_t message_len,
         std::vector<uint8_t>& signature);

/**
 * Verify an ML-DSA-65 signature
 * 
 * @param pubkey Public key (1952 bytes)
 * @param message Message that was signed
 * @param message_len Length of message in bytes
 * @param signature Signature to verify (3309 bytes)
 * @return true if signature is valid, false otherwise
 * 
 * This is the critical function for consensus validation.
 * MUST be deterministic and match liboqs reference implementation.
 */
bool Verify(const std::vector<uint8_t>& pubkey,
           const uint8_t* message, size_t message_len,
           const std::vector<uint8_t>& signature);

/**
 * Get the ML-DSA algorithm name
 * @return "ML-DSA-65" (for logging/debugging)
 */
const char* GetAlgorithmName();

/**
 * Check if ML-DSA is available and initialized
 * @return true if ML-DSA operations are available
 */
bool IsAvailable();

} // namespace MLDSA

#endif // AUMCOIN_CRYPTO_MLDSA_H
