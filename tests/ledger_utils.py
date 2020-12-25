#!/usr/bin/env python3

# ==============================================================================
import asyncio
import os
import json
import argparse
import logging
import base64
import time
from tonclient.client import TonClient, DEVNET_BASE_URL
from tonclient.types import Abi, DeploySet, CallSet, Signer, AccountForExecutor, StateInitSource, KeyPair,  ParamsOfEncodeMessage, \
                            ClientConfig, ResultOfEncodeMessage, FunctionHeader, ParamsOfAttachSignature, ResultOfAttachSignature, \
                            ParamsOfSendMessage, ResultOfSendMessage, ParamsOfWaitForTransaction, ResultOfProcessMessage
#from tonclient.test.helpers import async_core_client, sync_core_client
from ledgerblue.comm import getDongle
from binascii import unhexlify
import pathlib

# ==============================================================================
# 
WORKCHAIN_ID = 0

INS_GET_APP_CONFIG = 1
INS_GET_PUBLIC_KEY = 2
INS_SIGN_MESSAGE   = 3

ACTION_DEPLOY      = 1
ACTION_SEND        = 2

ROOT_PATH = str(pathlib.Path(__file__).parent.absolute())
TVC_PATH  = ROOT_PATH + '/../wallet/SetcodeMultisigWallet.tvc'
ABI_PATH  = ROOT_PATH + '/../wallet/SetcodeMultisigWallet.abi.json'

# ==============================================================================
# 
dongle = getDongle(False)

client_config = ClientConfig()
client_config.network.server_address = "https://main.ton.dev"
asyncClient   = TonClient(config=client_config)
syncClient    = TonClient(config=client_config, is_core_async=False)
abi           = Abi.from_path(path=ABI_PATH)
with open(TVC_PATH, 'rb') as fp:
        tvc        = base64.b64encode(fp.read()).decode()
        deploy_set = DeploySet(tvc=tvc)

# ==============================================================================
# OperationID - integer, operation ID
# ArgsList    - List of arguments to send to Ledger
#
# returns: Byte array without first byte (which is array size)
#
def SendMessageToLedger(OperationID, ArgsList):
    argsLen = len(bytearray.fromhex(ArgsList))
    apduMessage = "E0" + '{:02x}'.format(OperationID) + "0100" + '{:02x}'.format(argsLen) + ArgsList
    apdu = bytearray.fromhex(apduMessage)
    
    result = dongle.exchange(apdu)
    return result[1:]

# ==============================================================================
# pubKey - HEX string with the Public Key
#
# returns: HEX string with address
#
def ConvertPubKeyToAddress(pubKey):
    signer   = Signer.External(public_key=pubKey)
    call_set = CallSet(function_name='constructor', input={
        'owners':['0x' + pubKey],
        'reqConfirms': 1
    })

    params  = ParamsOfEncodeMessage(abi=abi, signer=signer, deploy_set=deploy_set, call_set=call_set)            
    encoded = asyncClient.abi.encode_message(params=params)
    return encoded.address

