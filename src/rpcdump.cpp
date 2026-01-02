// Copyright (c) 2009-2012 Bitcoin Developers
// Copyright (c) 2011-2012 Litecoin Developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "init.h" // for pwalletMain
#include "bitcoinrpc.h"
#include "ui_interface.h"
#include "base58.h"

#include <boost/lexical_cast.hpp>

#define printf OutputDebugStringF

using namespace json_spirit;
using namespace std;

class CTxDump
{
public:
    CBlockIndex *pindex;
    int64 nValue;
    bool fSpent;
    CWalletTx* ptx;
    int nOut;
    CTxDump(CWalletTx* ptx = NULL, int nOut = -1)
    {
        pindex = NULL;
        nValue = 0;
        fSpent = false;
        this->ptx = ptx;
        this->nOut = nOut;
    }
};

Value importprivkey(const Array& params, bool fHelp)
{
    if (fHelp || params.size() < 1 || params.size() > 2)
        throw runtime_error(
            "importprivkey <litecoinprivkey> [label]\n"
            "Adds a private key (as returned by dumpprivkey) to your wallet.");

    string strSecret = params[0].get_str();
    string strLabel = "";
    if (params.size() > 1)
        strLabel = params[1].get_str();
    CBitcoinSecret vchSecret;
    bool fGood = vchSecret.SetString(strSecret);

    if (!fGood) throw JSONRPCError(-5,"Invalid private key");

    CKey key;
    bool fCompressed;
    CSecret secret = vchSecret.GetSecret(fCompressed);
    key.SetSecret(secret, fCompressed);
    CKeyID vchAddress = key.GetPubKey().GetID();
    {
        LOCK2(cs_main, pwalletMain->cs_wallet);

        pwalletMain->MarkDirty();
        pwalletMain->SetAddressBookName(vchAddress, strLabel);

        if (!pwalletMain->AddKey(key))
            throw JSONRPCError(-4,"Error adding key to wallet");

        pwalletMain->ScanForWalletTransactions(pindexGenesisBlock, true);
        pwalletMain->ReacceptWalletTransactions();
    }

    return Value::null;
}

Value dumpprivkey(const Array& params, bool fHelp)
{
    if (fHelp || params.size() != 1)
        throw runtime_error(
	    "dumpprivkey <litecoinaddress>\n"
	    "Reveals the private key corresponding to <litecoinaddress>.");

    string strAddress = params[0].get_str();
    CBitcoinAddress address;
    if (!address.SetString(strAddress))
	throw JSONRPCError(-5, "Invalid Litecoin address");
    CKeyID keyID;
    if (!address.GetKeyID(keyID))
        throw JSONRPCError(-3, "Address does not refer to a key");
    CSecret vchSecret;
    bool fCompressed;
    if (!pwalletMain->GetSecret(keyID, vchSecret, fCompressed))
        throw JSONRPCError(-4,"Private key for address " + strAddress + " is not known");
    return CBitcoinSecret(vchSecret, fCompressed).ToString();
}

#ifdef ENABLE_MLDSA
// Phase 3.4: Hybrid Key RPC Commands

Value getnewmldsaaddress(const Array& params, bool fHelp)
{
    if (fHelp || params.size() > 1)
        throw runtime_error(
            "getnewmldsaaddress [account]\n"
            "Returns a new quantum-resistant address using hybrid ECDSA+ML-DSA keys.\n"
            "If [account] is specified, it is added to the address book.\n"
            "The address uses both classical ECDSA and post-quantum ML-DSA-65 signatures.");

    // Parse the account first so we don't generate a key if there's an error
    string strAccount;
    if (params.size() > 0)
        strAccount = params[0].get_str();

    if (!pwalletMain->IsLocked())
        pwalletMain->TopUpKeyPool();

    // Generate a new hybrid key (ECDSA + ML-DSA)
    CKey key;
    key.MakeNewHybridKey(true);
    
    CPubKey pubkey = key.GetPubKey();
    if (!pubkey.IsHybrid())
        throw JSONRPCError(-1, "Failed to generate hybrid key");
    
    CKeyID keyID = pubkey.GetID();
    
    if (!pwalletMain->AddKey(key))
        throw JSONRPCError(-4, "Error adding key to wallet");

    pwalletMain->SetAddressBookName(keyID, strAccount);

    return CBitcoinAddress(keyID).ToString();
}

