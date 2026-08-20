/*
 * vesc_can.c
 *
 *  Created on: 27 Jul 2021
 *      Author: mehmedblazevic
 * Description: Here is the driver for communicating
 * with VESC through CAN protocol. BE CAREFUL, VESC must be
 * configured to send status messages. Otherwise, this code
 * will not work at all.
 */

#include "vesc_can.h"
#include "buffer.h"
#include "vesc.h"
#include "canTask.h"
#include "stdlib.h"
#include "virtualUartService.h"
#include "timestamp.h"
#include "crc16Vesc.h"

#define MASK_GET_VALUE_FAULT ((uint32_t)1 << 15)

#ifdef VESC_BRIDGE
#include "config.h"
#endif

#define MY_ADRESSE 0x80
static uint16_t nbDataReceive;

//static VESC_Values_t vesc = {.current=0.0, .duty_cycle=0.0, .erpm=0};
t_vescStatus statusVesc[NB_VESC]={0};

void vescInit()
{
    canRegistreCallBack(1,update_vesc);
}


static tCallBackVesc callback[NB_MAX_CALLBACK_VESC];
static uint8_t callbackRegistredCmd[NB_MAX_CALLBACK_VESC];


bool vesc_registre_call_back(uint8_t cmd, tCallBackVesc callBackCan)
{
    for(int l=0;l<NB_MAX_CALLBACK_VESC;l++)
    {
        if(callback[l]==0)
        {
            callback[l]=callBackCan;
            callbackRegistredCmd[l]=cmd;
            return 1;
        }
    }
    return 0;
}

bool vesc_unregistre_call_back(uint8_t cmd, tCallBackVesc callBackCan)
{
    for(int l=0;l<NB_MAX_CALLBACK_VESC;l++)
    {
        if(callback[l]==callBackCan && callbackRegistredCmd[l]==cmd)
        {
            callback[l]=0;
            return 1;
        }
    }
    return 0;
}

