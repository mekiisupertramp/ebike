/*
 * memoryTask.c
 *
 *  Created on: 14 janv. 2022
 *      Author: Mehmed Blazevic
 */

#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/knl/Event.h>
#include <ti/sysbios/knl/Queue.h>
#include <icall.h>
#include <util.h>


#include "memoryTask.h"
#include "bootLoader.h"
#include "serviceBootLoader.h"
//#include "logTravel.h"
#include "W25N01GV_driver.h"


#define SP_QUEUE_EVT                         UTIL_QUEUE_EVENT_ID // Event_Id_30

static ICall_EntityID selfEntity;
static Queue_Struct appMsgQueue;
static Queue_Handle appMsgQueueHandle;
static ICall_SyncHandle syncEvent;

static void serviceBootLoader_charValueChangeCB(uint8_t paramId);

// Callback when a characteristic value has changed
typedef void (*serviceBootChange_t)( uint8 paramID );

typedef struct
{
    serviceBootChange_t        pfnSimpleProfileChange;  // Called when characteristic value changes
} serviceBootCBs_t;

static serviceBootLoaderCBs_t serviceBootLoaderCBs =
{
 serviceBootLoader_charValueChangeCB // Simple GATT Characteristic value change callback
};
/*
 *  ======== mainThread ========
 */

void memoryThread(UArg a0, UArg a1)
{
    init_W25N01GV(12000000); //12MHz max of cc2652 control if is not 4 MHz

    ICall_registerApp(&selfEntity, &syncEvent);
    appMsgQueueHandle = Util_constructQueue(&appMsgQueue);

    ServiceBootLoader_RegisterAppCBs(&serviceBootLoaderCBs); //registre callback for service boot loader

    //init all code use memory
    bootLoaderInit();
    //logInit();
    //pixxilUpdate();

    while(1){
        Event_pend(syncEvent, Event_Id_NONE, SP_QUEUE_EVT,
                            ICALL_TIMEOUT_FOREVER); //wait event

        while (!Queue_empty(appMsgQueueHandle)) //if message in queue
        {
            mtEvt_t* pMsg = (mtEvt_t*)Util_dequeueMsg(appMsgQueueHandle); //read mesage
            if (pMsg)
            {
                //send message to concerned code
                if(pMsg->event==BOOLLOADER_PAQUET_EVT)
                {
                    bootLoaderWrite(pMsg->pData);
                }

                /*if(pMsg->event==LOG_PAQUET_EVT)
                {
                    logWrite(pMsg->pData);
                }

                if(pMsg->event==LOG_CMD_PAQUET_EVT)
                {
                   // logCmdReceive(pMsg->pData);
                }*/

                ICall_free(pMsg->pData);
                ICall_free(pMsg);
            }
        }

    }
}

#define MEMORY_TREAD_STACK_SIZE (3*1024+512)
uint8_t memoryThreadStack[MEMORY_TREAD_STACK_SIZE];
Task_Struct memoryThreadTask;

void memory_createTask(void) //create memory task this task manage the SPI flash
{
    Task_Params taskParams;

    // Configure task
    Task_Params_init(&taskParams);
    taskParams.stack = memoryThreadStack;
    taskParams.stackSize = MEMORY_TREAD_STACK_SIZE;
    taskParams.priority = 2; //low priority

    Task_construct(&memoryThreadTask, memoryThread, &taskParams, NULL);
}

uint8_t memoryThread_enqueueMsg(uint8_t event, uint8_t *pData) //send message to memory task
{
    mtEvt_t *pMsg = ICall_malloc(sizeof(mtEvt_t));

    // Create dynamic pointer to message.
    if (pMsg)
    {
        pMsg->event = event;
        pMsg->pData = pData;

        // Enqueue the message.
        if(!Util_enqueueMsg(appMsgQueueHandle, syncEvent, (uint8_t *)pMsg)) //free message if not send
        {
            return false;
        }
        else
        {
            return true;
        }
    }

    return (false);
}


static void serviceBootLoader_charValueChangeCB(uint8_t paramId) //if we recive message on ble for boot loader
{
    uint8_t *pValue = ICall_malloc(SERVICEBOOTLOADER_FIRMWARE_LEN);

    if (pValue)
    {
        ServiceBootLoader_GetParameter(paramId,pValue); //read data recieve on ble

        if (memoryThread_enqueueMsg(BOOLLOADER_PAQUET_EVT, pValue) != TRUE) //enqueue message
        {
            ICall_free(pValue);//if not possi to enqueue free memory
        }
    }
    else
    {
        volatile int drop=0; //for debug watch if alloc no work (no memory free)
        drop++;
    }
}
