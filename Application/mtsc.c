/*
 * Copyright (c) 2015-2020, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *  ======== uartecho.c ========
 */
#include <stdint.h>
#include <stddef.h>

#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/knl/Event.h>
#include <ti/sysbios/knl/Queue.h>
#include <ti/sysbios/knl/Semaphore.h>
#include <ti/drivers/utils/List.h>
#include <ti/drivers/dpl/HwiP.h>

/* Driver Header files */
#include <ti/drivers/GPIO.h>
#include <ti/drivers/UART.h>
#include <ti/drivers/SPI.h>
//#include <ti/drivers/ADC.h>
#include <ti/drivers/dpl/ClockP.h>
#include <ti/drivers/NVS.h>
#include <ti/drivers/nvs/NVSSPI25X.h>
#include <ti/drivers/Timer.h>
#include <ti/sysbios/knl/Semaphore.h>

#include <smartBmsService.h>

//#include "zlib.h"

#include <icall.h>
#include <bcomdef.h>
#include <mtsc.h>
#include "config.h"

/* Driver configuration */
#include "ti_drivers_config.h"

#include "pixxil_driver.h"
#include "pixxil_graphics.h"
//#include "qr_code_gen.h"
#include "mcp2515_driver.h"
#include "bms.h"
#include "tiny_BMS.h"
//#include "asi_can.h"
#include "vesc_can.h"
#include "bootLoader.h"
#include "W25N01GV_driver.h"
#include "tracy.h"
#include "screen.h"

#include "simple_peripheral.h"
#include "batteryService.h"
#include "bigBerta.h"
#include "tinyBms.h"
#include "timestamp.h"

#include "virtualUartService.h"
//#include "zlib.h"
#include "util.h"


//#define SIMULATION  1

#define NORMAL_TORK_SLOPE   1.2 // 1.5
#define FREQU_CONTROL_THREAD 200
#define CTRL_PERIODIC_EVT 1

typedef enum {NODEGR, DEGRA, DEGRB} degraded_t;

typedef struct
{
    uint8_t event;                //
    uint8_t data[];
} spClockEventData_t;

uint8_t testv=0;

static Clock_Struct clkPeriodic;

Semaphore_Handle semTimerControlThread;
Semaphore_Params semTimerControlThreadParams;

Semaphore_Handle semTimerPeriodiqueThread;
Semaphore_Params semTimerPeriodiqueThreadParams;

spClockEventData_t argPeriodicCtrl =
{ .event = CTRL_PERIODIC_EVT };

//------------- MtSC specific variables ------------------

// user/vehicle specific datas
volatile t_infoUserVehicle vInfos = {.ready=0};
volatile t_ctrlVehicle vCtrl = {.brakeState=0, .lightState=0, .realSpeed=0, .mode=OFF, .vMode=Normal, .trips=TRIP};
degraded_t degraded = NODEGR;

Timer_Handle timerhHandle=NULL;
Timer_Params timerParams;
bool fbtn1=false;   // detect kind of pressure (short or long) on screen btn
bool fbtn2=false;   // detect kind of pressure (short or long) on help btn
bool btn1LP=false;    // flag set if long pressure on btn 1
bool btn1SP=false;    // flag set if short pressure on btn1
bool btn2LP=false;
bool btn2SP=false;

extern bool screenThReady;

float torkSlope=NORMAL_TORK_SLOPE;

static void clockHandler200Hz(UArg arg)
{
    spClockEventData_t *pData = (spClockEventData_t *)arg;

    if (pData->event == CTRL_PERIODIC_EVT)
    {
        Semaphore_post(semTimerControlThread);
        static uint8_t div100ms=0;
        div100ms+=1000/FREQU_CONTROL_THREAD;
        if(div100ms>=100)
        {
            Semaphore_post(semTimerPeriodiqueThread);
            div100ms=0;
        }
    }
}