void update_vesc(can_t msg){
    uint8_t* data8=msg.data;
    static uint8_t buffer[NB_VESC][PACKET_BUFFER_LEN];
    uint16_t pt;

#ifdef SIMULATION
    for(int idVesc=0;idVesc<NB_VESC;idVesc++)
    {
        statusVesc[idVesc].rpm=rand();
        statusVesc[idVesc].current=rand();
        statusVesc[idVesc].temp_fet=rand();
        statusVesc[idVesc].temp_motor=rand();
        statusVesc[idVesc].acc[0]=rand();
        statusVesc[idVesc].acc[1]=rand();
        statusVesc[idVesc].acc[2]=rand();
        statusVesc[idVesc].gyro[0]=rand();
        statusVesc[idVesc].gyro[1]=rand();
        statusVesc[idVesc].gyro[2]=6;
    }

    /*for(int idVesc=0;idVesc<NB_VESC;idVesc++)
    {
        statusVesc[idVesc].rpm=1000;
        statusVesc[idVesc].current=9.9;
        statusVesc[idVesc].temp_fet=50;
        statusVesc[idVesc].temp_motor=51;
        statusVesc[idVesc].acc[0]=1;
        statusVesc[idVesc].acc[1]=2;
        statusVesc[idVesc].acc[2]=3;
        statusVesc[idVesc].gyro[0]=4;
        statusVesc[idVesc].gyro[1]=5;
        statusVesc[idVesc].gyro[2]=6;
    }*/
#endif

    if(msg.id>>16==0)
    {
        uint8_t cmd=msg.id>>8;
        volatile uint8_t idVesc=msg.id&0x7F; //0x80=0x00 for processe COMM_GET_IMU_DATA -> and for vesc with ID 128 and 129 = 0 and 1 (use in yakE for no conflit with battery paquet)
        int32_t indice=0;
        if(idVesc>=NB_VESC) //can happen if is a reply ton a request from LB (use virtual uart for comunicate with vescTool)
        {
            idVesc=0;//then use the buffer of vesc0
        }
        switch (cmd) {
        case CAN_PACKET_STATUS:
        {
            statusVesc[idVesc].rpm = (float)buffer_get_int32(data8, &indice);
            statusVesc[idVesc].current = (float)buffer_get_int16(data8, &indice) / 10.0;
            statusVesc[idVesc].duty = (float)buffer_get_int16(data8, &indice) / 1000.0;
            uint64_t timeStamp=getTimeStampMs();
            uint64_t dt=timeStamp-statusVesc[idVesc].timeLastData;
            if(dt<500) //else we have invalide old data
                statusVesc[idVesc].accRpm=(statusVesc[idVesc].rpm-statusVesc[idVesc].oldRpm)/(dt/1000.f); //compute axceleration in rpm/s^2
            statusVesc[idVesc].oldRpm=statusVesc[idVesc].rpm;
            statusVesc[idVesc].timeLastData=timeStamp;
        }
        break;

        case CAN_PACKET_STATUS_2:
            statusVesc[idVesc].amp_hours = (float)buffer_get_int32(data8, &indice) / 1e4;
            statusVesc[idVesc].amp_hours_charged = (float)buffer_get_int32(data8, &indice) / 1e4;
            break;

        case CAN_PACKET_STATUS_3:
            statusVesc[idVesc].watt_hours = (float)buffer_get_int32(data8, &indice) / 1e4;
            statusVesc[idVesc].watt_hours_charged = (float)buffer_get_int32(data8, &indice) / 1e4;
            break;

        case CAN_PACKET_STATUS_4:
            statusVesc[idVesc].temp_fet = (float)buffer_get_int16(data8, &indice) / 10.0;
            statusVesc[idVesc].temp_motor = (float)buffer_get_int16(data8, &indice) / 10.0;
            statusVesc[idVesc].current_in = (float)buffer_get_int16(data8, &indice) / 10.0;
            statusVesc[idVesc].pid_pos_now = (float)buffer_get_int16(data8, &indice) / 50.0;
            break;

        case CAN_PACKET_STATUS_5:
            statusVesc[idVesc].tacho_value = buffer_get_int32(data8, &indice);
            statusVesc[idVesc].v_in = (float)buffer_get_int16(data8, &indice) / 1e1;
            break;

        case CAN_PACKET_FILL_RX_BUFFER: //reply to a CAN_PACKET_PROCESS_SHORT_BUFFER
            nbDataReceive+=msg.length-1;
            pt=buffer_get_uint8(data8, &indice);
            for(int l=0;l<msg.length-1;l++)
            {
                buffer[idVesc][l+pt]=buffer_get_uint8(data8, &indice);
            }
            break;
        case CAN_PACKET_FILL_RX_BUFFER_LONG: //reply to a CAN_PACKET_PROCESS_SHORT_BUFFER
            nbDataReceive+=msg.length-2;
            pt=buffer_get_uint16(data8, &indice);
            for(int l=0;l<msg.length-2;l++)
            {
                if(l+pt<PACKET_MAX_PL_LEN)
                    buffer[idVesc][l+pt]=buffer_get_uint8(data8, &indice);
                else
                    while(1);
            }
            break;

        case CAN_PACKET_PROCESS_SHORT_BUFFER:
        {
            uint8_t rx_buffer_last_id=buffer_get_uint8(data8, &indice);
            uint8_t commands_send=buffer_get_uint8(data8, &indice);
            cmd=buffer_get_uint8(data8, &indice);
            if(commands_send==1 && cmd==COMM_GET_VALUES_SELECTIVE)
            {
                uint32_t mask=buffer_get_uint32(data8, &indice);
                if(mask==MASK_GET_VALUE_FAULT)
                    statusVesc[idVesc].mcFaultCode=buffer_get_uint8(data8, &indice);
            }
        }
        break;

        case CAN_PACKET_PROCESS_RX_BUFFER:
        {
            cmd=buffer_get_uint8(buffer[idVesc], &indice);

            for(int l=0;l<NB_MAX_CALLBACK_VESC;l++)
            {
                if(callback[l]!=0 && callbackRegistredCmd[l]==cmd)
                {
                    callback[l](idVesc,buffer[idVesc],nbDataReceive);
                }
            }

            int32_t temp=2;
            uint16_t len=buffer_get_uint16(data8, &temp);

            if(nbDataReceive==len)
            {
                //fixme check crc
                if(cmd==COMM_GET_IMU_DATA)
                {
                    uint16_t mask=buffer_get_uint16(buffer[idVesc], &indice);
                    statusVesc[idVesc].rpy[0]=buffer_get_float32(buffer[idVesc],1,&indice);
                    statusVesc[idVesc].rpy[1]=buffer_get_float32(buffer[idVesc],1,&indice);
                    statusVesc[idVesc].rpy[2]=buffer_get_float32(buffer[idVesc],1,&indice);
                    statusVesc[idVesc].acc[0]=buffer_get_float32(buffer[idVesc],1,&indice);
                    statusVesc[idVesc].acc[1]=buffer_get_float32(buffer[idVesc],1,&indice);
                    statusVesc[idVesc].acc[2]=buffer_get_float32(buffer[idVesc],1,&indice);
                    statusVesc[idVesc].gyro[0]=buffer_get_float32(buffer[idVesc],1,&indice);
                    statusVesc[idVesc].gyro[1]=buffer_get_float32(buffer[idVesc],1,&indice);
                    statusVesc[idVesc].gyro[2]=buffer_get_float32(buffer[idVesc],1,&indice);
                }


#ifdef VESC_BRIDGE
                uint8_t bufferTx[PACKET_BUFFER_LEN];
                uint32_t pt=0;

                if(len<256)
                {
                    bufferTx[0]=2;
                    bufferTx[1]=len;
                    pt=2;
                }
                else
                {
                    if(len>sizeof(bufferTx)-3)
                    {
                        return;
                    }
                    bufferTx[0]=3;
                    bufferTx[1]=len>>8;
                    bufferTx[2]=len;
                    pt=3;
                }

                memcpy(bufferTx+pt, buffer, len);
                pt+=len;
                bufferTx[pt]=data8[4]; //add crc
                pt++;
                bufferTx[pt]=data8[4+1];
                pt++;
                bufferTx[pt]=3;
                pt++;

                for(int l=0;l<pt;l+=VIRTUALUARTSERVICE_RX_LEN)
                {
                    VirtualUartService_SetParameter(VIRTUALUARTSERVICE_RX , MIN(VIRTUALUARTSERVICE_RX_LEN,pt-l), bufferTx+l);
                }
#endif
            }

            nbDataReceive=0;

        }
        break;
        }
    }
}

