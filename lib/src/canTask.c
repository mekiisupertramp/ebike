/*
 * canTask.c
 *
 *  Created on: 4 févr. 2022
 *      Author: Mehmed Blazevic
 *      task for can comunication
 */




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

#include "canTask.h"
#include "bigBerta.h"


#define SP_QUEUE_EVT    UTIL_QUEUE_EVENT_ID // Event_Id_30
#define SP_INT_EVT      Event_Id_10 // Event_Id_30

#define SEND_CAN_MSG_EVT        1
#define SEND_CAN_ENABLE_EVT     2
#define SEND_CAN_DISABLE_EVT    3

static ICall_EntityID selfEntity;
static Queue_Struct appMsgQueue;
static Queue_Handle appMsgQueueHandle;
static ICall_SyncHandle syncEvent;

static bool enable=0;

#define MAX_WAITING_MESAGE 80
volatile static uint32_t waitingMesageCount=0;

static int nbCallBackExtendedId=0;
static tCallBackCan callbackExtendedId[NB_MAX_CALLBACK_CAN];
static int nbCallBackStandardId=0;
static tCallBackCan callbackStandardId[NB_MAX_CALLBACK_CAN];

bool canRegistreCallBack(bool ExtendedId, tCallBackCan callBackCan)
{
    if(ExtendedId)
    {
        if(nbCallBackExtendedId<NB_MAX_CALLBACK_CAN)
        {
            callbackExtendedId[nbCallBackExtendedId]=callBackCan;
            nbCallBackExtendedId++;
        }
        else
        {
            return 0;
        }
    }
    else
    {
        if(nbCallBackStandardId<NB_MAX_CALLBACK_CAN)
        {
            callbackStandardId[nbCallBackStandardId]=callBackCan;
            nbCallBackStandardId++;
        }
        else
        {
            return 0;
        }
    }
    return 1;
}

void irqMcp2515(uint_least8_t index) //interrupt from can controler
{
    Event_post(syncEvent,SP_INT_EVT); //wakup task
}

void canThread(UArg a0, UArg a1)
{
    ICall_registerApp(&selfEntity, &syncEvent); //for passive wait
    appMsgQueueHandle = Util_constructQueue(&appMsgQueue); //message queue

#ifndef CAN_BAUDRATE
#define CAN_BAUDRATE CAN_125kbps
#endif
    /*we have start with a RTOS tick 1ms
     * then the logic are CAN at 125kbds with bigest baudrate we can lose Frame CAN Frame have a minimum of 44bit (in pratice we do not have < 52bit)ms
     * then at 125kbps we can have 2.4 Frame by ms in practice i have never see more of 2 Frame by ms
     * ti rtos have a 1ms tick then if a other task runing and we recive a IRQ when can receive a second Frame befor code is call and mcp2515 can store only 2 Frame
     * paquet lose can happen if icall task use to many CPU (highest priority of CAN task)
     *
     * But now the tick are set to 0.1ms but with the time of SPI transfer the performance are not * 10
     * Then we need try to 250kbps and 500kbps (500kbps are necessary for work with tinyBms uart to can) */

    init_mcp2515(SPI_DEFAULT_CLOCK, CAN_BAUDRATE);
    GPIO_setCallback(MCP2515_nINT, irqMcp2515); //if irq from can controler call irqMcp2515
    GPIO_enableInt(MCP2515_nINT); //enable irq

    mcp2515_clearInt(); //clean all interrupt befor start

    while(1){

#ifdef SIMULATION
        can_t resp={0};
        update_vesc(resp);
        update_bms(resp);
        bigBertaDecodeCan(resp);
#endif
        if(waitingMesageCount==0)
        	Event_pend(syncEvent, Event_Id_NONE, SP_QUEUE_EVT|SP_INT_EVT, //semaphore wait got envent
                            ICALL_TIMEOUT_FOREVER);
        else
        	Event_pend(syncEvent, Event_Id_NONE, SP_QUEUE_EVT|SP_INT_EVT, //semaphore wait got envent
        	                            2);

        //GPIO_write(LED, 1);
        while(GPIO_read(MCP2515_nINT)==0) //while interrupt pin say we have awaiting data read it
        {
            can_t resp;
            resp.id=0;

            while(mcp2515_get_message(&resp)){ //read all message
                if(resp.flags.extended){ //call all lib need receive extended id frame
                    for(int l=0;l<nbCallBackExtendedId;l++)
                        callbackExtendedId[l](resp);
                }else{ //call all lib need receive standard id frame
                    for(int l=0;l<nbCallBackStandardId;l++)
                        callbackStandardId[l](resp);
                }
            }
        }

        static int timeout=0;
        timeout++;
        int status = mcp2515_read_status();
        if(!enable || (status&((1<<2)/*|(1<<4)|(1<<6)*/))!=((1<<2)/*|(1<<4)|(1<<6)*/) || (timeout>=100)) //if not enable drop packet. If enable no send paquet if tx buffer is full
        {
            timeout=0;
            if (!Queue_empty(appMsgQueueHandle))//send only one paquet for try to read betwhen all write
            {
                canEvt_t* pMsg = (canEvt_t*)Util_dequeueMsg(appMsgQueueHandle); //read queue
                if (pMsg)
                {
                    waitingMesageCount--;
                    switch (pMsg->event)
                    {
                        case SEND_CAN_MSG_EVT:
                            if(enable) //if MCP not enable drop message
                            {
                                if(mcp2515_send_message(pMsg->pData)==0) //if message not send (tx_buffer_full)
                                {
                                    //canSendMessage(pMsg->pData); //back to queue
                                    volatile int l; //count error sending
                                    l++;
                                    //while(1);
                                }
                            }
                            break;
                        case SEND_CAN_ENABLE_EVT:
                            if(enable==0)
                            {
                                //GPIO_write(CONFIG_3V3_P_nEN,0);
                                enable=1;
                                ClockP_usleep(100000);//wait power up
                                mcp2515_rst();
                                Init2515();
                                ClockP_usleep(10000);//wait power up
                            }
                            break;
                        case SEND_CAN_DISABLE_EVT:
                            if(enable)
                            {
                                GPIO_write(MCP2515_nRst,0);
                                //GPIO_write(CONFIG_3V3_P_nEN,1);
                                enable=0;
                            }
                            break;
                    }

                    if(pMsg->pData!=NULL)
                        ICall_free(pMsg->pData);
                    ICall_free(pMsg);
                }
            }
            //GPIO_write(LED, 0);
        }
    }
}

