/**********************************************************************************************
 * Filename:       appsnippets.c
 *
 * Description:    This file contains snippets needed to utilize the generated services.
 *
 * Copyright (c) 2015-2016, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *************************************************************************************************/

#include <ti/drivers/NVS.h>
#include <mtsc.h>

#include "configService.h"
#include "virtualUartService.h"
#include "config.h"
#include "simple_peripheral.h"
#include "vesc_can.h"
#include "crc16Vesc.h"
#include "timestamp.h"

#define DEFAULT_NAME "Type-A"
#define DEFAULT_BLE_NAME "Motosacoche"
static char nameTab[MAX_NAME_LENGTH+1]=DEFAULT_NAME;
static char nameBLETab[MAX_NAME_LENGTH+1]=DEFAULT_BLE_NAME;
static char vinTab[MAX_VIN_LENGTH+1]="NOT_DEF";
static char rfidTab1[RFID_STRING_LENGTH+1]="NOT_DEF";
static char rfidTab2[RFID_STRING_LENGTH+1]="NOT_DEF";
static char rfidTab3[RFID_STRING_LENGTH+1]="NOT_DEF";
static char rfidTab4[RFID_STRING_LENGTH+1]="NOT_DEF";
static char ccidTab[CCID_LENGTH+1]="NOT_DEF";

//default config
const Mtsc__Config defautlConfig={
    .base=PROTOBUF_C_MESSAGE_INIT (&mtsc__config__descriptor),
    .bikename=nameTab,
    .bikenameble=nameBLETab,
    .vin=vinTab,
    .idrfid1=rfidTab1,
    .idrfid2=rfidTab2,
    .idrfid3=rfidTab3,
    .idrfid4=rfidTab4,
    .ccid=ccidTab,
    .brakingforce=1,.has_brakingforce=1,
    .brakingtrigger=true, .has_brakingtrigger=1,
    .testmode=0,.has_testmode=1,
    .currenttrig=0.1, .has_currenttrig=1,
    .torkcoeff=0.3, .has_torkcoeff=1,
    .torkintcoeff=0.2, .has_torkintcoeff=1,
    .speedcoeff=0.5, .has_speedcoeff=1,
    .ecocurrent=0.12, .has_ecocurrent=1,
    .brakecurrent=30.0, .has_brakecurrent=1,
    .ecotorkslope=0.5, .has_ecotorkslope=1,
    .normtorkslope=1.2, .has_normtorkslope=1,
    .sportorkslope=3.2, .has_sportorkslope=1,
    .maxcurrent=30.0, .has_maxcurrent=1,
    .speedmultiplier=1.0, .has_speedmultiplier=1,
    .tripmultiplier=1.0, .has_tripmultiplier=1,
    .brakingstate=1, .has_brakingstate=1,
    .currpercdegraded=0.2, .has_currpercdegraded=1,
    .tempstartdegraded=200.0, .has_tempstartdegraded=1,
    .tempstopdegraded=200.0, .has_tempstopdegraded=1,
    .temperatureemergency=200.0, .has_temperatureemergency=1,
    .degrtorkecoslope=0.5, .has_degrtorkecoslope=1,
    .degrtorknormslope=1.2, .has_degrtorknormslope=1,
    .degrtorksportslope=3.2, .has_degrtorksportslope=1,
    .degrmaxcurrent=30.0, .has_degrmaxcurrent=1,
    .degrcurrtrig=0.06, .has_degrcurrtrig=1,
    .betafactor=2.0, .has_betafactor=1, // examples: B=1 => min=0 ; B=1.5 => min=0.333 ; B=1.666 => min=0.4 ; B=2 => min=0.5 ; B=3 => min=0.666 ; B=4 => min=0.75
};

volatile Mtsc__Config bikeConfig;

extern uint8 configService_ConfigVal[CONFIGSERVICE_CONFIG_LEN]; //for config characteritique
extern uint32_t configServiceActualLength; //config characteritique length for send bluetooth characteristique with correct length

