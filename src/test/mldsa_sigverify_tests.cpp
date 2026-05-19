// M1.3: ML-DSA-65 (NIST FIPS 204) sign/verify regression coverage.
//
// Locks down the post-quantum signature path that the project's
// headline differentiator depends on. Tests pin the FIPS 204 key/sig
// sizes via static_assert (so a future liboqs upgrade that picks a
// different parameter set fails loud at compile time) and exercise
// the round-trip plus the rejection paths (bit-flip, wrong pubkey,
// truncated, empty).
//
// Only compiled when liboqs is available (LIBOQS_AVAILABLE=yes in
// makefile.unix → -DENABLE_MLDSA). The Makefile filters this whole
// file out otherwise.

#ifdef ENABLE_MLDSA

#include <boost/test/unit_test.hpp>

#include "key.h"
#include "uint256.h"
#include "util.h"
#include "crypto/mldsa.h"

// FIPS 204 parameter set 65: 1952-byte pubkey, 3309-byte signature.
// If liboqs ever ships a build with different numbers in these slots
// the chain consensus byte layout changes silently — catch it here.
static_assert(MLDSA::PUBLIC_KEY_BYTES == 1952, "ML-DSA-65 pubkey must be 1952 bytes (FIPS 204)");
static_assert(MLDSA::SIGNATURE_BYTES == 3309, "ML-DSA-65 sig must be 3309 bytes (FIPS 204)");

BOOST_AUTO_TEST_SUITE(mldsa_sigverify_tests)

namespace {

uint256 hash_of(const std::string& s) {
    return Hash(s.begin(), s.end());
}

// Generate a fresh hybrid keypair (ECDSA + ML-DSA). We only need the
// ML-DSA half here but MakeNewHybridKey is the canonical entry point.
CKey new_hybrid() {
    CKey k;
    k.MakeNewHybridKey();
    BOOST_REQUIRE(k.HasMLDSAKey());
    return k;
}

} // namespace

// Round-trip: a freshly-generated ML-DSA key signs a hash and the
// matching pubkey verifies it.
BOOST_AUTO_TEST_CASE(roundtrip_signs_and_verifies)
{
    CKey k = new_hybrid();
    uint256 h = hash_of("hello aumcoin");

    std::vector<unsigned char> sig;
    BOOST_REQUIRE(k.SignMLDSA(h, sig));
    BOOST_CHECK_EQUAL(sig.size(), (size_t)MLDSA::SIGNATURE_BYTES);

    BOOST_CHECK(CKey::VerifyMLDSA(h, sig, k.GetMLDSAPubKey()));
}

// Verifying against a DIFFERENT ML-DSA pubkey must fail.
BOOST_AUTO_TEST_CASE(rejects_wrong_pubkey)
{
    CKey alice = new_hybrid();
    CKey bob   = new_hybrid();
    uint256 h = hash_of("transfer to bob");

    std::vector<unsigned char> sig;
    BOOST_REQUIRE(alice.SignMLDSA(h, sig));

    // Alice's sig must verify with alice's pubkey, NOT with bob's.
    BOOST_CHECK( CKey::VerifyMLDSA(h, sig, alice.GetMLDSAPubKey()));
    BOOST_CHECK(!CKey::VerifyMLDSA(h, sig, bob.GetMLDSAPubKey()));
}

// Verifying a tampered signature must fail. Probe a handful of byte
// positions; full per-byte fuzz would explode the test runtime
// (verification is ~50 µs and the sig is 3309 bytes).
BOOST_AUTO_TEST_CASE(rejects_tampered_signature)
{
    CKey k = new_hybrid();
    uint256 h = hash_of("ok then");

    std::vector<unsigned char> sig;
    BOOST_REQUIRE(k.SignMLDSA(h, sig));

    auto pub = k.GetMLDSAPubKey();
    BOOST_REQUIRE(CKey::VerifyMLDSA(h, sig, pub));

    // Probe positions across the sig: start, middle, end.
    size_t positions[] = {0u, sig.size() / 2u, sig.size() - 1u};
    for (size_t pos : positions) {
        std::vector<unsigned char> bad = sig;
        bad[pos] ^= 0x01;
        BOOST_CHECK_MESSAGE(!CKey::VerifyMLDSA(h, bad, pub),
            "tampered sig at byte " << pos << " unexpectedly verified");
    }
}

// Verifying a tampered HASH (i.e. checking the sig against a
// different message) must fail.
BOOST_AUTO_TEST_CASE(rejects_wrong_hash)
{
    CKey k = new_hybrid();
    uint256 h1 = hash_of("message one");
    uint256 h2 = hash_of("message two");

    std::vector<unsigned char> sig;
    BOOST_REQUIRE(k.SignMLDSA(h1, sig));

    BOOST_CHECK( CKey::VerifyMLDSA(h1, sig, k.GetMLDSAPubKey()));
    BOOST_CHECK(!CKey::VerifyMLDSA(h2, sig, k.GetMLDSAPubKey()));
}

// Empty signature must be cleanly rejected, not crash or accept.
BOOST_AUTO_TEST_CASE(rejects_empty_signature)
{
    CKey k = new_hybrid();
    uint256 h = hash_of("anything");
    std::vector<unsigned char> empty;
    BOOST_CHECK(!CKey::VerifyMLDSA(h, empty, k.GetMLDSAPubKey()));
}

// Truncated signature must be rejected.
BOOST_AUTO_TEST_CASE(rejects_truncated_signature)
{
    CKey k = new_hybrid();
    uint256 h = hash_of("payload");

    std::vector<unsigned char> sig;
    BOOST_REQUIRE(k.SignMLDSA(h, sig));

    // Drop the last byte.
    std::vector<unsigned char> truncated(sig.begin(), sig.end() - 1);
    BOOST_CHECK(!CKey::VerifyMLDSA(h, truncated, k.GetMLDSAPubKey()));
}

// Oversized (extra byte appended) signature must be rejected.
BOOST_AUTO_TEST_CASE(rejects_oversized_signature)
{
    CKey k = new_hybrid();
    uint256 h = hash_of("payload");

    std::vector<unsigned char> sig;
    BOOST_REQUIRE(k.SignMLDSA(h, sig));

    sig.push_back(0x00);
    BOOST_CHECK(!CKey::VerifyMLDSA(h, sig, k.GetMLDSAPubKey()));
}

// Wrong-size pubkey must be rejected.
BOOST_AUTO_TEST_CASE(rejects_wrong_size_pubkey)
{
    CKey k = new_hybrid();
    uint256 h = hash_of("payload");

    std::vector<unsigned char> sig;
    BOOST_REQUIRE(k.SignMLDSA(h, sig));

    std::vector<unsigned char> short_pub(k.GetMLDSAPubKey().begin(),
                                         k.GetMLDSAPubKey().end() - 1);
    BOOST_CHECK(!CKey::VerifyMLDSA(h, sig, short_pub));
}

// Determinism + uniqueness check: two fresh keys must produce
// different pubkeys (sanity that key generation isn't degenerate).
BOOST_AUTO_TEST_CASE(fresh_keys_are_distinct)
{
    CKey a = new_hybrid();
    CKey b = new_hybrid();
    BOOST_CHECK(a.GetMLDSAPubKey() != b.GetMLDSAPubKey());
}

BOOST_AUTO_TEST_SUITE_END()

#endif // ENABLE_MLDSA
