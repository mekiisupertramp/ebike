/*
 * canProtocole.h
 *
 *  Created on: 14 févr. 2022
 *      Author: Mehmed Blazevic
 */

#ifndef APPLICATION_BIGBERTA_H_
#define APPLICATION_BIGBERTA_H_

#include "stdbool.h"
#include "stdint.h"
#include "canTask.h"

#define BB_ACC_FULL_SCALL 16384.f

typedef enum
{
    POWER_STATE_OFF=0,
    POWER_STATE_LOW=1,
    POWER_STATE_FULL=2,
    POWER_STATE_NO_MOTOR=3
} t_powerState;


typedef enum
{
    BLINK_0FF=0,
    BLINK_LEFT=1,
    BLINK_RIGHT=2,
    BLINK_BOTH=3
} t_blinkLight;

typedef enum
{
    BB_OK=0,
    BB_TIMEOUT=1
} t_BbStatus;

typedef void (*tCallBackImu)();


void bigBertaInit();
void bigBertaSetPowerState(t_powerState power);
void bigBertaSetOut();
void bigBertaSetRegenForce(uint16_t force); //0 to 2048
void bigBertaDecodeCan(can_t msg);
t_BbStatus bigBertaGetStatus();

void bigBertaSetLight(bool val);
void bigBertaSetHorn(bool val);
void bigBertaSetBreakLight(bool val);
void bigBertaSetBlink(t_blinkLight val);
static void bigBertaWriteOnUart(uint8_t* data, int len);
void bigBertaSetBip(uint8_t val); //value from 0 to 5
void bigBertaSetBlinkSound(bool val);

bool bigBertaGetLight();
uint32_t bigBertaGetNbBlink();
uint32_t bigBertaGetLeftBlink();
uint32_t bigBertaGetRightBlink();
bool bigBertaGetHorn();
bool bigBertaGetBrakeLight();
bool bigBertaGetSound();
#ifdef LB_V3
void bigBertaSetCallBackImu(tCallBackImu _callBackImu);
bool bigBertaGetAux1();
bool bigBertaGetAux2();
bool bigBertaGetAux3();
bool bigBertaGetAuxVbat();
#endif

int8_t bigBertaGetTemperature(uint8_t pt);
int16_t bigBertaGetCurrent(uint8_t pt);
int16_t bigBertaGetMagneticField(uint8_t pt);
int16_t bigBertaGetAcceleration(uint8_t pt);
int16_t bigBertaGetAngularRate(uint8_t pt);
uint16_t bigBertaGetAdcIn1();
uint16_t bigBertaGetAdcIn2();

extern volatile int16_t bigBertaPasSpeed;
extern volatile uint16_t bigBertaAdcValue[3];
extern volatile bool bigBertaPrechargeEnd;
extern volatile bool regenNewValue;
extern volatile bool adcNewValue;

#endif /* APPLICATION_BIGBERTA_H_ */