Value signmessagemldsa(const Array& params, bool fHelp)
{
    if (fHelp || params.size() != 2)
        throw runtime_error(
            "signmessagemldsa <aumcoinaddress> <message>\n"
            "Sign a message with the ML-DSA private key of an address.\n"
            "Returns a base64-encoded ML-DSA-65 signature (quantum-resistant).");

    if (pwalletMain->IsLocked())
        throw JSONRPCError(-13, "Error: Please enter the wallet passphrase with walletpassphrase first.");

    string strAddress = params[0].get_str();
    string strMessage = params[1].get_str();

    CBitcoinAddress addr(strAddress);
    if (!addr.IsValid())
        throw JSONRPCError(-3, "Invalid address");

    CKeyID keyID;
    if (!addr.GetKeyID(keyID))
        throw JSONRPCError(-3, "Address does not refer to key");

    CKey key;
    if (!pwalletMain->GetKey(keyID, key))
        throw JSONRPCError(-4, "Private key not available");
        
    if (!key.IsHybrid())
        throw JSONRPCError(-5, "Address does not have ML-DSA key (use getnewmldsaaddress)");

    CDataStream ss(SER_GETHASH, 0);
    ss << strMessageMagic;
    ss << strMessage;

    vector<unsigned char> vchSig;
    if (!key.SignMLDSA(Hash(ss.begin(), ss.end()), vchSig))
        throw JSONRPCError(-5, "ML-DSA sign failed");

    return EncodeBase64(&vchSig[0], vchSig.size());
}

Value verifymessagemldsa(const Array& params, bool fHelp)
{
    if (fHelp || params.size() != 3)
        throw runtime_error(
            "verifymessagemldsa <aumcoinaddress> <signature> <message>\n"
            "Verify a ML-DSA signed message (quantum-resistant verification).");

    string strAddress  = params[0].get_str();
    string strSign     = params[1].get_str();
    string strMessage  = params[2].get_str();

    CBitcoinAddress addr(strAddress);
    if (!addr.IsValid())
        throw JSONRPCError(-3, "Invalid address");

    CKeyID keyID;
    if (!addr.GetKeyID(keyID))
        throw JSONRPCError(-3, "Address does not refer to key");

    bool fInvalid = false;
    vector<unsigned char> vchSig = DecodeBase64(strSign.c_str(), &fInvalid);

    if (fInvalid)
        throw JSONRPCError(-5, "Malformed base64 encoding");

    CDataStream ss(SER_GETHASH, 0);
    ss << strMessageMagic;
    ss << strMessage;

    CPubKey pubkey;
    if (!pwalletMain->GetPubKey(keyID, pubkey))
        throw JSONRPCError(-4, "Public key not available");
        
    if (!pubkey.IsHybrid())
        throw JSONRPCError(-5, "Address does not have ML-DSA key");

    vector<unsigned char> vchMLDSAPubKey = pubkey.GetMLDSAPubKey();
    
    return (CKey::VerifyMLDSA(Hash(ss.begin(), ss.end()), vchSig, vchMLDSAPubKey));
}

Value gethybridkeyinfo(const Array& params, bool fHelp)
{
    if (fHelp || params.size() != 1)
        throw runtime_error(
            "gethybridkeyinfo <aumcoinaddress>\n"
            "Get information about a hybrid key (ECDSA + ML-DSA).\n"
            "Returns details about key types and sizes.");

    string strAddress = params[0].get_str();
    CBitcoinAddress addr(strAddress);
    if (!addr.IsValid())
        throw JSONRPCError(-3, "Invalid address");

    CKeyID keyID;
    if (!addr.GetKeyID(keyID))
        throw JSONRPCError(-3, "Address does not refer to key");

    CPubKey pubkey;
    if (!pwalletMain->GetPubKey(keyID, pubkey))
        throw JSONRPCError(-4, "Public key not available");

    Object result;
    result.push_back(Pair("address", strAddress));
    result.push_back(Pair("ecdsa_pubkey_size", (int)pubkey.Raw().size()));
    result.push_back(Pair("ecdsa_compressed", pubkey.IsCompressed()));
    result.push_back(Pair("has_mldsa", pubkey.HasMLDSAKey()));
    result.push_back(Pair("is_hybrid", pubkey.IsHybrid()));
    
    if (pubkey.HasMLDSAKey()) {
        vector<unsigned char> mldsa_pub = pubkey.GetMLDSAPubKey();
        result.push_back(Pair("mldsa_pubkey_size", (int)mldsa_pub.size()));
        result.push_back(Pair("quantum_resistant", true));
    } else {
        result.push_back(Pair("quantum_resistant", false));
    }

    return result;
}

