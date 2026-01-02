// Test AumCoin ML-DSA wrapper
// Validates our mldsa.h/cpp implementation

#include <iostream>
#include <cassert>
#include <cstring>
#include "../crypto/mldsa.h"
#include "../uint256.h"

using namespace std;

int main() {
    cout << "🔮 Testing AumCoin ML-DSA Wrapper..." << endl;
    cout << "====================================" << endl << endl;
    
    // Test 1: Availability check
    cout << "Test 1: ML-DSA availability... ";
    {
        assert(MLDSA::IsAvailable());
        cout << "(" << MLDSA::GetAlgorithmName() << ") ";
    }
    cout << "✅ PASS" << endl;
    
    // Test 2: Key generation
    cout << "Test 2: Key generation... ";
    {
        vector<uint8_t> pubkey, privkey;
        assert(MLDSA::GenerateKeypair(pubkey, privkey));
        assert(pubkey.size() == MLDSA::PUBLIC_KEY_BYTES);
        assert(privkey.size() == MLDSA::PRIVATE_KEY_BYTES);
    }
    cout << "✅ PASS" << endl;
    
    // Test 3: Sign and verify
    cout << "Test 3: Sign and verify... ";
    {
        vector<uint8_t> pubkey, privkey;
        MLDSA::GenerateKeypair(pubkey, privkey);
        
        const char* message = "Hello, AumCoin!";
        vector<uint8_t> signature;
        
        assert(MLDSA::Sign(privkey, (const uint8_t*)message, strlen(message), signature));
        assert(signature.size() == MLDSA::SIGNATURE_BYTES);
        assert(MLDSA::Verify(pubkey, (const uint8_t*)message, strlen(message), signature));
    }
    cout << "✅ PASS" << endl;
    
    // Test 4: Wrong message fails verification
    cout << "Test 4: Wrong message detection... ";
    {
        vector<uint8_t> pubkey, privkey;
        MLDSA::GenerateKeypair(pubkey, privkey);
        
        const char* message = "Original message";
        vector<uint8_t> signature;
        MLDSA::Sign(privkey, (const uint8_t*)message, strlen(message), signature);
        
        const char* wrong_message = "Modified message";
        assert(!MLDSA::Verify(pubkey, (const uint8_t*)wrong_message, 
                             strlen(wrong_message), signature));
    }
    cout << "✅ PASS" << endl;
    
    // Test 5: Bitcoin-style transaction hash signing
    cout << "Test 5: Bitcoin-style hash signing... ";
    {
        vector<uint8_t> pubkey, privkey;
        MLDSA::GenerateKeypair(pubkey, privkey);
        
        // Simulate a transaction hash (32 bytes)
        uint256 tx_hash;
        for (int i = 0; i < 32; i++)
            ((unsigned char*)&tx_hash)[i] = i;
        
        // Sign the hash
        vector<uint8_t> signature;
        assert(MLDSA::Sign(privkey, (const uint8_t*)&tx_hash, 32, signature));
        
        // Verify the signature
        assert(MLDSA::Verify(pubkey, (const uint8_t*)&tx_hash, 32, signature));
        
        // Wrong hash should fail
        uint256 wrong_hash = tx_hash;
        ((unsigned char*)&wrong_hash)[0] ^= 1;
        assert(!MLDSA::Verify(pubkey, (const uint8_t*)&wrong_hash, 32, signature));
    }
    cout << "✅ PASS" << endl;
    
    // Test 6: Multiple operations
    cout << "Test 6: Multiple sign/verify cycles... ";
    {
        vector<uint8_t> pubkey, privkey;
        MLDSA::GenerateKeypair(pubkey, privkey);
        
        for (int i = 0; i < 10; i++) {
            char message[64];
            snprintf(message, sizeof(message), "Transaction %d", i);
            
            vector<uint8_t> signature;
            assert(MLDSA::Sign(privkey, (const uint8_t*)message, 
                             strlen(message), signature));
            assert(MLDSA::Verify(pubkey, (const uint8_t*)message, 
                               strlen(message), signature));
        }
    }
    cout << "✅ PASS" << endl;
    
    // Test 7: Invalid key sizes rejected
    cout << "Test 7: Invalid key size detection... ";
    {
        vector<uint8_t> pubkey, privkey;
        MLDSA::GenerateKeypair(pubkey, privkey);
        
        const char* message = "Test";
        vector<uint8_t> signature;
        
        // Invalid private key size
        vector<uint8_t> bad_privkey(100);  // Wrong size
        assert(!MLDSA::Sign(bad_privkey, (const uint8_t*)message, 
                          strlen(message), signature));
        
        // Generate valid signature for next test
        MLDSA::Sign(privkey, (const uint8_t*)message, strlen(message), signature);
        
        // Invalid public key size
        vector<uint8_t> bad_pubkey(100);  // Wrong size
        assert(!MLDSA::Verify(bad_pubkey, (const uint8_t*)message, 
                            strlen(message), signature));
    }
    cout << "✅ PASS" << endl;
    
    // Test 8: Empty message/NULL pointer handling
    cout << "Test 8: Edge case handling... ";
    {
        vector<uint8_t> pubkey, privkey;
        MLDSA::GenerateKeypair(pubkey, privkey);
        
        vector<uint8_t> signature;
        
        // NULL message pointer
        assert(!MLDSA::Sign(privkey, NULL, 10, signature));
        
        // Zero length message
        const char* message = "test";
        assert(!MLDSA::Sign(privkey, (const uint8_t*)message, 0, signature));
    }
    cout << "✅ PASS" << endl;
    
    // Test 9: Key size constants
    cout << "Test 9: Key size constants... ";
    {
        assert(MLDSA::PUBLIC_KEY_BYTES == 1952);
        assert(MLDSA::PRIVATE_KEY_BYTES == 4032);
        assert(MLDSA::SIGNATURE_BYTES == 3309);
    }
    cout << "✅ PASS" << endl;
    
    cout << endl << "====================================" << endl;
    cout << "🎉 ALL MLDSA WRAPPER TESTS PASSED!" << endl;
    cout << "✅ AumCoin ML-DSA wrapper working" << endl;
    cout << "✅ Key generation validated" << endl;
    cout << "✅ Sign/verify operations correct" << endl;
    cout << "✅ Bitcoin-style hash signing working" << endl;
    cout << "✅ Input validation working" << endl;
    cout << "✅ Ready for OP_CHECKMLDSASIG integration" << endl;
    
    return 0;
}
