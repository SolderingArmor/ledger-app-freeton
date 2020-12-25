//******************************************************************************
//
#include "os.h"
#include "cx.h"
#include "globals.h"
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include "menu.h"

#ifndef _UTILS_H_
#define _UTILS_H_

//******************************************************************************
//
void     u64ToString  (char *stringOut, const uint64_t value, const uint32_t maxLength);
void     binToHex     (char *hexOut,    const uint8_t *binIn, const int outLen);

void     getPublicKey (uint32_t accountNumber, uint8_t *publicKey);
void     getPrivateKey(uint32_t accountNumber, cx_ecfp_private_key_t *privateKey);

uint32_t readUint32BE (uint8_t *buffer);
uint64_t readUint64BE (uint8_t *buffer);
void     sendResponse (uint8_t tx, bool approve);

//******************************************************************************
//
#endif

//******************************************************************************