NVS_Handle nvsHandle; //nvs for store config
NVS_Attrs regionAttrs;
volatile uint64_t inConfigLastTic; //use for virtual uart bridge protocol with vesc timeout

volatile bool flagScanCan=0; //we have receive a vesc scan request

// Declaration of service callback handlers
static void user_configServiceValueChangeCB(uint8_t paramID, uint32_t length); // Callback from the service config

// Service callback function implementation
// ConfigService callback handler. The type configServiceCBs_t is defined in configService.h
static configServiceCBs_t user_configServiceCBs =
{
 user_configServiceValueChangeCB // Characteristic value change callback handler
};

// Declaration of service callback handlers
static void user_virtualUartServiceValueChangeCB(uint8_t* val,int len); // Callback from the service virtual uart (bridge for vesc)

// Service callback function implementation
// ConfigService callback handler. The type configServiceCBs_t is defined in configService.h
static virtualUartServiceCBs_t user_virtualUartServiceCBs =
{
 user_virtualUartServiceValueChangeCB // Characteristic value change callback handler
};

void decodeUart(uint8_t val);
static void decodeConfig(Mtsc__Config* newBikeConfig, bool admin);


void configInit() //initialize config code
{
    bikeConfig=defautlConfig;
    //registre ble service config
    ConfigService_AddService();
    ConfigService_RegisterAppCBs(&user_configServiceCBs);

#ifdef VESC_BRIDGE
    //registre ble service virtual uart
    VirtualUartService_AddService();
    VirtualUartService_RegisterAppCBs(&user_virtualUartServiceCBs);
#endif

    NVS_Params nvsParams;

    NVS_Params_init(&nvsParams); //vns for stor config
    nvsHandle=NVS_open(CONFIG_NVS_CONFIG, &nvsParams);
    NVS_getAttrs(nvsHandle, &regionAttrs);

    while(nvsHandle == NULL);

    uint8_t buffer[CONFIGSERVICE_CONFIG_LEN];//config have a max length of 510 byte + 2byte for length
    NVS_read(nvsHandle, 0, (void*)buffer, sizeof(buffer));

    uint32_t len=buffer[0]<<8|buffer[1];
    if(len<sizeof(buffer)-2) //else invalide data in NVS
    {
        Mtsc__Config* newBikeConfig=mtsc__config__unpack(NULL, len, buffer+2);//2 first byte is length other is protobuff data
        decodeConfig(newBikeConfig,TRUE); //update default config from read config
        mtsc__config__free_unpacked(newBikeConfig, NULL); //free memory
    }

    updateConfigCharacteristique();//update ble characteristique with new config

    SimplePeripheral_changeName((void*)bikeConfig.bikenameble); //rename in ble advertisement
}

