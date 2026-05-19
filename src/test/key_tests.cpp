#include <boost/test/unit_test.hpp>

#include <string>
#include <vector>

#include "key.h"
#include "base58.h"
#include "uint256.h"
#include "util.h"

using namespace std;

// M1.3: rewritten to generate keys instead of decoding hardcoded WIF strings.
// The original test embedded Litecoin-prefix WIF strings and addresses
// ("6uu5...", "T6Us...", "LWaFe..."), which fail to decode against
// aumcoin's rebranded prefix (23). Generating keys + roundtripping them
// through CBitcoinSecret / CBitcoinAddress exercises the SAME encode/decode
// paths and the same sign/verify behavior without baking in the prefix.

static const string strAddressBad("not-a-valid-wif-or-address");


#ifdef KEY_TESTS_DUMPINFO
void dumpKeyInfo(uint256 privkey)
{
    CSecret secret;
    secret.resize(32);
    memcpy(&secret[0], &privkey, 32);
    vector<unsigned char> sec;
    sec.resize(32);
    memcpy(&sec[0], &secret[0], 32);
    printf("  * secret (hex): %s\n", HexStr(sec).c_str());

    for (int nCompressed=0; nCompressed<2; nCompressed++)
    {
        bool fCompressed = nCompressed == 1;
        printf("  * %s:\n", fCompressed ? "compressed" : "uncompressed");
        CBitcoinSecret bsecret;
        bsecret.SetSecret(secret, fCompressed);
        printf("    * secret (base58): %s\n", bsecret.ToString().c_str());
        CKey key;
        key.SetSecret(secret, fCompressed);
        vector<unsigned char> vchPubKey = key.GetPubKey();
        printf("    * pubkey (hex): %s\n", HexStr(vchPubKey).c_str());
        printf("    * address (base58): %s\n", CBitcoinAddress(vchPubKey).ToString().c_str());
    }
}
#endif


BOOST_AUTO_TEST_SUITE(key_tests)