void sleepy(int ms){
    while(ms>0){
        ClockP_usleep(1000);
        ms--;
    }
}
void blinkLed(int times){
    while(times>0){
        GPIO_toggle(LED);
        sleepy(500);
        times--;
    }
}
void gpios_callback(uint_least8_t index){
    if(timerhHandle==NULL) //timer not configured
        return;
    switch(index){
    case CONFIG_BTN_SCREEN: //CONFIG_BTN_SCREEN
        fbtn2 = true;
        Timer_start(timerhHandle);
        break;
    case CONFIG_BTN_HELP: // CONFIG_BTN_HELP
        fbtn1 = true;
        Timer_start(timerhHandle);
        break;
    }
}
void updateTorkSlope(Vmode vmode, bool degraded){
    switch(vmode){
    case Eco:    torkSlope = (degraded==true) ? bikeConfig.degrtorkecoslope : bikeConfig.ecotorkslope;
    break;
    case Normal: torkSlope = (degraded==true) ? bikeConfig.degrtorknormslope : bikeConfig.normtorkslope;
    break;
    case Sport:  torkSlope = (degraded==true) ? bikeConfig.degrtorksportslope : bikeConfig.sportorkslope;
    break;
    }
}
#define MVAV_TORK_SIZE  10 // 5
float movingAverageTork(float tork){
    static char i=0;
    static float torks[MVAV_TORK_SIZE] = {0.0};
    float buf_t=0.0;
    torks[i] = tork;
    i=(i+1)%MVAV_TORK_SIZE;

    char j=0;
    for (j = 0; j < MVAV_TORK_SIZE; j++) {
        buf_t += torks[j];
    }
    return buf_t/MVAV_TORK_SIZE;
}
float computeCurrent(float tor, int16_t pasSpeed){
    float tork=0.0;
    float tork_I=0.0;
    float speed=0.0;
    float current=0.0;

    tork = (torkSlope*pow(tor,3))/bikeConfig.maxcurrent;
    tork_I = movingAverageTork(tork);  //fixme move to 0 if we brake?
    speed = (pasSpeed*7.0)/bikeConfig.maxcurrent;

    // current compute:
    // (tork + integrated tork) + speed
    current = ((tork*bikeConfig.torkcoeff)+(tork_I*bikeConfig.torkintcoeff))+(speed*bikeConfig.speedcoeff);

    /* if(!degraded){
        tork = (torkSlope*pow(tor,3))/bikeConfig.maxcurrent;
        tork_I = movingAverageTork(tork);
        speed = (pasSpeed*7.0)/bikeConfig.maxcurrent;

        // current compute:
        // (tork + integrated tork) + speed
        current = ((tork*bikeConfig.torkcoeff)+(tork_I*bikeConfig.torkintcoeff))+(speed*bikeConfig.speedcoeff);
    }else{
        tork = (torkSlope*pow(tor,3))/bikeConfig.degrmaxcurrent;
        tork_I = movingAverageTork(tork);
        speed = (pasSpeed*7.0)/bikeConfig.degrmaxcurrent;
        current = ((tork*bikeConfig.degrtorkcoeff)+(tork_I*bikeConfig.degrtorkintcoeff))+(speed*bikeConfig.degrtorkspeedcoeff);
    }   */

    if(vCtrl.vMode == Eco){
        if(current > bikeConfig.ecocurrent){
            current = bikeConfig.ecocurrent;
        }
    }

    return current;
}
// called every 5ms
void requestDataBMS(){
    static char divT=0;
    // requesting new datas from BMS
    divT++;
    if(divT>=50)
        divT=0;
    switch(divT)
    {
    case 0:
        //vesc_can_get_imu(0);
        break;
    case 5:
        request_bms(BMS_S_SOC);
        break;
    case 10:
        request_bms(BMS_S_VOLTAGE);
        break;
    case 15:
        request_bms(BMS_S_CURRENT);
        break;
    case 20:
        request_bms(BMS_S_STATUS);
        break;
    case 25:
        request_bms(BMS_S_CELLS_VOLTAGE);
        break;
    case 30:
        request_bms(BMS_S_TEMPERATURES);
        break;
    }
}

/*
 *  ======== controlThread ===========
 *  This thread is managing the powertrain
 *  and program states
 */
