/*
 * screen.c
 *
 *  Created on: 18 oct. 2024
 *      Author: Mehmed Blazevic
 */
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/knl/Event.h>
#include <ti/sysbios/knl/Queue.h>
#include <ti/drivers/utils/List.h>
#include <ti/drivers/dpl/HwiP.h>

#include <icall.h>
#include <bcomdef.h>
#include <mtsc.h>
#include <stddef.h>
#include <bms.h>


#include "screen.h"
#include "mtsc.h"
#include "config.h"
#include "pixxil_graphics.h"
#include "vesc_can.h"


volatile bool once=true;
volatile bool once2=false;
volatile Pixel pipix = {.x=190, .y=120};
bool freeze = false;

bool screenThReady=false;
extern volatile t_ctrlVehicle vCtrl;
extern volatile t_infoUserVehicle vInfos;

/*
 *  ======== screenThread ========
 */
void screenThread(UArg a0, UArg a1)
{
    //uint32_t nextTick;
    enum MTSC_Modes oldMode=OFF;
    GPIO_write(CONFIG_5V_P_EN, 0);
    uint16_t debug_temp=0;

    ClockP_sleep(1);//wait simplePeripheral is init

    //nextTick = Clock_getTicks();

    while(1){

        switch(vCtrl.mode){
            case WELCOME:
                screenThReady=true; // make sure to enter in WELCOME mode at least one time (for screen animation)
                startupScreen();
#ifndef DEBUG_TEMP
                computeDisplayTrips(true);
#endif
                computeDisplayBattery(true);
                oldMode=WELCOME;
            break;
            case NORMAL:
                computeDisplaySpeed();
#ifdef DEBUG_TEMP
                showTemperature(vesc_get_value(0).temp_motor);
                clearTrip();
                if(degraded==NODEGR) showTrip(0);
                if(degraded==DEGRA) showTrip(1);
                if(degraded==DEGRB) showTrip(2);
#else
                computeDisplayTrips(false);
#endif
                computeDisplayBattery(false);
                //computeDisplayDate(); //disabled for now
//                demo();
                oldMode=NORMAL;
            break;
            case AUTHENT:
                if(isTracyConnected()){
                    if(oldMode != AUTHENT) {
                        img_Show(ilock);
                        oldMode = AUTHENT;
                    }
                }
            break;
            case GPS:
                // show speed
                //computeDisplaySpeed();
            break;
            case CHARGING:
                if(oldMode==OFF){
                    GPIO_write(CONFIG_5V_P_EN, 1);
                    init_Pixxil(LCD_WIDTH, LCD_HEIGHT);
                    gfx_BGcolour(MTSC_BLACK);
                    showStatus(Charging);
                }else{
                    if(oldMode != CHARGING){
                        gfx_BGcolour(MTSC_BLACK);
                        gfx_CLS(MTSC_BLACK);
                        showStatus(Charging);
                    }
                }

                oldMode=CHARGING;
            break;
            case OFF:
                if((oldMode==ON)||(oldMode==CHARGING)||(oldMode==WELCOME)||(oldMode==NORMAL)||(oldMode==AUTHENT)){
                    close_Pixxil();
                    GPIO_write(CONFIG_5V_P_EN, 0);
                }
                ClockP_sleep(1);
                oldMode=OFF;
                // lock mutex ? doing so will make the task sleep (this task is no longer needed during "OFF")
            break;
            case ON:
                if(oldMode==OFF){
                  //  flag=true;
                    GPIO_write(CONFIG_5V_P_EN, 1);
                    init_Pixxil(LCD_WIDTH, LCD_HEIGHT);
                    gfx_BGcolour(MTSC_BLACK);
                    gfx_CLS(MTSC_BLACK);
                    screenThReady=true;
                }
                oldMode=ON;
            break;
        }

        if(oldMode != OFF){
            while(freeze){
                if(once){
                    gfx_CircleFilled(pipix, 2, 0xFFA0);
                    sleepy(150);
                    gfx_CircleFilled(pipix, 2, MTSC_BLACK);
                    once=false;
                    once2=true;
                }
                sleepy(10);
            }
            once=true;
            if(once2){
                gfx_CircleFilled(pipix, 1, 0xFFA0);
                sleepy(150);
                gfx_CircleFilled(pipix, 1, MTSC_BLACK);
                once2=false;
            }
        }

        /*nextTick+=50000/Clock_tickPeriod;//fix loop speed to 50ms (20Hz)
        int32_t wait=nextTick-Clock_getTicks();
        while(wait>0 && wait<1000000)
        {*/
            //wait=nextTick-Clock_getTicks();
            ClockP_usleep(1000);
        //}
    }
}

#define MAIN_TREAD_STACK_SIZE 2048
uint8_t screenThreadStack[MAIN_TREAD_STACK_SIZE];
Task_Struct screenThreadTask;

