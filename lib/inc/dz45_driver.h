/*
 * dz45_driver.h
 *
 *  Created on: 14 Aug 2024
 *      Author: mehme
 */

#ifndef LIB_INC_DZ45_DRIVER_H_
#define LIB_INC_DZ45_DRIVER_H_

#include "stdbool.h"
#include "stdint.h"
#include "canTask.h"

// see Bafang CAN Communication protocol.pdf
#define PUB_INFO0   0x30
#define PUB_INFO1   0x31
#define PUB_INFO2   0x32
// ....
#define SUB_INDEX0  0x00
#define SUB_INDEX1  0x01
#define SUB_INDEX2  0x02
#define SUB_INDEX3  0x03
#define SUB_INDEX4  0x04
// ....
#define ID_SENSOR   0x01
#define ID_CTRL     0x02
#define ID_HMI      0x03
#define ID_BAT1     0x04
#define ID_BESST    0x05
#define ID_TRANSM   0x06
#define ID_LOCK     0x07
#define ID_TORK     0x08
#define ID_INT_MOD  0x09
#define ID_CHARGER  0x0E
#define ID_BAT2     0x0F
#define ID_TEST_TA  0x10
#define ID_TEST_TB  0x11
#define ID_ABS      0x12
#define ID_BLUET    0x13


union b_id{
    struct{
        uint8_t subindex:8;
        uint8_t index:8;
        uint8_t instr:3;
        uint8_t dst:5;
        uint8_t src:5;
    };
    uint32_t extId;
};

typedef struct{
    union b_id bafId;
    uint8_t len;
    uint8_t datas[8];
}dictionary_t;

void dz45_init();
void dz45_send(dictionary_t dict);
void dz45_decodeCan(can_t msg);
void dz45_sendSoc(uint8_t soc);
void dz45_sendSpeed(uint8_t speed);


#endif /* LIB_INC_DZ45_DRIVER_H_ */
