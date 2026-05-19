// M2.1: bech32 / bech32m codec tests.
//
// Includes the official BIP 173 / BIP 350 test vectors so any change to
// our codec that drifts from the reference is caught immediately.

#include <boost/test/unit_test.hpp>

#include "bech32.h"

#include <string>
#include <vector>

BOOST_AUTO_TEST_SUITE(bech32_tests)

namespace {

// Valid BIP 350 (bech32m) strings from §6.
const std::vector<std::string> kValidBech32M = {
    "A1LQFN3A",
    "a1lqfn3a",
    "an83characterlonghumanreadablepartthatcontainsthetheexcludedcharactersbioandnumber11sg7hg6",
    "abcdef1l7aum6echk45nj3s0wdvt2fg8x9yrzpqzd3ryx",
    "11llllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllludsr8",
    "split1checkupstagehandshakeupstreamerranterredcaperredlc445v",
    "?1v759aa",
};

// Valid BIP 173 (bech32) strings from §6.
const std::vector<std::string> kValidBech32 = {
    "A12UEL5L",
    "a12uel5l",
    "an83characterlonghumanreadablepartthatcontainsthenumber1andtheexcludedcharactersbio1tt5tgs",
    "abcdef1qpzry9x8gf2tvdw0s3jn54khce6mua7lmqqqxw",
    "11qqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqc8247j",
    "split1checkupstagehandshakeupstreamerranterredcaperred2y9e3w",
    "?1ezyfcl",
};

// Invalid strings (mix from both BIPs).
const std::vector<std::string> kInvalid = {
    " 1nwldj5",   // HRP contains space
    "abc1\x7F""2y9e3w",   // HRP contains DEL
    "an84characterslonghumanreadablepartthatcontainsthetheexcludedcharactersbioandnumber11d6pts4",  // overlong (>90)
    "qyrz8wqd2c9m",                      // no separator
    "1qyrz8wqd2c9m",                     // empty HRP
    "y1b0jsk6g",                         // invalid data char ('b')
    "lt1igcx5c0",                        // invalid data char ('i')
    "in1muywd",                          // checksum too short (<6 chars)
    "mm1crxm3i",                         // invalid data char ('i')
    "au1s5cgs",                          // invalid data char (extra 'i')
};

} // namespace

BOOST_AUTO_TEST_CASE(roundtrip_bech32m_simple)
{
    std::vector<uint8_t> data{0, 1, 2, 3, 4, 5, 6, 7};
    std::string s = bech32::Encode(bech32::Encoding::BECH32M, "aumq", data);
    BOOST_REQUIRE(!s.empty());
    BOOST_CHECK(s.substr(0, 5) == "aumq1");

    bech32::DecodeResult r = bech32::Decode(s);
    BOOST_CHECK(r.encoding == bech32::Encoding::BECH32M);
    BOOST_CHECK_EQUAL(r.hrp, "aumq");
    BOOST_CHECK(r.data == data);
}

BOOST_AUTO_TEST_CASE(roundtrip_bech32_simple)
{
    std::vector<uint8_t> data{15, 7, 11, 3};
    std::string s = bech32::Encode(bech32::Encoding::BECH32, "tb", data);
    BOOST_REQUIRE(!s.empty());

    bech32::DecodeResult r = bech32::Decode(s);
    BOOST_CHECK(r.encoding == bech32::Encoding::BECH32);
    BOOST_CHECK_EQUAL(r.hrp, "tb");
    BOOST_CHECK(r.data == data);
}

BOOST_AUTO_TEST_CASE(bip350_official_valid_strings_decode)
{
    for (const std::string& s : kValidBech32M) {
        bech32::DecodeResult r = bech32::Decode(s);
        BOOST_CHECK_MESSAGE(r.encoding == bech32::Encoding::BECH32M,
            "BIP 350 valid string did not decode as BECH32M: " << s);
    }
}

BOOST_AUTO_TEST_CASE(bip173_official_valid_strings_decode)
{
    for (const std::string& s : kValidBech32) {
        bech32::DecodeResult r = bech32::Decode(s);
        BOOST_CHECK_MESSAGE(r.encoding == bech32::Encoding::BECH32,
            "BIP 173 valid string did not decode as BECH32: " << s);
    }
}

