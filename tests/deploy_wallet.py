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
accountNum = int(args.account_number)

DeployWalletAddress(accountNum)

# ==============================================================================