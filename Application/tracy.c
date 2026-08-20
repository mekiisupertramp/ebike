/*
 * tracy.c
 *
 *  Created on: 24 Jan 2023
 *      Author: mehmedblazevic
 */

#include "tracy.h"
#include "datasService.h"
#include <ti/drivers/NVS.h>
#include "simple_peripheral.h"
#include "config.h"

const Mtsc__Datas defautlDatas={
    .base=PROTOBUF_C_MESSAGE_INIT (&mtsc__datas__descriptor),
    .powermode=0, .has_powermode=1,
    .assistmode=MTSC__ASSIST_MODES__AM_NORMAL, .has_assistmode=1,
    .minutes=21, .has_minutes=1,
    .hours=22, .has_hours=1,
    .weath=MTSC__WEATHER__WE_SUN, .has_weath=1,
    .odo=0, .has_odo=1,
    .tripa=0, .has_tripa=1,
    .tripb=0, .has_tripb=1,
    .soc=1,.has_soc=1,
    .erpm=0, .has_erpm=1,
    //.trackerPresent=0, .has_trackerPresent=1,
};

volatile Mtsc__Datas bikeDatas;
static char rfid[RFID_ID_LENGTH] = {0}; // fixme check if {0} set zeros everywhere

bool assistChanged=0;
bool powerChanged=0;
bool dateChanged=0;
bool rfidReceived=0;
bool ccidReceived=0;
bool tracyConnected=0;
bool resetOdo=0;

// Declaration of service callback handlers
static void user_datasServiceValueChangeCB(uint8_t paramID, uint32_t length); // Callback from the service config
extern uint8 datasService_DatasVal[DATASSERVICE_DATAS_LEN]; //for config characteritique
extern uint32_t datasServiceActualLength; //config characteritique length for send bluetooth characteristique with correct length

// Service callback function implementation
// ConfigService callback handler. The type configServiceCBs_t is defined in configService.h
static datasServiceCBs_t user_datasServiceCBs =
{
 user_datasServiceValueChangeCB // Characteristic value change callback handler
};

NVS_Handle nvsHandle; //nvs for store datas
NVS_Attrs regionAttrs;

void tracy_init(){
    bikeDatas=defautlDatas;
    DatasService_RegisterAppCBs(&user_datasServiceCBs);
	canRegistreCallBack(0,tracy_manage_comm);

    updateDatasCharacteristique();
}
static void decodeDatas(Mtsc__Datas* newBikeDatas) //update config from a new config read from NVS or recive in BLE
{
    if(newBikeDatas==NULL)
        return;

    if(newBikeDatas->has_powermode)
        bikeDatas.powermode = newBikeDatas->powermode;
    if(newBikeDatas->has_assistmode)
        bikeDatas.assistmode = newBikeDatas->assistmode;
    if(newBikeDatas->has_minutes)
        bikeDatas.minutes = newBikeDatas->minutes;
    if(newBikeDatas->has_hours)
        bikeDatas.hours = newBikeDatas->hours;
    if(newBikeDatas->has_weath)
        bikeDatas.weath = newBikeDatas->weath;
    if(newBikeDatas->has_odo)
        bikeDatas.odo = newBikeDatas->odo;
    if(newBikeDatas->has_tripa)
        bikeDatas.tripa = newBikeDatas->tripa;
    if(newBikeDatas->has_tripb)
        bikeDatas.tripb = newBikeDatas->tripb;
    if(newBikeDatas->has_soc)
        bikeDatas.soc = newBikeDatas->soc;
    if(newBikeDatas->has_erpm)
        bikeDatas.erpm = newBikeDatas->erpm;
//    if(newBikeDatas->has_trackerPresent)
//            bikeDatas.trackerPresent = newBikeDatas->trackerPresent;

}
static void user_datasServiceValueChangeCB(uint8_t paramID, uint32_t length) //receive new datas on ble
{
    if(length>20)
    {
        volatile int test;
        test++;
    }
    switch (paramID)
    {
        case DATASSERVICE_DATAS:
        {
            Mtsc__Datas* newWriteBikeDatas=mtsc__datas__unpack(NULL, length, datasService_DatasVal);
            Mtsc__AssistModes bAsMode = bikeDatas.assistmode;
            bool bPoMode = bikeDatas.powermode;
            uint32_t min = bikeDatas.minutes;
            uint32_t hou = bikeDatas.hours;

            decodeDatas(newWriteBikeDatas);

            if(bAsMode != bikeDatas.assistmode){
                assistChanged=1;
            }
            if(bPoMode != bikeDatas.powermode){
                powerChanged=1;
                if(bikeDatas.powermode==false){
                    bikeDatas.minutes=0;
                    bikeDatas.hours=0;
                }
            }
            if((min != bikeDatas.minutes) || (hou != bikeDatas.hours)){
                //dateChanged=1; // enable this line for showing the hours
            }
            if((min==66)&&(hou==66)){
                resetOdo=1;
            }

           // saveBikeDatas();
            updateDatasCharacteristique();

            mtsc__datas__free_unpacked(newWriteBikeDatas, NULL);

            break;
        }
    }
}
void updateDatasCharacteristique(){
    uint32_t len = mtsc__datas__get_packed_size((const Mtsc__Datas*)&bikeDatas);
    if(len>DATASSERVICE_DATAS_LEN){
        return;
    }
    datasServiceActualLength=len;
    mtsc__datas__pack((const Mtsc__Datas*)&bikeDatas, datasService_DatasVal);
}

