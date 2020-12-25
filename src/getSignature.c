//******************************************************************************
//
#include "getSignature.h"

static uint8_t  addressBin      [FULL_ADDRESS_LENGTH_BIN];
static char     addressTxt      [FULL_ADDRESS_LENGTH_TXT];
static uint8_t  toSignMessageBin[TO_SIGN_MESSAGE_LENGTH_BIN];
static char     toSignMessageTxt[TO_SIGN_MESSAGE_LENGTH_TXT];
static uint8_t  signResultBin   [SIGN_RESULT_LENGTH];
static uint32_t accountNum;
static uint32_t actionNum;
static uint64_t amountTon;
static char     amountTonTxt[65];

//******************************************************************************
//
static uint8_t set_result_get_signature() 
{
    cx_ecfp_private_key_t privateKey;
    getPrivateKey(accountNum, &privateKey);
    cx_eddsa_sign(&privateKey, CX_LAST, CX_SHA512, toSignMessageBin, TO_SIGN_MESSAGE_LENGTH_BIN, NULL, 0, signResultBin, SIGN_RESULT_LENGTH, NULL);
    os_memset(&privateKey, 0, sizeof(privateKey));

    uint8_t tx = 0;
    G_io_apdu_buffer[tx++] = SIGN_RESULT_LENGTH;
    os_memmove(G_io_apdu_buffer + tx, signResultBin, SIGN_RESULT_LENGTH);
    tx += SIGN_RESULT_LENGTH;
    return tx;
}

//******************************************************************************
// Deploy wallet transaction
UX_STEP_NOCB(ux_display_DEPLOY_flow_1_step, pnn,                                                 { &C_icon_certificate, "Deploy",  "wallet?",     });
UX_STEP_NOCB(ux_display_DEPLOY_flow_2_step, bnnn_paging,                                         { .title = "Wallet:", .text = addressTxt,        });
UX_STEP_CB  (ux_display_DEPLOY_flow_3_step, pbb, sendResponse(0, false),                         { &C_icon_crossmark,   "Cancel",  "deployment",  });
UX_STEP_CB  (ux_display_DEPLOY_flow_4_step, pbb, sendResponse(set_result_get_signature(), true), { &C_icon_validate_14, "Confirm", "deployment",  });

// Send TONs transaction
UX_STEP_NOCB(ux_display_SEND_flow_1_step, pnn,                                                   { &C_icon_certificate, "Sign",  "transaction?",  });
UX_STEP_NOCB(ux_display_SEND_flow_2_step, bnnn_paging,                                           { .title = "Amount (TON):", .text = amountTonTxt,});
UX_STEP_NOCB(ux_display_SEND_flow_3_step, bnnn_paging,                                           { .title = "Receiver:", .text = addressTxt,      });
UX_STEP_CB  (ux_display_SEND_flow_4_step, pbb, sendResponse(0, false),                           { &C_icon_crossmark,   "Cancel",  "transaction", });
UX_STEP_CB  (ux_display_SEND_flow_5_step, pbb, sendResponse(set_result_get_signature(), true),   { &C_icon_validate_14, "Confirm", "transaction", });

//******************************************************************************
//
UX_FLOW(ux_display_DEPLOY_flow,
    &ux_display_DEPLOY_flow_1_step,
    &ux_display_DEPLOY_flow_2_step,
    &ux_display_DEPLOY_flow_3_step,
    &ux_display_DEPLOY_flow_4_step
);

UX_FLOW(ux_display_SEND_flow,
    &ux_display_SEND_flow_1_step,
    &ux_display_SEND_flow_2_step,
    &ux_display_SEND_flow_3_step,
    &ux_display_SEND_flow_4_step,
    &ux_display_SEND_flow_5_step
);


//******************************************************************************
//
void handleGetSignature(uint8_t p1, uint8_t p2, uint8_t *dataBuffer, uint16_t dataLength, volatile unsigned int *flags, volatile unsigned int *tx) 
{
    int expectedLength = sizeof(actionNum)        // 1
                       + sizeof(accountNum)       // 2
                       + sizeof(amountTon)        // 3
                       + sizeof(toSignMessageBin) // 4
                       + sizeof(addressBin);      // 5
    
    if(dataLength != expectedLength)
    {
        PRINTF("Error! %s: %d\n", __FILE__, __LINE__); 
        THROW(EXCEPTION_SECURITY);
        return;
    }
  
    uint32_t size1 = sizeof(actionNum);
    uint32_t size2 = size1 + sizeof(accountNum);
    uint32_t size3 = size2 + sizeof(amountTon );
    uint32_t size4 = size3 + FULL_ADDRESS_LENGTH_BIN;

    actionNum  = readUint32BE(dataBuffer);
    accountNum = readUint32BE(dataBuffer + size1);
    amountTon  = readUint64BE(dataBuffer + size2);

    memcpy     (toSignMessageBin, dataBuffer + size3, TO_SIGN_MESSAGE_LENGTH_BIN);
    memcpy     (addressBin,       dataBuffer + size4, FULL_ADDRESS_LENGTH_BIN   );    
    
    u64ToString(amountTonTxt,     amountTon,          sizeof(amountTonTxt)      );
    binToHex   (toSignMessageTxt, toSignMessageBin,   TO_SIGN_MESSAGE_LENGTH_TXT);
    binToHex   (addressTxt,       addressBin,         FULL_ADDRESS_LENGTH_TXT   );

    switch(actionNum)
    {
        case ACTION_DEPLOY:  ux_flow_init(0, ux_display_DEPLOY_flow, NULL);  break;
        case ACTION_SEND:    ux_flow_init(0, ux_display_SEND_flow,   NULL);  break;
    }
    *flags |= IO_ASYNCH_REPLY;
}

//******************************************************************************
