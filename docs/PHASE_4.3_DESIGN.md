# Phase 4.3: Quantum-Resistant Multisig Transaction Workflow

## Overview
Implement the complete workflow for creating and signing multi-party transactions using ML-DSA-65 multisignature addresses.

## Architecture

### Transaction Creation Flow
```
1. User creates multisig address (Phase 4.2 - DONE)
   └─> addmultisigmldsaaddress 2 '["addr1","addr2","addr3"]'
   
2. Funds are sent to P2SH address
   └─> sendtoaddress <multisig_address> <amount>
   
3. Create unsigned transaction (Phase 4.3.1 - NEW)
   └─> createmultisigmldsatx
       Input: Previous UTXO from multisig address
       Input: Destination address and amount
       Output: Unsigned transaction hex
   
4. Each required party signs (Phase 4.3.2 - NEW)
   └─> signmldsatx <hex> <address>
       Input: Unsigned or partially signed transaction
       Input: Signing address (must be one of the multisig keys)
       Output: Transaction with one more ML-DSA signature
   
5. After M signatures, transaction is complete (Phase 4.3.3 - NEW)
   └─> sendrawtransaction <fully_signed_hex>
```

### Key Components

#### 1. `createmultisigmldsatx` RPC Command
**Purpose**: Create an unsigned transaction spending from ML-DSA multisig output

**Parameters**:
- `txid` (string): Transaction ID of the UTXO to spend
- `vout` (int): Output index in the transaction
- `scriptPubKey` (string): The multisig redeem script (hex)
- `destination` (string): Destination address
- `amount` (double): Amount to send (satoshis minus fee)

**Returns**: Hex-encoded unsigned transaction

**Implementation**:
```cpp
Value createmultisigmldsatx(const Array& params, bool fHelp) {
    // 1. Validate parameters
    // 2. Fetch UTXO information
    // 3. Create transaction with input referencing multisig output
    // 4. Add output to destination
    // 5. Leave scriptSig empty (unsigned)
    // 6. Serialize and return hex
}
```

#### 2. `signmldsatx` RPC Command
**Purpose**: Add one ML-DSA signature to a partially signed transaction

**Parameters**:
- `hex` (string): Unsigned or partially signed transaction
- `address` (string): Address to sign with (must be in wallet and in multisig)

**Returns**: Updated hex with additional signature

**Implementation**:
```cpp
Value signmldsatx(const Array& params, bool fHelp) {
    // 1. Deserialize transaction
    // 2. Find which input needs signing
    // 3. Get ML-DSA private key for address
    // 4. Reconstruct signing message (tx hash + input details)
    // 5. Generate ML-DSA-65 signature (1952 bytes)
    // 6. Append signature to scriptSig
    // 7. Check if M signatures reached
    // 8. Serialize and return
}
```

#### 3. Signature Aggregation Logic
**Purpose**: Track and validate signature collection

**Data Structure** (in scriptSig):
```
<sig_count> <sig1> <sig2> ... <sigM> <redeemScript>
```

**Validation**:
- Each signature must be valid ML-DSA-65 (1952 bytes)
- Each signature must be from a different public key in the redeem script
- Must have exactly M signatures to be spendable
- Signatures verify against transaction hash

## Transaction Structure

### Unsigned Transaction
```
CTransaction {
  vin: [
    CTxIn {
      prevout: COutPoint(txid, vout),
      scriptSig: <empty> or <redeemScript>,
      nSequence: 0xffffffff
    }
  ],
  vout: [
    CTxOut {
      nValue: amount,
      scriptPubKey: <destination_script>
    }
  ],
  nLockTime: 0
}
```

### Partially Signed Transaction
```
scriptSig: <1> <sig1> <redeemScript>
```

### Fully Signed Transaction (2-of-3 example)
```
scriptSig: <2> <sig1> <sig2> <redeemScript>
```

## Security Considerations

### 1. **Signature Order Independence**
- Signatures can be collected in any order
- System tracks which pubkeys have signed
- Prevents duplicate signatures from same key

### 2. **Transaction Immutability**
- Once created, transaction hash is fixed
- All signatures must be for the SAME transaction
- Changing any field invalidates all signatures

### 3. **Threshold Enforcement**
- Transaction invalid until M signatures collected
- Excess signatures (> M) rejected
- Nodes validate M matches redeem script requirement

### 4. **Key Management**
- Each party must have their ML-DSA private key
- Keys never leave local wallet
- Partial signatures can be exchanged safely (public info)

## Implementation Files

