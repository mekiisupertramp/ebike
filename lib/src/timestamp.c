/*
 * timestamp can bze set by BLE and get for log or other
 *
 *  Created on: 13 nov. 2022
 *      Author: Mehmed Blazevic
 */

#include <ti/drivers/Timer.h>
#include <ti/sysbios/hal/Hwi.h>
#include <stdint.h>

#include "ti_drivers_config.h"

uint64_t timeStampMs=0;

static Timer_Handle    handle;

void ticCallBack(Timer_Handle handle, int_fast16_t status) //call every 5ms
{
    uint32_t hwiKey = Hwi_disable();
    timeStampMs+=5;
    Hwi_restore(hwiKey);
}

void setTimeStampMs(uint64_t temp)//set the timestamp
{
    uint32_t hwiKey = Hwi_disable(); //timestamp is uint64 then is not atomic then we need block interrupt
    timeStampMs=temp;
    Hwi_restore(hwiKey);
}

uint64_t getTimeStampMs()
{
    uint32_t hwiKey = Hwi_disable();//timestamp is uint64 then is not atomic then we need block interrupt
    uint64_t temp=timeStampMs;
    Hwi_restore(hwiKey);
    return temp;
}

void initTimeStamp()
{

    Timer_Params    params;
    Timer_Params_init(&params);
    params.periodUnits = Timer_PERIOD_HZ; //timer call ticCallBack every 5ms
    params.period = 1000/5;//5ms
    params.timerMode  = Timer_CONTINUOUS_CALLBACK;
    params.timerCallback = ticCallBack;
    handle = Timer_open(CONFIG_TIMER_0, &params);
    Timer_start(handle);
    // Import Timer Driver definitions

}

