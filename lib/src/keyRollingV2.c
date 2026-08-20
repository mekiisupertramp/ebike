/*
 * keyRollingV2.c
 *
 *  Created on: 11 mars 2023
 *      Author: Mehmed Blazevic
 */

#include <ti/drivers/AESCCM.h>
#include <ti/drivers/cryptoutils/cryptokey/CryptoKeyPlaintext.h>
#include "stdbool.h"
#include "keyRollingV2.h"
#include <ti_drivers_config.h>
#include "cryptoService.h"
#include <ti/drivers/NVS.h>
#include <driverlib/sys_ctrl.h>

static uint8_t keyingMaterial[32] = {};
static uint8_t nonce[] = "dignissim";
static AESCCM_Handle handle;
static uint8_t numKey=0;
static uint64_t cpt=0;

static NVS_Params nvsParams;
static NVS_Handle nvsHandle;

static void cryptoService_charValueChangeCB(uint8_t paramId);

static cryptoServiceCBs_t cryptoServiceCBs =
{
 cryptoService_charValueChangeCB // Simple GATT Characteristic value change callback
};

void keyLoadCrypto()
{
    NVS_read(nvsHandle, 0, keyingMaterial, sizeof(keyingMaterial));
    uint8_t temp[4];
    NVS_read(nvsHandle, sizeof(keyingMaterial), temp, 4);
    numKey=temp[0];
}


void keyInit()
{
    handle = AESCCM_open(CONFIG_AESCCM_0_CONST, NULL);
    CryptoService_AddService();
    CryptoService_RegisterAppCBs(&cryptoServiceCBs);

    NVS_Params_init(&nvsParams);
    nvsHandle=NVS_open(CONFIG_NVS_KEY, &nvsParams);

    keyLoadCrypto();
}


void keySendCommand(uint8_t* paquet,t_keyComand cmd)
{
    uint8_t clear[KEY_SIZE]; //clear message containe commande and 64 bit counter

    clear[0]=cmd;
    clear[1]=numKey;
    for(int l=0;l<8;l++)
    {
        clear[HEADER_SIZE+l]=cpt>>8*l;
    }
    crypte(clear, paquet, paquet+KEY_SIZE); //write crypted paquet and mac in paquet
    cpt++;
}

t_keyComand keyReceiveCommand(uint8_t* paquet)
{
    uint8_t clear[KEY_SIZE]; //clear message containe commande and 64 bit counter
    static uint64_t oldCpt[NB_KEY]={0};

    if(decrypte(clear,paquet,paquet+KEY_SIZE))
    {
        uint8_t numKey=clear[1];
        if(numKey>=NB_KEY)
        {
            return KEY_UNVALIDE;
        }
        uint64_t cpt=0;
        for(int l=0;l<8;l++)
        {
            cpt|=((uint64_t)clear[l+HEADER_SIZE])<<8*l;
        }
        if(cpt>oldCpt[numKey] && cpt<oldCpt[numKey]+KEY_NB_MAX_NOT_RECEIVED) //if the paquet folow a previous paquet with a max of KEY_NB_MAX_NOT_RECEIVED interval paquet

        {
            oldCpt[numKey]=cpt;
            return (t_keyComand)clear[0];
        }
        if(cpt<10 //if key resend one of 10 first paquet 0 reset counter (change key battery but wait next paquet for valide paquet if
                || oldCpt[numKey]==0) //if is the first paquet receive from this key (for exemple bike restart)
        {
            oldCpt[numKey]=cpt;
            return KEY_UNVALIDE;
        }
    }
    return KEY_UNVALIDE;
}

void crypte(uint8_t* input, uint8_t* output, uint8_t* mac)
{

    CryptoKey cryptoKey;
    int_fast16_t encryptionResult;
    if (handle == NULL) {
        // handle error
    }
    CryptoKeyPlaintext_initKey(&cryptoKey, keyingMaterial, sizeof(keyingMaterial));
    AESCCM_Operation operation;
    AESCCM_Operation_init(&operation);
    operation.key               = &cryptoKey;
    operation.aad               = NULL;
    operation.aadLength         = 0;
    operation.input             = input;
    operation.output            = output;
    operation.inputLength       = KEY_SIZE;
    operation.nonce             = nonce;
    operation.nonceLength       = sizeof(nonce);
    operation.mac               = mac;
    operation.macLength         = KEY_MAC_SIZE;
    encryptionResult = AESCCM_oneStepEncrypt(handle, &operation);
    if (encryptionResult != AESCCM_STATUS_SUCCESS) {
        // handle error
    }
}

bool decrypte(uint8_t* output, uint8_t* input, uint8_t* mac)
{
    CryptoKey cryptoKey;
    if (handle == NULL) {
        // handle error
    }
    CryptoKeyPlaintext_initKey(&cryptoKey, keyingMaterial, sizeof(keyingMaterial));
    AESCCM_Operation operation;
    AESCCM_Operation_init(&operation);
    operation.key               = &cryptoKey;
    operation.aad               = NULL;
    operation.aadLength         = 0;
    operation.input             = input;
    operation.output            = output;
    operation.inputLength       = KEY_SIZE;
    operation.nonce             = nonce;
    operation.nonceLength       = sizeof(nonce);
    operation.mac               = mac;
    operation.macLength         = KEY_MAC_SIZE;
    int_fast16_t decryptionResult = AESCCM_oneStepDecrypt(handle, &operation);
    return decryptionResult == AESCCM_STATUS_SUCCESS;
}

static void cryptoService_charValueChangeCB(uint8_t paramId) //if we recive message on ble for boot loader
{
    volatile int_fast16_t ret1= NVS_erase(nvsHandle, 0, 0x2000);
    volatile int_fast16_t ret= NVS_write(nvsHandle, 0, cryptoService_KeyNumKeyVal, 36,0); //+3 for multiple of 4
    NVS_close(nvsHandle);
    keyLoadCrypto();
    //NVS_STATUS_SUCCESS
}
