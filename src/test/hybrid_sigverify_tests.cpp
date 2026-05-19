// M1.3: hybrid ECDSA + ML-DSA-65 signature coverage.
//
// The "quantum-safe P2PKH" path documented in commit 36ae25e produces
// signatures of the layout:
//
//   [1B ECDSA sig length] [ECDSA sig] [ML-DSA pubkey 1952B] [ML-DSA sig 3309B]
//
// VerifyHybrid extracts the ML-DSA pubkey from the sig itself and
// requires BOTH halves to verify. These tests pin the format size and
// exercise the cross-component tampering matrix (valid+valid passes,
// any mutation fails).
//
// Note: this test suite documents the CURRENT behavior of the
// implementation. The plan's Phase 2 (M2.1-M2.6) replaces this format
// with on-chain-committed PQ addresses. When that lands, this suite
// is retired and a new format-suite takes its place.
//
// Only compiled when liboqs is available.

#ifdef ENABLE_MLDSA

#include <boost/test/unit_test.hpp>

#include "key.h"
#include "uint256.h"
#include "util.h"
#include "crypto/mldsa.h"

BOOST_AUTO_TEST_SUITE(hybrid_sigverify_tests)

namespace {

uint256 hash_of(const std::string& s) {
    return Hash(s.begin(), s.end());
}

CKey new_hybrid() {
    CKey k;
    k.MakeNewHybridKey();
    BOOST_REQUIRE(k.IsHybrid());
    return k;
}

} // namespace

// Round-trip: a hybrid key signs and the same key verifies.
BOOST_AUTO_TEST_CASE(roundtrip_signs_and_verifies)
{
    CKey k = new_hybrid();
    uint256 h = hash_of("hybrid round-trip");

    std::vector<unsigned char> sig;
    BOOST_REQUIRE(k.SignHybrid(h, sig));
    BOOST_CHECK(k.VerifyHybrid(h, sig));
}

// The hybrid sig layout is:
//   1B(ecdsa_len) + ecdsa_len B(ecdsa_sig) + 1952B(mldsa_pub) + 3309B(mldsa_sig)
// ECDSA DER sigs are typically 70-72 bytes, so total ~ 1 + 71 + 1952 + 3309
// = 5333 ± a couple bytes. This test asserts the layout fields parse
// out at the documented offsets.
BOOST_AUTO_TEST_CASE(layout_matches_documented_format)
{
    CKey k = new_hybrid();
    uint256 h = hash_of("layout check");

    std::vector<unsigned char> sig;
    BOOST_REQUIRE(k.SignHybrid(h, sig));

    BOOST_REQUIRE_GT(sig.size(), 1u);
    size_t ecdsa_len = sig[0];

    // ECDSA DER sigs fit in one byte length (max ~72), well below 255.
    BOOST_CHECK_LT(ecdsa_len, 100u);

    // Total size = 1 + ecdsa_len + MLDSA pubkey + MLDSA sig.
    size_t expected = 1 + ecdsa_len + MLDSA::PUBLIC_KEY_BYTES + MLDSA::SIGNATURE_BYTES;
    BOOST_CHECK_EQUAL(sig.size(), expected);

    // The embedded ML-DSA pubkey at offset (1 + ecdsa_len) must match
    // what the key reports independently.
    std::vector<unsigned char> embedded_pub(
        sig.begin() + 1 + ecdsa_len,
        sig.begin() + 1 + ecdsa_len + MLDSA::PUBLIC_KEY_BYTES);
    BOOST_CHECK(embedded_pub == k.GetMLDSAPubKey());
}

// Tamper a byte inside each section in turn and confirm verification
// fails for every mutation.
BOOST_AUTO_TEST_CASE(rejects_tampering_in_any_section)
{
    CKey k = new_hybrid();
    uint256 h = hash_of("tampering matrix");

    std::vector<unsigned char> sig;
    BOOST_REQUIRE(k.SignHybrid(h, sig));
    BOOST_REQUIRE(k.VerifyHybrid(h, sig));

    size_t ecdsa_len = sig[0];
    size_t mldsa_pub_start = 1 + ecdsa_len;
    size_t mldsa_sig_start = mldsa_pub_start + MLDSA::PUBLIC_KEY_BYTES;

    // One probe inside each region.
    size_t positions[] = {
        2u,                              // inside ECDSA sig (skip the length byte)
        mldsa_pub_start + 100u,          // inside ML-DSA pubkey
        mldsa_sig_start + 100u,          // inside ML-DSA sig
        sig.size() - 1u,                 // trailing byte
    };
    for (size_t pos : positions) {
        std::vector<unsigned char> bad = sig;
        bad[pos] ^= 0x01;
        BOOST_CHECK_MESSAGE(!k.VerifyHybrid(h, bad),
            "tampered hybrid sig at byte " << pos << " unexpectedly verified");
    }
}

// Truncation must fail (drops a byte from the ML-DSA sig tail).
BOOST_AUTO_TEST_CASE(rejects_truncated_signature)
{
    CKey k = new_hybrid();
    uint256 h = hash_of("truncate");

    std::vector<unsigned char> sig;
    BOOST_REQUIRE(k.SignHybrid(h, sig));

    sig.pop_back();
    BOOST_CHECK(!k.VerifyHybrid(h, sig));
}

// Verifying against a different message must fail.
BOOST_AUTO_TEST_CASE(rejects_wrong_hash)
{
    CKey k = new_hybrid();
    uint256 h1 = hash_of("first");
    uint256 h2 = hash_of("second");

    std::vector<unsigned char> sig;
    BOOST_REQUIRE(k.SignHybrid(h1, sig));

    BOOST_CHECK( k.VerifyHybrid(h1, sig));
    BOOST_CHECK(!k.VerifyHybrid(h2, sig));
}

// Empty signature must fail cleanly.
BOOST_AUTO_TEST_CASE(rejects_empty_signature)
{
    CKey k = new_hybrid();
    uint256 h = hash_of("anything");
    std::vector<unsigned char> empty;
    BOOST_CHECK(!k.VerifyHybrid(h, empty));
}

// Swapping the embedded ML-DSA pubkey for a different valid ML-DSA
// pubkey must fail — the ECDSA half is bound to the ORIGINAL key and
// the new ML-DSA pubkey isn't the signer.
BOOST_AUTO_TEST_CASE(rejects_swapped_mldsa_pubkey)
{
    CKey alice = new_hybrid();
    CKey bob   = new_hybrid();
    uint256 h = hash_of("alice signs");

    std::vector<unsigned char> sig;
    BOOST_REQUIRE(alice.SignHybrid(h, sig));
    BOOST_REQUIRE(alice.VerifyHybrid(h, sig));

    // Splice bob's ML-DSA pubkey into alice's sig.
    size_t ecdsa_len = sig[0];
    size_t pub_start = 1 + ecdsa_len;
    auto bob_pub = bob.GetMLDSAPubKey();
    BOOST_REQUIRE_EQUAL(bob_pub.size(), (size_t)MLDSA::PUBLIC_KEY_BYTES);
    std::copy(bob_pub.begin(), bob_pub.end(), sig.begin() + pub_start);

    BOOST_CHECK(!alice.VerifyHybrid(h, sig));
}

BOOST_AUTO_TEST_SUITE_END()

#endif // ENABLE_MLDSA
