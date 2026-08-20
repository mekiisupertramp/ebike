/*
 * tiny_BMS.h
 *
 *  Created on: 21 Jul 2021
 *      Author: mehmedblazevic
 */

#ifndef LIB_INC_TINY_BMS_HD_
#define LIB_INC_TINY_BMS_HD_


#include "mcp2515_driver.h"
#include <stdint.h>


#define BMS_HIGH_ID    0x200
#define BMS_REPLY_ID    0x240

#define BMS_ID         0x01

#define OK      0
#define ERR     -1
#define MAX_VOLTAGE 54.0
#define MIN_VOLTAGE 30.0
#define RANGE_VOLT (MAX_VOLTAGE-MIN_VOLTAGE)

#define BMS_TIMEOUT 1000

#define NB_CELL_BMS 16

#define TINYBMS_REG_CHARGE_DETECTION 335
#define TINYBMS_CHARGE_DETECTION_INTERNAL 0x01
#define TINYBMS_CHARGE_DETECTION_DIDO2 0x05

enum BMS_Status{BMS_S_CHARGING, BMS_S_FULLY_CHARGED, BMS_S_DISCHARGING, BMS_S_REGENERATION, BMS_S_IDLE, BMS_S_FAULT, BMS_NO_COM};
enum BMS_Values{
    BMS_S_RESET=0x02,
    BMS_S_READ_REGS =0x03,
    BMS_S_WRITE_REGS =0x10,
    BMS_S_READ_NEW_EVENTS =0x11,
    BMS_S_READ_ALL_EVENTS =0x11,
    BMS_S_VOLTAGE = 0x14,//tested
    BMS_S_CURRENT = 0x15,//tested
    BMS_S_MAX_CELL_VOLTAGE =0x16,
    BMS_S_MIN_CELL_VOLTAGE =0x17,
    BMS_S_STATUS = 0x18,
    BMS_S_LIFETIME = 0x19,
    BMS_S_SOC = 0x1A,//tested
    BMS_S_TEMPERATURES = 0x1B,//tested
    BMS_S_CELLS_VOLTAGE = 0x1C,//tested
    BMS_S_SETTING_VAL = 0x1D,
    BMS_S_READ_VERSION = 0x1E,
    BMS_S_SPEED = 0x20,
    //BMS_S_READ_CANNID =0x28
};

#define NB_TEMPERATURE_BMS 3

typedef struct{
    enum BMS_Status stat;
    float bms_ubat;
    float bms_ibat;
    float bms_soc; // 0 to 100
    uint64_t timeLastData;
    // not used for now
    float bms_c_speed;
    uint32_t bms_l_dist;
    uint32_t bms_estim_time;
    float temperature[NB_TEMPERATURE_BMS];
    float cellsVoltages[NB_CELL_BMS];
}BMS_Values_t;

int8_t init_tiny_BMS();

void update_bms(can_t msg);
void request_bms(enum BMS_Values bmsv);
void writeRegister_bms(uint16_t regAddress, uint16_t value);
void stopCharge_bms();
void startCharge_bms();

float bms_get_ubat();
float bms_get_ibat();
float bms_get_soc();
enum BMS_Status bms_get_status();
BMS_Values_t* bms_get_values(); //for compatibility with ibms
BMS_Values_t bms_get_value();
bool bms_get_timeout();

float intensitySocToPowerSoc(float SOC); //soc in value form 0 to 100 calibrate with a VTC6 cell the difference of power soc vs current soc are only 3% at 50%

// maybe to implement
float bms_calculated_speed();
uint32_t bms_left_distance();
uint32_t bms_estimated_time();
float bms_get_max_temperature();

#endif /* LIB_INC_TINY_BMS_HD_ */
