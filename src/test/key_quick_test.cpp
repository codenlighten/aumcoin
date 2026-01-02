// Quick ECDSA/Key validation test for OpenSSL 3.x
// Tests critical key operations after refactoring

#include <iostream>
#include <cassert>
#include "../key.h"
#include "../util.h"

using namespace std;

int main() {
    cout << "🔐 Testing ECDSA Key Operations with OpenSSL 3.x..." << endl;
    cout << "=================================================" << endl << endl;
    
    // Test 1: Key generation
    cout << "Test 1: Key generation... ";
    {
        CKey key;
        key.MakeNewKey(true);  // compressed
        assert(key.IsValid());
        assert(key.IsCompressed());
        
        CKey key2;
        key2.MakeNewKey(false);  // uncompressed
        assert(key2.IsValid());
        assert(!key2.IsCompressed());
    }
    cout << "✅ PASS" << endl;
    
    // Test 2: Public key derivation
    cout << "Test 2: Public key derivation... ";
    {
        CKey key;
        key.MakeNewKey(true);
        CPubKey pubkey = key.GetPubKey();
        
        assert(pubkey.IsValid());
        assert(pubkey.IsCompressed());
        vector<unsigned char> rawPubkey = pubkey.Raw();
        assert(rawPubkey.size() == 33);  // compressed pubkey size
    }
    cout << "✅ PASS" << endl;
    
    // Test 3: Signature creation and verification
    cout << "Test 3: Signature creation and verification... ";
    {
        CKey key;
        key.MakeNewKey(true);
        
        // Create a hash to sign
        uint256 hash;
        for (int i = 0; i < 32; i++)
            ((unsigned char*)&hash)[i] = i;
        
        // Sign the hash
        vector<unsigned char> sig;
        assert(key.Sign(hash, sig));
        assert(sig.size() > 0);
        
        // Verify signature with correct key
        assert(key.Verify(hash, sig));
        
        // Verify signature fails with different hash
        uint256 wrongHash = hash;
        ((unsigned char*)&wrongHash)[0] ^= 1;
        assert(!key.Verify(wrongHash, sig));
    }
    cout << "✅ PASS" << endl;
    
    // Test 4: Compact signatures
    cout << "Test 4: Compact signature operations... ";
    {
        CKey key;
        key.MakeNewKey(true);
        
        uint256 hash;
        for (int i = 0; i < 32; i++)
            ((unsigned char*)&hash)[i] = i * 2;
        
        // Create compact signature
        vector<unsigned char> compactSig;
        assert(key.SignCompact(hash, compactSig));
        assert(compactSig.size() == 65);  // 1 byte header + 64 bytes sig
        
        // Verify compact signature
        assert(key.VerifyCompact(hash, compactSig));
        
        // Wrong hash should fail
        uint256 wrongHash = hash;
        ((unsigned char*)&wrongHash)[0] ^= 1;
        assert(!key.VerifyCompact(wrongHash, compactSig));
    }
    cout << "✅ PASS" << endl;
    
    // Test 5: Key serialization
    cout << "Test 5: Key serialization... ";
    {
        CKey key;
        key.MakeNewKey(true);
        CPubKey pubkey = key.GetPubKey();
        
        // Serialize private key
        CPrivKey privkey = key.GetPrivKey();
        assert(privkey.size() > 0);
        
        // Deserialize and verify
        CKey key2;
        assert(key2.SetPrivKey(privkey));
        assert(key2.IsValid());
        
        // Public keys should match
        CPubKey pubkey2 = key2.GetPubKey();
        assert(pubkey == pubkey2);
    }
    cout << "✅ PASS" << endl;
    
    // Test 6: Secret operations
    cout << "Test 6: Secret key operations... ";
    {
        CKey key;
        key.MakeNewKey(true);
        
        bool fCompressed;
        CSecret secret = key.GetSecret(fCompressed);
        assert(secret.size() == 32);
        assert(fCompressed == true);
        
        CKey key2;
        assert(key2.SetSecret(secret, fCompressed));
        assert(key2.IsValid());
        
        // Keys should match
        assert(key.GetPubKey() == key2.GetPubKey());
    }
    cout << "✅ PASS" << endl;
    
    // Test 7: Multiple signatures with same key
    cout << "Test 7: Multiple signatures with same key... ";
    {
        CKey key;
        key.MakeNewKey(true);
        
        for (int i = 0; i < 10; i++) {
            uint256 hash;
            for (int j = 0; j < 32; j++)
                ((unsigned char*)&hash)[j] = i * 32 + j;
            
            vector<unsigned char> sig;
            assert(key.Sign(hash, sig));
            assert(key.Verify(hash, sig));
        }
    }
    cout << "✅ PASS" << endl;
    
    // Test 8: Key reset and reuse
    cout << "Test 8: Key reset and reuse... ";
    {
        CKey key;
        key.MakeNewKey(true);
        CPubKey pubkey1 = key.GetPubKey();
        
        // Reset and create new key
        key.Reset();
        assert(!key.IsValid());
        
        key.MakeNewKey(false);
        assert(key.IsValid());
        CPubKey pubkey2 = key.GetPubKey();
        
        // New key should be different
        assert(pubkey1 != pubkey2);
    }
    cout << "✅ PASS" << endl;
    
    cout << endl << "================================================" << endl;
    cout << "🎉 ALL ECDSA/KEY TESTS PASSED!" << endl;
    cout << "✅ Key generation working correctly" << endl;
    cout << "✅ ECDSA signatures valid with OpenSSL 3.x" << endl;
    cout << "✅ Compact signatures working" << endl;
    cout << "✅ Key serialization working" << endl;
    cout << "✅ Ready for production use" << endl;
    
    return 0;
}
