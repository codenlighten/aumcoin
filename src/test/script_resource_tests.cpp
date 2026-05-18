// M1.3: regression coverage for the script-engine resource limits that
// already exist in src/script.cpp. The project's "Scaling Liberation"
// phase intentionally lifted some bounds (block size, scriptPubKey
// size); these tests pin down the ones that are STILL in effect so
// they can't silently regress.
//
// Limits exercised:
//   - nMaxBigNumBits = 4096   (src/script.cpp:46)   - OP_MUL/DIV/MOD operand cap
//   - 10240-byte OP_CAT result cap                  (src/script.cpp:603)
//   - 10240-byte single push value cap              (src/script.cpp:283)

#include <boost/test/unit_test.hpp>

#include "bignum.h"
#include "script.h"
#include "main.h"

extern bool VerifyScript(const CScript& scriptSig, const CScript& scriptPubKey,
                         const CTransaction& txTo, unsigned int nIn,
                         bool fValidatePayToScriptHash, int nHashType);

BOOST_AUTO_TEST_SUITE(script_resource_tests)

// OP_MUL must reject operands with bitSize > 4096.
BOOST_AUTO_TEST_CASE(op_mul_rejects_oversized_bignum)
{
    // 513 bytes of payload + a leading 0x00 keeps the sign positive
    // through CBigNum's signed-magnitude encoding. The resulting
    // bitSize is well above 4096.
    std::vector<unsigned char> vchBig(514, 0xFF);
    vchBig[0] = 0x00;  // sign byte: positive
    CBigNum big;
    big.setvch(vchBig);
    BOOST_REQUIRE(big.bitSize() > 4096);

    CScript scriptSig;
    scriptSig << big.getvch() << big.getvch();
    CScript scriptPubKey;
    scriptPubKey << OP_MUL;

    CTransaction tx;
    BOOST_CHECK(!VerifyScript(scriptSig, scriptPubKey, tx, 0, false, SIGHASH_NONE));
}

// OP_MUL must accept operands at the limit (sanity counterpart to the above).
BOOST_AUTO_TEST_CASE(op_mul_accepts_small_bignum)
{
    // 7 * 6 = 42.
    CScript scriptSig;
    scriptSig << 7 << 6;
    CScript scriptPubKey;
    scriptPubKey << OP_MUL << 42 << OP_EQUAL;

    CTransaction tx;
    BOOST_CHECK(VerifyScript(scriptSig, scriptPubKey, tx, 0, false, SIGHASH_NONE));
}

// OP_DIV must reject operands with bitSize > 4096.
BOOST_AUTO_TEST_CASE(op_div_rejects_oversized_bignum)
{
    std::vector<unsigned char> vchBig(514, 0xFF);
    vchBig[0] = 0x00;
    CBigNum big;
    big.setvch(vchBig);

    CScript scriptSig;
    scriptSig << big.getvch() << big.getvch();
    CScript scriptPubKey;
    scriptPubKey << OP_DIV;

    CTransaction tx;
    BOOST_CHECK(!VerifyScript(scriptSig, scriptPubKey, tx, 0, false, SIGHASH_NONE));
}

// OP_MOD must reject operands with bitSize > 4096.
BOOST_AUTO_TEST_CASE(op_mod_rejects_oversized_bignum)
{
    std::vector<unsigned char> vchBig(514, 0xFF);
    vchBig[0] = 0x00;
    CBigNum big;
    big.setvch(vchBig);

    CScript scriptSig;
    scriptSig << big.getvch() << big.getvch();
    CScript scriptPubKey;
    scriptPubKey << OP_MOD;

    CTransaction tx;
    BOOST_CHECK(!VerifyScript(scriptSig, scriptPubKey, tx, 0, false, SIGHASH_NONE));
}

// OP_CAT must reject when the concatenated result would exceed 10240 bytes.
BOOST_AUTO_TEST_CASE(op_cat_rejects_oversized_result)
{
    std::vector<unsigned char> half(5121, 0x42);  // 5121 + 5121 = 10242 > 10240
    CScript scriptSig;
    scriptSig << half << half;
    CScript scriptPubKey;
    scriptPubKey << OP_CAT;

    CTransaction tx;
    BOOST_CHECK(!VerifyScript(scriptSig, scriptPubKey, tx, 0, false, SIGHASH_NONE));
}

// OP_CAT at exactly the limit must succeed (the limit is INCLUSIVE in
// the > check at src/script.cpp:603, i.e. 10240-byte result is allowed).
BOOST_AUTO_TEST_CASE(op_cat_accepts_at_limit)
{
    std::vector<unsigned char> half(5120, 0x42);  // 5120 + 5120 = 10240, allowed
    CScript scriptSig;
    scriptSig << half << half;
    CScript scriptPubKey;
    scriptPubKey << OP_CAT << OP_DROP << OP_TRUE;

    CTransaction tx;
    BOOST_CHECK(VerifyScript(scriptSig, scriptPubKey, tx, 0, false, SIGHASH_NONE));
}

// A single pushed value exceeding 10240 bytes must be rejected during
// script evaluation (src/script.cpp:283).
BOOST_AUTO_TEST_CASE(push_value_rejects_above_10240)
{
    std::vector<unsigned char> tooBig(10241, 0xAA);
    CScript scriptSig;
    scriptSig << tooBig;
    CScript scriptPubKey;
    scriptPubKey << OP_TRUE;

    CTransaction tx;
    BOOST_CHECK(!VerifyScript(scriptSig, scriptPubKey, tx, 0, false, SIGHASH_NONE));
}

BOOST_AUTO_TEST_SUITE_END()