static void decodeConfig(Mtsc__Config* newBikeConfig, bool admin) //update config from a new config read from NVS or recive in BLE
{
    /*if(newBikeConfig->has_testmode){
        bikeConfig.testmode=newBikeConfig->testmode;
        simplePeriferalEnableVescAdv();
    }*/
    if(newBikeConfig==NULL)
        return;

    if(newBikeConfig->has_resetconfig && newBikeConfig->resetconfig)
    {
        newBikeConfig->resetconfig=0;
        bikeConfig=defautlConfig;
        return;
    }

    int nameLength=strlen(newBikeConfig->bikename);
    if(nameLength) //if name length = 0 name is not define
    {
        nameLength+=1;//+1 for zero terminal

        memcpy((void*)bikeConfig.bikename, (void*)newBikeConfig->bikename, MIN(MAX_NAME_LENGTH,nameLength));
        if(MAX_NAME_LENGTH>nameLength) //limite name length to max length
            memset((void*)(bikeConfig.bikename+nameLength), ' ', MAX_NAME_LENGTH-nameLength);
        bikeConfig.bikename[MAX_NAME_LENGTH]='\0';
    }
    int nameBLELength=strlen(newBikeConfig->bikenameble);
    if(nameBLELength) //if name length = 0 name is not define
    {
        nameBLELength+=1;//+1 for zero terminal

        memcpy((void*)bikeConfig.bikenameble, (void*)newBikeConfig->bikenameble, MIN(MAX_NAME_LENGTH,nameBLELength));
        if(MAX_NAME_LENGTH>nameBLELength) //limite name length to max length
            memset((void*)(bikeConfig.bikenameble+nameBLELength), ' ', MAX_NAME_LENGTH-nameBLELength);
        bikeConfig.bikenameble[MAX_NAME_LENGTH]='\0';
    }
    int vinLength=strlen(newBikeConfig->vin);
    if(vinLength) //if vinLength length = 0 name is not define
    {
        memcpy((void*)bikeConfig.vin, (void*)newBikeConfig->vin, MIN(MAX_VIN_LENGTH,vinLength));
        if(MAX_NAME_LENGTH>vinLength) //limite vim length to max length
            memset((void*)(bikeConfig.vin+vinLength), ' ', MAX_VIN_LENGTH-vinLength);
        bikeConfig.vin[MAX_VIN_LENGTH]='\0';
    }
    int rfidLength1=strlen(newBikeConfig->idrfid1);
    if(rfidLength1) //if vinLength length = 0 name is not define
    {
        memcpy((void*)bikeConfig.idrfid1, (void*)newBikeConfig->idrfid1, MIN(RFID_STRING_LENGTH,rfidLength1));
        if(RFID_STRING_LENGTH>rfidLength1) //limite vim length to max length
            memset((void*)(bikeConfig.idrfid1+rfidLength1), ' ', RFID_STRING_LENGTH-rfidLength1);
        bikeConfig.idrfid1[RFID_STRING_LENGTH]='\0';
    }
    int rfidLength2=strlen(newBikeConfig->idrfid2);
    if(rfidLength2) //if vinLength length = 0 name is not define
    {
        memcpy((void*)bikeConfig.idrfid2, (void*)newBikeConfig->idrfid2, MIN(RFID_STRING_LENGTH,rfidLength2));
        if(RFID_STRING_LENGTH>rfidLength2) //limite vim length to max length
            memset((void*)(bikeConfig.idrfid2+rfidLength2), ' ', RFID_STRING_LENGTH-rfidLength2);
        bikeConfig.idrfid2[RFID_STRING_LENGTH]='\0';
    }
    int rfidLength3=strlen(newBikeConfig->idrfid3);
    if(rfidLength3) //if vinLength length = 0 name is not define
    {
        memcpy((void*)bikeConfig.idrfid3, (void*)newBikeConfig->idrfid3, MIN(RFID_STRING_LENGTH,rfidLength3));
        if(RFID_STRING_LENGTH>rfidLength3) //limite vim length to max length
            memset((void*)(bikeConfig.idrfid3+rfidLength3), ' ', RFID_STRING_LENGTH-rfidLength3);
        bikeConfig.idrfid3[RFID_STRING_LENGTH]='\0';
    }
    int rfidLength4=strlen(newBikeConfig->idrfid4);
    if(rfidLength4) //if vinLength length = 0 name is not define
    {
        memcpy((void*)bikeConfig.idrfid4, (void*)newBikeConfig->idrfid4, MIN(RFID_STRING_LENGTH,rfidLength4));
        if(RFID_STRING_LENGTH>rfidLength4) //limite vim length to max length
            memset((void*)(bikeConfig.idrfid4+rfidLength4), ' ', RFID_STRING_LENGTH-rfidLength4);
        bikeConfig.idrfid4[RFID_STRING_LENGTH]='\0';
    }
    int ccidLength=strlen(newBikeConfig->ccid);
    if(ccidLength) //if vinLength length = 0 name is not define
    {
        memcpy((void*)bikeConfig.ccid, (void*)newBikeConfig->ccid, MIN(CCID_LENGTH,ccidLength));
        if(CCID_LENGTH>ccidLength) //limite vim length to max length
            memset((void*)(bikeConfig.ccid+ccidLength), ' ', CCID_LENGTH-ccidLength);
        bikeConfig.ccid[CCID_LENGTH]='\0';
    }
    if(newBikeConfig->has_brakingforce)
        bikeConfig.brakingforce = newBikeConfig->brakingforce;
    if(newBikeConfig->has_brakingtrigger)
        bikeConfig.brakingtrigger = newBikeConfig->brakingtrigger;
    if(newBikeConfig->has_testmode){
        bikeConfig.testmode = newBikeConfig->testmode;
        simplePeriferalEnableVescAdv();
    }
    if(newBikeConfig->has_currenttrig)
        bikeConfig.currenttrig = newBikeConfig->currenttrig;
    if(newBikeConfig->has_torkcoeff)
        bikeConfig.torkcoeff = newBikeConfig->torkcoeff;
    if(newBikeConfig->has_torkintcoeff)
        bikeConfig.torkintcoeff = newBikeConfig->torkintcoeff;
    if(newBikeConfig->has_speedcoeff)
        bikeConfig.speedcoeff = newBikeConfig->speedcoeff;
    if(newBikeConfig->has_ecocurrent)
        bikeConfig.ecocurrent = newBikeConfig->ecocurrent;
    if(newBikeConfig->has_brakecurrent)
        bikeConfig.brakecurrent = newBikeConfig->brakecurrent;
    if(newBikeConfig->has_ecotorkslope)
        bikeConfig.ecotorkslope = newBikeConfig->ecotorkslope;
    if(newBikeConfig->has_normtorkslope)
        bikeConfig.normtorkslope = newBikeConfig->normtorkslope;
    if(newBikeConfig->has_sportorkslope)
        bikeConfig.sportorkslope = newBikeConfig->sportorkslope;
    if(newBikeConfig->has_maxcurrent)
        bikeConfig.maxcurrent = newBikeConfig->maxcurrent;
    if(newBikeConfig->has_speedmultiplier)
        bikeConfig.speedmultiplier = newBikeConfig->speedmultiplier;
    if(newBikeConfig->has_tripmultiplier)
        bikeConfig.tripmultiplier = newBikeConfig->tripmultiplier;
    if(newBikeConfig->has_brakingstate)
        bikeConfig.brakingstate = newBikeConfig->brakingstate;
    if(newBikeConfig->has_currpercdegraded)
        bikeConfig.currpercdegraded = newBikeConfig->currpercdegraded;
    if(newBikeConfig->has_tempstartdegraded)
        bikeConfig.tempstartdegraded = newBikeConfig->tempstartdegraded;
    if(newBikeConfig->has_tempstopdegraded)
        bikeConfig.tempstopdegraded = newBikeConfig->tempstopdegraded;
    if(newBikeConfig->has_temperatureemergency)
        bikeConfig.temperatureemergency = newBikeConfig->temperatureemergency;
    if(newBikeConfig->has_degrtorkecoslope)
        bikeConfig.degrtorkecoslope = newBikeConfig->degrtorkecoslope;
    if(newBikeConfig->has_degrtorknormslope)
        bikeConfig.degrtorknormslope = newBikeConfig->degrtorknormslope;
    if(newBikeConfig->has_degrtorksportslope)
        bikeConfig.degrtorksportslope = newBikeConfig->degrtorksportslope;
    if(newBikeConfig->has_degrmaxcurrent)
        bikeConfig.degrmaxcurrent = newBikeConfig->degrmaxcurrent;
    if(newBikeConfig->has_degrcurrtrig)
        bikeConfig.degrcurrtrig = newBikeConfig->degrcurrtrig;
    if(newBikeConfig->has_betafactor)
        bikeConfig.betafactor = newBikeConfig->betafactor;
}