void controlThread(UArg a0, UArg a1)
{
    float tork=0.0;
    float current=0.0;
    float brakeCur=0.0;
    volatile uint16_t cptTracy=0;
    volatile uint16_t cptLock=0; // count to 6000 (30 secondes)
    bool unlockBLE=0;
    float trig=0.0;

    ICall_EntityID selfEntity;
    ICall_SyncHandle syncEvent;
    ICall_registerApp(&selfEntity, &syncEvent);
    Semaphore_Params_init(&semTimerControlThreadParams);
    semTimerControlThread = Semaphore_create(0, &semTimerControlThreadParams, NULL); /* Memory allocated in here */


    initTimeStamp();

    ClockP_sleep(1);//wait simplePeripheral is init
    canEnable();
    bigBertaInit();
    vescInit();
    init_tiny_BMS();
    configInit();
    tracy_init();


    for(int l=0;l<5;l++)
    {
        volatile int l;
        GPIO_write(LED, 0);
        for(l=0;l<100000;l++);
        GPIO_write(LED, 1);
        for(l=0;l<100000;l++);
    }

    GPIO_write(LED, 0);

    if(bikeDatas.assistmode == MTSC__ASSIST_MODES__AM_SPORT)
        vCtrl.vMode = Sport;
    if(bikeDatas.assistmode == MTSC__ASSIST_MODES__AM_ECO)
        vCtrl.vMode = Eco;
    if(bikeDatas.assistmode == MTSC__ASSIST_MODES__AM_NORMAL)
        vCtrl.vMode = Normal;

    int b = 20;
#ifndef SIMULATION
    // wait for precharge to be done
    //while(!bigBertaPrechargeEnd){
    while(b>0){
        ClockP_usleep(5000);
        request_bms(BMS_S_SOC);
        ClockP_usleep(5000);
        bigBertaSetPowerState(POWER_STATE_FULL);
        b--;
    }
    //char req = 5;
    //    while(!isTracyConnected() && req>0){
    //        requestPing();
    //        sleepy(100);
    //        req--;
    //    }
#endif

    while(!vInfos.ready){sleepy(10);}

    while(b>0){
        ClockP_usleep(5000);
        request_bms(BMS_S_SOC);
        ClockP_usleep(5000);
        bigBertaSetPowerState(POWER_STATE_FULL);
        b--;
    }

    Util_constructClock(&clkPeriodic, clockHandler200Hz,
                        10, 1000/FREQU_CONTROL_THREAD, true, (UArg)&argPeriodicCtrl);

    // somwhere should check battery & ESC status and make sure everything is fine
    while(1){

        Semaphore_pend(semTimerControlThread, 1000*1000/Clock_tickPeriod);

        //GPIO_toggle(LED);
        //        vCtrl.lightState = !vCtrl.lightState;   // only for test purpose
        //        bigBertaSetLight(vCtrl.lightState);     // only for test purpose
        switch(vCtrl.mode){
        case WELCOME:
            while(!screenThReady){sleepy(10);}
            screenThReady=false;
            vCtrl.mode = NORMAL;
            break;
        case NORMAL:
        case GPS:
            bigBertaSetHorn(GPIO_read(CONFIG_HORN));
            vCtrl.lightState = 1;
            bigBertaSetLight(vCtrl.lightState);
            bigBertaSetPowerState(POWER_STATE_FULL);

            // first managing the brake current
            if((GPIO_read(BRAKE_L)==bikeConfig.brakingstate) || (GPIO_read(BRAKE_R)==bikeConfig.brakingstate)){
                if(bikeConfig.brakingtrigger){
                    if((GPIO_read(BRAKE_L)==bikeConfig.brakingstate) && (GPIO_read(BRAKE_R)==bikeConfig.brakingstate)){
                        brakeCur = bikeConfig.brakecurrent;
                    }else{
                        brakeCur = bikeConfig.brakecurrent/2;
                    }
                }else{
                    brakeCur = bikeConfig.brakecurrent;
                }
                //brakeCur = (BRAKE_CURRENT*GPIO_read(BRAKE_L))+(BRAKE_CURRENT*GPIO_read(BRAKE_R));
                // if activ use motor brake
                vCtrl.brakeState=1;
                bigBertaSetBreakLight(vCtrl.brakeState);
                static unsigned int div=0;
                div++;
                if(div>=4)
                {
                    vesc_can_set_current_brake(VESC_BROADCAST,brakeCur);
                }
                movingAverageTork(0.0);
                adcNewValue=0;
                regenNewValue=0;
            }else{
                if(adcNewValue){
                    adcNewValue=0;
                    regenNewValue=0;
                    vCtrl.brakeState=0;
                    bigBertaSetBreakLight(vCtrl.brakeState);
                    tork = (float)bigBertaAdcValue[2]/1000.0;
                    updateTorkSlope(vCtrl.vMode, ((degraded==DEGRA) || (degraded==DEGRB)) ? true:false);
                    current = computeCurrent(tork, bigBertaPasSpeed);
                    if(bikeConfig.testmode == 66){
                        if(vCtrl.vMode == Sport){
                            //comm_can_set_rpm(VESC_BROADCAST, bikeDatas.erpm);
                            comm_can_set_rpm(VESC_BROADCAST, 14000);
                            //vesc_can_set_current_rel(VESC_BROADCAST, 1.0);
                        }
                    }else{
                        if(bikeConfig.testmode == 99){
                            if(vCtrl.vMode == Sport){
                                if(vCtrl.realSpeed<11){
                                    vesc_can_set_current_rel(VESC_BROADCAST, 0.8);
                                }else{
                                    vesc_can_set_current_rel(VESC_BROADCAST, 1.0);
                                }
                            }
                        }else{
                            trig = ((degraded==DEGRA) || (degraded==DEGRB)) ? bikeConfig.degrcurrtrig : bikeConfig.currenttrig;
                            //current = (degraded==true) ? current*bikeConfig.currpercdegraded : current;
                            if(current > trig){
                                // degradation depending on temperature
                                if(degraded==DEGRA){
                                    current = current * (1.0f-((vesc_get_value(0).temp_motor-bikeConfig.tempstartdegraded)/
                                            ((bikeConfig.temperatureemergency-bikeConfig.tempstartdegraded)*bikeConfig.betafactor)));
                                }else{
                                    // degradation limited by a parameter
                                    if(degraded==DEGRB){
                                        current = current * bikeConfig.currpercdegraded;
                                    }
                                }
                                vesc_can_set_current_rel(VESC_BROADCAST, current);
                            }else{
                                vesc_can_set_current_rel(VESC_BROADCAST, 0.0);
                            }
                        }
                    }
                }
            }
            if(btn2SP){
                btn2SP=false;
                if(vCtrl.trips == TRIP){
                    vCtrl.trips = TRIPA;
                }else if(vCtrl.trips == TRIPA){
                    vCtrl.trips = TRIPB;
                }else if(vCtrl.trips == TRIPB){
                    vCtrl.trips = TRIP;
                }
            }
            if(btn1SP){
                btn1SP=false;
                if(vCtrl.vMode == Normal){
                    vCtrl.vMode = Sport;
                    bikeDatas.assistmode = MTSC__ASSIST_MODES__AM_SPORT;
                }else if(vCtrl.vMode == Sport){
                    vCtrl.vMode = Eco;
                    bikeDatas.assistmode = MTSC__ASSIST_MODES__AM_ECO;
                }else if(vCtrl.vMode == Eco){
                    vCtrl.vMode = Normal;
                    bikeDatas.assistmode = MTSC__ASSIST_MODES__AM_NORMAL;
                }
                //    updateDatasCharacteristique();
            }else{
                if(assistChanged){// if changed from the BLE/App
                    assistChanged=0;
                    if(bikeDatas.assistmode == MTSC__ASSIST_MODES__AM_SPORT)
                        vCtrl.vMode = Sport;
                    if(bikeDatas.assistmode == MTSC__ASSIST_MODES__AM_ECO)
                        vCtrl.vMode = Eco;
                    if(bikeDatas.assistmode == MTSC__ASSIST_MODES__AM_NORMAL)
                        vCtrl.vMode = Normal;
                }
            }
            break;
        case AUTHENT:
            vCtrl.lightState = 0;
            bigBertaSetLight(vCtrl.lightState);
            if(unlockBLE){
                unlockBLE=0;
                vCtrl.mode = WELCOME;
            }else{
                if(isTracyConnected()){
                    if(rfidReceived){
                        rfidReceived = false;
                        if(isRfidOK()){
                            vCtrl.mode = WELCOME;
                            cptLock=0;
                        }
                    }else{
                        cptLock++;
                        if(cptLock>6000) {
                            vCtrl.mode = OFF;
                            cptLock=0;
                        }
                    }
                }else{
                    cptTracy++;
                    //requestPing();
                    if(cptTracy>1500){ // try to detect tracy during 8 seconds
                        vCtrl.mode = WELCOME;
                        cptTracy=0;
                    }
                }
            }
            break;
        case OFF: // save anything!
            vCtrl.lightState = 0;
            bigBertaSetLight(vCtrl.lightState);
            bigBertaSetPowerState(POWER_STATE_LOW);
            movingAverageTork(0.0);
            //ClockP_sleep(1);
            cptLock=0;
            cptTracy=0;
            unlockBLE=0;
            //                ClockP_usleep(100000);
            break;
        case ON:
            // restore anything
            //  vCtrl.lightState = 1;
            while(!screenThReady){
                sleepy(10);
                bigBertaSetPowerState(POWER_STATE_FULL);
                //  bigBertaSetLight(vCtrl.lightState);//resend for ensure is receive by BB
            }
            screenThReady=false;
            vCtrl.mode = AUTHENT;
            break;
        case CHARGING:
            bigBertaSetPowerState(POWER_STATE_LOW);
            if(bms_get_value().stat != BMS_S_CHARGING) vCtrl.mode=OFF;
            break;
        }

        static int timePressed=0;
        if(GPIO_read(CONFIG_BTN_HELP))
        {
            timePressed++;
            if(timePressed==FREQU_CONTROL_THREAD*1.5)
            {
                if((vCtrl.mode==ON) || (vCtrl.mode==GPS) || (vCtrl.mode==NORMAL) || (vCtrl.mode==WELCOME) || (vCtrl.mode==AUTHENT)){
                    vCtrl.mode = OFF;
                    //bikeDatas.powermode=false;
                }else{
                    vCtrl.mode = ON;
                }
            }
        }
        else
        {
            timePressed=0;
        }


        // from dataService
        if(powerChanged){
            powerChanged=0;
            unlockBLE=1;
            if(bikeDatas.powermode){
                if(vCtrl.mode == OFF)
                    vCtrl.mode = ON;
            }else{
                vCtrl.mode = OFF;
            }
        }

        if(requestReboot){
            if(vCtrl.realSpeed<1){
                bootLoaderReboot();
            }
        }

        if(bms_get_value().stat == BMS_S_CHARGING) vCtrl.mode=CHARGING;


        // managing the degraded modes
        if(vesc_get_value(0).temp_motor > bikeConfig.tempstartdegraded){
            degraded=DEGRA;
            if(vesc_get_value(0).temp_motor > bikeConfig.temperatureemergency) degraded=DEGRB;
        }else{
            if(vesc_get_value(0).temp_motor < bikeConfig.tempstopdegraded) degraded=NODEGR;
        }

        requestDataBMS();
    }
}

