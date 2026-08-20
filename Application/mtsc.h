/*
 * mtsc.h
 *
 *  Created on: 25 sept. 2021
 *      Author: Mehmed
 */

#include "vesc.h"
#include "tiny_BMS.h"
#include "pixxil_graphics.h"
#include <ti/drivers/NVS.h>

#ifndef APPLICATION_MTSC_H_
#define APPLICATION_MTSC_H_

enum MTSC_Modes{NORMAL, GPS, OFF, ON, WELCOME, AUTHENT, CHARGING};
enum TRIPS{TRIP,TRIPA, TRIPB};

typedef struct{
    bool brakeState;
    bool lightState;
    enum MTSC_Modes mode;
    enum TRIPS trips;
    Vmode vMode;    // defined in graphics cause needed for display too
    uint8_t realSpeed;
}t_ctrlVehicle;

// generic datas used for/by flash & tracy's system
typedef struct{
    bool ready; //datas from flash & tracy ready
    double kmTot;   // those 3 should be removed
    double tripA;
    double tripB;
} t_infoUserVehicle;

void controlThread_createTask(void);
void periodic_createTask(void);



// drive compute
void updateTorkSlope(Vmode vmode, bool degraded);
float computeCurrent(float tor, int16_t pasSpeed);

void getNameFromNvs(char *name);
void registerNameToNvs(char *name);
void initTripNVS(NVS_Params *nvsParams, NVS_Attrs *regionAttrs, NVS_Handle *nvsHandle, uint32_t *ptNvs, enum TRIPS trip);
void registerTripToNVS(NVS_Handle *nvsHandle, uint32_t *ptNvs, enum TRIPS trip);
void passCodeValidation(bool ok);
void requestPairing();

// BLE
void demo();

#define VERSION_PROTOCOL_BLE 1



// vehicle specific parameters
#define MOT_PINION      18.0 //18
#define WHEEL_PINION    186.0
#define MOTOR_POLES     14.0
//#define MOT_POLES_PAIR  7.0
#define GEARS           (WHEEL_PINION/MOT_PINION)
#define WHEEL_DIAM      (0.724) // 0.7 m
#define WHEEL_RADIUS    (WHEEL_DIAM/2)
//#define SPEED_RATIO     0.377
//#define SPEED_CONST     0.77 // 0.733333333
//#define ERPM_TO_RPM     MOTOR_POLES
//#define TACHO_TO_ROT    (MOTOR_POLES*3)
//#define ERPM_TO_SPEED   (1.f/MOTOR_POLES/GEARS*WHEEL_DIAM*3.1416*60/1000*0.77)  2.75
#define ERPM_TO_SPEED   ((1.f/(MOTOR_POLES/2)/GEARS*60*WHEEL_RADIUS*2*3.1416)/1000.f) // erpm to km/h, works only with MTSC
/*#define TACHO_TO_KM (1/GEARS/ERPM_TO_RPM/3*R_WHEEL*2*3.1416/1000/2)*/
//#define TACHO_TO_KM     (1.f/ERPM_TO_RPM/GEARS*WHEEL_DIAM*3.14159265359*SPEED_CONST/1000)
#define TACHO_TO_KM     (ERPM_TO_SPEED/6/60)

// 556 5 3

// red�marrage: 562 11 8


#define NB_BMS 1

#define VESC_BROADCAST 0xFF

#define NVS_SECTOR_SIZE (1024*8)


#define SOC_TO_KM 1.5

void sleepy(int ms);


#endif /* APPLICATION_MTSC_H_ */