static void user_configServiceValueChangeCB(uint8_t paramID, uint32_t length) //receive new config on ble
{
    switch (paramID)
    {
        case CONFIGSERVICE_CONFIG:
        {
            Mtsc__Config* newWriteBikeConfig=mtsc__config__unpack(NULL, length, configService_ConfigVal);

            decodeConfig(newWriteBikeConfig,TRUE);
            saveBikeConfig();

            updateConfigCharacteristique();
            SimplePeripheral_changeName((char*)bikeConfig.bikenameble);

            mtsc__config__free_unpacked(newWriteBikeConfig, NULL);

            break;
        }
    }
}

static void user_virtualUartServiceValueChangeCB(uint8_t* buffer,int len) //receive data un virtual uart (bridge from ble to vesc tool) cesc toll send to ble virtual uart LB forward on CAN to vesc
{
    for(int l=0;l<len;l++)
    {
        decodeUart(buffer[l]);
    }
}

typedef enum
{
    RXS_WAIT,
    RXS_LENGTH_SHORT,
    RXS_LENGTH_LONG,
    RXS_DATA,
    //RXS_CRC, take in data
    RXS_STOP
} uartRxStatus;

void configCompute()
{
#ifdef VESC_BRIDGE

    if(flagScanCan) //if we have receive a scanCan on virtual uart (scanCan is send by vesc tool for show devices on can)
    {
        flagScanCan=0;
        uint8_t reply[]={2,2,COMM_PING_CAN,1,0,0,3}; //no a real scan we only reply we have on device on CAN with ID 1
        uint16_t crc=crc16Vesc(reply+2, 2);
        reply[sizeof(reply)-3]=crc>>8;
        reply[sizeof(reply)-2]=crc;
        VirtualUartService_SetParameter(VIRTUALUARTSERVICE_RX , sizeof(reply), reply); //send reply on virtual can
    }
#endif
}