BOOST_AUTO_TEST_CASE(invalid_strings_reject)
{
    for (const std::string& s : kInvalid) {
        bech32::DecodeResult r = bech32::Decode(s);
        BOOST_CHECK_MESSAGE(r.encoding == bech32::Encoding::INVALID,
            "Invalid string unexpectedly decoded: " << s);
    }
}

BOOST_AUTO_TEST_CASE(mixed_case_rejected)
{
    // Take a known-valid lowercase string and uppercase a single
    // character — BIP 173 §1 mandates this be rejected.
    std::string s = "abcdef1l7aum6echk45nj3s0wdvt2fg8x9yrzpqzd3ryx";  // valid bech32m
    BOOST_REQUIRE(bech32::Decode(s).encoding == bech32::Encoding::BECH32M);
    s[0] = 'A';  // now mixed
    BOOST_CHECK(bech32::Decode(s).encoding == bech32::Encoding::INVALID);
}

BOOST_AUTO_TEST_CASE(checksum_mutation_rejected)
{
    // Flip the last character (corrupts checksum). The encoded char
    // 'a' maps to value 29; flipping the last lower-letter changes
    // the checksum and decode must fail.
    std::string s = bech32::Encode(bech32::Encoding::BECH32M, "aumq",
                                   {1, 2, 3, 4, 5, 6, 7, 8});
    BOOST_REQUIRE(!s.empty());
    char& last = s.back();
    last = (last == 'a') ? 'p' : 'a';  // any different valid char
    BOOST_CHECK(bech32::Decode(s).encoding == bech32::Encoding::INVALID);
}

// BIP 173 / 350 encodings are NOT interchangeable: a bech32-encoded
// string must not decode as bech32m, and vice versa. The checksum
// constants differ specifically to make this so.
BOOST_AUTO_TEST_CASE(bech32_does_not_decode_as_bech32m)
{
    for (const std::string& s : kValidBech32) {
        bech32::DecodeResult r = bech32::Decode(s);
        BOOST_CHECK_MESSAGE(r.encoding != bech32::Encoding::BECH32M,
            "BIP 173 string unexpectedly decoded as BECH32M: " << s);
    }
}

BOOST_AUTO_TEST_CASE(bech32m_does_not_decode_as_bech32)
{
    for (const std::string& s : kValidBech32M) {
        bech32::DecodeResult r = bech32::Decode(s);
        BOOST_CHECK_MESSAGE(r.encoding != bech32::Encoding::BECH32,
            "BIP 350 string unexpectedly decoded as BECH32: " << s);
    }
}

// ConvertBits: 8-bit → 5-bit → 8-bit round-trip for sample payloads.
BOOST_AUTO_TEST_CASE(convertbits_8_to_5_to_8_roundtrip)
{
    std::vector<uint8_t> orig{0xDE, 0xAD, 0xBE, 0xEF, 0x01, 0x23, 0x45, 0x67,
                              0x89, 0xAB, 0xCD, 0xEF, 0xFE, 0xDC, 0xBA, 0x98,
                              0x76, 0x54, 0x32, 0x10};  // 20 bytes (HASH160)
    std::vector<uint8_t> five;
    BOOST_REQUIRE(bech32::ConvertBits(five, 5, orig, 8, true));

    std::vector<uint8_t> back;
    BOOST_REQUIRE(bech32::ConvertBits(back, 8, five, 5, false));
    BOOST_CHECK(back == orig);
}

// Round-trip a 20-byte payload (HASH160-sized) through full encode/decode.
BOOST_AUTO_TEST_CASE(roundtrip_20_byte_payload)
{
    std::vector<uint8_t> raw(20);
    for (size_t i = 0; i < 20; ++i)
        raw[i] = static_cast<uint8_t>(i * 7 + 3);

    std::vector<uint8_t> five;
    BOOST_REQUIRE(bech32::ConvertBits(five, 5, raw, 8, true));

    std::string s = bech32::Encode(bech32::Encoding::BECH32M, "aumq", five);
    BOOST_REQUIRE(!s.empty());

    bech32::DecodeResult r = bech32::Decode(s);
    BOOST_REQUIRE(r.encoding == bech32::Encoding::BECH32M);
    BOOST_CHECK_EQUAL(r.hrp, "aumq");

    std::vector<uint8_t> back;
    BOOST_REQUIRE(bech32::ConvertBits(back, 8, r.data, 5, false));
    BOOST_CHECK(back == raw);
}

BOOST_AUTO_TEST_SUITE_END()
