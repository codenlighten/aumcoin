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
        strAccount = params[2].get_str();

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

// Phase 4.3: Quantum-Resistant Multisig Transaction Workflow
Value createmultisigmldsatx(const Array& params, bool fHelp)
{
    if (fHelp || params.size() < 5 || params.size() > 6)
        throw runtime_error(
            "createmultisigmldsatx <txid> <vout> <redeemscript> <destination> <amount> [fee]\n"
            "Create an unsigned transaction spending from ML-DSA multisig output\n"
            "\nArguments:\n"
            "1. txid          (string, required) Transaction ID containing the multisig output\n"
            "2. vout          (numeric, required) Output index in the transaction\n"
            "3. redeemscript  (string, required) Hex-encoded multisig redeem script\n"
            "4. destination   (string, required) Destination address\n"
            "5. amount        (numeric, required) Amount to send in satoshis\n"
            "6. fee           (numeric, optional, default=10000) Transaction fee in satoshis\n"
            "\nResult:\n"
            "\"hex\"          (string) Hex-encoded unsigned transaction\n"
        );

    // Parse parameters
    uint256 txid;
    txid.SetHex(params[0].get_str());
    
    int nOut = params[1].get_int();
    if (nOut < 0)
        throw JSONRPCError(-8, "Invalid output index");

    vector<unsigned char> redeemScriptData = ParseHex(params[2].get_str());
    if (redeemScriptData.empty())
        throw JSONRPCError(-8, "Invalid redeem script hex");
    CScript redeemScript(redeemScriptData.begin(), redeemScriptData.end());

    CBitcoinAddress destAddress(params[3].get_str());
    if (!destAddress.IsValid())
        throw JSONRPCError(-5, "Invalid destination address");

    int64 nAmount = params[4].get_int64();
    int64 nFee = (params.size() > 5) ? params[5].get_int64() : 10000; // Default 0.0001 LTC

    if (nAmount <= 0)
        throw JSONRPCError(-3, "Invalid amount");
    if (nFee < 0)
        throw JSONRPCError(-3, "Invalid fee");

    // Create transaction
    CTransaction txNew;
    txNew.nVersion = 1;
    txNew.nLockTime = 0;

    // Add input (spending from multisig)
    CTxIn txin(txid, nOut);
    // Leave scriptSig empty for unsigned transaction
    // Store redeem script in scriptSig for signing context
    txin.scriptSig = redeemScript;
    txNew.vin.push_back(txin);

    // Add output (to destination, minus fee)
    CScript scriptPubKey;
    scriptPubKey.SetDestination(destAddress.Get());
    CTxOut txout(nAmount - nFee, scriptPubKey);
    txNew.vout.push_back(txout);

    // Serialize transaction to hex
    CDataStream ss(SER_NETWORK, PROTOCOL_VERSION);
    ss << txNew;
    return HexStr(ss.begin(), ss.end());
}

