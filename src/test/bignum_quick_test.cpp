// Quick BigNum OpenSSL 3.x validation test
// Tests critical operations after refactoring

#include <iostream>
#include <cassert>
#include "../bignum.h"

using namespace std;

int main() {
    cout << "🧪 Testing BigNum with OpenSSL 3.x..." << endl;
    cout << "====================================" << endl << endl;
    
    // Test 1: Construction and basic assignment
    cout << "Test 1: Construction and basic assignment... ";
    {
        CBigNum a(0);
        CBigNum b(1);
        CBigNum c(-1);
        CBigNum d(1000000);
        
        assert(a.ToString() == "0");
        assert(b.ToString() == "1");
        assert(c.ToString() == "-1");
        assert(d.ToString() == "1000000");  // decimal
        assert(d.GetHex() == "f4240");  // hex
    }
    cout << "✅ PASS" << endl;
    
    // Test 2: Addition
    cout << "Test 2: Addition operations... ";
    {
        CBigNum a(100);
        CBigNum b(200);
        CBigNum c = a + b;
        assert(c.getint() == 300);
        
        CBigNum d(-50);
        CBigNum e = a + d;
        assert(e.getint() == 50);
    }
    cout << "✅ PASS" << endl;
    
    // Test 3: Subtraction
    cout << "Test 3: Subtraction operations... ";
    {
        CBigNum a(100);
        CBigNum b(30);
        CBigNum c = a - b;
        assert(c.getint() == 70);
        
        CBigNum d = b - a;
        assert(d.getint() == -70);
    }
    cout << "✅ PASS" << endl;
    
    // Test 4: Multiplication
    cout << "Test 4: Multiplication operations... ";
    {
        CBigNum a(10);
        CBigNum b(20);
        CBigNum c = a * b;
        assert(c.getint() == 200);
        
        CBigNum d(-5);
        CBigNum e = a * d;
        assert(e.getint() == -50);
    }
    cout << "✅ PASS" << endl;
    
    // Test 5: Division
    cout << "Test 5: Division operations... ";
    {
        CBigNum a(100);
        CBigNum b(10);
        CBigNum c = a / b;
        assert(c.getint() == 10);
        
        CBigNum d(7);
        CBigNum e = a / d;
        assert(e.getint() == 14);  // 100/7 = 14 (integer division)
    }
    cout << "✅ PASS" << endl;
    
    // Test 6: Comparison operators
    cout << "Test 6: Comparison operators... ";
    {
        CBigNum a(100);
        CBigNum b(200);
        CBigNum c(100);
        
        assert(a < b);
        assert(b > a);
        assert(a == c);
        assert(a != b);
        assert(a <= c);
        assert(b >= a);
    }
    cout << "✅ PASS" << endl;
    
    // Test 7: Shift operations
    cout << "Test 7: Bit shift operations... ";
    {
        CBigNum a(16);  // 10000 in binary
        CBigNum b = a << 2;  // Should be 64 (1000000)
        assert(b.getint() == 64);
        
        CBigNum c = b >> 1;  // Should be 32 (100000)
        assert(c.getint() == 32);
    }
    cout << "✅ PASS" << endl;
    
    // Test 8: Large number handling
    cout << "Test 8: Large number handling... ";
    {
        CBigNum a;
        a.setint64(9223372036854775807LL);  // max int64
        assert(a.GetHex() == "7fffffffffffffff");
        
        CBigNum b;
        b.setint64(-9223372036854775807LL - 1);  // min int64
        assert(b.ToString() == "-9223372036854775808");
    }
    cout << "✅ PASS" << endl;
    
    // Test 9: Copy constructor and assignment
    cout << "Test 9: Copy constructor and assignment... ";
    {
        CBigNum a(12345);
        CBigNum b(a);  // Copy constructor
        assert(b.getint() == 12345);
        
        CBigNum c;
        c = a;  // Assignment operator
        assert(c.getint() == 12345);
        
        // Modify original, ensure copies unchanged
        a = CBigNum(99999);
        assert(b.getint() == 12345);
        assert(c.getint() == 12345);
    }
    cout << "✅ PASS" << endl;
    
    // Test 10: SetHex and GetHex
    cout << "Test 10: Hex conversion... ";
    {
        CBigNum a;
        a.SetHex("deadbeef");
        assert(a.GetHex() == "deadbeef");
        
        CBigNum b;
        b.SetHex("0");
        assert(b.getint() == 0);
    }
    cout << "✅ PASS" << endl;
    
    // Test 11: bitSize() method (new in OpenSSL 3.x refactoring)
    cout << "Test 11: bitSize() method... ";
    {
        CBigNum a(255);  // 11111111 in binary (8 bits)
        assert(a.bitSize() == 8);
        
        CBigNum b(256);  // 100000000 in binary (9 bits)
        assert(b.bitSize() == 9);
        
        CBigNum c(0);
        assert(c.bitSize() == 0);
    }
    cout << "✅ PASS" << endl;
    
    cout << endl << "========================================" << endl;
    cout << "🎉 ALL TESTS PASSED!" << endl;
    cout << "✅ BigNum OpenSSL 3.x refactoring validated" << endl;
    cout << "✅ Composition pattern working correctly" << endl;
    cout << "✅ Ready for production use" << endl;
    
    return 0;
}
