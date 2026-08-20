#ifndef IBMS_H
#define IBMS_H

#ifdef IBMS

#include "canTask.h"

#define NB_TEMPERATURE_BMS 5
#define NB_CELL_BMS 12
#define BMS_TIMEOUT 3200 //paquet envoyé tout les 1000ms donc si 3 paquet non recu

#ifdef LB_V3
#define NB_MAX_BAT 2 //we can mount 2 battery pack
#else
#define NB_MAX_BAT 1 //we can mount 1 battery pack
#endif

enum BMS_Status{BMS_S_CHARGING, BMS_S_FULLY_CHARGED, BMS_S_DISCHARGING, BMS_S_FAULT, BMS_NO_COM};
enum IBMS_Status{BMS_S_INIT, BMS_S_READY, BMS_S_PRE_CHARGE, BMS_S_ACTIV,BMS_S_ERROR}; //i-BMS15_User_Manual_CIP_v.3.7.0.pdf


typedef struct{
    enum IBMS_Status stat;
    uint8_t error;
    uint8_t version;
    uint64_t timeLastData; //timestampms last data received from bms
    uint32_t capacity;
    uint32_t rtc;
    uint32_t cycles;
    uint64_t id;

    float bms_ubat;
    float bms_ibat;
    float bms_soc; // 0 to 100
    float bms_soe;//wh
    float bms_soh;

    float temperature[NB_TEMPERATURE_BMS];
    float cellsVoltages[NB_CELL_BMS];
}BMS_Values_t;

float bms_get_ubat();
float bms_get_lower_ucell();
float bms_get_energy();
float bms_get_ibat();
float bms_get_soc();
uint32_t bms_get_capacity();
uint32_t bms_get_timestamp();
enum BMS_Status bms_get_status();
BMS_Values_t* bms_get_values();
float intensitySocToPowerSoc(float SOC);
void iBMS_RequestLoadCharge(bool load,bool charge,bool ballancing);
void iBMS_Init();
uint32_t tabToUint32(uint8_t* buff);
uint16_t tabToUint16(uint8_t* buff);
void iBMS_Decode_Standard_Can(can_t msg);
void iBMS_Decode_Ext_Can(can_t msg);
bool bms_get_timeout();

#endif

#endif //IBMS
