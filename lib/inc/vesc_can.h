/*
 * vesc_can.h
 *
 *  Created on: 27 Jul 2021
 *      Author: mehmedblazevic
 * Description: Here is the driver for communicating
 * with VESC through CAN protocol. BE CAREFUL, VESC must be
 * configured to send status messages. Otherwise, this code
 * will not work at all.
 */

#ifndef LIB_INC_VESC_CAN_H_
#define LIB_INC_VESC_CAN_H_


#define VESC_CAN        (1)

#define PACKET_MAX_PL_LEN       512
#define PACKET_BUFFER_LEN       (PACKET_MAX_PL_LEN + 8)
#define VESC_TIMEOUT 9000 //9s for no show timeout when ibms switch from charde to load mode


#include <stdint.h>
#include "vesc.h"
#include "mcp2515_driver.h"

// Messages id's (for more details see: https://vesc-project.com/node/2759)
// here are the CAN commands: https://github.com/vedderb/bldc/blob/master/datatypes.h
#define SET_DUTY_C      0x000
#define SET_CURRENT     0x100
#define SET_CUR_BRAKE   0x200
#define SET_RPM         0x300
#define SET_POSITION    0x400
#define GET_DATAS_LB    0x500
#define PROCESS_BUFFER  0x700
#define ASK_FOR_DATA    0x800
#define STATUS_BROADC    0x900
#define STATUS_BROADC2   0xE00
#define STATUS_BROADC3   0xF00
#define STATUS_BROADC4   0x1000
#define STATUS_BROADC5   0x1B00


// addresses when using Get Data Long Buff (GET_DATAS_LB)
#define MOS_TEMP123     0x0004  // mos temp1, mos temp2 & mos temp3
#define MOS_TEMP456     0x07    // mos temp4, mos temp5, mos temp6 & temp pcb
#define TP_AMC_AIC      0x0E    // temp pcb, average motor current & average input current
#define AIC_DC_RPM      0x15    // average input current, duty cycle & rpm
#define RPM_VOLT_AHC    0x1C    // rpm, voltage & amp hours consumed
#define AHC_WHC         0x23    // amp hours charged & watt hours consumed
#define WHC_WHC_ODOM    0x2A    // whc, watt hours charged & odometer
#define ODO_ODOA_FAULT  0x31    // odometer, odometer abs & fault code

#define OK      0
#define ERR     -1

#ifdef VESC_CAN

#define NB_MAX_CALLBACK_VESC 2

typedef void (*tCallBackVesc)(uint8_t idVesc, uint8_t* data, uint32_t length);

void vescInit();
bool vesc_registre_call_back(uint8_t cmd, tCallBackVesc callBackCan);
bool vesc_unregistre_call_back(uint8_t cmd, tCallBackVesc callBackCan);
void update_vesc(can_t msg);
t_vescStatus vesc_get_value();

void vesc_can_set_current(uint8_t controller_id, float current);
void vesc_can_set_current_brake(uint8_t controller_id, float current);
void vesc_can_set_current_rel(uint8_t controller_id, float current_rel);
void vesc_can_set_current_brake_rel(uint8_t controller_id, float current_rel);
void comm_can_set_rpm(uint8_t controller_id, int32_t rpm);
void comm_can_conf_foc_erpms(uint8_t controller_id, float foc_openloop_rpm, float foc_sl_erpm);
void vesc_can_get_imu(uint8_t controller_id);
void comm_can_transmit_eid_replace_vesc(uint32_t id, const uint8_t *data, uint8_t len, _Bool replace);
void comm_can_transmit_eid_replace(uint32_t id, const uint8_t *data, uint8_t len, _Bool replace);
void vesc_send_buffer(uint8_t controller_id, uint8_t* buffer, uint32_t length);
void vescs_can_request_fault(uint8_t controller_id);
bool vescs_can_get_is_timeout();
bool vesc_can_get_is_timeout(uint8_t numVesc);
bool vesc_can_get_timeoutMs(uint8_t numVesc);
bool vescs_can_as_error();
#endif
#endif /* LIB_INC_VESC_CAN_H_ */
