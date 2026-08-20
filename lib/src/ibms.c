/*
 * tiny_BMS.c
 *
 *  Created on: 21 Jul 2021
 *      Author: mehmedblazevic
 */

#ifdef IBMS

//doc https://docs.google.com/spreadsheets/d/1kK3Kgsjy0tpJPh2lRL1Q39QaoKK-fSO02F7tbBMT5bI/edit#gid=0

#include "ibms.h"
#include "canTask.h"
#include "stdlib.h"
#include "timestamp.h"

static BMS_Values_t bms[NB_MAX_BAT] = {{.stat=BMS_S_DISCHARGING, .bms_ubat=0.0, .bms_ibat=0.0, .bms_soc=0, .timeLastData=0}};
static bool requestCharge;
bool load=0;
bool charge=0;
bool ballancing=0;

typedef enum
{
    CAN_IBMS_STATUS_CHARRACTERISTIQUE=0x218,//Battery capacity mAh (70000) 32bit, Status 8bit,  Code Erreur8bit, 0x01
    CAN_IBMS_CURRENT_VOLTAGE=0x220, //Current mA 32bit, Voltage 16bit,
    CAN_IBMS_SOC_SOE_SOH=0x228, //SOC 16bit, SOE 16bit, SOH 16bit
    CAN_IBMS_TEMEPRATURE1_8=0x230, //Temeprature 1 -> 8 8bit
    //CAN_IBMS_TEMEPRATURE9_10=0x238,//Temeprature 9 et 10 8bit
    CAN_IBMS_CELL_VOLTAGE_1_4=0x280,//Cell Voltage 1-4, 16bit
    CAN_IBMS_CELL_VOLTAGE_5_8=0x288,//Cell Voltage 5-8, 16bit
    CAN_IBMS_CELL_VOLTAGE_9_12=0x290,//Cell Voltage 9-12, 16bit
    CAN_IBMS_CELL_VOLTAGE_13=0x298,//Cell Voltage 13, 16bit
    CAN_IBMS_RTC_CYCLE=0x2D0,//Cell Voltage 13, 16bit
    CAN_IBMS_SET_CHARGE_DECHARGE=0x123,//Cell Voltage 13, 16bit
} t_ibms_can_id;


void iBMS_Init(){
    for(int l=1;l<NB_MAX_BAT;l++)
        bms[l]=bms[0];
    canRegistreCallBack(0,iBMS_Decode_Standard_Can);
    canRegistreCallBack(1,iBMS_Decode_Ext_Can);
}

