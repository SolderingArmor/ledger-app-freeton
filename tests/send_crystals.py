#!/usr/bin/env python3

# ==============================================================================
# 
from ledger_utils import *
import argparse

# ==============================================================================
#
parser = argparse.ArgumentParser()
parser.add_argument('--account_number', help="BIP32 account to retrieve. e.g. \"12345\".")
parser.add_argument('--destination',    help="Destination Wallet address."               )
parser.add_argument('--amount',         help="Amount in NANO (!!!) TON Crystals."        )
args = parser.parse_args()

if args.account_number == None:
    print("ERROR!\nSpecify account number!")
    exit()

if args.destination == None:
    print("ERROR!\nSpecify destination wallet address!")
    exit()

if args.amount == None:
    print("ERROR!\nSpecify amount of TON Crystals!")
    exit()

# ==============================================================================
#
accountNum  = int(args.account_number)
destAddress = args.destination
amountTONs  = int(args.amount)

SendTonsToAddress(destAddress, accountNum, amountTONs);

# ==============================================================================