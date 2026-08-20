/*
 * ble_stream_status.c
 *
 *  Created on: 17 oct. 2023
 *      Author: Mehmed Blazevic
 */

#include "ble_stream_status.h"
#include "smartBmsService.h"
#include "mtsc.h"
#include "protocol.pb-c.h"
#include "vesc_can.h"
#include "tiny_BMS.h"

/********************
 * send all info from bms and vesac on ble
 */
#define addInt8ToBuffer(val) buffer[posBuffer+1]=((int8_t)(val)); posBuffer+=1
#define addInt16ToBuffer(val) buffer[posBuffer]=((int16_t)(val))>>8; buffer[posBuffer+1]=((int16_t)(val)); posBuffer+=2
#define addInt32ToBuffer(val) buffer[posBuffer]=((int32_t)(val))>>24;buffer[posBuffer+1]=((int32_t)(val))>>16;buffer[posBuffer+2]=((int32_t)(val))>>8; buffer[posBuffer+3]=((int32_t)(val)); posBuffer+=4

#define CMD_CURRENTVALUE 1
#define NB_CELL_TINY_BMS 13

extern volatile Mtsc__Config bikeConfig;


void bleSendStatus()
{
    t_infoVehicle infoVehicle;
    infoVehicle.esc[0]=vesc_get_value(0);
    infoVehicle.bms=bms_get_value();

    uint8_t buffer[SMARTBMSSERVICE_DATABMS_LEN]={'S',VERSION_PROTOCOL_BLE,CMD_CURRENTVALUE,0/*size*/,0/*size*/,NB_BMS};
    volatile int posBuffer=6;
    int l;

    addInt16ToBuffer(infoVehicle.bms.bms_soc*100);//1 *100 -> 0 to 10'000
    addInt16ToBuffer(infoVehicle.bms.bms_ubat*100);//3
    addInt16ToBuffer(infoVehicle.bms.bms_ibat*100);//5
    addInt16ToBuffer(0);//infoVehicle.leftDistance*100);//7 1=10m
    addInt16ToBuffer(0);//infoVehicle.leftTime*100);//9
    addInt16ToBuffer(infoVehicle.bms.temperature[0]*100);//11
    addInt16ToBuffer(infoVehicle.bms.temperature[1]*100);//13
    addInt16ToBuffer(infoVehicle.bms.temperature[2]*100);//15
    for(l=0;l<NB_CELL_TINY_BMS;l++)
    {
        addInt16ToBuffer(infoVehicle.bms.cellsVoltages[l]*1000);//17->63
    }

    #define NB_CELL_TINY_LI_BAL_C_24 24 //for compatibility with cbms
    for(;l<NB_CELL_TINY_LI_BAL_C_24;l++) //for comaptibility with cbms24
    {
        addInt16ToBuffer(0);
    }

    buffer[posBuffer]=NB_VESC;//81
    posBuffer++;

    for(l=0;l<NB_VESC;l++)
    {
        addInt16ToBuffer(infoVehicle.esc[l].rpm*100*ERPM_TO_SPEED*bikeConfig.speedmultiplier);//66 78
        addInt16ToBuffer(infoVehicle.esc[l].current*100);//68 80
        addInt16ToBuffer(infoVehicle.esc[l].current_in*100);//70 82
        addInt16ToBuffer(infoVehicle.esc[l].v_in*100);//72 84
        addInt16ToBuffer(infoVehicle.esc[l].temp_fet*100);//74 86
        addInt16ToBuffer(infoVehicle.esc[l].temp_motor*100);//76 88
    }


    buffer[3]=((uint16_t)(posBuffer))>>8; //add paquet size
    buffer[4]=((uint16_t)(posBuffer));

    SmartBmsService_SetParameter(SMARTBMSSERVICE_DATABMS, posBuffer, buffer);
}

