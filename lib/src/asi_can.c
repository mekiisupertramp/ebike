/*
 * asi_can.c
 *
 *  Created on: 6 Jul 2021
 *      Author: mehmedblazevic
 */


#include "asi_can.h"

#ifdef ASI_CAN

int8_t asi_exp_read(char cmd, int16_t index, char sub_index){
    can_t msg,resp;

    memset(&msg.data, 0, 8);
    memset(&resp.data, 0, 8);

    msg.id = ASI_ID | SDO_EXP;
    msg.flags.rtr = 0;
    msg.length = 8;
    msg.data[0] = cmd; // command
    msg.data[1] = index>>8; // index MSB (or LSB)
    msg.data[2] = index; // index LSB (or MSB)
    msg.data[3] = sub_index; // sub index

    mcp2515_send_message(&msg);

    if(mcp2515_check_message()){
        mcp2515_get_message(&resp);
    }

    return 0;
}

int8_t asi_exp_write(char cmd, int16_t index, char sub_index, char *datas){
    can_t msg,resp;
    msg.id = ASI_ID | SDO_EXP;
    msg.flags.rtr = 0;
    msg.length = 8;
    memset(&msg.data, 0, 8);
    msg.data[0] = cmd; // command
    msg.data[1] = index>>8; // index MSB (or LSB)
    msg.data[2] = index; // index LSB (or MSB)
    msg.data[3] = sub_index; // sub index
    msg.data[4] = datas[0];
    msg.data[5] = datas[1];
    msg.data[6] = datas[2];
    msg.data[7] = datas[3];

    mcp2515_send_message(&msg);

    if(mcp2515_check_message()){
        mcp2515_get_message(&resp);
    }

    return 0;
}

int8_t init_asi_can(){

    asi_exp_read(CMD_ROD_READ, DEVICE_ELEC_TEMP, 0);

    return OK;
}

uint8_t asi_get_speed();

int8_t asi_mbrakes(bool active);

#endif
