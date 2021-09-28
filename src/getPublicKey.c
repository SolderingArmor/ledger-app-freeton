//******************************************************************************
//
#include "getPublicKey.h"

static uint8_t  publicKeyBin[FULL_ADDRESS_LENGTH_BIN];
static char     publicKeyTxt[FULL_ADDRESS_LENGTH_TXT];
static uint32_t accountNum;

//******************************************************************************
//
static uint8_t set_result_get_public_key() 
{
    uint8_t tx = 0;
    G_io_apdu_buffer[tx++] = FULL_ADDRESS_LENGTH_BIN;
    os_memmove(G_io_apdu_buffer + tx, publicKeyBin, FULL_ADDRESS_LENGTH_BIN);
    tx += FULL_ADDRESS_LENGTH_BIN;
    return tx;
}

//******************************************************************************
// Get Public Key transaction
UX_STEP_NOCB(ux_display_PKEY_flow_1_step, pnn,                                                  { &C_icon_warning,     "Share",   "Public Key?", });
UX_STEP_CB  (ux_display_PKEY_flow_2_step, pbb, sendResponse(0, false),                          { &C_icon_crossmark,   "Cancel",  "share",       });
UX_STEP_CB  (ux_display_PKEY_flow_3_step, pbb, sendResponse(set_result_get_public_key(), true), { &C_icon_validate_14, "Confirm", "share",       });

//******************************************************************************
//
UX_FLOW(ux_display_PKEY_flow,
  &ux_display_PKEY_flow_1_step,
  &ux_display_PKEY_flow_2_step,
  &ux_display_PKEY_flow_3_step
);

//******************************************************************************
//
void handleGetPublicKey(uint8_t p1, uint8_t p2, uint8_t *dataBuffer, uint16_t dataLength, volatile unsigned int *flags, volatile unsigned int *tx) 
{
    int expectedLength = sizeof(accountNum);
    if(dataLength != expectedLength)
    {
        PRINTF("Error! %s: %d\n", __FILE__, __LINE__); 
        THROW(EXCEPTION_SECURITY);
        return;
    }
    
    accountNum = readUint32BE(dataBuffer);

    getPublicKey(accountNum,   publicKeyBin);
    binToHex    (publicKeyTxt, publicKeyBin, FULL_ADDRESS_LENGTH_TXT);

    if (p1 == P1_NON_CONFIRM) {
        *tx = set_result_get_public_key();
        THROW(0x9000);
    } else {
        ux_flow_init(0, ux_display_PKEY_flow, NULL);
        *flags |= IO_ASYNCH_REPLY;
    }
}

//******************************************************************************
