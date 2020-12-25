#!/usr/bin/env python3

# ==============================================================================
# 
from ledger_utils import *
import argparse

# ==============================================================================
#
parser = argparse.ArgumentParser()
parser.add_argument('--account_number', help="BIP32 account to retrieve. e.g. \"12345\".")
args = parser.parse_args()

if args.account_number == None:
    print("ERROR!\nSpecify account number!")
    exit()

# ==============================================================================
#
account = '{:08x}'.format(int(args.account_number))

print("\nPlease confirm Public Key sharing on Ledger device...\n")

pubKey  = SendMessageToLedger(INS_GET_PUBLIC_KEY, account)
address = ConvertPubKeyToAddress(pubKey.hex())

print("Wallet Address: " + address + "\n")

# ==============================================================================