Value signmldsatx(const Array& params, bool fHelp)
{
    if (fHelp || params.size() != 2)
        throw runtime_error(
            "signmldsatx <hex> <address>\n"
            "Add one ML-DSA signature to a partially signed multisig transaction\n"
            "\nArguments:\n"
            "1. hex       (string, required) Unsigned or partially signed transaction hex\n"
            "2. address   (string, required) Address to sign with (must be in wallet)\n"
            "\nResult:\n"
            "{\n"
            "  \"hex\": \"value\",           (string) Updated transaction hex\n"
            "  \"complete\": true|false      (boolean) True if transaction has enough signatures\n"
            "  \"signatures\": n              (numeric) Number of signatures collected\n"
            "}\n"
        );

    // Deserialize transaction
    vector<unsigned char> txData = ParseHex(params[0].get_str());
    CDataStream ssData(txData, SER_NETWORK, PROTOCOL_VERSION);
    CTransaction tx;
    try {
        ssData >> tx;
    } catch (std::exception &e) {
        throw JSONRPCError(-22, "Invalid transaction hex");
    }

    if (tx.vin.empty())
        throw JSONRPCError(-8, "Transaction has no inputs");

    // Get signing address
    CBitcoinAddress address(params[1].get_str());
    if (!address.IsValid())
        throw JSONRPCError(-5, "Invalid address");

    CKeyID keyID;
    if (!address.GetKeyID(keyID))
        throw JSONRPCError(-5, "Address does not refer to a key");

    // Get key from wallet
    CKey key;
    if (!pwalletMain->GetKey(keyID, key))
        throw JSONRPCError(-4, "Private key not available");

    if (!key.HasMLDSAKey())
        throw JSONRPCError(-5, "Key does not have ML-DSA component");

    // For now, sign the first input (multisig transactions typically have one input)
    CTxIn& txin = tx.vin[0];
    
    // Extract redeem script from scriptSig
    // In unsigned tx, scriptSig contains just the redeem script
    // In partially signed tx, scriptSig has format: <sig_count> <sig1> <sig2> ... <redeemScript>
    
    CScript redeemScript;
    vector<vector<unsigned char> > existingSignatures;
    
    // Parse existing scriptSig
    CScript::const_iterator pc = txin.scriptSig.begin();
    opcodetype opcode;
    vector<unsigned char> vchData;
    
    // Collect all push data from scriptSig
    vector<vector<unsigned char> > scriptData;
    while (pc < txin.scriptSig.end())
    {
        if (!txin.scriptSig.GetOp(pc, opcode, vchData))
            break;
        if (opcode >= 0 && opcode <= OP_PUSHDATA4 && vchData.size() > 0)
            scriptData.push_back(vchData);
    }
    
    if (scriptData.empty())
        throw JSONRPCError(-8, "No redeem script found in transaction");
    
    // Last element is always the redeem script
    redeemScript = CScript(scriptData.back().begin(), scriptData.back().end());
    
    // Everything before the last element are signatures (skip first element which is sig count)
    int nSigCount = 0;
    if (scriptData.size() > 1)
    {
        // First element is signature count (if it exists and is small)
        if (scriptData[0].size() == 1 && scriptData[0][0] <= 15)
        {
            nSigCount = scriptData[0][0];
            // Collect existing signatures
            for (size_t i = 1; i < scriptData.size() - 1; ++i)
                existingSignatures.push_back(scriptData[i]);
        }
    }

    // Verify this key is part of the multisig
    CPubKey pubkey = key.GetPubKey();
    vector<unsigned char> vchMLDSAPubKey = pubkey.GetMLDSAPubKey();
    
    // Extract required count and pubkeys from redeem script
    int nRequired = 0;
    vector<vector<unsigned char> > vchPubKeys;
    
    CScript::const_iterator pcScript = redeemScript.begin();
    if (!redeemScript.GetOp(pcScript, opcode, vchData))
        throw JSONRPCError(-8, "Failed to parse redeem script");
    
    if (opcode >= OP_1 && opcode <= OP_16)
        nRequired = CScript::DecodeOP_N(opcode);
    else
        throw JSONRPCError(-8, "Invalid redeem script format");
    
    // Collect public keys from redeem script
    while (pcScript < redeemScript.end())
    {
        if (!redeemScript.GetOp(pcScript, opcode, vchData))
            break;
        if (opcode >= 0 && opcode <= OP_PUSHDATA4 && vchData.size() == 1952)
            vchPubKeys.push_back(vchData);
    }
    
    // Check if our key is in the multisig
    bool fKeyFound = false;
    for (const auto& pk : vchPubKeys)
    {
        if (pk == vchMLDSAPubKey)
        {
            fKeyFound = true;
            break;
        }
    }
    
    if (!fKeyFound)
        throw JSONRPCError(-5, "Key is not part of this multisig");
    
    // Create signing message (transaction hash)
    uint256 txHash = tx.GetHash();
    
    // Sign with ML-DSA
    vector<unsigned char> vchSig;
    if (!key.SignMLDSA(txHash, vchSig))
        throw JSONRPCError(-5, "Signing failed");
    
    // Verify signature immediately using static CKey method
    if (!CKey::VerifyMLDSA(txHash, vchSig, vchMLDSAPubKey))
        throw JSONRPCError(-5, "Signature verification failed");
    
    // Add signature to list
    existingSignatures.push_back(vchSig);
    nSigCount++;
    
    // Rebuild scriptSig: <sig_count> <sig1> <sig2> ... <sigN> <redeemScript>
    CScript newScriptSig;
    newScriptSig << CScript::EncodeOP_N(nSigCount);
    for (const auto& sig : existingSignatures)
        newScriptSig << sig;
    
    // Push redeem script as data
    vector<unsigned char> redeemScriptVec(redeemScript.begin(), redeemScript.end());
    newScriptSig << redeemScriptVec;
    
    txin.scriptSig = newScriptSig;
    
    // Determine if transaction is complete
    bool fComplete = (nSigCount >= nRequired);
    
    // Serialize updated transaction
    CDataStream ss(SER_NETWORK, PROTOCOL_VERSION);
    ss << tx;
    
    Object result;
    result.push_back(Pair("hex", HexStr(ss.begin(), ss.end())));
    result.push_back(Pair("complete", fComplete));
    result.push_back(Pair("signatures", nSigCount));
    result.push_back(Pair("required", nRequired));
    
    return result;
}

#endif // ENABLE_MLDSA

