//******************************************************************************
//
#include "os.h"
#include "ux.h"
#include "os_io_seproxyhal.h"

#ifndef _GLOBALS_H_
#define _GLOBALS_H_

#define P1_CONFIRM 0x01
#define P1_NON_CONFIRM 0x00
#define P1_FIRST 0x00
#define P1_MORE 0x80

#define FULL_ADDRESS_LENGTH_BIN 32
#define FULL_ADDRESS_LENGTH_TXT (FULL_ADDRESS_LENGTH_BIN * 2 + 1)
#define BIP32_PATH 5
#define PUBLIC_KEY_LENGTH 32
#define TO_SIGN_MESSAGE_LENGTH_BIN 32
#define TO_SIGN_MESSAGE_LENGTH_TXT (TO_SIGN_MESSAGE_LENGTH_BIN * 2 + 1)
#define SIGN_RESULT_LENGTH 64

//******************************************************************************
//
#define DO_GET_APP_CONFIG 0x01
#define DO_GET_PUBLIC_KEY 0x02
#define DO_SIGN_MESSAGE   0x03

#define ACTION_DEPLOY     0x01
#define ACTION_SEND       0x02


#define CLA 0xE0
#define OFFSET_CLA 0
#define OFFSET_INS 1
#define OFFSET_P1 2
#define OFFSET_P2 3
#define OFFSET_LC 4
#define OFFSET_CDATA 5

//******************************************************************************
//
extern ux_state_t ux;
// display stepped screens
extern unsigned int ux_step;
extern unsigned int ux_step_count;

typedef struct internalStorage_t 
{
    uint8_t initialized;
} internalStorage_t;

extern const internalStorage_t N_storage_real;
#define N_storage (*(volatile internalStorage_t*) PIC(&N_storage_real))
#endif

//******************************************************************************
