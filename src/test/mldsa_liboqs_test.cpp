// Test ML-DSA-65 (FIPS 204) using liboqs
// Validates post-quantum signature algorithm

#include <iostream>
#include <cassert>
#include <cstring>
#include <oqs/oqs.h>

using namespace std;

int main() {
    cout << "🔮 Testing ML-DSA-65 (Post-Quantum Signatures)..." << endl;
    cout << "================================================" << endl << endl;
    
    // Test 1: Algorithm availability
    cout << "Test 1: ML-DSA-65 availability... ";
    {
        int is_enabled = OQS_SIG_alg_is_enabled("ML-DSA-65");
        assert(is_enabled == 1);
    }
    cout << "✅ PASS" << endl;
    
    // Test 2: Signature object creation
    cout << "Test 2: ML-DSA-65 object creation... ";
    {
        OQS_SIG *sig = OQS_SIG_new("ML-DSA-65");
        assert(sig != NULL);
        
        cout << endl;
        cout << "  - Public key size: " << sig->length_public_key << " bytes" << endl;
        cout << "  - Secret key size: " << sig->length_secret_key << " bytes" << endl;
        cout << "  - Signature size: " << sig->length_signature << " bytes" << endl;
        
        OQS_SIG_free(sig);
    }
    cout << "✅ PASS" << endl;
    
    // Test 3: Key generation
    cout << "Test 3: ML-DSA-65 key generation... ";
    {
        OQS_SIG *sig = OQS_SIG_new("ML-DSA-65");
        
        uint8_t *public_key = (uint8_t *)malloc(sig->length_public_key);
        uint8_t *secret_key = (uint8_t *)malloc(sig->length_secret_key);
        
        OQS_STATUS rc = OQS_SIG_keypair(sig, public_key, secret_key);
        assert(rc == OQS_SUCCESS);
        
        free(public_key);
        free(secret_key);
        OQS_SIG_free(sig);
    }
    cout << "✅ PASS" << endl;
    
    // Test 4: Sign and verify
    cout << "Test 4: ML-DSA-65 sign and verify... ";
    {
        OQS_SIG *sig = OQS_SIG_new("ML-DSA-65");
        
        uint8_t *public_key = (uint8_t *)malloc(sig->length_public_key);
        uint8_t *secret_key = (uint8_t *)malloc(sig->length_secret_key);
        OQS_SIG_keypair(sig, public_key, secret_key);
        
        // Create a message to sign
        const char *message = "Hello, post-quantum world!";
        size_t message_len = strlen(message);
        
        // Sign the message
        uint8_t *signature = (uint8_t *)malloc(sig->length_signature);
        size_t signature_len;
        OQS_STATUS rc = OQS_SIG_sign(sig, signature, &signature_len,
                                     (const uint8_t *)message, message_len,
                                     secret_key);
        assert(rc == OQS_SUCCESS);
        assert(signature_len == sig->length_signature);
        
        // Verify the signature
        rc = OQS_SIG_verify(sig, (const uint8_t *)message, message_len,
                           signature, signature_len, public_key);
        assert(rc == OQS_SUCCESS);
        
        free(signature);
        free(public_key);
        free(secret_key);
        OQS_SIG_free(sig);
    }
    cout << "✅ PASS" << endl;
    
    // Test 5: Wrong signature fails
    cout << "Test 5: ML-DSA-65 wrong signature detection... ";
    {
        OQS_SIG *sig = OQS_SIG_new("ML-DSA-65");
        
        uint8_t *public_key = (uint8_t *)malloc(sig->length_public_key);
        uint8_t *secret_key = (uint8_t *)malloc(sig->length_secret_key);
        OQS_SIG_keypair(sig, public_key, secret_key);
        
        const char *message = "Original message";
        size_t message_len = strlen(message);
        
        uint8_t *signature = (uint8_t *)malloc(sig->length_signature);
        size_t signature_len;
        OQS_SIG_sign(sig, signature, &signature_len,
                    (const uint8_t *)message, message_len, secret_key);
        
        // Try to verify with wrong message
        const char *wrong_message = "Modified message";
        OQS_STATUS rc = OQS_SIG_verify(sig, (const uint8_t *)wrong_message,
                                      strlen(wrong_message),
                                      signature, signature_len, public_key);
        assert(rc == OQS_ERROR);  // Should fail
        
        free(signature);
        free(public_key);
        free(secret_key);
        OQS_SIG_free(sig);
    }
    cout << "✅ PASS" << endl;
    
    // Test 6: Multiple sign/verify cycles
    cout << "Test 6: Multiple ML-DSA-65 operations... ";
    {
        OQS_SIG *sig = OQS_SIG_new("ML-DSA-65");
        
        uint8_t *public_key = (uint8_t *)malloc(sig->length_public_key);
        uint8_t *secret_key = (uint8_t *)malloc(sig->length_secret_key);
        OQS_SIG_keypair(sig, public_key, secret_key);
        
        for (int i = 0; i < 10; i++) {
            char message[64];
            snprintf(message, sizeof(message), "Test message %d", i);
            size_t message_len = strlen(message);
            
            uint8_t *signature = (uint8_t *)malloc(sig->length_signature);
            size_t signature_len;
            
            OQS_STATUS rc = OQS_SIG_sign(sig, signature, &signature_len,
                                        (const uint8_t *)message, message_len,
                                        secret_key);
            assert(rc == OQS_SUCCESS);
            
            rc = OQS_SIG_verify(sig, (const uint8_t *)message, message_len,
                               signature, signature_len, public_key);
            assert(rc == OQS_SUCCESS);
            
            free(signature);
        }
        
        free(public_key);
        free(secret_key);
        OQS_SIG_free(sig);
    }
    cout << "✅ PASS" << endl;
    
    // Test 7: Hash signing (Bitcoin-style)
    cout << "Test 7: ML-DSA-65 with SHA256 hash (Bitcoin-style)... ";
    {
        OQS_SIG *sig = OQS_SIG_new("ML-DSA-65");
        
        uint8_t *public_key = (uint8_t *)malloc(sig->length_public_key);
        uint8_t *secret_key = (uint8_t *)malloc(sig->length_secret_key);
        OQS_SIG_keypair(sig, public_key, secret_key);
        
        // Simulate a Bitcoin transaction hash (32 bytes)
        uint8_t tx_hash[32];
        for (int i = 0; i < 32; i++)
            tx_hash[i] = i;
        
        // Sign the hash
        uint8_t *signature = (uint8_t *)malloc(sig->length_signature);
        size_t signature_len;
        OQS_STATUS rc = OQS_SIG_sign(sig, signature, &signature_len,
                                     tx_hash, 32, secret_key);
        assert(rc == OQS_SUCCESS);
        
        // Verify the signature
        rc = OQS_SIG_verify(sig, tx_hash, 32, signature, signature_len,
                           public_key);
        assert(rc == OQS_SUCCESS);
        
        free(signature);
        free(public_key);
        free(secret_key);
        OQS_SIG_free(sig);
    }
    cout << "✅ PASS" << endl;
    
    cout << endl << "================================================" << endl;
    cout << "🎉 ALL ML-DSA-65 TESTS PASSED!" << endl;
    cout << "✅ Post-quantum signatures working" << endl;
    cout << "✅ Key generation validated" << endl;
    cout << "✅ Sign/verify operations correct" << endl;
    cout << "✅ Wrong signature detection working" << endl;
    cout << "✅ Bitcoin-style hash signing validated" << endl;
    cout << "✅ Ready for AumCoin integration" << endl;
    
    return 0;
}
