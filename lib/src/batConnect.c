/*
 * tiny_BMS.c
 *
 *  Created on: 21 Jul 2021
 *      Author: mehmedblazevic
 */

#ifdef BAT_CONNECT

#include "batConnect.h"
#include "canTask.h"
#include "stdlib.h"
#include "timestamp.h"

#define BMS_ID 0x18000140
#define BMS_RECEIVE_ID 0x18004001


static BMS_Values_t bms[NB_MAX_BAT] = {{.stat=BMS_S_DISCHARGING, .bms_ubat=0.0, .bms_ibat=0.0, .bms_soc=0, .timeLastData=0}};

void batConnect_Init(){
    for(int l=1;l<NB_MAX_BAT;l++)
        bms[l]=bms[0];
    canRegistreCallBack(1,batConnect_Decode_Can);
}

uint64_t tabToUint64(uint8_t* buff)
{
    volatile uint64_t val=0;
    for(int i=0 ; i<8 ; i++) {
        val = (val<<8);
        val|=buff[i];
    }
    return val;
}

uint32_t tabToUint32(uint8_t* buff)
{
    uint32_t val=0;
    for(int i=0 ; i<4 ; i++) {
        val = (val<<8);
        val|=buff[i];
    }
    return val;
}

int32_t tabToint32(uint8_t* buff)
{
    int32_t val=0;
    for(int i=0 ; i<4 ; i++) {
        val = (val<<8);
        val|=buff[i];
    }
    return val;
}

uint16_t tabToUint16(uint8_t* buff)
{
    uint16_t val=0;
    for(int i=0 ; i<2 ; i++) {
        val = (val<<8);
        val|=buff[i];
    }
    return val;
}

void batConnect_Decode_Can(can_t msg){
    if((msg.id&0xFF00FFFF) != BMS_RECEIVE_ID) //is data from BMS
        return;

    uint32_t numBms=0;
    if(numBms>=NB_MAX_BAT)
        numBms=0;

    uint8_t dataId=(msg.id>>16)&0xFF;

    if(dataId == BC_SOC_VOLT_CURRENT){
        bms[numBms].timeLastData=getTimeStampMs();
        bms[numBms].bms_ubat=tabToUint16(msg.data)*0.1;
        bms[numBms].bms_ibat = (tabToUint16(msg.data+4)-30000)*0.1;
        bms[numBms].bms_soc = tabToUint16(msg.data+6)*0.1;
    }
    else if(dataId == BC_CAPACITY){
        bms[numBms].capacity=tabToUint32(msg.data);
    }
    else if(dataId == BC_CHARG_DISCHAR){
        bms[numBms].stat=(enum BMS_Status)(msg.data[0]);
    }
    else if(dataId == BC_TEMEPRATURE){
        bms[numBms].temperature[0]=msg.data[0]-40;
        bms[numBms].temperature[1]=msg.data[2]-40;
    }
    else if(dataId == BC_CELL_VOLTAGE){
        uint8_t numCell=(msg.data[0]-1)*3;
        if(numCell<NB_CELL_BMS)
            bms[numBms].cellsVoltages[numCell]=tabToUint16(msg.data+1)*0.001;
        numCell++;
        if(numCell<NB_CELL_BMS)
            bms[numBms].cellsVoltages[numCell]=tabToUint16(msg.data+3)*0.001;
        numCell++;
        if(numCell<NB_CELL_BMS)
            bms[numBms].cellsVoltages[numCell]=tabToUint16(msg.data+5)*0.001;
    }
}

float bms_get_soc(){
    float soc=0;
    int nbBms=0;
    for(int numBms=0;numBms<NB_MAX_BAT;numBms++)
    {
        if((getTimeStampMs()-bms[numBms].timeLastData)<BMS_TIMEOUT)
        {
            nbBms++;
            soc+=bms[numBms].bms_soc;
        }
    }
    if(nbBms==0)
        return 0; //no bms detected
    return soc/nbBms;
}


float bms_get_ubat(){
    volatile float ubat=0;
    int nbBms=0;
    for(int numBms=0;numBms<NB_MAX_BAT;numBms++)
    {
        if((getTimeStampMs()-bms[numBms].timeLastData)<BMS_TIMEOUT)
        {
            nbBms++;
            ubat+=bms[numBms].bms_ubat;

        }
    }
    if(nbBms==0)
        return 0; //no bms detected
    return ubat/nbBms;
}
// for now we're receiving only zeros
float bms_get_ibat(){
    float ibat=0;
    int nbBms=0;
    for(int numBms=0;numBms<NB_MAX_BAT;numBms++)
    {
        if((getTimeStampMs()-bms[numBms].timeLastData)<BMS_TIMEOUT)
        {
            nbBms++;
            ibat+=bms[numBms].bms_ibat;
        }
    }
    if(nbBms==0)
        return 0; //no bms detected
    return ibat;
}

uint32_t bms_get_capacity(){
    uint32_t capacity=0;
    int nbBms=0;
    for(int numBms=0;numBms<NB_MAX_BAT;numBms++)
    {
        if((getTimeStampMs()-bms[numBms].timeLastData)<BMS_TIMEOUT)
        {
            nbBms++;
            capacity+=bms[numBms].capacity;
        }
    }
    if(nbBms==0)
        return 0; //no bms detected
    return capacity;
}

uint32_t bms_get_max_temperature(){
    int32_t temperature=-100;
    int nbBms=0;
    for(int numBms=0;numBms<NB_MAX_BAT;numBms++)
    {
        if((getTimeStampMs()-bms[numBms].timeLastData)<BMS_TIMEOUT)
        {
            for(int l=0;l<NB_TEMPERATURE_BMS;l++)
            {
                nbBms++;
                if(bms[numBms].temperature[l]>temperature)
                    temperature=bms[numBms].temperature[l];
            }
        }
    }
    if(nbBms==0)
        return 0; //no bms detected
    return temperature;
}

enum BMS_Status bms_get_status(){
    if((getTimeStampMs()-bms[0].timeLastData)>=BMS_TIMEOUT)
        return BMS_NO_COM; //no bms comunication
    return bms[0].stat;
}

BMS_Values_t* bms_get_values(){
    return bms;
}

void request_bms(BT_can_id_t id){
    can_t msg;
    msg.flags.extended = 1;
    msg.flags.rtr = 0; //fix me need to be 1?
    msg.id = BMS_ID|(id<<16);
    msg.length = 0;
    /*msg.data[0] = 0;
    msg.data[1] = 0;
    msg.data[2] = 0;
    msg.data[3] = 0;
    msg.data[4] = 0;
    msg.data[5] = 0;
    msg.data[6] = 0;
    msg.data[7] = 0;*/
    canSendMessage(&msg);
}


#endif //BAT_CONNECT