void iBMS_RequestLoadCharge(bool _load,bool _charge,bool _ballancing)
{
    load=_load;
    charge=_charge;
    ballancing=_ballancing;
    can_t msg;
    msg.flags.extended = 0;
    msg.flags.rtr = 0;
    msg.id = CAN_IBMS_SET_CHARGE_DECHARGE;
    msg.length = 8;
    msg.data[0] = load;
    msg.data[1] = charge;
    msg.data[2] = ballancing;
    msg.data[3] = 0;
    msg.data[4] = 0;
    msg.data[5] = 0;
    msg.data[6] = 0;
    msg.data[7] = 0;
    canSendMessage(&msg);
    requestCharge=charge;
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

void iBMS_Decode_Standard_Can(can_t msg){
    uint32_t numBms=msg.id&0x7;
    if(numBms==7) //contourne bug programation par enix certain bms on une adresse de -1 ...
    {
        msg.id+=2;
        numBms=msg.id&0x7;
    }
    if(numBms>=NB_MAX_BAT)
        numBms=1;

    int id=msg.id&0xFF8;

    if(id == CAN_IBMS_STATUS_CHARRACTERISTIQUE){
        bms[numBms].timeLastData=getTimeStampMs();
        bms[numBms].capacity = tabToUint16(msg.data)*10;
        bms[numBms].stat = (enum BMS_Status)msg.data[2];
        bms[numBms].error = msg.data[3];
        bms[numBms].version = msg.data[4];
    }
    else if(id == CAN_IBMS_CURRENT_VOLTAGE){
        bms[numBms].bms_ibat = tabToint32(msg.data)*0.01/1000.f;
        bms[numBms].bms_ubat = tabToUint16(msg.data+4)*0.1;
    }
    else if(id == CAN_IBMS_SOC_SOE_SOH){
        bms[numBms].bms_soc = tabToUint16(msg.data)*0.01;
        bms[numBms].bms_soe = tabToUint32(msg.data+2)/3600;
        bms[numBms].bms_soh = tabToUint16(msg.data+6)*0.01;
    }
    else if(id == CAN_IBMS_TEMEPRATURE1_8){
        for(int l=0;l<msg.length && l<NB_TEMPERATURE_BMS;l++)
        {
            bms[numBms].temperature[l]=msg.data[l];
        }
    }
    /*else if(msg.id == CAN_IBMS_TEMEPRATURE9_10){
        memcpy(bms[numBms].temperature+8, msg.data,2);
    }*/
    else if(id >= CAN_IBMS_CELL_VOLTAGE_1_4 && msg.id < CAN_IBMS_CELL_VOLTAGE_13){
        volatile int numCell=(msg.id-CAN_IBMS_CELL_VOLTAGE_1_4)/2; // id espacé de 8 et 4 cell par paquet
        for(int l=0;l<msg.length/2;l++)
        {
            if(numCell<NB_CELL_BMS)
                bms[numBms].cellsVoltages[numCell]=tabToUint16(msg.data+l*2)/10000.f;
            numCell++;
        }
    }else if(id == CAN_IBMS_RTC_CYCLE){
        bms[numBms].rtc=tabToUint32(msg.data);
        bms[numBms].cycles=tabToUint32(msg.data+4);
    }
}

void iBMS_Decode_Ext_Can(can_t msg)
{
    if(msg.id==0x01000000)
    {
        int numBms=msg.data[0]&0x07; //see 5.2.4 BMS Heartbeat message in i-BMS_User_Manual...pdf
        if(numBms>=NB_MAX_BAT)
            numBms=0;
        bms[numBms].id=tabToUint64(msg.data);
    }
}

float intensitySocToPowerSoc(float SOC)
{
    float socP= 0.00119*SOC*SOC+0.888*SOC-0.338;
    if(socP>=100)
        socP=100;
    if(socP<0)
        socP=0;
    return socP;
}

float bms_get_soc(){ //0 to 100
    float soc=0;
    int nbBms=0;
    for(int numBms=0;numBms<NB_MAX_BAT;numBms++)
    {
        if(((getTimeStampMs()-bms[numBms].timeLastData)<BMS_TIMEOUT) && (bms[numBms].stat!=BMS_S_ERROR))
        {
            nbBms++;
            soc+=bms[numBms].bms_soc;
        }
    }
    if(nbBms==0)
        return 0; //no bms detected
    return soc/nbBms;
}

float bms_get_energy(){
    float energy=0;
    int nbBms=0;
    for(int numBms=0;numBms<NB_MAX_BAT;numBms++)
    {
        if(((getTimeStampMs()-bms[numBms].timeLastData)<BMS_TIMEOUT) && (bms[numBms].stat!=BMS_S_ERROR))
        {
            nbBms++;
            energy+=bms[numBms].bms_soe;
        }
    }
    if(nbBms==0)
        return 0; //no bms detected
    return energy;
}

float bms_get_ubat(){
    volatile float ubat=0;
    int nbBms=0;
    for(int numBms=0;numBms<NB_MAX_BAT;numBms++)
    {
        if(((getTimeStampMs()-bms[numBms].timeLastData)<BMS_TIMEOUT) && (bms[numBms].stat!=BMS_S_ERROR))
        {
            nbBms++;
            ubat+=bms[numBms].bms_ubat;

        }
    }
    if(nbBms==0)
        return 0; //no bms detected
    return ubat/nbBms;
}

float bms_get_lower_ucell(){
    int nbBms=0;
    volatile float ucellMin=5;
    for(int numBms=0;numBms<NB_MAX_BAT;numBms++)
    {
        if(((getTimeStampMs()-bms[numBms].timeLastData)<BMS_TIMEOUT) && (bms[numBms].stat!=BMS_S_ERROR))
        {
            nbBms++;
            for(int i=0;i<NB_CELL_BMS;i++)
            {
                if(bms[numBms].cellsVoltages[i]<ucellMin && bms[numBms].cellsVoltages[i]>1) //>1 else is a invalide value
                {
                    ucellMin=bms[numBms].cellsVoltages[i];
                }
            }

        }
    }
    if(nbBms==0)
        return 0; //no bms detected
    return ucellMin;
}
// for now we're receiving only zeros
float bms_get_ibat(){
    volatile float ibat=0;
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

uint32_t bms_get_timestamp(){
    uint32_t maxRtc=0;
    for(int numBms=0;numBms<NB_MAX_BAT;numBms++)
    {
        if((getTimeStampMs()-bms[numBms].timeLastData)<BMS_TIMEOUT)
        {
            if(bms[numBms].rtc>maxRtc)
                maxRtc+=bms[numBms].rtc;
        }
    }
    return maxRtc;
}

uint32_t bms_get_capacity(){
    uint32_t capacity=0;
    int nbBms=0;
    for(int numBms=0;numBms<NB_MAX_BAT;numBms++)
    {
        if(((getTimeStampMs()-bms[numBms].timeLastData)<BMS_TIMEOUT) && (bms[numBms].stat!=BMS_S_ERROR))
        {
            nbBms++;
            capacity+=bms[numBms].capacity/(bms[numBms].bms_soc/100);
        }
    }
    if(nbBms==0)
        return 0; //no bms detected
    return capacity;
}

enum BMS_Status bms_get_status(){
    if(     ((getTimeStampMs()-bms[0].timeLastData)<=BMS_TIMEOUT && bms[0].stat==BMS_S_ERROR) ||
            ((getTimeStampMs()-bms[1].timeLastData)<=BMS_TIMEOUT && bms[1].stat==BMS_S_ERROR))
    {
        return BMS_S_FAULT;
    }

    if((getTimeStampMs()-bms[0].timeLastData)>=BMS_TIMEOUT && (getTimeStampMs()-bms[1].timeLastData)>=BMS_TIMEOUT )
        return BMS_NO_COM; //no bms comunication
    if(requestCharge && bms_get_ibat()>0)
        return BMS_S_CHARGING;
    return BMS_S_DISCHARGING;
}

bool bms_get_timeout()
{
    return ((getTimeStampMs()-bms[0].timeLastData)>=BMS_TIMEOUT && (getTimeStampMs()-bms[1].timeLastData)>=BMS_TIMEOUT);
}

BMS_Values_t* bms_get_values(){
    return bms;
}


#endif //IBMS
