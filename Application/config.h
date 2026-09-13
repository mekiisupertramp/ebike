#ifndef _CONFIGCONFIG_H_
#define _CONFIGCONFIG_H_

#include "stdint.h"
#include "iot.h"
#include "protocol.pb-c.h"

#define MAX_NAME_LENGTH 11
#define MAX_VIN_LENGTH  14
#define RFID_STRING_LENGTH  (RFID_ID_LENGTH*2)
#define CCID_LENGTH     20

extern volatile Mtsc__Config bikeConfig;
extern volatile uint64_t inConfigLastTic;


void saveBikeConfig();
void configInit();
void configCompute();
void configValidator();
void eraseConfig();

void updateConfigCharacteristique();


#endif //_CONFIGCONFIG_H_