#ifdef VESC_BRIDGE

void decodeUart(uint8_t val) //decode uart vesc protocol and forward it to CAN
{

    if(bikeConfig.testmode==0) //autorize config of vesc only in test mode protect reconfig from landa user
    {
        return;
    }

    static uartRxStatus statusRx=RXS_WAIT;
    static uint16_t length=0;
    static uint16_t count=0;
    static uint8_t buffer[PACKET_BUFFER_LEN];
    if(inConfigLastTic>getTimeStampMs()+3000) //timout 3 seconde reset protocol
    {
        statusRx=RXS_WAIT;
    }
    inConfigLastTic=getTimeStampMs();
    switch(statusRx)
    {
    case RXS_WAIT:
        count=0;
        if(val == 2)
            statusRx=RXS_LENGTH_SHORT;
        else if(val == 3)
            statusRx=RXS_LENGTH_LONG;
        break;
    case RXS_LENGTH_LONG:
        if(count==0)
        {
            length=val<<8;
            count=1;
        }
        else
        {
            length|=val;
            statusRx=RXS_DATA;
            count=0;
            while(length>PACKET_MAX_PL_LEN);
        }
        break;
    case RXS_LENGTH_SHORT:
        length=val;
        count=0;
        statusRx=RXS_DATA;
        break;
    case RXS_DATA:
        if(count<PACKET_MAX_PL_LEN)
            buffer[count]=val;
        else
            while(1);
        count++;
        if(count==length+2)
        {
            statusRx=RXS_STOP;
        }
        break;
        /*case RXS_CRC:
        if(count==length)
        {
            buffer[count]=val;
            count=1;
        }
        else
        {
            buffer[count]=val;
            statusRx=RXS_WAIT;


        }
        break;*/
    case RXS_STOP:

        if(val==3)
        {

            int canId=0;
            if(buffer[0]==COMM_PING_CAN)
            {
                flagScanCan=1;
            }
            else
            {
                if(buffer[0]==COMM_FORWARD_CAN && length>=3)
                {
                    canId=buffer[1];
                    length-=2;

                    for(int l=0;l<length;l++)//-2 do not copy crc
                    {
                        buffer[l]=buffer[l+2];
                    }
                    uint16_t crc=crc16Vesc(buffer, length);
                    buffer[length]=crc>>8;
                    buffer[length+1]=crc;
                }
                if(length<=6)
                {
                    uint8_t bufferCan[8];
                    bufferCan[0] = 254; //idVesc
                    bufferCan[1] = 0; //for commands_process_packet
                    memcpy(bufferCan+2, buffer, length);
                    comm_can_transmit_eid_replace(canId |
                                                  ((uint32_t)CAN_PACKET_PROCESS_SHORT_BUFFER << 8), bufferCan, length+2, 1);
                }
                else
                {
                    int ptBuffer;
                    uint8_t bufferCan[8];

                    //for(volatile int l=0;l<1000;l++);

                    int len;
                    for(ptBuffer=0;ptBuffer<length;)
                    {
                        if(ptBuffer<255)
                        {
                            bufferCan[0]=ptBuffer;
                            len=MIN(8,length-ptBuffer+1);//+ 1 for header
                            memcpy(bufferCan+1, buffer+ptBuffer, len-1);
                            comm_can_transmit_eid_replace(canId |
                                                          ((uint32_t)CAN_PACKET_FILL_RX_BUFFER << 8), bufferCan, len, 1);
                            ptBuffer+=len-1;
                        }
                        else
                        {
                            bufferCan[0]=ptBuffer>>8;
                            bufferCan[1]=ptBuffer;
                            len=MIN(8,length-ptBuffer+2);//+2 for header
                            memcpy(bufferCan+2, buffer+ptBuffer, len-2);
                            comm_can_transmit_eid_replace(canId |
                                                          ((uint32_t)CAN_PACKET_FILL_RX_BUFFER_LONG << 8), bufferCan, len, 1);
                            ptBuffer+=len-2;
                        }
                    }

                    uint32_t inde = 0;
                    bufferCan[inde++] = 254;
                    bufferCan[inde++] = 0;
                    bufferCan[inde++] = length >> 8;
                    bufferCan[inde++] = length & 0xFF;
                    bufferCan[inde++] = buffer[ptBuffer];
                    bufferCan[inde++] = buffer[ptBuffer+1];

                    comm_can_transmit_eid_replace(canId |
                                                  ((uint32_t)CAN_PACKET_PROCESS_RX_BUFFER << 8), bufferCan, 6, 1);
                }
            }
        }
        statusRx=RXS_WAIT;

        break;
    }
}
#endif

