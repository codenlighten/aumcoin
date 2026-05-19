// M2.1: CHybridPubKey tests.
//
// Pins the wire format (1986 bytes = 1 + 33 + 1952), the hash
// commitment recipe (Hash160(algo_id || ecdsa || mldsa)), and the
// Parse() rejection paths for wrong sizes / unknown algo_ids.

#ifdef ENABLE_MLDSA

#include <boost/test/unit_test.hpp>

#include "hybridpubkey.h"
#include "key.h"
#include "uint256.h"
#include "util.h"

BOOST_AUTO_TEST_SUITE(hybridpubkey_tests)

namespace {

// Build deterministic-but-fake ECDSA and ML-DSA pubkeys for layout tests.
// These are NOT valid keys cryptographically (no point validation here);
// CHybridPubKey doesn't care because layout is the only thing it owns.
CHybridPubKey make_fake(uint8_t seed) {
    std::vector<unsigned char> ecdsa(33);
    for (size_t i = 0; i < 33; ++i)
        ecdsa[i] = static_cast<unsigned char>(seed + i);
    std::vector<unsigned char> mldsa(MLDSA::PUBLIC_KEY_BYTES);
    for (size_t i = 0; i < mldsa.size(); ++i)
        mldsa[i] = static_cast<unsigned char>(seed + i + 7);
    return CHybridPubKey(ecdsa, mldsa);
}

CHybridPubKey make_real() {
    CKey k;
    k.MakeNewHybridKey();
    BOOST_REQUIRE(k.IsHybrid());
    return CHybridPubKey(k.GetPubKey().Raw(), k.GetMLDSAPubKey());
}

} // namespace

BOOST_AUTO_TEST_CASE(default_constructed_is_invalid)
{
    CHybridPubKey empty;
    BOOST_CHECK(!empty.IsValid());
}

BOOST_AUTO_TEST_CASE(serialized_length_is_1986)
{
    CHybridPubKey k = make_fake(0x42);
    auto s = k.Serialize();
    BOOST_CHECK_EQUAL(s.size(),
                      (size_t)(1 + 33 + MLDSA::PUBLIC_KEY_BYTES));
    BOOST_CHECK_EQUAL(s.size(), 1986u);
}

BOOST_AUTO_TEST_CASE(serialized_layout_algo_id_first_then_ecdsa_then_mldsa)
{
    CHybridPubKey k = make_fake(0x10);
    auto s = k.Serialize();

    BOOST_CHECK_EQUAL(s[0], HYBRID_ALGO_ID_ECDSA_MLDSA65);
    // ECDSA section
    for (size_t i = 0; i < 33; ++i)
        BOOST_CHECK_EQUAL(s[1 + i], k.GetECDSAPubKey()[i]);
    // ML-DSA section
    for (size_t i = 0; i < MLDSA::PUBLIC_KEY_BYTES; ++i)
        BOOST_CHECK_EQUAL(s[1 + 33 + i], k.GetMLDSAPubKey()[i]);
}

BOOST_AUTO_TEST_CASE(parse_roundtrip)
{
    CHybridPubKey orig = make_fake(0xA0);
    CHybridPubKey parsed;
    BOOST_CHECK(CHybridPubKey::Parse(orig.Serialize(), parsed));
    BOOST_CHECK(parsed == orig);
    BOOST_CHECK(parsed.IsValid());
}

BOOST_AUTO_TEST_CASE(parse_rejects_wrong_length)
{
    CHybridPubKey orig = make_fake(0x55);
    auto s = orig.Serialize();

    // Too short by one byte
    CHybridPubKey p1;
    s.pop_back();
    BOOST_CHECK(!CHybridPubKey::Parse(s, p1));

    // Restore and add a byte (too long)
    s = orig.Serialize();
    s.push_back(0);
    CHybridPubKey p2;
    BOOST_CHECK(!CHybridPubKey::Parse(s, p2));

    // Empty
    CHybridPubKey p3;
    BOOST_CHECK(!CHybridPubKey::Parse(std::vector<unsigned char>{}, p3));
}

BOOST_AUTO_TEST_CASE(parse_rejects_unknown_algo_id)
{
    CHybridPubKey orig = make_fake(0x77);
    auto s = orig.Serialize();
    s[0] = 0x02;  // not HYBRID_ALGO_ID_ECDSA_MLDSA65

    CHybridPubKey p;
    BOOST_CHECK(!CHybridPubKey::Parse(s, p));
}

// GetID == Hash160 of the serialized form. Important: this is a
// DIFFERENT hash than CPubKey::GetID() on the same ECDSA half, which
// is precisely the consensus property M2 is delivering — hybrid
// addresses live in a different address space than legacy P2PKH.
BOOST_AUTO_TEST_CASE(getid_is_hash160_of_serialized_blob)
{
    CHybridPubKey k = make_fake(0xDE);
    CKeyID id = k.GetID();
    BOOST_CHECK(id == CKeyID(Hash160(k.Serialize())));
}

BOOST_AUTO_TEST_CASE(getid_differs_from_legacy_cpubkey_getid)
{
    // Build a real ECDSA+MLDSA pair via CKey, then compare GetIDs.
    CKey k;
    k.MakeNewHybridKey();
    BOOST_REQUIRE(k.IsHybrid());

    CPubKey legacy_pub = k.GetPubKey();
    CHybridPubKey hybrid_pub(legacy_pub.Raw(), k.GetMLDSAPubKey());

    BOOST_CHECK(legacy_pub.GetID() != hybrid_pub.GetID());
}

// Fresh hybrid keys produce different addresses (sanity that we're
// not collapsing to a constant by accident).
BOOST_AUTO_TEST_CASE(fresh_hybrid_pubkeys_have_distinct_addresses)
{
    CHybridPubKey a = make_real();
    CHybridPubKey b = make_real();
    BOOST_CHECK(a.GetID() != b.GetID());
}

BOOST_AUTO_TEST_CASE(invalid_sizes_reported)
{
    // Wrong ECDSA size
    std::vector<unsigned char> short_ecdsa(32, 0x01);
    std::vector<unsigned char> mldsa_ok(MLDSA::PUBLIC_KEY_BYTES, 0x02);
    BOOST_CHECK(!CHybridPubKey(short_ecdsa, mldsa_ok).IsValid());

    // Wrong ML-DSA size
    std::vector<unsigned char> ecdsa_ok(33, 0x03);
    std::vector<unsigned char> short_mldsa(MLDSA::PUBLIC_KEY_BYTES - 1, 0x04);
    BOOST_CHECK(!CHybridPubKey(ecdsa_ok, short_mldsa).IsValid());

    // Unknown algo_id
    BOOST_CHECK(!CHybridPubKey(ecdsa_ok, mldsa_ok, 0x99).IsValid());
}

BOOST_AUTO_TEST_SUITE_END()

#endif  // ENABLE_MLDSA