### New Files
1. **src/rpc_mldsa_multisig_tx.cpp** (NEW)
   - `createmultisigmldsatx()`
   - `signmldsatx()`
   - Helper functions for signature aggregation

### Modified Files
1. **src/bitcoinrpc.cpp**
   - Register new RPC commands
   - Add extern declarations

2. **src/script.cpp** (if needed)
   - `VerifyMLDSAMultisigScript()` - validate signature count
   - `ExtractMLDSASignatures()` - parse scriptSig

3. **makefile.unix**
   - Add rpc_mldsa_multisig_tx.cpp to build

## Testing Strategy

### Unit Tests (test/mldsa_multisig_tx_test.cpp)
```cpp
BOOST_AUTO_TEST_CASE(test_create_unsigned_tx)
BOOST_AUTO_TEST_CASE(test_sign_with_one_key)
BOOST_AUTO_TEST_CASE(test_sign_with_two_keys_2of3)
BOOST_AUTO_TEST_CASE(test_sign_duplicate_key_rejected)
BOOST_AUTO_TEST_CASE(test_wrong_key_rejected)
BOOST_AUTO_TEST_CASE(test_threshold_enforcement)
BOOST_AUTO_TEST_CASE(test_transaction_immutability)
BOOST_AUTO_TEST_CASE(test_signature_serialization)
```

### Integration Test Scenario
```bash
# Setup: Create 2-of-3 multisig address
ADDR1=$(aumcoind getnewmldsaaddress)
ADDR2=$(aumcoind getnewmldsaaddress) 
ADDR3=$(aumcoind getnewmldsaaddress)
MULTISIG=$(aumcoind addmultisigmldsaaddress 2 "[\"$ADDR1\",\"$ADDR2\",\"$ADDR3\"]")

# Fund the multisig address
aumcoind sendtoaddress $MULTISIG 10.0
# Wait for confirmation...

# Get UTXO info
UTXO=$(aumcoind listunspent | jq -r ".[] | select(.address==\"$MULTISIG\")")
TXID=$(echo $UTXO | jq -r ".txid")
VOUT=$(echo $UTXO | jq -r ".vout")
SCRIPT=$(echo $UTXO | jq -r ".scriptPubKey")

# Create unsigned transaction
UNSIGNED=$(aumcoind createmultisigmldsatx $TXID $VOUT $SCRIPT <destination> 9.99)

# Sign with first key (Alice)
SIGNED1=$(aumcoind signmldsatx "$UNSIGNED" "$ADDR1")

# Sign with second key (Bob) - now complete!
SIGNED2=$(aumcoind signmldsatx "$SIGNED1" "$ADDR2")

# Broadcast
aumcoind sendrawtransaction "$SIGNED2"
```

## Success Criteria

1. ✅ Create unsigned transactions from multisig UTXOs
2. ✅ Sign with individual ML-DSA keys
3. ✅ Aggregate M signatures correctly
4. ✅ Validate threshold requirements
5. ✅ Prevent double-signing and wrong-key errors
6. ✅ Complete 2-of-3 and 3-of-5 test scenarios
7. ✅ Transaction broadcasts successfully
8. ✅ All unit tests pass (aim for 10+ tests)

## Milestones

- **Phase 4.3.1**: `createmultisigmldsatx` implementation
- **Phase 4.3.2**: `signmldsatx` implementation  
- **Phase 4.3.3**: Signature aggregation and validation
- **Phase 4.3.4**: Comprehensive testing
- **Phase 4.3.5**: Integration test and documentation

## Performance Considerations

- **Signature Size**: 1952 bytes per ML-DSA-65 signature
- **2-of-3 multisig**: ~4KB scriptSig (2 sigs + redeem script)
- **3-of-5 multisig**: ~6KB scriptSig
- **Verification Time**: ~0.5ms per ML-DSA signature
- **Total overhead**: Acceptable for secure cold storage / DAO scenarios

## Future Enhancements (Post Phase 4)

1. **Partial Signature Export/Import**
   - Export partially signed tx for offline signing
   - Import signatures from air-gapped devices

2. **Hardware Wallet Support**
   - ML-DSA signing on secure hardware
   - USB/Bluetooth communication

3. **Multisig UI/UX**
   - Graphical multisig workflow
   - QR code signature exchange
   - Progress tracking dashboard

4. **Advanced Policies**
   - Time-locked multisig (M-of-N after block height)
   - Weighted signatures (different key priorities)
   - Backup key escrow

---

**Document Version**: 1.0  
**Date**: January 2, 2026  
**Status**: Planning Phase  
**Next**: Implementation of Phase 4.3.1