#define SECONDE_TREAD_STACK_SIZE 2048
uint8_t controlThreadStack[SECONDE_TREAD_STACK_SIZE];
Task_Struct controlThreadTask;

void controlThread_createTask(void)
{
    Task_Params taskParams;

    // Configure task
    Task_Params_init(&taskParams);
    taskParams.stack = controlThreadStack;
    taskParams.stackSize = SECONDE_TREAD_STACK_SIZE;
    taskParams.priority = 3;

    Task_construct(&controlThreadTask, controlThread, &taskParams, NULL);
}

void buttonsTimerCallback(Timer_Handle handle, int_fast16_t status){ //read GPIO 300ms after change for debounce
    // if(vCtrl.mode != CHARGING){
    if(fbtn1 && fbtn2){
        fbtn1=false;
        fbtn2=false;
        if(vCtrl.mode == NORMAL){
            requestReboot=true;
        }
    }else{
        if(fbtn1){
            fbtn1=false;
            if(GPIO_read(CONFIG_BTN_HELP)){
                btn1LP=true;
                btn1SP=false;
            }else{
                btn1LP=false;
                btn1SP=true;
            }
        }else{
            if(fbtn2){
                fbtn2=false;
                if(GPIO_read(CONFIG_BTN_SCREEN)){
                    btn2LP=true;
                    btn2SP=false;
                }else{
                    btn2LP=false;
                    btn2SP=true;
                }
            }
        }
    }
    //}
}