t_vescStatus vesc_get_value(int id){
    return statusVesc[id];
}

void comm_can_transmit_eid_replace_vesc(uint32_t id, const uint8_t *data, uint8_t len, _Bool replace)
{
    comm_can_transmit_eid_replace(id,data,len,replace);
}

void comm_can_transmit_eid_replace(uint32_t id, const uint8_t *data, uint8_t len, _Bool replace)
{
    (void) replace; //no use compatibility with comm_can.c
    if (len > 8) {
        len = 8;
    }
    can_t msg;
    msg.id=id;
    msg.length=len;
    msg.flags.extended=1;
    msg.flags.rtr=0;
    memcpy(msg.data, data, len);
    canSendMessage(&msg);
}

void comm_can_conf_foc_erpms(uint8_t controller_id, float foc_openloop_rpm, float foc_sl_erpm) {
    int32_t send_index = 0;
    uint8_t buffer[8];
    buffer_append_float32(buffer, foc_openloop_rpm, 1e3, &send_index);
    buffer_append_float32(buffer, foc_sl_erpm, 1e3, &send_index);
    comm_can_transmit_eid_replace_vesc(controller_id |
                                       ((uint32_t)CAN_PACKET_CONF_FOC_ERPMS << 8), buffer, send_index, 1);
}

void vesc_can_set_current(uint8_t controller_id, float current) {
    int32_t send_index = 0;
    uint8_t buffer[4];
    buffer_append_int32(buffer, (int32_t)(current * 1000.0), &send_index);
    comm_can_transmit_eid_replace_vesc(controller_id |
                                       ((uint32_t)CAN_PACKET_SET_CURRENT << 8), buffer, send_index, 1);
}

void vesc_can_set_duty(uint8_t controller_id, float duty) {
    int32_t send_index = 0;
    uint8_t buffer[4];
    buffer_append_int32(buffer, (int32_t)(duty * 100000.0), &send_index);
    comm_can_transmit_eid_replace_vesc(controller_id |
                                       ((uint32_t)CAN_PACKET_SET_DUTY << 8), buffer, send_index, 1);
}

void vesc_can_set_current_brake(uint8_t controller_id, float current) {
    int32_t send_index = 0;
    uint8_t buffer[4];
    buffer_append_int32(buffer, (int32_t)(current * 1000.0), &send_index);
    comm_can_transmit_eid_replace_vesc(controller_id |
                                       ((uint32_t)CAN_PACKET_SET_CURRENT_BRAKE << 8), buffer, send_index, 1);
}

void vesc_can_set_current_rel(uint8_t controller_id, float current_rel) {
    int32_t send_index = 0;
    uint8_t buffer[4];
    buffer_append_float32(buffer, current_rel, 1e5, &send_index);
    comm_can_transmit_eid_replace_vesc(controller_id |
                                       ((uint32_t)CAN_PACKET_SET_CURRENT_REL << 8), buffer, send_index, 1);
}

void vesc_can_set_current_brake_rel(uint8_t controller_id, float current_rel) {
    int32_t send_index = 0;
    uint8_t buffer[4];
    buffer_append_float32(buffer, current_rel, 1e5, &send_index);
    comm_can_transmit_eid_replace_vesc(controller_id |
                                       ((uint32_t)CAN_PACKET_SET_CURRENT_BRAKE_REL << 8), buffer, send_index, 1);
}