void screenThread_createTask(void)
{
    Task_Params taskParams;

    // Configure task
    taskParams.stack = screenThreadStack;
    taskParams.stackSize = MAIN_TREAD_STACK_SIZE;
    taskParams.priority = 1;

    Task_construct(&screenThreadTask, screenThread, &taskParams, NULL);
}

void showTemperature(float temp){
    uint8_t dizaine = ((uint8_t)temp)%100;
    uint8_t centaine = ((uint8_t)temp)/100;
    static uint8_t oldDiz = 0;
    static uint8_t oldCent = 0;

    if((oldDiz != dizaine) || (oldCent != centaine)){
        clearDate();
        showDate(centaine, dizaine, Pol3);
        oldDiz = dizaine;
        oldCent = centaine;
    }
}

void computeDisplayDate(){
    static uint16_t cpt=0;
    if(dateChanged){
        dateChanged=0;
        cpt=0;
        clearDate();
        showDate((int8_t)bikeDatas.hours,(int8_t)bikeDatas.minutes, Pol3);
    }else{
        cpt++;
    }
    if(cpt>3000){
        cpt=0;
        clearDate();
    }
}
void computeDisplayTrips(bool force){
    static uint16_t oldKmTot=0;
    static uint16_t oldKmTotA=0;
    static uint16_t oldKmTotB=0;
    static enum TRIPS oldTrip=TRIP;

    switch(vCtrl.trips){
        case TRIP:
            // if(show kmTot flag) // from periodic task
            if((ABS((uint16_t)vInfos.kmTot-oldKmTot)>=1) || (oldTrip != vCtrl.trips) || force){
                clearTrip();
                showTrip((uint16_t)vInfos.kmTot);
                oldKmTot=(uint16_t)vInfos.kmTot;
            }
        break;
        case TRIPA:
            if((ABS((uint16_t)vInfos.tripA-oldKmTotA)>=1) || (oldTrip != vCtrl.trips) || force){
                clearTrip();
                showTripP((uint16_t)vInfos.tripA,'a');
                oldKmTotA=(uint16_t)vInfos.tripA;
            }
        break;
        case TRIPB:
            if((ABS((uint16_t)vInfos.tripB-oldKmTotB)>=1) || (oldTrip != vCtrl.trips) || force){
                clearTrip();
                showTripP((uint16_t)vInfos.tripB,'b');
                oldKmTotB=(uint16_t)vInfos.tripB;
            }
        break;
    }
    oldTrip = vCtrl.trips;
}

void computeDisplayBattery(bool poweringUp){
    //ProgressBar pb = {.progColour = WHITE_PROGB, .progHintCol = HINT_PROGB, .progHintVal = 100-(uint8_t)bms_get_soc(), .progValue = (uint8_t)bms_get_soc()};
    ProgressBar pb = {.progColour = WHITE_PROGB, .progHintCol = HINT_PROGB, .progHintVal = 100-(uint8_t)bmsLbGetSoc(), .progValue = (uint8_t)bmsLbGetSoc()};
    //ProgressBar pb = {.progColour = WHITE_PROGB, .progHintCol = HINT_PROGB, .progHintVal = 100-(uint8_t)bikeConfig.testmode, .progValue = (uint8_t)bikeConfig.testmode};
    //static enum BMS_Status oldBMSStat = BMS_S_IDLE;

    if(pb.progValue<=20){
        pb.progHintCol = ORANGE_PROGB;
    }

    //if(bms_get_value().stat != oldBMSStat){
        if(bms_get_status() == BMS_S_FAULT || bms_get_status()==BMS_NO_COM){ //if in error
            showStatus(Settings);
            // should we do something else ? maybe not here
        }//else{ //stay show untile next restart of bike?
//            showStatus(None);
//            //pb.progHintCol = HINT_PROGB;
//            showPB(pb);
//        }
        //oldBMSStat = bms_get_value().stat;
    //}
    if(poweringUp){ //if bike start
        if(pb.progValue<=20){
            showPB(pb);
        }else{
            showPB2(pb);
        }
    }else{
        updatePB(pb);
    }
}
void manageWeather(){
   if(bikeDatas.has_weath)
    switch(bikeDatas.weath){
        case MTSC__WEATHER__WE_RAIN:
            showWeather(Rain);
            sleepy(1000);
        break;
        case MTSC__WEATHER__WE_SNOW:
            showWeather(Snow);
            sleepy(1000);
        break;
        case MTSC__WEATHER__WE_STORM:
            showWeather(Storm);
        break;
        case MTSC__WEATHER__WE_SUN:
            showWeather(Sun);
        break;
    }
}
void startupScreen(){
    ProgressBar pb = {.progColour = WHITE_PROGB, .progHintCol = HINT_PROGB, .progHintVal = 100, .progValue = 0};
#ifndef SIMULATION
    showSplash();
    sleepy(1500);
#endif
    while(!vInfos.ready){sleepy(10);} // wait for datas to be ready
    gfx_CLS(MTSC_BLACK);
//#ifndef SIMULATION
    showBonjour((char*)bikeConfig.bikename);
    sleepy(1000);
    gfx_CLS(MTSC_BLACK);
    manageWeather();
    gfx_CLS(MTSC_BLACK);
//#endif
    pb.progValue=0;
    pb.progHintVal=100-pb.progValue;
    showPB(pb);
    showVmode(vCtrl.vMode);
    //showTrip((int16_t)vInfos.kmTot);  // get that from memory
    //showDatestr("01.23",Pol3); // get that from smartphone App (not available for now)
    showSpeed(0, true);
    img_Show(iheadLight);
}

