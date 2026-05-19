// M2.1 Slice 3: hybrid-address codec tests.
//
// Pins the encoded HRP ("aumq" mainnet, "taumq" testnet), round-trip
// behavior for arbitrary 20-byte CKeyIDs, and rejection of common
// failure modes (bad checksum, wrong HRP, mixed case, bech32-not-
// bech32m, wrong-size payload).

#include <boost/test/unit_test.hpp>

#include "bech32.h"
#include "hybridaddress.h"
#include "key.h"
#include "uint256.h"
#include "util.h"

#ifdef ENABLE_MLDSA
#include "hybridpubkey.h"
#endif

#include <cstring>
#include <vector>

BOOST_AUTO_TEST_SUITE(hybridaddress_tests)

namespace {

// Save/restore fTestNet across tests so toggling for testnet-HRP
// coverage doesn't leak into other suites.
struct TestnetGuard {
    bool prev;
    TestnetGuard() : prev(fTestNet) {}
    ~TestnetGuard() { fTestNet = prev; }
};

CKeyID make_keyid(uint8_t seed) {
    CKeyID id;
    for (size_t i = 0; i < 20; ++i)
        reinterpret_cast<unsigned char*>(&id)[i] =
            static_cast<unsigned char>(seed + i);
    return id;
}

}  // namespace

BOOST_AUTO_TEST_CASE(roundtrip_arbitrary_keyids)
{
    TestnetGuard g;
    fTestNet = false;
    for (uint8_t seed : {0u, 1u, 0x42u, 0xABu, 0xFFu}) {
        CKeyID id = make_keyid(seed);
        std::string s = HybridAddress::Encode(id);
        BOOST_REQUIRE(!s.empty());
        BOOST_CHECK(s.substr(0, 5) == "aumq1");
        CKeyID back;
        BOOST_REQUIRE_MESSAGE(HybridAddress::Decode(s, back),
            "decode failed for seed=" << static_cast<int>(seed));
        BOOST_CHECK_MESSAGE(back == id,
            "round-trip mismatch for seed=" << static_cast<int>(seed));
    }
}

BOOST_AUTO_TEST_CASE(testnet_uses_taumq_hrp)
{
    TestnetGuard g;
    fTestNet = true;
    CKeyID id = make_keyid(0x77);
    std::string s = HybridAddress::Encode(id);
    BOOST_REQUIRE(!s.empty());
    BOOST_CHECK_EQUAL(s.substr(0, 6), "taumq1");

    CKeyID back;
    BOOST_REQUIRE(HybridAddress::Decode(s, back));
    BOOST_CHECK(back == id);
}

BOOST_AUTO_TEST_CASE(mainnet_rejects_testnet_address)
{
    TestnetGuard g;
    // Encode on testnet.
    fTestNet = true;
    CKeyID id = make_keyid(0x10);
    std::string testnet_str = HybridAddress::Encode(id);
    BOOST_REQUIRE(!testnet_str.empty());

    // Try to decode on mainnet → wrong HRP → reject.
    fTestNet = false;
    CKeyID back;
    BOOST_CHECK(!HybridAddress::Decode(testnet_str, back));
}

BOOST_AUTO_TEST_CASE(rejects_mutated_checksum)
{
    TestnetGuard g;
    fTestNet = false;
    CKeyID id = make_keyid(0x33);
    std::string s = HybridAddress::Encode(id);
    BOOST_REQUIRE(!s.empty());
    // Flip the last character (corrupts checksum).
    s.back() = (s.back() == 'a') ? 'p' : 'a';
    CKeyID back;
    BOOST_CHECK(!HybridAddress::Decode(s, back));
}

BOOST_AUTO_TEST_CASE(rejects_mixed_case)
{
    TestnetGuard g;
    fTestNet = false;
    CKeyID id = make_keyid(0x55);
    std::string s = HybridAddress::Encode(id);
    BOOST_REQUIRE(!s.empty());
    // Uppercase the first HRP char — now mixed case, banned by BIP 173.
    s[0] = 'A';
    CKeyID back;
    BOOST_CHECK(!HybridAddress::Decode(s, back));
}

