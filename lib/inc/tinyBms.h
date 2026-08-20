#ifndef TINYBMS_H
#define TINYBMS_H

//#define TINY_BMS 0

#ifdef TINY_BMS

#ifdef __cplusplus
extern "C"{
#endif

#include "stdint.h"

#include <ti/drivers/UART.h>
#include <ti/drivers/GPIO.h>
#include "ti_drivers_config.h"

#define UARTTinyBms &huart4 //uart4=P5

#define ERROR_REPLY_LENGHT 5
#define WRITE_REGISTERS_REPLY_LENGHT 8
#define READ_4byte 8
#define READ_SPEED_LENGTH 16

#define READ_TEMPERATURES_REPLY_LENGHT 11



typedef enum
{
    SB_ERROR,
	SB_WAIT_NEW_COMANDE,
	SB_WRITE_REGISTERS,
	SB_READ_PACK_VOLTAGE,
	SB_READ_CELLS_VOLTAGE,
    SB_READ_PACK_CURRENT,
    SB_READ_TEMPERATURES,
	SB_READ_SPEED,
	SB_READ_SOC,

} t_bmsState;

typedef struct
{
	int error;
	t_bmsState type;
	int length;
	uint8_t data[256];
} t_bmsReply;

#define NB_TEMPERATURE_TINYBMS 3
#define NB_CELL_TINY_BMS 16
typedef struct
{
	float uPack;
	float iPack;
	float temperature[NB_TEMPERATURE_TINYBMS];
	float uCell[NB_CELL_TINY_BMS];
	float speed;
	float leftDistance; //km
	uint32_t leftTime; //seconde
	float soc;
} t_infoBms;

extern t_infoBms infoBms;


int8_t init_tinyBMS(uint32_t baudrate);
float bms_read_voltage();


//int readRepVariableLength(uint8_t *data, uint8_t length);
int tinyBmsWriteRegister(uint16_t regAddress, uint16_t val);
int tinyBmsWriteRegisters(uint16_t regAddress, uint8_t* buffer, uint8_t length);
int tinyBmsReadRegisters(uint16_t regAddress, uint8_t* buffer, uint8_t length);
int tinyBmsReadPackVoltage();
int tinyBmsReadPackCurrent();
int tinyBmsReadTemperatures();
int tinyBmsReadUcells();
int tinyBmsReadSpeed();
int tinyBmsReadSoc();
void tinyBmsCompute(bool Start);
void tinyBmsDecodeUart(uint8_t* buffer,uint32_t size);


//void tinyBmsInit(int (*writeTmp)(uint8_t *buffer, uint8_t length),int (*flushTmp)(),int (*timerTimemoutTmp)(),int (*timerTimemoutStopTmp)());
t_bmsReply tinyBmsRead(uint8_t data);
float getFloatReply(t_bmsReply reply,int pos);
uint32_t getUint32Reply(t_bmsReply reply,int pos);

void tinyBmsReset();

#define BMS_ERROR -1
#define BMS_BUSY -2
#define BMS_NULL 0 //comunication not end
#define BMS_OK 1

#ifdef __cplusplus
}
#endif

#endif //TINYBMS_H

#endif
