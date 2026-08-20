/*
 * memoryTask.h
 *
 *  Created on: 14 janv. 2022
 *      Author: Mehmed Blazevic
 */

/*memory map
 * 0 to FIRMWARE_SIZE (256kB) firmware for update
 * FIRMWARE_SIZE (256kB) to 2*FIRMWARE_SIZE (512kB) original firmware can be restore by boot loader
 * 2*FIRMWARE_SIZE (512kB) to FLAS
 */

#ifndef APPLICATION_MEMORYTASK_H_
#define APPLICATION_MEMORYTASK_H_

typedef struct
{
  uint8_t event;                // event type
  void    *pData;               // pointer to message
} mtEvt_t;

void memory_createTask(void);
uint8_t memoryThread_enqueueMsg(uint8_t event, uint8_t *pData);

//#define NVS_SECTRO_SIZE_FLASH (1024*4)

#define BOOLLOADER_PAQUET_EVT 1
#define LOG_PAQUET_EVT 2
#define LOG_CMD_PAQUET_EVT 3


#endif /* APPLICATION_MEMORYTASK_H_ */