void periodicTask(UArg a0, UArg a1){
    Semaphore_Params_init(&semTimerPeriodiqueThreadParams);
    semTimerPeriodiqueThread = Semaphore_create(0, &semTimerPeriodiqueThreadParams, NULL); /* Memory allocated in here */

    bool save=1;
    uint32_t nextTick;

    uint8_t soc=0;

    ICall_EntityID selfEntity;
    ICall_SyncHandle syncEvent;
    ICall_registerApp(&selfEntity, &syncEvent);
    SmartBmsService_AddService();

    Timer_Params_init(&timerParams);
    timerParams.periodUnits = Timer_PERIOD_US;
    timerParams.period = 300000;
    timerParams.timerMode = Timer_ONESHOT_CALLBACK;
    timerParams.timerCallback = buttonsTimerCallback;
    timerhHandle = Timer_open(CONFIG_TIMER_1, &timerParams);

    // nvs variables for kms
    NVS_Params nvsParams;
    NVS_Attrs regionAttrs;
    NVS_Handle nvsHandle;
    uint32_t ptNvs;

    // nvs variables for tripA
    NVS_Params nvsParamsA;
    NVS_Attrs regionAttrsA;
    NVS_Handle nvsHandleA;
    uint32_t ptNvsA;

    // nvs variables for tripB
    NVS_Params nvsParamsB;
    NVS_Attrs regionAttrsB;
    NVS_Handle nvsHandleB;
    uint32_t ptNvsB;

    // variables for compute kms
    double oldKmTotal=0.0;
    double oldTripA=0.0;
    double oldTripB=0.0;
    double tacho=0.0;
    double diffDist=0.0;
    double oldTach = vesc_get_value(0).tacho_value; ////i think that nor work because vesc is not started


    // init vehicle/user structure
    vInfos.ready = 0;

    // init kilometer's flash registration
    initTripNVS(&nvsParams, &regionAttrs, &nvsHandle, &ptNvs, TRIP);

    // init kilometer's flash registration
    initTripNVS(&nvsParamsA, &regionAttrsA, &nvsHandleA, &ptNvsA, TRIPA);

    // init kilometer's flash registration
    initTripNVS(&nvsParamsB, &regionAttrsB, &nvsHandleB, &ptNvsB, TRIPB);


    oldKmTotal=vInfos.kmTot;
    oldTripA=vInfos.tripA;
    oldTripB=vInfos.tripB;

    vInfos.ready = 1;

    ClockP_sleep(1);    //wait simplePeripheral is init

    nextTick = Clock_getTicks();

    while(1){

        bmsLbInteg(vCtrl.mode != OFF, 0.1);

        switch(vCtrl.mode){
        case WELCOME:
        case NORMAL:
        case GPS:
            save=1;
            // vehicle specific
            tacho = vesc_get_value(0).tacho_value;
            diffDist = ABS((tacho-oldTach)*TACHO_TO_KM)*bikeConfig.tripmultiplier; //fix me pourquoi * 0.65?
            if(diffDist>0.1) //not possible value currently due to a restart of VESC
                diffDist=0;
            vInfos.kmTot += diffDist;
            vInfos.tripA += diffDist;
            vInfos.tripB += diffDist;
            //if((uint16_t)oldKmTotal!=(uint16_t)vInfos.kmTot){
            // register every ~50 meters
            if(ABS(vInfos.kmTot-oldKmTotal)>0.05){
                registerTripToNVS(&nvsHandle, &ptNvs, TRIP);
                bikeDatas.odo = vInfos.kmTot;
                oldKmTotal=vInfos.kmTot;
            }
            if(ABS(vInfos.tripA-oldTripA)>0.05){
                registerTripToNVS(&nvsHandleA, &ptNvsA, TRIPA);
                bikeDatas.tripa = vInfos.tripA;
                oldTripA=vInfos.tripA;
            }
            if(ABS(vInfos.tripB-oldTripB)>0.05){
                registerTripToNVS(&nvsHandleB, &ptNvsB, TRIPB);
                bikeDatas.tripb = vInfos.tripB;
                oldTripB=vInfos.tripB;
            }
            oldTach = tacho;
            if(btn2LP){
                btn2LP=false;
                if(vCtrl.trips == TRIPA) {
                    vInfos.tripA=0.0;
                    oldTripA = 0.0;
                    registerTripToNVS(&nvsHandleA, &ptNvsA, TRIPA);
                }
                if(vCtrl.trips == TRIPB) {
                    vInfos.tripB=0.0;
                    oldTripB = 0.0;
                    registerTripToNVS(&nvsHandleB, &ptNvsB, TRIPB);
                }
            }
            break;
        case AUTHENT:
            break;
        case OFF: // save anything!
            if(save){
                registerTripToNVS(&nvsHandle, &ptNvs, TRIP);
                registerTripToNVS(&nvsHandleA, &ptNvsA, TRIPA);
                registerTripToNVS(&nvsHandleB, &ptNvsB, TRIPB);
                oldKmTotal=vInfos.kmTot;
                oldTripA=vInfos.tripA;
                oldTripB=vInfos.tripB;
                save=0;
            }
            // lock mutex ? doing so will make the task sleep (this task is no longer needed during "OFF")
            break;
        case ON:
            // restore anything
            break;
        }

        //        if(vCtrl.mode != CHARGING) computeButtonsState();

        configCompute();
        //soc = (uint8_t)bms_get_soc();
        soc = (uint8_t)bmsLbGetSoc();
        Battery_SetParameter(BATTERY_LEVEL,sizeof(soc),(void*)&soc);
        bikeDatas.soc = soc;
        if(vCtrl.mode != OFF) {
            bikeDatas.powermode=true;
        }else{
            bikeDatas.powermode=false;
        }
        updateDatasCharacteristique();

        static int test;
        test++;
        GPIO_write(LED,(test/2)%2);

        if(!isTracyConnected()){
            requestPing();
        }else{
            if(!ccidReceived){
                requestCcid();
            }
        }
        if(resetOdo){
            resetOdo=0;
            vInfos.kmTot=bikeDatas.odo;
        }

        Semaphore_pend(semTimerPeriodiqueThread, 1000*1000/Clock_tickPeriod); //200ms 5Hz

    }
}