void comm_can_set_rpm(uint8_t controller_id, int32_t rpm) {
    int32_t send_index = 0;
    uint8_t buffer[4];
    buffer_append_int32(buffer, (int32_t)rpm, &send_index);
    comm_can_transmit_eid_replace_vesc(controller_id |
                                       ((uint32_t)CAN_PACKET_SET_RPM << 8), buffer, send_index, 1);
}

void vesc_can_get_imu(uint8_t controller_id) {
    //int32_t send_index = 0;
    uint16_t mask=0x7<<0;//rpy //watch commands.c COMM_GET_IMU_DATA
    mask|=0x7<<3;//acc
    mask|=0x7<<6;//gyro
    //mask|=0xF<<9;//quaternions add if need
    nbDataReceive=0;
    uint8_t buffer[8]={MY_ADRESSE|controller_id,0x00/*process packet*/,COMM_GET_IMU_DATA,(uint8_t)(mask>>8),(uint8_t)mask}; //reply with 8 low bit adresse = MY_ADRESSE|controller_id
    comm_can_transmit_eid_replace_vesc(controller_id |
                                       ((uint32_t)CAN_PACKET_PROCESS_SHORT_BUFFER << 8), buffer, sizeof(buffer), 1);
}

void vesc_send_buffer(uint8_t controller_id, uint8_t* buffer, uint32_t length)
{
    if(length<=6)
    {
        uint8_t bufferCan[8];
        bufferCan[0] = 254; //idVesc
        bufferCan[1] = 0; //for commands_process_packet
        memcpy(bufferCan+2, buffer, length);
        comm_can_transmit_eid_replace(controller_id |
                                      ((uint32_t)CAN_PACKET_PROCESS_SHORT_BUFFER << 8), bufferCan, length+2, 1);
    }
    else
    {
        int ptBuffer;
        uint8_t bufferCan[8];

        int len;
        for(ptBuffer=0;ptBuffer<length;)
        {
            if(ptBuffer<255)
            {
                bufferCan[0]=ptBuffer;
                len=MIN(8,length-ptBuffer+1);//+ 1 for header
                memcpy(bufferCan+1, buffer+ptBuffer, len-1);
                comm_can_transmit_eid_replace(controller_id |
                                              ((uint32_t)CAN_PACKET_FILL_RX_BUFFER << 8), bufferCan, len, 1);
                ptBuffer+=len-1;
            }
            else
            {
                bufferCan[0]=ptBuffer>>8;
                bufferCan[1]=ptBuffer;
                len=MIN(8,length-ptBuffer+2);//+2 for header
                memcpy(bufferCan+2, buffer+ptBuffer, len-2);
                comm_can_transmit_eid_replace(controller_id |
                                              ((uint32_t)CAN_PACKET_FILL_RX_BUFFER_LONG << 8), bufferCan, len, 1);
                ptBuffer+=len-2;
            }
        }

        uint32_t indice = 0;
        bufferCan[indice++] = 254;
        bufferCan[indice++] = 0;
        bufferCan[indice++] = length >> 8;
        bufferCan[indice++] = length & 0xFF;

        uint16_t crc=crc16Vesc(buffer, length);

        bufferCan[indice++] = crc>>8;
        bufferCan[indice++] = crc;

        comm_can_transmit_eid_replace(controller_id |
                                      ((uint32_t)CAN_PACKET_PROCESS_RX_BUFFER << 8), bufferCan, 6, 1);
    }
}

void vescs_can_request_fault(uint8_t controller_id){ //send a request for vesc return Fault code
    uint8_t buffer[]={COMM_GET_VALUES_SELECTIVE,(uint8_t)(MASK_GET_VALUE_FAULT>>24),(uint8_t)(MASK_GET_VALUE_FAULT>>16),(uint8_t)(MASK_GET_VALUE_FAULT>>8),(uint8_t)MASK_GET_VALUE_FAULT}; //bit 15 = fault_code
    vesc_send_buffer(controller_id,buffer,sizeof(buffer));
}


bool vescs_can_get_is_timeout(){
    bool ret=0;
    for(int l=0;l<NB_VESC;l++)
    {
        if((getTimeStampMs()-statusVesc[l].timeLastData)>=VESC_TIMEOUT)
            ret=1;
    }
    return ret;
}

bool vesc_can_get_is_timeout(uint8_t numVesc){
    bool ret=1;
    if(numVesc<NB_VESC)
        if((getTimeStampMs()-statusVesc[numVesc].timeLastData)<VESC_TIMEOUT)
            ret=0;
    return ret;
}

bool vesc_can_get_timeoutMs(uint8_t numVesc){
    return (getTimeStampMs()-statusVesc[numVesc].timeLastData);
}

bool vescs_can_as_error(){
    bool ret=0;
    for(int l=0;l<NB_VESC;l++)
    {
        if(statusVesc[l].mcFaultCode)
            ret=1;
    }
    return ret;
}

