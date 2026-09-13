/*
 * iot.h
 *
 *  Created on: 24 Jan 2023
 *      Author: mehmedblazevic
 */

#ifndef APPLICATION_IOT_H_
#define APPLICATION_IOT_H_

#include "string.h"
#include "stdbool.h"
#include "stdint.h"
#include "canTask.h"
#include "mcp2515_driver.h"
#include "mtsc.h"
#include "config.h"
#include "protocol.pb-c.h"


typedef enum {ok, nok_size, nok_chars} Username;
typedef enum {Sunny, Cloudy, Rainy, Snowy} Weath;

extern volatile Mtsc__Datas bikeDatas;


extern bool assistChanged;
extern bool powerChanged;
extern bool dateChanged;
extern bool rfidReceived;
extern bool ccidReceived;
extern bool unlockBLE;
extern bool resetOdo;


// requests
#define SEND_ID_1       0x301
#define SEND_ID_2       0x302
#define GET_DEV_CCID1   0x303
#define GET_DEV_CCID2   0x304
#define GET_DEV_CCID3   0x305
#define GET_PING        0x306

// responds
#define SEND_ID_1_ACK       0x311
#define SEND_ID_2_ACK       0x312
#define GET_DEV_CCID_ACK1   0x313
#define GET_DEV_CCID_ACK2   0x314
#define GET_DEV_CCID_ACK3   0x315
#define GET_PING_ACK        0x316

#define RFID_ID_LENGTH 11 //length of RFID id in Byte

void iot_init();
void saveBikeDatas();
void eraseBikeDatas();
void updateDatasCharacteristique();

void requestPing();
bool isRfidOK();
void requestCcid();
bool isIotConnected();
void resetIotConn();
void convStrToHexArray(char* hexArray, char* strArray, uint8_t n);
void iot_manage_comm(can_t msg);

#endif /* APPLICATION_IOT_H_ */
