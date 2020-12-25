# Ledger Free TON app

## Overview
This app is a Free Ton app for a Nano S/X app (based on boilerplate app).
App has minimal footprint, performs Public Key generation and payload signing on the Ledger device.

## Building and installing
To build and install the app on your Ledger Nano S you must set up the Ledger Nano S build environments. Please follow the Getting Started instructions at [here](https://ledger.readthedocs.io/en/latest/userspace/getting_started.html).

If you don't want to setup a global environnment, you can also setup one just for this app by sourcing `prepare-devenv.sh` with the right target (`s` or `x`).

install prerequisite and switch to a Nano X dev-env:

```bash
sudo apt install python3-venv python3-dev libudev-dev libusb-1.0-0-dev

# (x or s, depending on your device)
source prepare-devenv.sh s
pip install ton-client-py
```

Compile and load the app onto the device:
```bash
make load
```

Refresh the repo (required after Makefile edits):
```bash
make clean
```

Remove the app from the device:
```bash
make delete
```


## Example of Ledger Free Ton wallet functionality

Test functionality (typical workflow):

(Don't forget to open "Free Ton" app on the Ledger device before running commands!)

```bash
# (x or s, depending on your device)
source prepare-devenv.sh s

# Getting public key (not actually needed for the workflow)
python tests/get_public_key.py --account_number=12345

# 1. Get wallet address
python tests/get_address.py --account_number=12345

# 2. Use the given wallet address to send some Crystals to it
# ...

# 3. When wallet has Crystals, perform activation/deployment
python tests/deploy_wallet.py --account_number=12345

# 4. After that we can start spending Crystals from this wallet
python tests/send_crystals.py --account_number=12345 --destination=0:cba39007bdb0f025aac0609b25e96a7d2153f06d22fa47b5f6c26cf756b8b2d6 --amount=100000001
```

## Tips
Be sure to use v1.4.0.1 "ton-client-py" python package, v1.3.* versions are unsupported.
If you see errors in python, check this first.

C build outputs warnings, but they are from Ledger SDK, didn't want to touch that.

## Documentation
This follows the specification available in the [`api.asc`](https://github.com/SolderingArmor/ledger-app-freeton/blob/master/doc/api.asc).
