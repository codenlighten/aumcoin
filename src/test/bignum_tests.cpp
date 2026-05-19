#include <boost/test/unit_test.hpp>
#include <limits>

#include "bignum.h"
#include "util.h"

BOOST_AUTO_TEST_SUITE(bignum_tests)

// Unfortunately there's no standard way of preventing a function from being
// inlined, so we define a macro for it.
//
// You should use it like this:
//   NOINLINE void function() {...}
#if defined(__GNUC__)
// This also works and will be defined for any compiler implementing gcc
// extensions, such as clang and icc.
#define NOINLINE __attribute__((noinline))
#elif defined(_MSC_VER)
#define NOINLINE __declspec(noinline)
#else
// We give out a warning because it impacts the correctness of one bignum test.
#warning You should define NOINLINE for your compiler.
#define NOINLINE
#endif

// For the following test case, it is useful to use additional tools.
//
// The simplest one to use is the compiler flag -ftrapv, which detects integer
// overflows and similar errors. However, due to optimizations and compilers
// taking advantage of undefined behavior sometimes it may not actually detect
// anything.
//
// You can also use compiler-based stack protection to possibly detect possible
// stack buffer overruns.
//
// For more accurate diagnostics, you can use an undefined arithmetic operation
// detector such as the clang-based tool:
//
// "IOC: An Integer Overflow Checker for C/C++"
//
// Available at: http://embed.cs.utah.edu/ioc/
//
// It might also be useful to use Google's AddressSanitizer to detect
// stack buffer overruns, which valgrind can't currently detect.

// Let's force this code not to be inlined, in order to actually
// test a generic version of the function. This increases the chance
// that -ftrapv will detect overflows.
NOINLINE void mysetint64(CBigNum& num, int64 n)
{
	num.setint64(n);
}

// For each number, we do 2 tests: one with inline code, then we reset the
// value to 0, then the second one with a non-inlined function.
BOOST_AUTO_TEST_CASE(bignum_setint64)
{
    int64 n;

    {
        n = 0;
        CBigNum num(n);
        BOOST_CHECK(num.ToString() == "0");
        num.setulong(0);
        BOOST_CHECK(num.ToString() == "0");
        mysetint64(num, n);
        BOOST_CHECK(num.ToString() == "0");
    }
    {
        n = 1;
        CBigNum num(n);
        BOOST_CHECK(num.ToString() == "1");
        num.setulong(0);
        BOOST_CHECK(num.ToString() == "0");
        mysetint64(num, n);
        BOOST_CHECK(num.ToString() == "1");
    }
    {
        n = -1;
        CBigNum num(n);
        BOOST_CHECK(num.ToString() == "-1");
        num.setulong(0);
        BOOST_CHECK(num.ToString() == "0");
        mysetint64(num, n);
        BOOST_CHECK(num.ToString() == "-1");
    }
    {
        n = 5;
        CBigNum num(n);
        BOOST_CHECK(num.ToString() == "5");
        num.setulong(0);
        BOOST_CHECK(num.ToString() == "0");
        mysetint64(num, n);
        BOOST_CHECK(num.ToString() == "5");
    }
    {
        n = -5;
        CBigNum num(n);
        BOOST_CHECK(num.ToString() == "-5");
        num.setulong(0);
        BOOST_CHECK(num.ToString() == "0");
        mysetint64(num, n);
        BOOST_CHECK(num.ToString() == "-5");
    }
    {
        n = std::numeric_limits<int64>::min();
        CBigNum num(n);
        BOOST_CHECK(num.ToString() == "-9223372036854775808");
        num.setulong(0);
        BOOST_CHECK(num.ToString() == "0");
        mysetint64(num, n);
        BOOST_CHECK(num.ToString() == "-9223372036854775808");
    }
    {
        n = std::numeric_limits<int64>::max();
        CBigNum num(n);
        BOOST_CHECK(num.ToString() == "9223372036854775807");
        num.setulong(0);
        BOOST_CHECK(num.ToString() == "0");
        mysetint64(num, n);
        BOOST_CHECK(num.ToString() == "9223372036854775807");
    }
}

// M1.2: byte-exact CBigNum::getvch / setvch.
//
// CBigNum::getvch() produces signed-magnitude bytes in little-endian
// (high bit of the most-significant byte = sign). Internally this goes
// through BN_bn2mpi / BN_mpi2bn from OpenSSL.
//
// If OpenSSL's MPI encoding ever changes — or if a future libcrypto
// substitution silently shifts a byte — these assertions break loud
// and immediately. With both OpenSSL 1.1 and OpenSSL 3 build jobs in
// CI, this test catches drift across versions, which would otherwise
// silently fork the chain (every script signature hash depends on
// these bytes).
//
// Do NOT update the expected byte arrays without also bumping a
// consensus rule and coordinating chain activation.

