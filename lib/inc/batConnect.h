/*
 * batConnect.h
 *
 *  Created on: 16 févr. 2024
 *      Author: Mehmed Blazevic
 */

#ifndef LIB_INC_BATCONNECT_H_
#define LIB_INC_BATCONNECT_H_

#include "canTask.h"

#define NB_MAX_BAT 1
#define NB_CELL_BMS 16
#define NB_TEMPERATURE_BMS 2
#define BMS_TIMEOUT 2000

enum BMS_Status{BMS_S_IDLE=0, BMS_S_CHARGING=1, BMS_S_DISCHARGING=2, BMS_NO_COM=3}; //page 6 CAN_DataAcquisition_version_oui_cycle.pdf

typedef enum
{
    BC_CAPACITY=0x50,
    BC_SOC_VOLT_CURRENT=0x90,
    BC_TEMEPRATURE=0x92,
    BC_CHARG_DISCHAR=0x93,
    BC_CELL_VOLTAGE=0x95
} BT_can_id_t;

typedef struct{
    enum BMS_Status stat;
    float bms_ubat;
    float bms_ibat;
    float bms_soc;
    uint32_t capacity;
    uint64_t timeLastData;
    float temperature[NB_TEMPERATURE_BMS];
    float cellsVoltages[NB_CELL_BMS];
}BMS_Values_t;

void batConnect_Init();
void batConnect_Decode_Can(can_t msg);
float bms_get_soc();
float bms_get_ubat();
float bms_get_ibat();
uint32_t bms_get_capacity();
enum BMS_Status bms_get_status();
BMS_Values_t* bms_get_values();
void request_bms(BT_can_id_t id);
uint32_t bms_get_max_temperature();

#endif /* LIB_INC_BATCONNECT_H_ */
