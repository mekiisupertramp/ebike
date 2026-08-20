/*
 * canTask.h
 *
 *  Created on: 4 févr. 2022
 *      Author: Mehmed Blazevic
 */

#ifndef APPLICATION_CANTASK_H_
#define APPLICATION_CANTASK_H_

#include "mcp2515_driver.h"

typedef struct
{
  uint8_t event;                // event type
  void    *pData;               // pointer to message
} canEvt_t;

typedef void (*tCallBackCan)(can_t msg);

void can_createTask(void);
bool canSendMessage(can_t* msg);
bool canEnable();
bool canDisable();
bool canRegistreCallBack(bool ExtendedId, tCallBackCan callBackCan);


#ifndef NB_MAX_CALLBACK
    #define NB_MAX_CALLBACK_CAN 10
#endif

#endif /* APPLICATION_CANTASK_H_ */
