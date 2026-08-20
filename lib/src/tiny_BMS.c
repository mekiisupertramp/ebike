/*
 * tiny_BMS.c
 *
 *  Created on: 21 Jul 2021
 *      Author: mehmedblazevic
 */

#include <virtualUartService.h>
#include "tiny_BMS.h"
#include "canTask.h"
#include "stdlib.h"
#include "timestamp.h"

static BMS_Values_t bms = {.stat=BMS_S_DISCHARGING, .bms_ubat=0.0, .bms_ibat=0.0, .bms_soc=0};


int8_t init_tiny_BMS(){

    canRegistreCallBack(0,update_bms);
#ifdef SIMULATION
    bms.bms_soc=1;
#endif
    return OK;

}

void stopCharge_bms()
{
    writeRegister_bms(TINYBMS_REG_CHARGE_DETECTION,TINYBMS_CHARGE_DETECTION_DIDO2);
}

void startCharge_bms()
{
    writeRegister_bms(TINYBMS_REG_CHARGE_DETECTION,TINYBMS_CHARGE_DETECTION_INTERNAL);
}

void writeRegister_bms(uint16_t regAddress, uint16_t value)
{
    can_t msg;
    msg.flags.extended = 0;
    msg.flags.rtr = 0;
    msg.id = BMS_HIGH_ID | BMS_ID;
    msg.length = 8;
    msg.data[0] = 0x10;
    msg.data[1] =(uint8_t)(regAddress>>8);
    msg.data[2] =(uint8_t)(regAddress);
    msg.data[3] =0x00;
    msg.data[4] =1;
    msg.data[5] =(uint8_t)(value>>8);
    msg.data[6] =(uint8_t)(value);
    msg.data[7] =0;
    canSendMessage(&msg);
}


void request_bms(enum BMS_Values bmsv){
    can_t msg;
    msg.flags.extended = 0;
    msg.flags.rtr = 0;
    msg.id = BMS_HIGH_ID | BMS_ID;
    msg.length = 1;

    msg.data[0] = bmsv;
    canSendMessage(&msg);
}

void update_bms(can_t msg){
    uint32_t val = 0;
    int i=0;

#ifdef SIMULATION
    bms.bms_ubat=rand()%60;
    bms.bms_ibat=rand()%100;
    bms.stat=BMS_S_DISCHARGING;
    bms.bms_soc=rand()%100;
    for(int l=0;l<12;l++)
        bms.cellsVoltages[l]=rand();
    for(int l=0;l<NB_TEMPERATURE_BMS;l++)
        bms.temperature[l]=rand();

    /*bms.bms_ubat=40;
    bms.bms_ibat=10;
    bms.stat=BMS_S_DISCHARGING;
    bms.bms_soc-=0.001;
    for(int l=0;l<12;l++)
        bms.cellsVoltages[l]=3.7+l/10;
    for(int l=0;l<NB_TEMPERATURE_BMS;l++)
        bms.temperature[l]=rand();*/

#endif

    if(msg.id == (BMS_REPLY_ID | BMS_ID)){
        bms.timeLastData=getTimeStampMs();
        if(msg.data[0] == 0x01){
            switch(msg.data[1]){
            case BMS_S_VOLTAGE:
                for(i=0 ; i<4 ; i++) {
                    val |= msg.data[i+2]<<i*8;
                }
                bms.bms_ubat = *((float*)&val);
                break;
            case BMS_S_CURRENT:
                for(i=0 ; i<4 ; i++) {
                    val |= msg.data[i+2]<<i*8;
                }
                bms.bms_ibat = *((float*)&val);
                break;
            case BMS_S_STATUS:
                switch(msg.data[2]){
                case 0x91:  bms.stat = BMS_S_CHARGING; break;
                case 0x92:  bms.stat = BMS_S_FULLY_CHARGED; break;
                case 0x93:  bms.stat = BMS_S_DISCHARGING;  break;
                case 0x96:  bms.stat = BMS_S_REGENERATION; break;
                case 0x97:  bms.stat = BMS_S_IDLE; break;
                case 0x9B:  bms.stat = BMS_S_FAULT; break;
                default: bms.stat = BMS_S_FAULT; break;
                }
                break;
                case BMS_S_SOC:
                    for(i=0 ; i<4 ; i++) {
                        val |= msg.data[i+2]<<i*8;
                    }
                    bms.bms_soc = val/1000000.f;
                    break;
                case BMS_S_CELLS_VOLTAGE:
                    if(msg.data[5]<NB_CELL_BMS)
                    {
                        for(i=0 ; i<2 ; i++) {
                            val |= msg.data[i+3]<<i*8;
                        }
                        bms.cellsVoltages[msg.data[5]] = val/10000.f;
                    }
                    break;
                case BMS_S_TEMPERATURES:
                    if(msg.data[5]<NB_TEMPERATURE_BMS)
                    {
                        for(i=0 ; i<2 ; i++) {
                            val |= msg.data[i+3]<<i*8;
                        }
                        bms.temperature[msg.data[5]] = (int32_t)val/10.f;
                    }
                    break;
            }
        }
    }
}
float bms_get_soc(){
if((getTimeStampMs()-bms.timeLastData)>=BMS_TIMEOUT)
        return 0; //no bms comunication
    return bms.bms_soc;
}

float bms_get_ubat(){
if((getTimeStampMs()-bms.timeLastData)>=BMS_TIMEOUT)
        return 0; //no bms comunication
    return bms.bms_ubat;
}
// for now we're receiving only zeros
float bms_get_ibat(){
if((getTimeStampMs()-bms.timeLastData)>=BMS_TIMEOUT)
        return 0; //no bms comunication
    return bms.bms_ibat;
}

float bms_get_max_temperature(){
if((getTimeStampMs()-bms.timeLastData)>=BMS_TIMEOUT)
        return 0; //no bms comunication
    return bms.temperature[0];
}
enum BMS_Status bms_get_status(){
    volatile int dt=getTimeStampMs()-bms.timeLastData;
    if(dt>=BMS_TIMEOUT)
        return BMS_NO_COM; //no bms comunication
    return bms.stat;
}

BMS_Values_t* bms_get_values(){
    return &bms;
}

BMS_Values_t bms_get_value(){
    return bms;
}

bool bms_get_timeout()
{
    return getTimeStampMs()-bms.timeLastData>BMS_TIMEOUT;
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
