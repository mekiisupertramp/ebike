/*
 * bigBerta.c
 *
 *  Created on: 14 févr. 2022
 *      Author: Mehmed Blazevic
 */

#include "bigBerta.h"
#include "string.h"
#include <ti/sysbios/knl/Clock.h>
#include <tiny_BMS.h>
#include <virtualUartService.h>
#include <timestamp.h>
#include <math.h>

volatile int16_t bigBertaPasSpeed=0;
volatile uint16_t bigBertaAdcValue[3]={0,0,0}; //entre A, B, IN
volatile bool bigBertaPrechargeEnd=0;
volatile bool regenNewValue=0;
volatile bool adcNewValue=0;

static volatile bool light=0;
static volatile bool leftBlink=0;
static volatile bool rightBlink=0;
static volatile bool horn=0;
static volatile bool breakLight=0;
static volatile bool blinkSound=0;
static volatile uint8_t bip=0;
static volatile bool aux1=1;
static volatile bool aux2=1;
static volatile bool aux3=1;
static volatile bool auxVbat=1;
static volatile uint64_t timeLastData=0;

volatile can_t lastConfirmOutReceive;
#ifdef BB_V3
volatile can_t lastConfirmAuxReceive;
volatile int16_t magneticField[3];
volatile int16_t acceleration[3];
volatile int16_t angularRate[3];
volatile int8_t temperature[2]; //temperature regen, power
volatile int16_t current[2]; //Courant Regen Ibat
volatile tCallBackImu callBackImu=0;
#endif
volatile t_powerState lastConfirmPowerModeReceive;
#define BB_COM_TIMEOUT 250

typedef enum
{
    CAN_BIG_BERTA_SET_OUT=0x001, //received by bigBerta set OUTPUT each Byte for a OUTPUT wait 5Byte order of Byte HORN,BREAK,LEFT,RIGHT,LIGHT
    CAN_BIG_BERTA_SET_PWM_REGEN=0x002, //received by bigBerta set pwm for regen (low value = easy to rotate Crankset, 2024 = hard to rotate Crankset)
    CAN_BIG_BERTA_SET_POWER_MODE=0x003,
    CAN_BIG_BERTA_SET_AUX=0x006,
    CAN_BIG_BERTA_CONFIRM_OUT=0x101,
    CAN_BIG_BERTA_CONFIRM_SET_POWER_MODE=0x103,
    CAN_BIG_BERTA_VALUE_0=0x104, //send by bigBerta send the value of regen speed (16bit) + value of adc 16bit (value form 0 to 1024) //high prioryty
    CAN_BIG_BERTA_VALUE_ADC=0x105, //send by bigBerta send the value of regen speed (16bit) + value of adc 16bit (value form 0 to 1024) //high prioryty
    CAN_BIG_BERTA_CONFIRM_AUX=0x106,
    CAN_BIG_BERTA_VALUE_TEMPERATURE_CURRENT=0x107,//send by bigBerta send the value of regen speed (16bit) + value of adc 16bit (value form 0 to 1024) //high prioryty
    CAN_BIG_BERTA_VALUE_MANGETOMETRE=0x108,
    CAN_BIG_BERTA_VALUE_ACCELEROMETER=0x109,
    CAN_BIG_BERTA_VALUE_GYROSCOPE=0x110,
    CAN_BIG_BERTA_TINY_BMS_READ=BMS_HIGH_ID|BMS_ID
} t_can_id;


void bigBertaSetPowerState(t_powerState power)
{
    static uint64_t timeLastSend=0;
    volatile uint64_t tic=(uint64_t)Clock_getTicks()*Clock_tickPeriod;

    if(/*(power!=lastConfirmPowerModeReceive) &&*/ (tic>(timeLastSend+50000) | (tic<timeLastSend))) //si big berta pas deja dans se mode et au'on ne vien pas d'envoyer la commande
    {
        switch(power)
        {
        case POWER_STATE_OFF:
            //do after send message
            lastConfirmOutReceive.data[0]=0xFF; //force resend light state when restart
#ifdef BB_V3
            lastConfirmAuxReceive.data[0]=0xFF;
#endif
            break;
        case POWER_STATE_LOW:
#ifdef DCDC_LOW_POWER
            GPIO_write(DCDC_LOW_POWER,1);
#endif
            canEnable();
            break;
        case POWER_STATE_FULL:
#ifdef DCDC_LOW_POWER
            GPIO_write(DCDC_LOW_POWER,1);
#endif
            canEnable();
            break;
        case POWER_STATE_NO_MOTOR:
#ifdef DCDC_LOW_POWER
            GPIO_write(DCDC_LOW_POWER,1);
#endif
            canEnable();
            bigBertaPrechargeEnd=0;
            break;
        }


        can_t msg;
        msg.id=CAN_BIG_BERTA_SET_POWER_MODE;
        msg.length=1;
        msg.flags.extended=0;
        msg.flags.rtr=0;
        msg.data[0]=power;

        canSendMessage(&msg);
        timeLastSend=tic;
    }
    // if POWER_STATE_FULL but precharge not ready
    if(((power&lastConfirmPowerModeReceive) == POWER_STATE_FULL)){
        if(!bigBertaPrechargeEnd){
#ifdef DCDC_LOW_POWER
            GPIO_write(DCDC_LOW_POWER,1); // just to be sure, but useless
#endif

            //lastConfirmOutReceive.data[0]=0xFF; // not sure if needed here no do not need her Mehmed Blazevic

            can_t msg;
            msg.id=CAN_BIG_BERTA_SET_POWER_MODE;
            msg.length=1;
            msg.flags.extended=0;
            msg.flags.rtr=0;
            msg.data[0]=power;

            canSendMessage(&msg);
        }
    }

    /*if(power==POWER_STATE_OFF)
    {
        ClockP_usleep(10000);//wait mesage is send
        canDisable();
#ifdef DCDC_LOW_POWER
        GPIO_write(DCDC_LOW_POWER,0);
#endif
        bigBertaPrechargeEnd=0;
        lastConfirmPowerModeReceive=POWER_STATE_OFF;
    }*/
}