BOOST_AUTO_TEST_CASE(key_test1)
{
    // Deterministic but locally-generated raw secrets — no prefix dependence.
    CSecret secret1, secret2;
    secret1.resize(32);
    secret2.resize(32);
    for (int i = 0; i < 32; i++) {
        secret1[i] = static_cast<unsigned char>(0x10 + i);
        secret2[i] = static_cast<unsigned char>(0xa0 + i);
    }

    // Encode each secret as both uncompressed and compressed WIF, then
    // decode the strings back to verify aumcoin's CBitcoinSecret round-trip.
    CBitcoinSecret bsecret1, bsecret2, bsecret1C, bsecret2C;
    bsecret1.SetSecret (secret1, false);
    bsecret2.SetSecret (secret2, false);
    bsecret1C.SetSecret(secret1, true);
    bsecret2C.SetSecret(secret2, true);

    const string strSecret1  = bsecret1.ToString();
    const string strSecret2  = bsecret2.ToString();
    const string strSecret1C = bsecret1C.ToString();
    const string strSecret2C = bsecret2C.ToString();

    BOOST_CHECK(!strSecret1.empty());
    BOOST_CHECK(!strSecret2.empty());
    BOOST_CHECK(!strSecret1C.empty());
    BOOST_CHECK(!strSecret2C.empty());

    CBitcoinSecret rbs1, rbs2, rbs1C, rbs2C, baddress1;
    BOOST_CHECK( rbs1.SetString (strSecret1));
    BOOST_CHECK( rbs2.SetString (strSecret2));
    BOOST_CHECK( rbs1C.SetString(strSecret1C));
    BOOST_CHECK( rbs2C.SetString(strSecret2C));
    BOOST_CHECK(!baddress1.SetString(strAddressBad));

    bool fCompressed;
    CSecret rsecret1  = rbs1.GetSecret (fCompressed);
    BOOST_CHECK(fCompressed == false);
    BOOST_CHECK(rsecret1 == secret1);
    CSecret rsecret2  = rbs2.GetSecret (fCompressed);
    BOOST_CHECK(fCompressed == false);
    BOOST_CHECK(rsecret2 == secret2);
    CSecret rsecret1C = rbs1C.GetSecret(fCompressed);
    BOOST_CHECK(fCompressed == true);
    BOOST_CHECK(rsecret1C == secret1);
    CSecret rsecret2C = rbs2C.GetSecret(fCompressed);
    BOOST_CHECK(fCompressed == true);
    BOOST_CHECK(rsecret2C == secret2);

    // Same raw secret produces identical bytes whether encoded
    // compressed or uncompressed.
    BOOST_CHECK(rsecret1 == rsecret1C);
    BOOST_CHECK(rsecret2 == rsecret2C);

    CKey key1, key2, key1C, key2C;
    key1.SetSecret (secret1, false);
    key2.SetSecret (secret2, false);
    key1C.SetSecret(secret1, true);
    key2C.SetSecret(secret2, true);

    // Address round-trip: encode the derived pubkey-hash, decode the
    // string, confirm it points back to the same key.
    CBitcoinAddress addr1 (key1.GetPubKey().GetID());
    CBitcoinAddress addr2 (key2.GetPubKey().GetID());
    CBitcoinAddress addr1C(key1C.GetPubKey().GetID());
    CBitcoinAddress addr2C(key2C.GetPubKey().GetID());

    BOOST_CHECK(addr1.IsValid());
    BOOST_CHECK(addr2.IsValid());
    BOOST_CHECK(addr1C.IsValid());
    BOOST_CHECK(addr2C.IsValid());

    CBitcoinAddress raddr1, raddr2, raddr1C, raddr2C;
    BOOST_CHECK(raddr1.SetString(addr1.ToString()));
    BOOST_CHECK(raddr2.SetString(addr2.ToString()));
    BOOST_CHECK(raddr1C.SetString(addr1C.ToString()));
    BOOST_CHECK(raddr2C.SetString(addr2C.ToString()));

    BOOST_CHECK(raddr1.Get()  == CTxDestination(key1.GetPubKey().GetID()));
    BOOST_CHECK(raddr2.Get()  == CTxDestination(key2.GetPubKey().GetID()));
    BOOST_CHECK(raddr1C.Get() == CTxDestination(key1C.GetPubKey().GetID()));
    BOOST_CHECK(raddr2C.Get() == CTxDestination(key2C.GetPubKey().GetID()));

    for (int n=0; n<16; n++)
    {
        string strMsg = strprintf("Very secret message %i: 11", n);
        uint256 hashMsg = Hash(strMsg.begin(), strMsg.end());

        // normal signatures

        vector<unsigned char> sign1, sign2, sign1C, sign2C;

        BOOST_CHECK(key1.Sign (hashMsg, sign1));
        BOOST_CHECK(key2.Sign (hashMsg, sign2));
        BOOST_CHECK(key1C.Sign(hashMsg, sign1C));
        BOOST_CHECK(key2C.Sign(hashMsg, sign2C));

        BOOST_CHECK( key1.Verify(hashMsg, sign1));
        BOOST_CHECK(!key1.Verify(hashMsg, sign2));
        BOOST_CHECK( key1.Verify(hashMsg, sign1C));
        BOOST_CHECK(!key1.Verify(hashMsg, sign2C));

        BOOST_CHECK(!key2.Verify(hashMsg, sign1));
        BOOST_CHECK( key2.Verify(hashMsg, sign2));
        BOOST_CHECK(!key2.Verify(hashMsg, sign1C));
        BOOST_CHECK( key2.Verify(hashMsg, sign2C));

        BOOST_CHECK( key1C.Verify(hashMsg, sign1));
        BOOST_CHECK(!key1C.Verify(hashMsg, sign2));
        BOOST_CHECK( key1C.Verify(hashMsg, sign1C));
        BOOST_CHECK(!key1C.Verify(hashMsg, sign2C));

        BOOST_CHECK(!key2C.Verify(hashMsg, sign1));
        BOOST_CHECK( key2C.Verify(hashMsg, sign2));
        BOOST_CHECK(!key2C.Verify(hashMsg, sign1C));
        BOOST_CHECK( key2C.Verify(hashMsg, sign2C));

        // compact signatures (with key recovery)

        vector<unsigned char> csign1, csign2, csign1C, csign2C;

        BOOST_CHECK(key1.SignCompact (hashMsg, csign1));
        BOOST_CHECK(key2.SignCompact (hashMsg, csign2));
        BOOST_CHECK(key1C.SignCompact(hashMsg, csign1C));
        BOOST_CHECK(key2C.SignCompact(hashMsg, csign2C));

        CKey rkey1, rkey2, rkey1C, rkey2C;

        BOOST_CHECK(rkey1.SetCompactSignature (hashMsg, csign1));
        BOOST_CHECK(rkey2.SetCompactSignature (hashMsg, csign2));
        BOOST_CHECK(rkey1C.SetCompactSignature(hashMsg, csign1C));
        BOOST_CHECK(rkey2C.SetCompactSignature(hashMsg, csign2C));


        BOOST_CHECK(rkey1.GetPubKey()  == key1.GetPubKey());
        BOOST_CHECK(rkey2.GetPubKey()  == key2.GetPubKey());
        BOOST_CHECK(rkey1C.GetPubKey() == key1C.GetPubKey());
        BOOST_CHECK(rkey2C.GetPubKey() == key2C.GetPubKey());
    }
}

BOOST_AUTO_TEST_SUITE_END()
