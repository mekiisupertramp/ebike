/*
 * asi_can.h
 *
 *  Created on: 6 Jul 2021
 *      Author: mehmedblazevic
 */

#ifndef ASI_CAN_H_
#define ASI_CAN_H_

//#define ASI_CAN (1)

#ifdef ASI_CAN

#include <stdint.h>
#include "mcp2515_driver.h"

// CAN ASI headers client
#define SDO_EXP     0x600 // SDO expedited read client transmit message

// CAN ASI headers server
#define SDO_EXP_RESP       0x580 // SDO expedited read server response message


// CAN ASI commands
#define CMD_RDO_R_4B    0x43    // read dictionary object 4 bytes
#define CMD_RDO_R_3B    0x47    // read dictionary object 3 bytes
#define CMD_RDO_R_2B    0x4B    // read dictionary object 2 bytes
#define CMD_RDO_R_1B    0x4F    // read dictionary object 1 byte
#define CMD_RDO_W_4B    0x23    // write dictionary object 4 bytes
#define CMD_RDO_W_3B    0x27    // write dictionary object 3 bytes
#define CMD_RDO_W_2B    0x2B    // write dictionary object 2 bytes
#define CMD_RDO_W_1B    0x2F    // write dictionary object 1 byte
#define CMD_ROD_READ    0x40    // read dictionnary object

#define CMD_R_ACK       0x40    // acknoledge for read command
#define CMD_W_ACK       0x60    // acknoledge for write command

// CAN ASI dictionary objects
#define DEVICE_TYPE         0x1000
#define ERROR_REGISTER      0x1001
#define COBID_SYNC          0x1005
#define MANUFACT_HARD_VERS  0x1009
#define MANUFACT_SOFT_VERS  0x100A
#define DEVICE_CTRL         0x6001
#define DEVICE_STAT         0x6002
#define DEVICE_RAT_VOLTAGE  0x6008
#define DEVICE_MIN_VOLTAGE  0x6027
#define DEVICE_MAX_VOLTAGE  0x6026
#define DEVICE_ACTUAL_CUR   0x603E
#define DEVICE_ACTUAL_VOLT  0x6040
#define DEVICE_ELEC_TEMP    0x6042
#define DEVICE_ASSIST_LVL   0x6306
#define DEVICE_MAX_SPEED    0x6308

#define VEHICLE_SPEED       0x0260 // not sure at all, maybe 260 in base10
#define WHEEL_RPM           0x0312
#define MOTOR_RPM           0x0263


// CAN Node Id
#define ASI_ID  0x01 // range from 0x01 to 0x7F

#define DISABLE_HEARTBEAT   0x0 // ranging from 0x0010 to 0x7FFF
#define DISABLE_SYNC_LOSS   0x0 // ranging from 0x0001 to 0x7FFF

#define OK      0
#define ERR     -1



int8_t init_asi_can();
uint8_t asi_get_speed();
int8_t asi_mbrakes(bool active);
int8_t asi_assist_lvl();


#endif

#endif /* ASI_CAN_H_ */