void updateConfigCharacteristique() //update ble characteristique with new config
{
    uint32_t len = mtsc__config__get_packed_size((const Mtsc__Config*)&bikeConfig);
    if(len>CONFIGSERVICE_CONFIG_LEN)
    {
        return;
    }
    configServiceActualLength=len;
    mtsc__config__pack((const Mtsc__Config*)&bikeConfig,configService_ConfigVal);

}

void eraseConfig() //remove config from NVS (reset config)
{
    bikeConfig=defautlConfig;
    memcpy(nameTab,DEFAULT_NAME,sizeof(DEFAULT_NAME));
    memcpy(nameBLETab,DEFAULT_BLE_NAME,sizeof(DEFAULT_BLE_NAME));
    bikeConfig.vin=vinTab;//restor vin in config
    saveBikeConfig();
}

void saveBikeConfig() //save config in NVS
{

    uint32_t len = mtsc__config__get_packed_size((const Mtsc__Config*)&bikeConfig);
    if(len>CONFIGSERVICE_CONFIG_LEN-2)
    {
        return;
    }

    uint8_t buf[CONFIGSERVICE_CONFIG_LEN]={len>>8,len};
    mtsc__config__pack((const Mtsc__Config*)&bikeConfig,buf+2);
    NVS_erase(nvsHandle, 0, regionAttrs.sectorSize);
    NVS_write(nvsHandle, 0, (void*)buf, sizeof(buf),0);
}
