// Debug BigNum ToString() behavior
#include <iostream>
#include "../bignum.h"

using namespace std;

int main() {
    CBigNum a(0);
    CBigNum b(1);
    CBigNum c(-1);
    CBigNum d(1000000);
    
    cout << "a(0).ToString() = '" << a.ToString() << "'" << endl;
    cout << "b(1).ToString() = '" << b.ToString() << "'" << endl;
    cout << "c(-1).ToString() = '" << c.ToString() << "'" << endl;
    cout << "d(1000000).ToString() = '" << d.ToString() << "'" << endl;
    
    cout << endl << "getint() values:" << endl;
    cout << "a.getint() = " << a.getint() << endl;
    cout << "b.getint() = " << b.getint() << endl;
    cout << "c.getint() = " << c.getint() << endl;
    cout << "d.getint() = " << d.getint() << endl;
    
    return 0;
}
