//******************************************************************************
//
#include "utils.h"

#define ACCOUNT_ADDRESS_PREFIX 1

//******************************************************************************
//
uint32_t readUint32BE(uint8_t *buffer) 
{
    return (buffer[0] << 24) | 
           (buffer[1] << 16) | 
           (buffer[2] << 8 ) | 
           (buffer[3]);
}

uint64_t readUint64BE(uint8_t *buffer) 
{
  return ((uint64_t)buffer[0] << 56) | 
         ((uint64_t)buffer[1] << 48) | 
         ((uint64_t)buffer[2] << 40) | 
         ((uint64_t)buffer[3] << 32) | 
         ((uint64_t)buffer[4] << 24) | 
         ((uint64_t)buffer[5] << 16) | 
         ((uint64_t)buffer[6] << 8 ) | 
         ((uint64_t)buffer[7]);
}

static const uint32_t HARDENED_OFFSET = 0x80000000;

static const uint32_t derivePath[BIP32_PATH] = {
  44 | HARDENED_OFFSET,
  1234 | HARDENED_OFFSET,
  0 | HARDENED_OFFSET,
  0 | HARDENED_OFFSET,
  0 | HARDENED_OFFSET
};

//******************************************************************************
//
void u64ToString(char *stringOut, const uint64_t value, const uint32_t maxLength)
{
    if(maxLength < 22)
    {
        THROW(EXCEPTION_IO_OVERFLOW);
        return;
    }
    
    char    *b = stringOut;
    uint64_t x = value;

    int count;
    for(count = 0; count < (int)maxLength; count++)
    {
        if(count == 9)
        {
            *b = '.';
        }
        else
        {
            *b = '0' + (x % 10);
            x /= 10;
        }        
        b++;

        if(!x) break;
    }

    // If amount is less than 1.0 Crystal
    while(count <= 9)
    {
        *b = count == 8 ? '.' : '0';
        count++;
        b++;
    }
    *b = '\0';

    // Now reverse the string
    for (int i = 0, j = count; i < j; i++, j--)
	{
		char a = stringOut[i];
		stringOut[i] = stringOut[j];
		stringOut[j] = a;
	}

    // Trim the zeroes at the end
    // P.S. We don't care about validating "i+1" and "i+2" addresses, because maximum length
    //      of uint64_t string representation is 20 (21 with dot in our case)
    //      and we are using 64 byte char arrays
    for(int i = count; i >=0; i--)
    {
        if(stringOut[i] != '0')
        {
            if(stringOut[i] == '.')
            {
                stringOut[i+1] = '0';
                stringOut[i+2] = '\0';
            }
            else
            {
                stringOut[i+1] = '\0';
            }
            
            
            break;
        }
    }
}

//******************************************************************************
//
void binToHex(char *hexOut, const uint8_t *binIn, const int outLen) 
{
    int n = outLen - 1;

    hexOut[n] = '\0';
    const char xx[]= "0123456789abcdef";
    while (--n >= 0)
    {
        hexOut[n] = xx[(binIn[n>>1] >> ((1 - (n&1)) << 2)) & 0xF];
    }   
}

//******************************************************************************
//
void getPublicKey(uint32_t accountNumber, uint8_t *publicKey) 
{
    cx_ecfp_private_key_t _privateKey;
    cx_ecfp_public_key_t  _publicKey;

    getPrivateKey(accountNumber, &_privateKey);
    cx_ecfp_generate_pair(CX_CURVE_Ed25519, &_publicKey, &_privateKey, 1);
    os_memset(&_privateKey, 0, sizeof(_privateKey));

    for (int i = 0; i < 32; i++) {
        publicKey[i] = _publicKey.W[64 - i];
    }
    if ((_publicKey.W[32] & 1) != 0) {
        publicKey[31] |= 0x80;
    }
}

//******************************************************************************
//
void getPrivateKey(uint32_t accountNumber, cx_ecfp_private_key_t *privateKey) 
{
    uint8_t privateKeyData[32];
    uint32_t bip32Path[BIP32_PATH];

    os_memmove(bip32Path, derivePath, sizeof(derivePath));
    bip32Path[2] = accountNumber | HARDENED_OFFSET;
    PRINTF("BIP32: %.*H\n", BIP32_PATH*4, bip32Path);
    os_perso_derive_node_bip32_seed_key(HDW_ED25519_SLIP10, CX_CURVE_Ed25519, bip32Path, BIP32_PATH, privateKeyData, NULL, NULL, 0);
    cx_ecfp_init_private_key(CX_CURVE_Ed25519, privateKeyData, 32, privateKey);
    os_memset(privateKeyData, 0, sizeof(privateKeyData));
}

//******************************************************************************
//
void sendResponse(uint8_t tx, bool approve) {
    G_io_apdu_buffer[tx++] = approve? 0x90 : 0x69;
    G_io_apdu_buffer[tx++] = approve? 0x00 : 0x85;
    // Send back the response, do not restart the event loop
    io_exchange(CHANNEL_APDU | IO_RETURN_AFTER_TX, tx);
    // Display back the original UX
    ui_idle();
}

//******************************************************************************