// A bech32 (not bech32m) string with the right HRP and a valid 5-bit
// payload must still be rejected — the codec layer differentiates by
// checksum constant, and we only accept bech32m for the new format.
BOOST_AUTO_TEST_CASE(rejects_bech32_encoded_payload)
{
    TestnetGuard g;
    fTestNet = false;
    CKeyID id = make_keyid(0xAA);
    std::vector<uint8_t> payload(20);
    std::memcpy(payload.data(), &id, 20);
    std::vector<uint8_t> five;
    BOOST_REQUIRE(bech32::ConvertBits(five, 5, payload, 8, true));
    // Encode as BECH32 (the wrong family) with our HRP.
    std::string bech32_str = bech32::Encode(bech32::Encoding::BECH32, "aumq", five);
    BOOST_REQUIRE(!bech32_str.empty());

    CKeyID back;
    BOOST_CHECK(!HybridAddress::Decode(bech32_str, back));
}

// A bech32m string with the right HRP but the wrong payload size
// (e.g. encoding a 19-byte or 21-byte CKeyID by mistake) must be
// rejected. Important because the consensus rule will assume the
// decoded byte length is exactly 20.
BOOST_AUTO_TEST_CASE(rejects_wrong_size_payload)
{
    TestnetGuard g;
    fTestNet = false;

    // 19-byte payload (one short).
    std::vector<uint8_t> short_raw(19, 0x42);
    std::vector<uint8_t> short_five;
    BOOST_REQUIRE(bech32::ConvertBits(short_five, 5, short_raw, 8, true));
    std::string short_str = bech32::Encode(bech32::Encoding::BECH32M, "aumq", short_five);
    BOOST_REQUIRE(!short_str.empty());
    CKeyID back;
    BOOST_CHECK(!HybridAddress::Decode(short_str, back));

    // 21-byte payload (one long).
    std::vector<uint8_t> long_raw(21, 0x77);
    std::vector<uint8_t> long_five;
    BOOST_REQUIRE(bech32::ConvertBits(long_five, 5, long_raw, 8, true));
    std::string long_str = bech32::Encode(bech32::Encoding::BECH32M, "aumq", long_five);
    BOOST_REQUIRE(!long_str.empty());
    BOOST_CHECK(!HybridAddress::Decode(long_str, back));
}

// Empty string and garbage must reject cleanly.
BOOST_AUTO_TEST_CASE(rejects_obvious_garbage)
{
    TestnetGuard g;
    fTestNet = false;
    CKeyID back;
    BOOST_CHECK(!HybridAddress::Decode("", back));
    BOOST_CHECK(!HybridAddress::Decode("not-an-address", back));
    BOOST_CHECK(!HybridAddress::Decode("aumq1", back));      // no data
    BOOST_CHECK(!HybridAddress::Decode("1aumqaaaa", back));  // empty HRP
}

#ifdef ENABLE_MLDSA
// End-to-end: take a real CHybridPubKey, encode its GetID() to a
// string, decode it back, and confirm the bytes match. This is the
// path wallets will use.
BOOST_AUTO_TEST_CASE(end_to_end_from_chybridpubkey)
{
    TestnetGuard g;
    fTestNet = false;

    CKey k;
    k.MakeNewHybridKey();
    BOOST_REQUIRE(k.IsHybrid());
    CHybridPubKey hpk(k.GetPubKey().Raw(), k.GetMLDSAPubKey());
    BOOST_REQUIRE(hpk.IsValid());

    CKeyID id = hpk.GetID();
    std::string addr = HybridAddress::Encode(id);
    BOOST_REQUIRE(!addr.empty());
    BOOST_CHECK(addr.substr(0, 5) == "aumq1");

    CKeyID decoded;
    BOOST_REQUIRE(HybridAddress::Decode(addr, decoded));
    BOOST_CHECK(decoded == id);
}
#endif

BOOST_AUTO_TEST_SUITE_END()