#define PERIODIC_STACK_SIZE (1*1024)
uint8_t periodicStack[PERIODIC_STACK_SIZE];
Task_Struct ledTask;

void periodic_createTask(void)
{
    Task_Params taskParams;

    // Configure task
    Task_Params_init(&taskParams);
    taskParams.stack = periodicStack;
    taskParams.stackSize = PERIODIC_STACK_SIZE;
    taskParams.priority = 1;

    Task_construct(&ledTask, periodicTask, &taskParams, NULL);
}

void initTripNVS(NVS_Params *nvsParams, NVS_Attrs *regionAttrs, NVS_Handle *nvsHandle, uint32_t *ptNvs, enum TRIPS trip){
    float bufKmTot=0.0;
    float bufKmA=0.0;
    float bufKmB=0.0;
    // then get the kilometers value
    NVS_Params_init(nvsParams);
    switch(trip){
    case TRIP: *nvsHandle=NVS_open(CONFIG_NVS_KM, nvsParams); break;
    case TRIPA: *nvsHandle=NVS_open(CONFIG_NVS_TRIPA, nvsParams); break;
    case TRIPB: *nvsHandle=NVS_open(CONFIG_NVS_TRIPB, nvsParams); break;
    }

    while(*nvsHandle == NULL){ // can't open NVS region
        sleepy(500);
    }
    NVS_getAttrs(*nvsHandle, regionAttrs);
    while(NVS_SECTOR_SIZE!=regionAttrs->sectorSize);

    // then read and get the addresse of the kilometers value
    for(*ptNvs=0;*ptNvs<NVS_SECTOR_SIZE;*ptNvs+=4){
        uint32_t data;
        NVS_read(*nvsHandle, *ptNvs, &data, sizeof(data));
        if(data==0xFFFFFFFF)
            break;
    }

    //memory empty
    if(*ptNvs==0){
        switch(trip){
        case TRIP: vInfos.kmTot=0.0; break;
        case TRIPA: vInfos.tripA=0.0; break;
        case TRIPB: vInfos.tripB=0.0; break;
        }
    }else{
        switch(trip){
        case TRIP:
            NVS_read(*nvsHandle, (*ptNvs)-4, (void*)&bufKmTot, sizeof(bufKmTot));
            vInfos.kmTot = (double)bufKmTot;
            break;
        case TRIPA:
            NVS_read(*nvsHandle, (*ptNvs)-4, (void*)&bufKmA, sizeof(bufKmA));
            vInfos.tripA = (double)bufKmA;
            break;
        case TRIPB:
            NVS_read(*nvsHandle, (*ptNvs)-4, (void*)&bufKmB, sizeof(bufKmB));
            vInfos.tripB = (double)bufKmB;
            break;
        }
    }

    // if flash isn't filled with 0xFFFFFF
    switch(trip){
    case TRIP:
        if(vInfos.kmTot<0.0) {
            vInfos.kmTot=0.0;
            bufKmTot=0.0;
            NVS_erase(*nvsHandle, 0, NVS_SECTOR_SIZE);
            NVS_write(*nvsHandle, 0, (void*)&bufKmTot, sizeof(bufKmTot), 0);
        }
        break;
    case TRIPA:
        if(vInfos.tripA<0.0) {
            vInfos.tripA=0.0;
            bufKmA=0.0;
            NVS_erase(*nvsHandle, 0, NVS_SECTOR_SIZE);
            NVS_write(*nvsHandle, 0, (void*)&bufKmA, sizeof(bufKmA), 0);
        }
        break;
    case TRIPB:
        if(vInfos.tripB<0.0) {
            vInfos.tripB=0.0;
            bufKmB=0.0;
            NVS_erase(*nvsHandle, 0, NVS_SECTOR_SIZE);
            NVS_write(*nvsHandle, 0, (void*)&bufKmB, sizeof(bufKmB), 0);
        }
        break;
    }

}

