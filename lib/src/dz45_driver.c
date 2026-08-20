/*
 * dz45_driver.c
 *
 *  Created on: 14 Aug 2024
 *      Author: mehme
 */

#include "dz45_driver.h"


void dz45_init(){
    canRegistreCallBack(1, dz45_decodeCan);
}
void dz45_send(dictionary_t dict){
    can_t msg;
    int i = 0;
    msg.length = dict.len;
    for(i=0 ; i<dict.len ; i++){
        msg.data[i] = dict.datas[i];
    }
    msg.flags.rtr=0;
    msg.flags.extended=1;
    msg.id = dict.bafId.extId;
    canSendMessage(&msg);
}
void dz45_decodeCan(can_t msg){

}

void dz45_sendSoc(uint8_t soc){
    dictionary_t dict;
    dict.bafId.extId=0;
    dict.bafId.src=ID_CTRL;
    dict.bafId.dst=ID_HMI;
    dict.bafId.instr=0;
    dict.bafId.index=PUB_INFO2;
    dict.bafId.subindex=SUB_INDEX0;
    dict.datas[0] = soc;
    dict.datas[1] = 0;
    dict.datas[2] = 0;
    dict.datas[3] = 0;
    dict.datas[4] = 0;
    dict.datas[5] = 0;
    dict.datas[6] = 0;
    dict.datas[7] = 0;
    dict.len=8;
    dz45_send(dict);
}
void dz45_sendSpeed(uint8_t speed){
    uint16_t buf = (uint16_t)speed*100;
    dictionary_t dict;
    dict.bafId.extId=0;
    dict.bafId.src=ID_CTRL;
    dict.bafId.dst=ID_HMI;
    dict.bafId.instr=0;
    dict.bafId.index=PUB_INFO2;
    dict.bafId.subindex=SUB_INDEX1;
    dict.datas[0] = (uint8_t)(buf);
    dict.datas[1] = (uint8_t)(buf>>8);
    dict.datas[2] = 0;
    dict.datas[3] = 0;
    dict.datas[4] = 0;
    dict.datas[5] = 0;
    dict.datas[6] = 0;
    dict.datas[7] = 0;
    dict.len=8;
    dz45_send(dict);
}
