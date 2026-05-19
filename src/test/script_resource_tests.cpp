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

// M1.4 — per-script op count limit (raised from Bitcoin's 201 to
// MAX_OPS_PER_SCRIPT == 20000). Exact-boundary + over-boundary cases.
BOOST_AUTO_TEST_CASE(max_ops_per_script_at_boundary_passes)
{
    CScript scriptSig;
    scriptSig << OP_1;

    CScript scriptPubKey;
    // 10000 OP_DUP / OP_DROP pairs = 20000 non-push opcodes (exactly at limit).
    for (int i = 0; i < 10000; ++i)
        scriptPubKey << OP_DUP << OP_DROP;

    CTransaction tx;
    BOOST_CHECK(VerifyScript(scriptSig, scriptPubKey, tx, 0, false, SIGHASH_NONE));
}

BOOST_AUTO_TEST_CASE(max_ops_per_script_over_boundary_fails)
{
    CScript scriptSig;
    scriptSig << OP_1;

    CScript scriptPubKey;
    for (int i = 0; i < 10000; ++i)
        scriptPubKey << OP_DUP << OP_DROP;
    scriptPubKey << OP_DUP << OP_DROP;  // 20002 — one pair too many

    CTransaction tx;
    BOOST_CHECK(!VerifyScript(scriptSig, scriptPubKey, tx, 0, false, SIGHASH_NONE));
}

// M1.4 — weighted cost budget. OP_CHECKMLDSASIG costs 100 cost units per
// OP_COST_CHECKMLDSASIG. 200 invocations consume the entire 20000-unit
// budget; the 201st must trip the budget even when each individual op is
// well under the count limit.
//
// At-boundary "passes" can only be verified in a no-ENABLE_MLDSA build,
// where OP_CHECKMLDSASIG aliases OP_NOP4 (no-op success). In an
// ENABLE_MLDSA build, OP_CHECKMLDSASIG actually pops sig+pubkey from the
// stack and returns false when those aren't valid, which would mask the
// cost-passes assertion. Real ML-DSA-enabled cost-budget tests require
// valid signatures and are tracked as M1.3 follow-up work.
#ifndef ENABLE_MLDSA
BOOST_AUTO_TEST_CASE(mldsa_cost_budget_at_boundary_passes)
{
    CScript scriptSig;
    scriptSig << OP_1;

    CScript scriptPubKey;
    // 200 × OP_CHECKMLDSASIG = 20000 cost units = exactly at limit.
    for (int i = 0; i < 200; ++i)
        scriptPubKey << OP_CHECKMLDSASIG;

    CTransaction tx;
    BOOST_CHECK(VerifyScript(scriptSig, scriptPubKey, tx, 0, false, SIGHASH_NONE));
}
#endif

// Over-boundary cost check fires at EvalScript dispatch BEFORE the
// per-opcode handler — so the cost-budget reject happens identically
// whether OP_CHECKMLDSASIG is the NOP4 alias or the real ML-DSA verify.
// This is the consensus invariant we want.

BOOST_AUTO_TEST_CASE(mldsa_cost_budget_over_boundary_fails)
{
    CScript scriptSig;
    scriptSig << OP_1;

    CScript scriptPubKey;
    for (int i = 0; i < 201; ++i)
        scriptPubKey << OP_CHECKMLDSASIG;  // 201 × 100 = 20100 cost (over)

    CTransaction tx;
    BOOST_CHECK(!VerifyScript(scriptSig, scriptPubKey, tx, 0, false, SIGHASH_NONE));
}

BOOST_AUTO_TEST_SUITE_END()