void saveBikeDatas() //save config in NVS
{
    uint32_t len = mtsc__datas__get_packed_size((const Mtsc__Datas*)&bikeDatas);
    if(len>DATASSERVICE_DATAS_LEN-2)
    {
        return;
    }

    uint8_t buf[DATASSERVICE_DATAS_LEN]={len<<8,len};
    mtsc__datas__pack((const Mtsc__Datas*)&bikeDatas,buf+2);
    NVS_erase(nvsHandle, 0, regionAttrs.sectorSize);
    NVS_write(nvsHandle, 0, (void*)buf, sizeof(buf),0);
}

//void eraseBikeDatas();
void convStrToHexArray(char* hexArray, char* strArray, uint8_t n){
    char buff[3] = {0};
    int i=0;
    for(i=0 ; i<n ; i++){
        if(strArray[i*2]==0 || strArray[i*2+1]==0) //protect vs to short array
            break;
        buff[0] = strArray[i*2];
        buff[1] = strArray[i*2+1];
        hexArray[i] = strtol(buff, NULL, 16);
    }
}

bool isRfidOK(){
    //char buff[RFID_ID_LENGTH] = {0};
    //convStrToHexArray(buff, bikeConfig.idrfid1, RFID_ID_LENGTH);
    if(memcmp(bikeConfig.idrfid1,rfid,RFID_ID_LENGTH-1)==0){
        return true;
    }
   // convStrToHexArray(buff, bikeConfig.idrfid2, RFID_ID_LENGTH);
    if(memcmp(bikeConfig.idrfid2,rfid,RFID_ID_LENGTH-1)==0){
        return true;
    }
    //convStrToHexArray(buff, bikeConfig.idrfid3, RFID_ID_LENGTH);
    if(memcmp(bikeConfig.idrfid3,rfid,RFID_ID_LENGTH-1)==0){
        return true;
    }
    //convStrToHexArray(buff, bikeConfig.idrfid4, RFID_ID_LENGTH);
    if(memcmp(bikeConfig.idrfid4,rfid,RFID_ID_LENGTH-1)==0){
        return true;
    }
    return false;
}
bool isTracyConnected(){
//    bikeDatas.trackerPresent=1;
    return tracyConnected;
}
void resetTracyConn(){
    tracyConnected=false;
    rfidReceived=false;
}
void requestCcid(){
    can_t msg;
    msg.id = GET_DEV_CCID1;
    msg.flags.rtr=0;
    msg.flags.extended=0;
    msg.length=1;
    msg.data[0]=0;
    canSendMessage(&msg);
}
void requestPing(){
    can_t msg;
    msg.id = GET_PING;
    msg.flags.rtr=0;
    msg.flags.extended=0;
    msg.length=1;
    msg.data[0]=0;
    canSendMessage(&msg);
}
void sleepyy(int ms){
    while(ms>0){
        ClockP_usleep(1000);
        ms--;
    }
}
void tracy_manage_comm(can_t msg){
    can_t resp;
    switch(msg.id){
        case GET_PING_ACK:
            tracyConnected = true;
        break;
        case SEND_ID_1:
            memcpy(rfid, msg.data, 8);
            resp.id = SEND_ID_1_ACK;
            resp.data[0] = 0x00;
            resp.length = 1;
            resp.flags.extended=0;
            resp.flags.rtr=0;
            sleepyy(100);
            canSendMessage(&resp);
        break;
        case SEND_ID_2:
            memcpy(&rfid[8],msg.data,3);
            resp.id = SEND_ID_2_ACK;
            resp.data[0] = 0x00;
            resp.length = 1;
            resp.flags.extended=0;
            resp.flags.rtr=0;
            canSendMessage(&resp);
            rfidReceived=true;
        break;
        case GET_DEV_CCID_ACK1:
            memcpy(bikeConfig.ccid, msg.data, 8);
            resp.id = GET_DEV_CCID2;
            resp.data[0] = 0x00;
            resp.length = 1;
            resp.flags.extended=0;
            resp.flags.rtr=0;
            sleepyy(100);
            canSendMessage(&resp);
        break;
        case GET_DEV_CCID_ACK2:
            memcpy(&bikeConfig.ccid[8], msg.data, 8);
            resp.id = GET_DEV_CCID3;
            resp.data[0] = 0x00;
            resp.length = 1;
            resp.flags.extended=0;
            resp.flags.rtr=0;
            sleepyy(100);
            canSendMessage(&resp);
        break;
        case GET_DEV_CCID_ACK3:
            memcpy(&bikeConfig.ccid[16], msg.data, 4);
            saveBikeConfig();
            updateConfigCharacteristique();
            ccidReceived=1;
        break;
    }
}