Value addmultisigmldsaaddress(const Array& params, bool fHelp)
{
    if (fHelp || params.size() < 2 || params.size() > 3)
        throw runtime_error(
            "addmultisigmldsaaddress <nrequired> <'[\"key\",\"key\"]'> [account]\n"
            "Add a quantum-resistant M-of-N multisignature address to the wallet.\n"
            "Keys must be AumCoin addresses or hex-encoded public keys with ML-DSA component.\n"
            "Returns the new multisig P2SH address.\n"
            "\nArguments:\n"
            "1. nrequired      (numeric, required) The number of required signatures (M)\n"
            "2. keys           (string, required) JSON array of AumCoin addresses or hex pubkeys\n"
            "3. account        (string, optional) Account name to associate with address\n"
            "\nExample:\n"
            "  aumcoind addmultisigmldsaaddress 2 '[\"addr1\",\"addr2\",\"addr3\"]' \"multisig-cold-storage\"\n"
            "\nResult:\n"
            "  \"address\"  (string) The P2SH address for the multisig script\n");

    if (fHelp)
        return Value::null;

    int nRequired = params[0].get_int();
    const Array& keys = params[1].get_array();
    string strAccount;
    if (params.size() > 2)
        strAccount = AccountFromValue(params[2]);

    // Validate nRequired
    if (nRequired < 1)
        throw runtime_error("nrequired must be at least 1");
    if (nRequired > (int)keys.size())
        throw runtime_error("nrequired cannot be greater than number of keys");
    if (keys.size() > 15)
        throw runtime_error("Number of keys cannot exceed 15");

    // Collect ML-DSA public keys from addresses
    vector<vector<unsigned char> > vchMLDSAPubKeys;
    vchMLDSAPubKeys.reserve(keys.size());

    for (unsigned int i = 0; i < keys.size(); ++i)
    {
        CBitcoinAddress address(keys[i].get_str());
        if (!address.IsValid())
            throw JSONRPCError(-5, strprintf("Invalid AumCoin address: %s", keys[i].get_str().c_str()));

        CKeyID keyID;
        if (!address.GetKeyID(keyID))
            throw JSONRPCError(-5, "Address does not refer to a key");

        CPubKey pubkey;
        if (!pwalletMain->GetPubKey(keyID, pubkey))
            throw JSONRPCError(-5, strprintf("Public key not found in wallet: %s", keys[i].get_str().c_str()));

        if (!pubkey.HasMLDSAKey())
            throw JSONRPCError(-5, strprintf("Key does not have ML-DSA component: %s", keys[i].get_str().c_str()));

        vchMLDSAPubKeys.push_back(pubkey.GetMLDSAPubKey());
    }

    // Create the multisig redeem script
    CScript redeemScript;
    try {
        redeemScript = CreateMLDSAMultisigScript(nRequired, vchMLDSAPubKeys);
    } catch (const std::runtime_error& e) {
        throw JSONRPCError(-8, strprintf("Failed to create multisig script: %s", e.what()));
    }

    // Add redeem script to wallet
    if (!pwalletMain->AddCScript(redeemScript))
        throw JSONRPCError(-4, "Error adding redeemScript to wallet");

    // Create P2SH address
    CScriptID scriptID = redeemScript.GetID();
    CBitcoinAddress multisigAddress(scriptID);

    // Associate with account
    pwalletMain->SetAddressBookName(scriptID, strAccount);

    return multisigAddress.ToString();
}

#endif // ENABLE_MLDSA
