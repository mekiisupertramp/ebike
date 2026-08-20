/*
 * ble_stream_status.h
 *
 *  Created on: 17 oct. 2023
 *      Author: Mehmed Blazevic
 */

#ifndef APPLICATION_BLE_STREAM_STATUS_H_
#define APPLICATION_BLE_STREAM_STATUS_H_

#include "vesc.h"
#include "tiny_BMS.h"

#define VERSION_PROTOCOL_BLE 1
#define NB_BMS 1

void bleSendStatus();

typedef struct
{
    //uint64_t timestamp;
    t_vescStatus esc[NB_VESC];
    BMS_Values_t bms;
    float km;
} t_infoVehicle;


#endif /* APPLICATION_BLE_STREAM_STATUS_H_ */