# ==============================================================================
# accountNum - Integer with account number
#
# returns: none
#
def DeployWalletAddress(accountNum):

    print("\nPlease confirm Public Key sharing on Ledger device...\n")

    accountNumTxt = '{:08x}'.format(accountNum)
    pubKeyBin     = SendMessageToLedger(INS_GET_PUBLIC_KEY, accountNumTxt)
    pubKeyTxt     = pubKeyBin.hex()
    addressTxt    = ConvertPubKeyToAddress(pubKeyBin.hex())

    header    = FunctionHeader(pubkey=pubKeyTxt, expire=int(time.time()) + 60)
    call_set  = CallSet(function_name='constructor', header=header, input={
        'owners':['0x' + pubKeyTxt],
        'reqConfirms': 1
    })

    signer         = Signer.External(public_key=pubKeyTxt)
    encode_params  = ParamsOfEncodeMessage(abi=abi, signer=signer, address=addressTxt[2:], deploy_set=deploy_set, call_set=call_set)
    encodedMessage = asyncClient.abi.encode_message(params=encode_params)
    dataToSign     = base64.b64decode(encodedMessage.data_to_sign)

    print("\nPlease confirm Deployment of the wallet " + addressTxt + " on Ledger device...\n")

    signatureResult          = SendMessageToLedger(INS_SIGN_MESSAGE, '{:08x}'.format(ACTION_DEPLOY) + accountNumTxt + '{:016x}'.format(0) + dataToSign.hex() + addressTxt[2:])
    attach_signature_params  = ParamsOfAttachSignature(abi=abi, public_key=pubKeyTxt, message=encodedMessage.message, signature=signatureResult.hex())
    attach_signature_message = asyncClient.abi.attach_signature(params=attach_signature_params)

    print("\nPerforming deployment...\n")
    message_params = ParamsOfSendMessage(message=attach_signature_message.message, send_events=False, abi=abi)
    message_result = asyncClient.processing.send_message(params=message_params)

    wait_params    = ParamsOfWaitForTransaction(message=attach_signature_message.message, shard_block_id=message_result.shard_block_id, send_events=False, abi=abi)
    result         = asyncClient.processing.wait_for_transaction(params=wait_params)
    txFailed       = result.transaction['aborted'];

    if txFailed == True:
        print("WARNINRG! Deployment was unsuccessfull!")
    else:
        print("Wallet was deployed, transaction ID: " + result.transaction['id'])


# ==============================================================================
# receiverWalletAddress - HEX string with leading "0:"
# accountNum            - Integer with account number
# amountNanoTons        - Integer with nano Crystals (reminder, 1 Crystal = 1'000'000'000 nano Crystals)
#
# returns: none
#
def SendTonsToAddress(receiverWalletAddress, accountNum, amountNanoTons):
    
    print("\nPlease confirm Public Key sharing on Ledger device...\n")

    accountNumTxt   = '{:08x}'.format(accountNum)
    pubKeyBin       = SendMessageToLedger(INS_GET_PUBLIC_KEY, accountNumTxt)
    pubKeyTxt       = pubKeyBin.hex()
    myWalletAddress = ConvertPubKeyToAddress(pubKeyTxt)    

    header   = FunctionHeader(pubkey=pubKeyTxt, expire=int(time.time()) + 60)
    call_set = CallSet(function_name='submitTransaction', header=header, input={
        'dest': receiverWalletAddress, 
        'value': amountNanoTons, 
        'bounce': False, 
        'allBalance': False, 
        'payload': ''
    })

    signer         = Signer.External(public_key=pubKeyTxt)
    encode_params  = ParamsOfEncodeMessage(abi=abi, signer=signer, address=myWalletAddress, deploy_set=deploy_set, call_set=call_set)
    encodedMessage = asyncClient.abi.encode_message(params=encode_params)
    dataToSign     = base64.b64decode(encodedMessage.data_to_sign)

    print("\nPlease confirm Sending " + str(amountNanoTons) + " Nano Crystals to " + receiverWalletAddress + " address on Ledger device...\n")

    signatureResult          = SendMessageToLedger(INS_SIGN_MESSAGE, '{:08x}'.format(ACTION_SEND) + accountNumTxt + '{:016x}'.format(amountNanoTons) + dataToSign.hex() + receiverWalletAddress[2:])
    attach_signature_params  = ParamsOfAttachSignature(abi=abi, public_key=pubKeyTxt, message=encodedMessage.message, signature=signatureResult.hex())
    attach_signature_message = asyncClient.abi.attach_signature(params=attach_signature_params)

    print("\nPerforming Crystal sending...\n")
    message_params = ParamsOfSendMessage(message=attach_signature_message.message, send_events=False, abi=abi)
    message_result = asyncClient.processing.send_message(params=message_params)

    wait_params    = ParamsOfWaitForTransaction(message=attach_signature_message.message, shard_block_id=message_result.shard_block_id, send_events=False, abi=abi)
    result         = asyncClient.processing.wait_for_transaction(params=wait_params)
    txFailed       = result.transaction['aborted'];

    if txFailed == True:
        print("WARNINRG! Crystal sending was unsuccessfull!")
    else:
        print("Crystals were sent, transaction ID: " + result.transaction['id'])

# ==============================================================================
