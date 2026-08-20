/*
 * mpq5031.c
 *
 *  Created on: 23 août 2023
 *      Author: Mehmed Blazevic
 */

#ifdef MPQ5031
#include "mpq5031.h"
#include "watchdog.h"

static I2C_Handle      handle;


/*the mpq5031 are the USB-C controleur
we need call mpq5031Init one time (in the control thread)
and mpq5031SetPd() need to be call 3s after is power ON (for exemple on BBV3 3s after call bigBertaSetPowerState(POWER_STATE_FULL);
*/

bool mpq5031Init()
{
    I2C_Params      paramsI2c;
    I2C_Params_init(&paramsI2c);
    paramsI2c.transferMode  = I2C_MODE_BLOCKING;
    handle = I2C_open(CONFIG_I2C_0, &paramsI2c);
    return handle;
}

bool mpq5031SetPd() //set at 1.5A for 5V 9V 15V and 20V the card need to pe powered with >21V on pin VBAT_OUT
{
#define USB_C_MAX_CURRENT 1500 //1.5A the PCB can supply 3A but after testing it becomes too hot he stabilize to ~2A then chose 1.5A for security

    volatile bool ret;
    mpq5031WriteReg(0x00,0x70); //PDO2 to 4 enable in Fixed PDO
    //PDO1 (5V)
    mpq5031WriteReg(0x02,USB_C_MAX_CURRENT/10);
    //PDO2 9V
    mpq5031WriteReg(0x03,9000/50); //set voltage
    mpq5031WriteReg(0x04,USB_C_MAX_CURRENT/10); //set current
    //PDO3 15V
    mpq5031WriteReg(0x05,15000/50); //set voltage
    mpq5031WriteReg(0x06,USB_C_MAX_CURRENT/10); //set current
    //PDO4 20V
    mpq5031WriteReg(0x07,20000/50); //set voltage
    mpq5031WriteReg(0x08,USB_C_MAX_CURRENT/10); //set current
    //PDO5 20V NOT USE disable in registre 0x00
    mpq5031WriteReg(0x09,20000/50); //set voltage NOT USE
    mpq5031WriteReg(0x0A,USB_C_MAX_CURRENT/10); //set current NOT USE
    ret=mpq5031WriteReg(0x0B,I2C_SLAVE_ADDRESS|SLAVE_DEVICE_SEL|(1<<11)|(1<<14));//CTRL1
    ret=mpq5031WriteReg(0x0D,GPIO3_MODE);//CTRL3 GPIO3=SEL_OUT,GPIO2=QC_12,GPIO1=SEL_OUT
    ret=mpq5031WriteReg(0x0E,0x00);//CTRL4 GPIO6=PDO4_SEL_OUT GPIO7=discharge NTC= load-shedding

    /*while(1)
    {
        volatile int STATUS2=mpq5031ReadReg(0x11); //I2C_STATUS_BUS_BUSY

        ClockP_usleep(1000);
        watchdogClear();
    }*/

    return ret;
}

bool mpq5031WriteReg(uint8_t add, uint16_t value)
{
    I2C_Transaction i2cTransaction;
    char writeBuff[3]={add,value,value>>8};
    i2cTransaction.writeBuf = writeBuff;
    i2cTransaction.writeCount = sizeof(writeBuff);
    i2cTransaction.readBuf = NULL;
    i2cTransaction.readCount = 0;
    i2cTransaction.slaveAddress = MPQ_ADRESSE;
    return I2C_transfer(handle, &i2cTransaction);
}

int32_t mpq5031ReadReg(uint8_t add)
{
    I2C_Transaction i2cTransaction;
    char readBuff[2];
    i2cTransaction.writeBuf = &add;
    i2cTransaction.writeCount = 1;
    i2cTransaction.readBuf = readBuff;
    i2cTransaction.readCount =sizeof(readBuff);
    i2cTransaction.slaveAddress = MPQ_ADRESSE;
    volatile bool ret2=I2C_transfer(handle, &i2cTransaction);
    if(!ret2)
        return -1;
    else
        return (readBuff[0])|(readBuff[1]<<8);
}

#endif
