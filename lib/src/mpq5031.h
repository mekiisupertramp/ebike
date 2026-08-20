/*
 * mpq5031.h
 *
 *  Created on: 23 août 2023
 *      Author: Mehmed Blazevic
 */

#ifndef APPLICATION_MPQ5031_H_
#define APPLICATION_MPQ5031_H_

#define MPQ_ADRESSE 0x28

#include <ti/drivers/I2C.h>
#include <stdint.h>
#include "ti_drivers_config.h"

#define VBATT_LOW_PULL_PS_EN (1<<15)
#define VBATT_LOW_PULL_NTC_EN (1<<8)
#define I2C_SLAVE_ADDRESS (0b1000<<4)
#define SLAVE_DEVICE_SEL 0b101
#define GPIO3_MODE (0b101<<6) //PDO3_SEL_OUT
bool mpq5031Init();
bool mpq5031SetPd();
bool mpq5031WriteReg(uint8_t add, uint16_t value);
int32_t mpq5031ReadReg(uint8_t add);


#endif /* APPLICATION_MPQ5031_H_ */