#define SIZE    3
void computeDisplaySpeed(){
    static float rpm[SIZE] = {0};
    static float avRpm=0;
    static uint8_t cptRpm=0;
    static int pt_dir=0;
    static bool dir=true;
    static Vmode oldVmode = Normal;

    rpm[cptRpm++] = vesc_get_value(0).rpm;
    if(cptRpm>=SIZE){
        avRpm=0;
        int i=0;
        for(i=0; i<SIZE ; i++){
            avRpm += rpm[i];
        }
        avRpm = avRpm/SIZE;
        vCtrl.realSpeed = (uint8_t)(avRpm*ERPM_TO_SPEED*bikeConfig.speedmultiplier);
        if(vCtrl.realSpeed>99)vCtrl.realSpeed=0;
    }

    switch(vCtrl.mode){
        case WELCOME:
        break;
        case NORMAL:
            if(cptRpm>=SIZE){
                showSpeed(vCtrl.realSpeed, false);
                //showSpeed(testv, false);
                cptRpm=0;
            }
            if(vCtrl.vMode != oldVmode){
                clearVmode();
                showVmode(vCtrl.vMode);
                oldVmode = vCtrl.vMode;
            }
        break;
        case GPS:
            if(cptRpm>=SIZE){
                showGPSSpeed(vCtrl.realSpeed, false);
                cptRpm=0;
            }
            // point animation (will be removed)
            if(pt_dir >= 280){
                dir=false;
            }else{
                if(pt_dir <= 0){
                    dir=true;
                }
            }
            if(dir){
                moveGPSDir(pt_dir);
                pt_dir+=5;
            }else{
                moveGPSDir(pt_dir);
                pt_dir-=5;
            }
            if(vCtrl.vMode != oldVmode){
                clearGPSVmode();
                showGPSVmode(vCtrl.vMode);
                oldVmode = vCtrl.vMode;
            }
        break;
        case AUTHENT:
        break;
        case OFF:
        break;
        case ON:
        break;
    }
}

// screen demo
void demo(){
    ProgressBar pb = {.progColour = WHITE_PROGB, .progHintCol = HINT_PROGB, .progHintVal = 20, .progValue = 80};
    int cpt=0;

    showSplash();
    sleepy(2000);
    gfx_CLS(MTSC_BLACK);
    showBonjour("Massimiliano");
    sleepy(1000);
    gfx_CLS(MTSC_BLACK);
    showWeather(Rain);
    sleepy(1000);
    gfx_CLS(MTSC_BLACK);
    showCardinal(North);
    showTripP(8923,'b');
    showDatestr("10.12", Pol3);
    showPB(pb);
    sleepy(2000);
    pb.progValue = 20;
    pb.progHintVal = 80;
    updatePB(pb);
    sleepy(1000);
    pb.progValue = 50;
    pb.progHintVal = 50;
    updatePB(pb);
    sleepy(2000);
    showSpeed(cpt, true);
    while(cpt<25){
        showSpeed(cpt, false); // speed 0?
        cpt+=1;
        sleepy(500);
    }
    cpt=0;
    sleepy(3000);
    clearTrip();
    showTrip(321);
    sleepy(1000);
    showStatus(Charging); // charging?
    sleepy(3000);
    clearCardinal();
    clearDate();
    clearPB();
    clearStatus();
    clearTrip();
    clearSpeed();

    // GPS VIEW DEMO
    showGPSView();
    showGPSCardinal(North);
    showGPSTxt1("23.42");
    showGPSTxt2("11.22");
    cpt=0;
    showGPSSpeed(cpt, true);
    cpt++;
    sleepy(3000);

    while(cpt < 25){
        showGPSSpeed(cpt, false);
        sleepy(500);
        cpt++;
    }
    cpt=0;
    while(cpt < 90){
        showGPSSpeed(cpt, false);
        sleepy(500);
        cpt=cpt+10;
    }
    cpt=0;
    moveGPSDir(150);
    sleepy(1000);
    moveGPSDir(100);
    moveGPSDir(50);
    moveGPSDir(0);
    sleepy(1000);
    moveGPSDir(280);
    moveGPSDir(0);
    sleepy(1000);
    moveGPSDir(280);
    moveGPSDir(0);
    sleepy(2000);
    clearGPSCardinal();
    clearGPSDir();
    clearGPSTxt1();
    clearGPSTxt2();
    clearGPSSpeed();
    clearGPSView();

}