#ifdef BB_V3
void bigBertaSetCallBackImu(tCallBackImu _callBackImu)
{
    callBackImu=_callBackImu;
}
#endif

void bigBertaSetLight(bool val)
{
    light=val;
    bigBertaSetOut();
}

void bigBertaSetHorn(bool val)
{
    horn=val;
    bigBertaSetOut();
}

void bigBertaSetBreakLight(bool val)
{
    breakLight=val;
    bigBertaSetOut();
}

void bigBertaSetBlink(t_blinkLight val)
{
    leftBlink=val&0x01;
    rightBlink=(val&0x02)>>1;
    bigBertaSetOut();
}

void bigBertaSetBip(uint8_t val) //value from 0 to 5
{
    bip=val;
    bigBertaSetOut();
}

void bigBertaSetBlinkSound(bool val)
{
    blinkSound=val;
    bigBertaSetOut();
}

void bigBertaSetAux1(bool val)
{
    aux1=val;
    bigBertaSetOut();
}

void bigBertaSetAux2(bool val)
{
    aux2=val;
    bigBertaSetOut();
}

void bigBertaSetAux3(bool val)
{
    aux3=val;
    bigBertaSetOut();
}

void bigBertaSetAuxVbat(bool val)
{
    auxVbat=val;
    bigBertaSetOut();
}



bool bigBertaGetLight()
{
    return light;
}

uint32_t bigBertaGetNbBlink()
{
    return leftBlink+rightBlink;
}

uint32_t bigBertaGetLeftBlink()
{
    return leftBlink;
}

uint32_t bigBertaGetRightBlink()
{
    return rightBlink;
}

bool bigBertaGetHorn()
{
    return horn;
}

bool bigBertaGetBrakeLight()
{
    return breakLight;
}

bool bigBertaGetSound()
{
    return bip;
}

bool bigBertaGetAux1()
{
    return aux1;
}

bool bigBertaGetAux2()
{
    return aux2;
}

bool bigBertaGetAux3()
{
    return aux3;
}

bool bigBertaGetAuxVbat()
{
    return auxVbat;
}


#ifdef BB_V3
int8_t bigBertaGetTemperature(uint8_t pt)
{
    return temperature[pt];
}

int16_t bigBertaGetCurrent(uint8_t pt)
{
    return current[pt];
}

int16_t bigBertaGetMagneticField(uint8_t pt)
{
    return magneticField[pt];
}

int16_t bigBertaGetAcceleration(uint8_t pt)
{
    return acceleration[pt];
}

int16_t bigBertaGetAngularRate(uint8_t pt)
{
    return angularRate[pt];
}

uint16_t bigBertaGetAdcIn1()
{
    return bigBertaAdcValue[0];
}

uint16_t bigBertaGetAdcIn2()
{
    return bigBertaAdcValue[1];
}

#endif