void registerTripToNVS(NVS_Handle *nvsHandle, uint32_t *ptNvs, enum TRIPS trip){
    float bufKmTot=(float)vInfos.kmTot;
    float bufKmA=(float)vInfos.tripA;
    float bufKmB=(float)vInfos.tripB;

    if(*ptNvs>=NVS_SECTOR_SIZE){
        *ptNvs=0;
        NVS_erase(*nvsHandle, 0, NVS_SECTOR_SIZE);
    }
    switch(trip){
    case TRIP: NVS_write(*nvsHandle, *ptNvs, (void*)&bufKmTot, sizeof(bufKmTot), 0); break;
    case TRIPA: NVS_write(*nvsHandle, *ptNvs, (void*)&bufKmA, sizeof(bufKmA), 0); break;
    case TRIPB: NVS_write(*nvsHandle, *ptNvs, (void*)&bufKmB, sizeof(bufKmB), 0); break;
    }
    *ptNvs = (*ptNvs)+4;
}

void requestPairing()//call from simple_peripheral.c when a BLE pairing request is receive
{
    //fixme
    //un téléphone demande un pairing
    //sur le yak si la clé est sur on on afiche le code sur l'ecrant
}

void passCodeValidation(bool ok)
{
    //fixme ajouter le code desiré
    //est appelé apres que l'utilisateur ai mis le code de pairing sur sont telephone
    //si le code etait juste et que le pairing a marché ok=1
    //sinon ok=0
}