namespace {
std::vector<unsigned char> mkbytes(std::initializer_list<unsigned char> il) {
    return std::vector<unsigned char>(il);
}

void check_getvch_known(int64 input, std::vector<unsigned char> expected) {
    CBigNum b;
    b.setint64(input);
    BOOST_CHECK_MESSAGE(b.getvch() == expected,
        "getvch(" << input << ") differs from expected");
    // Round-trip: setvch(getvch(x)) == x.
    CBigNum b2;
    b2.setvch(b.getvch());
    BOOST_CHECK_MESSAGE(b == b2,
        "setvch(getvch(" << input << ")) did not round-trip");
}
} // namespace

BOOST_AUTO_TEST_CASE(bignum_getvch_byte_equivalence)
{
    check_getvch_known(           0LL, mkbytes({}));                       // empty
    check_getvch_known(           1LL, mkbytes({0x01}));
    check_getvch_known(          -1LL, mkbytes({0x81}));                   // sign bit on MSB
    check_getvch_known(        0x42LL, mkbytes({0x42}));
    check_getvch_known(        0xFFLL, mkbytes({0xFF, 0x00}));             // extra byte for sign
    check_getvch_known(       -0xFFLL, mkbytes({0xFF, 0x80}));
    check_getvch_known(       0x100LL, mkbytes({0x00, 0x01}));
    check_getvch_known(      0x7FFFLL, mkbytes({0xFF, 0x7F}));
    check_getvch_known(      0x8000LL, mkbytes({0x00, 0x80, 0x00}));
    check_getvch_known(    0xFFFFFFLL, mkbytes({0xFF, 0xFF, 0xFF, 0x00}));
    check_getvch_known(   -0xFFFFFFLL, mkbytes({0xFF, 0xFF, 0xFF, 0x80}));
    check_getvch_known(0x123456789ABCLL,
                       mkbytes({0xBC, 0x9A, 0x78, 0x56, 0x34, 0x12}));
}

BOOST_AUTO_TEST_CASE(bignum_setvch_roundtrip_random_lengths)
{
    // Round-trip every byte length up to 16 with an alternating pattern.
    // Exercises the BN_mpi2bn / BN_bn2mpi path on more shapes than the
    // hand-derived vectors above.
    for (size_t len = 1; len <= 16; ++len)
    {
        std::vector<unsigned char> v(len);
        for (size_t i = 0; i < len; ++i) v[i] = static_cast<unsigned char>(0x55 + i);
        v[len - 1] &= 0x7F;  // force positive (clear sign bit)
        CBigNum a;
        a.setvch(v);
        std::vector<unsigned char> w = a.getvch();
        BOOST_CHECK_MESSAGE(v == w,
            "setvch->getvch round-trip failed at length " << len);
    }
}

// Hash160 known-vector. SHA256 then RIPEMD160 — both implementations
// must produce these exact bytes for the chain to be consensus-stable.
// Test vectors: Hash160("") and Hash160("abc") are well-documented
// standard values; every empty-script P2SH would hash to the first.
//
// uint160::GetHex() emits little-endian (Bitcoin txid display order),
// so the expected strings here are the standard big-endian hash
// values BYTE-REVERSED. The underlying hash bytes are the standard
// values; only the display convention differs.
BOOST_AUTO_TEST_CASE(hash160_known_vectors)
{
    // Hash160("") == RIPEMD160(SHA256("")) == b472a266d0bd89c13706a4132ccfb16f7c3b9fcb (big-endian)
    std::vector<unsigned char> empty;
    BOOST_CHECK_EQUAL(Hash160(empty).GetHex(),
                      "cb9f3b7c6fb1cf2c13a40637c189bdd066a272b4");

    // Hash160("abc") == RIPEMD160(SHA256("abc")) == bb1be98c142444d7a56aa3981c3942a978e4dc33 (big-endian)
    std::vector<unsigned char> abc{'a', 'b', 'c'};
    BOOST_CHECK_EQUAL(Hash160(abc).GetHex(),
                      "33dce478a942391c98a36aa5d74424148ce91bbb");
}

BOOST_AUTO_TEST_SUITE_END()