void bigBertaSetOut()
{
    can_t msgOut;
    msgOut.id=CAN_BIG_BERTA_SET_OUT;
    msgOut.length=7;
    msgOut.flags.extended=0;
    msgOut.flags.rtr=0;
    msgOut.data[0]=light;
    msgOut.data[1]=leftBlink;
    msgOut.data[2]=rightBlink;
    msgOut.data[3]=horn;
    msgOut.data[4]=breakLight;
    msgOut.data[5]=bip;
    msgOut.data[6]=blinkSound;
    bool updateOut=0;
    for(int l=0;l<msgOut.length;l++)
    {
        if(lastConfirmOutReceive.data[l]!=msgOut.data[l])
        {
            updateOut=1;
        }
    }

    //fixme only in waiting fix can paquet lose
    /*static int debug=0;
    debug++;
    updateOut=!(debug%100);*/

    volatile uint64_t tic=(uint64_t)Clock_getTicks()*Clock_tickPeriod;

    if(updateOut)
    {
        static uint64_t timeLastSend=0;
        if(tic>(timeLastSend+50000) | tic<timeLastSend /*overflow*/) //if last send as more of 5ms old resend //fixme need to be ||
        {
            timeLastSend=tic;
            canSendMessage(&msgOut);
        }
    }

#ifdef BB_V3
    can_t msgAux;
    msgAux.id=CAN_BIG_BERTA_SET_AUX;
    msgAux.length=4;
    msgAux.flags.extended=0;
    msgAux.flags.rtr=0;
    msgAux.data[0]=aux1;
    msgAux.data[1]=aux2;
    msgAux.data[2]=aux3;
    msgAux.data[3]=auxVbat;
    bool updateAux=0;
    for(int l=0;l<msgAux.length;l++)
    {
       if(lastConfirmAuxReceive.data[l]!=msgAux.data[l])
       {
           updateAux=1;
       }
    }

    if(updateAux)
    {
        static uint64_t timeLastSend=0;
        if(tic>(timeLastSend+50000) | tic<timeLastSend /*overflow*/) //if last send as more of 5ms old resend
        {
            timeLastSend=tic;
            canSendMessage(&msgAux);
        }
    }
#endif
}

void bigBertaSetRegenForce(uint16_t force) //0 to 2048
{
    can_t msg;
    msg.id=CAN_BIG_BERTA_SET_PWM_REGEN;
    msg.length=2;
    msg.flags.extended=0;
    msg.flags.rtr=0;
    msg.data[0]=force>>8;
    msg.data[1]=force;
    canSendMessage(&msg);
}

t_BbStatus bigBertaGetStatus()
{
    if(getTimeStampMs()-timeLastData>BB_COM_TIMEOUT)
    {
        return BB_TIMEOUT;
    }
    else
    {
        return BB_OK;
    }
}

void bigBertaDecodeCan(can_t msg)
{
    if(msg.length==0) //bug sur le can on vois un mesage 0x105 avec un length de 0 on ne le prend donc pas en compte
        return;

    switch(msg.id)
    {
    case CAN_BIG_BERTA_VALUE_0:
            bigBertaPasSpeed=(((uint16_t)msg.data[0])<<8)|msg.data[1];
            regenNewValue=1;
            timeLastData=getTimeStampMs();
        break;
    case CAN_BIG_BERTA_VALUE_ADC:
        for(int l=0;l<3;l++)
        {
            bigBertaAdcValue[l]=(((uint16_t)msg.data[0+l*2])<<8)|msg.data[1+l*2];
        }
        bigBertaPrechargeEnd=msg.data[6];
        adcNewValue=1;
        break;
    case CAN_BIG_BERTA_CONFIRM_OUT:
        lastConfirmOutReceive=msg;
        break;

    case CAN_BIG_BERTA_CONFIRM_SET_POWER_MODE:
        lastConfirmPowerModeReceive=(t_powerState)msg.data[0];
        
        break;

#ifdef BB_V3

    case CAN_BIG_BERTA_CONFIRM_AUX:
        lastConfirmAuxReceive=msg;
        break;

    case CAN_BIG_BERTA_VALUE_TEMPERATURE_CURRENT:
        temperature[0]=msg.data[0];
        temperature[1]=msg.data[1];
        current[0]=msg.data[2]<<8|msg.data[3];
        current[1]=msg.data[4]<<8|msg.data[5];
        break;

    case CAN_BIG_BERTA_VALUE_MANGETOMETRE:
        magneticField[0]=msg.data[0]<<8|msg.data[1];
        magneticField[1]=msg.data[2]<<8|msg.data[3];
        magneticField[2]=msg.data[4]<<8|msg.data[5];
            break;

    case CAN_BIG_BERTA_VALUE_ACCELEROMETER:
        acceleration[0]=msg.data[0]<<8|msg.data[1];
        acceleration[1]=msg.data[2]<<8|msg.data[3];
        acceleration[2]=msg.data[4]<<8|msg.data[5];
            break;

    case CAN_BIG_BERTA_VALUE_GYROSCOPE:
        angularRate[0]=msg.data[0]<<8|msg.data[1];
        angularRate[1]=msg.data[2]<<8|msg.data[3];
        angularRate[2]=msg.data[4]<<8|msg.data[5];
        if(callBackImu)
            callBackImu();
            break;

#endif

    }
}

void bigBertaInit()
{
    canRegistreCallBack(0,bigBertaDecodeCan);
}