#define CAN_TREAD_STACK_SIZE (1024+512)
uint8_t canThreadStack[CAN_TREAD_STACK_SIZE];
Task_Struct canThreadTask;

void can_createTask(void)
{
    Task_Params taskParams;

    // Configure task
    Task_Params_init(&taskParams);
    taskParams.stack = canThreadStack;
    taskParams.stackSize = CAN_TREAD_STACK_SIZE;
    taskParams.priority = 4; //5 reserved for icall

    Task_construct(&canThreadTask, canThread, &taskParams, NULL);
}

static bool canThread_enqueueMsg(uint8_t event, uint8_t *pData)
{
    canEvt_t *pMsg = ICall_malloc(sizeof(canEvt_t));

    // Create dynamic pointer to message.
    if (pMsg)
    {
        pMsg->event = event;
        pMsg->pData = pData;

        // Enqueue the message.
        if(!Util_enqueueMsg(appMsgQueueHandle, syncEvent, (uint8_t *)pMsg)) //free mesage if not send is donne in calling function
        {
            ICall_free(pMsg);
            return false;
        }
        else
        {
            return true;
        }
    }

    return (false);
}

volatile int debug_nb_fail=0;
bool canSendMessage(can_t* msg)
{
    if(waitingMesageCount<MAX_WAITING_MESAGE)
    {
        waitingMesageCount++;

        can_t* malloc_msg=ICall_malloc(sizeof(can_t));
        *malloc_msg=*msg;

        int ret=canThread_enqueueMsg(SEND_CAN_MSG_EVT,(uint8_t*)malloc_msg);

        if(ret==false)
        {
            ICall_free(malloc_msg);
            waitingMesageCount--;
            debug_nb_fail++;
        }

        return ret;
    }
    else
    {
        debug_nb_fail++;
        return false;
    }
}

bool canEnable()
{
    if(waitingMesageCount<MAX_WAITING_MESAGE)
    {
        waitingMesageCount++;
        return canThread_enqueueMsg(SEND_CAN_ENABLE_EVT,NULL);
    }
    return false;
}

bool canDisable()
{
    if(waitingMesageCount<MAX_WAITING_MESAGE)
    {
        waitingMesageCount++;
        return canThread_enqueueMsg(SEND_CAN_DISABLE_EVT,NULL);
    }
    return false;
}
