/*
 * mcp2515_driver.c
 *
 *  Created on: 18 Jun 2021
 *      Author: mehmedblazevic
 * Description: This file contains functions needed to control
 * the MCP2515 from Microchip. Every commands to control the
 * MCP are implemented and a CAN abstraction is provided which
 * allow the user to send and receive CAN messages with ease.
 */

#include "mcp2515_driver.h"

#define SPI_MSG_LENGTH  (30)

static uint8_t can_b;

static SPI_Handle      spi;
static SPI_Params      spiParams;
static SPI_Transaction transaction;
static unsigned char masterRxBuffer[SPI_MSG_LENGTH];
static unsigned char masterTxBuffer[SPI_MSG_LENGTH];

char SPIPutc(char msg){
    masterTxBuffer[0] = msg;
    transaction.count = 1;
    SPI_transfer(spi, &transaction);
    return masterRxBuffer[0];
}
void SPIByteWrite(char cmd, char addr, char value )
{
    GPIO_write(MCP2515_nEN, 0);
    volatile int l;
    for(l=0;l<100;l++);
    masterTxBuffer[0] = cmd;
    masterTxBuffer[1] = addr;
    masterTxBuffer[2] = value;
    transaction.count = 3;
    //if(addr==0) transaction.count -=1;
    //if(value==0) transaction.count -=1;
    SPI_transfer(spi, &transaction);
    GPIO_write(MCP2515_nEN, 1);
}

void SPICmdDataWrite(char cmd, char value )
{
    GPIO_write(MCP2515_nEN, 0);
    masterTxBuffer[0] = cmd;
    masterTxBuffer[1] = value;
    transaction.count = 2;
    SPI_transfer(spi, &transaction);
    GPIO_write(MCP2515_nEN, 1);
}


void SPICmdWrite(char cmd)
{
    GPIO_write(MCP2515_nEN, 0);
    volatile int l;
    masterTxBuffer[0] = cmd;
    transaction.count = 1;
    SPI_transfer(spi, &transaction);
    GPIO_write(MCP2515_nEN, 1);
}

char SPIByteRead(char cmd, char addr)
{
    char tmp=0;
    GPIO_write(MCP2515_nEN, 0);
    volatile int l;
    masterTxBuffer[0] = cmd;
    masterTxBuffer[1] = addr;
    masterTxBuffer[2] = 0x00;
    transaction.count = 3;
    //if(addr==0) transaction.count -= 1;
    SPI_transfer(spi, &transaction);
    GPIO_write(MCP2515_nEN, 1);
    tmp = masterRxBuffer[transaction.count-1];
    return tmp;
}

void SPIBytesWrite(char addr, char *values, char n){
    char i=0;
    masterTxBuffer[0] = INST_WRITE;
    masterTxBuffer[1] = addr;
    while(i<n){
        masterTxBuffer[i+2] = values[i];
        i++;
    }
    transaction.count = n+2;
    GPIO_write(MCP2515_nEN, 0);
    SPI_transfer(spi, &transaction);
    GPIO_write(MCP2515_nEN, 1);
}

void SPIBytesRead(char addr, char *buffer, char n){
    int i=0;
    masterTxBuffer[0] = INST_READ;
    masterTxBuffer[1] = addr;
    transaction.count = n+2;
    GPIO_write(MCP2515_nEN, 0);
    SPI_transfer(spi, &transaction);
    GPIO_write(MCP2515_nEN, 1);
    for(i=0 ; i<n ; i++){
        buffer[i] = masterRxBuffer[i+2];
    }
}
/*
 * This function init every register of the MCP2515. Thanks to Microchip
 * for providing it.
 */
void Init2515()
{
    volatile char dummy = 0;

    //Clear masks to RX all messages
    SPIByteWrite(INST_WRITE,RXM0SIDH,0x00);
    SPIByteWrite(INST_WRITE,RXM0SIDL,0x00);

    //filter enable on extended ID
    SPIByteWrite(INST_WRITE,RXF0SIDL,1<<3);

    //Clear filter... really only concerned to clear EXIDE bit
    SPIByteWrite(INST_WRITE,RXB0CTRL,BUKT);

    //Set CNF1
    SPIByteWrite(INST_WRITE, CNF1, SJW_1TQ | can_b);

    //Set CNF2
    SPIByteWrite(INST_WRITE,CNF2,0x80 | PHSEG1_1TQ | PRSEG_1TQ);

    //Set CNF3
    SPIByteWrite(INST_WRITE,CNF3, PHSEG2_2TQ);

    //Set TXB0 DLC and Data for a "Write Register" Input Message to the MCP25020
    SPIByteWrite(INST_WRITE,TXB0SIDH,0xA0);    //Set TXB0 SIDH
    SPIByteWrite(INST_WRITE,TXB0SIDL,0x00);    //Set TXB0 SIDL
    SPIByteWrite(INST_WRITE,TXB0DLC,DLC_3);    //Set DLC = 3 bytes
    SPIByteWrite(INST_WRITE,TXB0D0,0x1E);      //D0 = Addr = 0x1E
    SPIByteWrite(INST_WRITE,TXB0D1,0x10);      //D1 = Mask = 0x10

    //Set TXB1 DLC and Data for a "READ I/O IRM"
    SPIByteWrite(INST_WRITE,TXB1SIDH,0x50);    //Set TXB0 SIDH
    SPIByteWrite(INST_WRITE,TXB1SIDL,0x00);    //Set TXB0 SIDL
    SPIByteWrite(INST_WRITE,TXB1DLC,0x40 | DLC_8);    //Set DLC = 3 bytes and RTR bit

    //Interrupt on RXB0 and RXB1 - CANINTE
    SPIByteWrite(INST_WRITE,CANINTE,0x03);

    //Set NORMAL mode
    SPIByteWrite(INST_WRITE,CANCTRL,REQOP_NORMAL | CLKOUT_ENABLED);

    volatile char c;
    c = SPIByteRead(INST_READ, CANCTRL);//for test SPI com
    //Verify device entered Normal mode
    dummy = SPIByteRead(INST_READ, CANSTAT);
    if (OPMODE_NORMAL != (dummy && 0xE0))
        SPIByteWrite(INST_WRITE,CANCTRL,REQOP_NORMAL | CLKOUT_ENABLED);
}

int8_t init_mcp2515(uint32_t spi_clock, uint8_t can_bitrate){
    SPI_Params_init(&spiParams);
    spiParams.frameFormat = SPI_POL0_PHA0;
    spiParams.bitRate = spi_clock;
    spiParams.mode = SPI_MASTER;

    transaction.txBuf = (void *) masterTxBuffer;
    transaction.rxBuf = (void *) masterRxBuffer;

    mcp2515_rst();

    spi = SPI_open(CONFIG_SPI_0, &spiParams);

    if(spi == NULL){
        return ERR;
    }

    // init tx buffer
    memset((void *)masterTxBuffer, 0, SPI_MSG_LENGTH);

    // init rx buffer
    memset((void *)masterRxBuffer, 0, SPI_MSG_LENGTH);

    can_b = can_bitrate;
    Init2515();

    //SPI_close(spi);


    return OK;
}

void mcp2515_close(){
    SPI_close(spi);
}

void mcp2515_rst(){
    GPIO_write(MCP2515_nEN, 0);
    GPIO_write(MCP2515_nRst, 0);
    ClockP_usleep(10000);
    GPIO_write(MCP2515_nRst, 1);
    ClockP_usleep(10000);
    GPIO_write(MCP2515_nEN, 1);
    ClockP_usleep(10000);
}

char mcp2515_read_status(){
    return SPIByteRead(INST_READ_STAT, 0);
}

inline char mcp2515_rx_status(){
    return SPIByteRead(INST_RX_STAT, 0);
}

void mcp2515_read(char adr, char *datas, char n){
    SPIBytesRead(adr,datas,n);
}
// nm can be 0x00, 0x02, 0x04, 0x06
char mcp2515_rx_buffer(char nm){
    return SPIByteRead(INST_READ_RX_BUF | (nm&0x06), 0);
}
void mcp2515_write(char *datas, char n){
    SPIBytesWrite(INST_WRITE, datas, n);
}
// abc can be any combination of the 3 LSB bits
void mcp2515_load_tx_buffer(char abc, char data){
    SPICmdDataWrite(INST_LOAD_TX_BUF | (abc&0x07), data);
}
// nnn can be any combination of the 3 LSB bits
void mcp2515_RTS(char nnn){
    SPICmdWrite(INST_RTS | (nnn&0x07));
}

void mcp2515_clearInt(){
    SPIByteWrite(INST_WRITE,CANINTF,0x00);
}
void mcp2515_bit_modify(char adr, char mask, char data){
    masterTxBuffer[0] = INST_BIT_MOD;
    masterTxBuffer[1] = adr;
    masterTxBuffer[2] = mask;
    masterTxBuffer[3] = data;
    transaction.count = 4;
    GPIO_write(MCP2515_nEN, 0);
    SPI_transfer(spi, &transaction);
    GPIO_write(MCP2515_nEN, 1);
}

#if SUPPORT_EXTENDED_CANID

uint8_t mcp2515_read_id(uint32_t *id)
{
    uint8_t first;
    uint8_t tmp;

    masterTxBuffer[0] = 0xFF;
    masterTxBuffer[1] = 0xFF;
    masterTxBuffer[2] = 0xFF;
    masterTxBuffer[3] = 0xFF;
    transaction.count = 4;

    SPI_transfer(spi, &transaction);

    first = masterRxBuffer[0];
    tmp   = masterRxBuffer[1];

    if (tmp & IDE) {

        *((uint16_t *) id + 1)  = (uint16_t) first << 5;
        *((uint8_t *)  id + 1)  = masterRxBuffer[2];

        *((uint8_t *)  id + 2) |= (tmp >> 3) & 0x1C;
        *((uint8_t *)  id + 2) |=  tmp & 0x03;

        *((uint8_t *)  id)      = masterRxBuffer[3];

        return 1;
    }
    else {
        //SPIPutc(0xff);

        *((uint8_t *)  id + 3) = 0;
        *((uint8_t *)  id + 2) = 0;

        *((uint16_t *) id) = (uint16_t) first << 3;

        //SPIPutc(0xff);

        *((uint8_t *) id) |= tmp >> 5;

        return 0;
    }
}

void mcp2515_write_id(uint32_t *id, uint8_t extended)
{
    uint8_t tmp;

    if (extended) {
        SPIPutc(*((uint16_t *) id + 1) >> 5);

        tmp  = (*((uint8_t *) id + 2) << 3) & 0xe0;
        tmp |= IDE;
        tmp |= (*((uint8_t *) id + 2)) & 0x03;


        SPIPutc(tmp);
        SPIPutc(*((uint8_t *) id + 1));
        SPIPutc(*((uint8_t *) id));
    }
    else {
        SPIPutc(*((uint16_t *) id) >> 3);

        tmp = *((uint8_t *) id) << 5;

        SPIPutc(tmp);
        SPIPutc(0);
        SPIPutc(0);
    }
}

#else

unsigned char mcp2515_read_id(uint16_t *id)
{
    char first;
    char tmp;

    first = SPIPutc(0xff);
    tmp   = SPIPutc(0xff);

    if (tmp & IDE) {
        SPIPutc(0xff);
        SPIPutc(0xff);

        return 1;           // extended-frame
    }
    else {
        SPIPutc(0xff);

        *id = (uint16_t) first << 3;

        SPIPutc(0xff);

        *((uint8_t *) id) |= tmp >> 5;


        if (tmp & SRR)
            return 2;       // RTR-frame
        else
            return 0;       // normal-frame
    }
}
void mcp2515_write_id(uint16_t *id){
    uint8_t tmp;

    SPIPutc(*id >> 3);
    tmp = *((uint8_t *) id) << 5;

    SPIPutc(tmp);
    SPIPutc(0);
    SPIPutc(0);
}
#endif

#ifndef USE_OPTIMIZED_CAN
unsigned char mcp2515_get_message(can_t *msg){
    unsigned char status = mcp2515_rx_status();
    unsigned char addr;

    // check where is the message received
    if (_bit_is_set(status,6)) {
        // message in buffer 0
        addr = INST_READ_RX_BUF;
    }
    else if (_bit_is_set(status,7)) {
        // message in buffer 1
        addr = INST_READ_RX_BUF | 0x04;
    }
    else {
        // Error: no message available
        return 0;
    }

    GPIO_write(MCP2515_nEN, 0);
    SPIPutc(addr);

    char tmp = mcp2515_read_id(&msg->id);

#if SUPPORT_EXTENDED_CANID
    msg->flags.extended = tmp & 0x01;
#else
    if(tmp & 0x01){
        GPIO_write(MCP2515_nEN, 1);
        if(_bit_is_set(status,6)){
            mcp2515_bit_modify(CANINTF, RX0IF, 0);
        }else{
            mcp2515_bit_modify(CANINTF, RX1IF, 0);
        }
        return 0;
    }
#endif

    // read DLC
    char length = SPIPutc(0xff);
    msg->flags.rtr = (_bit_is_set(status,3)) ? 1 : 0;

    length &= 0x0f;
    msg->length = length;


    transaction.rxBuf=msg->data;
    masterTxBuffer[0] = 0xFF;
    masterTxBuffer[1] = 0xFF;
    masterTxBuffer[2] = 0xFF;
    masterTxBuffer[3] = 0xFF;
    masterTxBuffer[4] = 0xFF;
    masterTxBuffer[5] = 0xFF;
    masterTxBuffer[6] = 0xFF;
    masterTxBuffer[7] = 0xFF;
    transaction.count = length;

    SPI_transfer(spi, &transaction);

    transaction.rxBuf=masterRxBuffer;

    // read datas
    /*for(i=0 ; i<length ; i++){
        msg->data[i] = SPIPutc(0xff);
    }*/
    GPIO_write(MCP2515_nEN, 1);

    // clear interrupt flag
    if(_bit_is_set(status,6)){
        mcp2515_bit_modify(CANINTF, RX0IF, 0);
    }else{
        mcp2515_bit_modify(CANINTF, RX1IF, 0);
    }

    /*if(SPIByteRead(INST_READ, EFLG)!=0)
    {
        SPIByteWrite(INST_WRITE,EFLG,0x00);
        return 0;
    }*/



    return (status & 0x07) + 1;

}

#else //USE_OPTIMIZED_CAN


unsigned char mcp2515_get_message(can_t *msg){
    unsigned char status = mcp2515_rx_status();
    unsigned char addr;

    // check where is the message received
    if (_bit_is_set(status,6)) {
        // message in buffer 0
        addr = INST_READ_RX_BUF;
    }
    else if (_bit_is_set(status,7)) {
        // message in buffer 1
        addr = INST_READ_RX_BUF | 0x04;
    }
    else {
        // Error: no message available
        return 0;
    }

    GPIO_write(MCP2515_nEN, 0);
    //SPIPutc(addr);
    masterTxBuffer[0]=addr;
    masterTxBuffer[1]=0xFF; //read id
    masterTxBuffer[2]=0xFF;
    masterTxBuffer[3]=0xFF;
    masterTxBuffer[4]=0xFF;
    masterTxBuffer[5]=0xFF; // read DLC
    masterTxBuffer[6]=0xFF;//read data
    masterTxBuffer[7]=0xFF;
    masterTxBuffer[8]=0xFF;
    masterTxBuffer[9]=0xFF;
    masterTxBuffer[10]=0xFF;
    masterTxBuffer[11]=0xFF;
    masterTxBuffer[12]=0xFF;
    masterTxBuffer[13]=0xFF;

    transaction.count = 6+8; //6+length but for optimization read all take lesse time of make 2 transaction?

    SPI_transfer(spi, &transaction);


    uint8_t first = masterRxBuffer[1];;
    uint8_t tmp = masterRxBuffer[2];

    uint32_t *id=&(msg->id);


    if (tmp & IDE) {

        *((uint16_t *) id + 1)  = (uint16_t) first << 5;
        *((uint8_t *)  id + 1)  = masterRxBuffer[3];

        *((uint8_t *)  id + 2) |= (tmp >> 3) & 0x1C;
        *((uint8_t *)  id + 2) |=  tmp & 0x03;

        *((uint8_t *)  id)      = masterRxBuffer[4];

        msg->flags.extended = 1;
    }
    else {
        //SPIPutc(0xff);

        *((uint8_t *)  id + 3) = 0;
        *((uint8_t *)  id + 2) = 0;

        *((uint16_t *) id) = (uint16_t) first << 3;

        //SPIPutc(0xff);

        *((uint8_t *) id) |= tmp >> 5;

        msg->flags.extended = 0;
    }

    // read DLC
    //char length = SPIPutc(0xff);
    char length = masterRxBuffer[5];
    msg->flags.rtr = (_bit_is_set(status,3)) ? 1 : 0;

    length &= 0x0f;
    msg->length = length;

    memcpy(msg->data, masterRxBuffer+6, length);

    GPIO_write(MCP2515_nEN, 1);

    return (status & 0x07) + 1;
}

#endif

bool mcp2515_check_message(){
    return ((mcp2515_rx_status() & 0xC0) ? true : false);
}

#ifndef USE_OPTIMIZED_CAN

unsigned char mcp2515_send_message(can_t *msg){
    volatile unsigned char status = mcp2515_read_status();
    char i=0;

    Ne plus utiliser la version non optimizer  ou fixer l'utilisation uniquement du buffer 0
    /* status byte
     * Bit  Function
     *  2   TXB0CNTRL.TXREQ
     *  4   TXB1CNTRL.TXREQ
     *  6   TXB2CNTRL.TXREQ
     */
    unsigned char adr;
    // search first available tx buffer
    if(_bit_is_clear(status,2)){
        adr = 0x00;
    }else if(_bit_is_clear(status,4)){
        adr = 0x02;
        return 0;
    }else if(_bit_is_clear(status,6)){
        adr = 0x04;
        return 0;
    }else{
        // all buffers occuped
        return 0;
    }

    GPIO_write(MCP2515_nEN, 0);
    SPIPutc(INST_LOAD_TX_BUF | adr); //only avalable on MCP2515 (not on 2510)

#if SUPPORT_EXTENDED_CANID
    mcp2515_write_id(&msg->id, msg->flags.extended);

#else
    mcp2515_write_id(&msg->id);
#endif
    unsigned char length = msg->length & 0x0f;

    // is there a transmission request?
    if(msg->flags.rtr){
        SPIPutc(RTR | length);
    }else{
        SPIPutc(length);
        for(i = 0 ; i<length ; i++){
            SPIPutc(msg->data[i]);
        }
    }

    GPIO_write(MCP2515_nEN, 1);

    ClockP_usleep(10); // is it necessary?

    // send can message
    /*GPIO_write(MCP2515_nEN, 0);
    adr = (adr == 0) ? 1 : adr;
    SPIPutc(INST_RTS | adr);
    GPIO_write(MCP2515_nEN, 1);*/

    adr = (adr == 0) ? 1 : adr;
    mcp2515_RTS(adr);  //request to send the message


    return adr;
}
#else //USE_OPTIMIZED_CAN
unsigned char mcp2515_send_message(can_t *msg){

    GPIO_write(MCP2515_nEN, 0);
    masterTxBuffer[0] = INST_LOAD_TX_BUF | 0; //only use buffer 0

    uint32_t *id=&(msg->id);
    uint8_t tmp;

    if (msg->flags.extended) {
        masterTxBuffer[1]=(*((uint16_t *) id + 1) >> 5);

        tmp  = (*((uint8_t *) id + 2) << 3) & 0xe0;
        tmp |= IDE;
        tmp |= (*((uint8_t *) id + 2)) & 0x03;


        masterTxBuffer[2]=(tmp);
        masterTxBuffer[3]=(*((uint8_t *) id + 1));
        masterTxBuffer[4]=(*((uint8_t *) id));
    }
    else {
        masterTxBuffer[1]=(*((uint16_t *) id) >> 3);

        tmp = *((uint8_t *) id) << 5;

        masterTxBuffer[2]=(tmp);
        masterTxBuffer[3]=(0);
        masterTxBuffer[4]=(0);
    }


    unsigned char length = msg->length & 0x0f;

    // is there a transmission request?
    if(msg->flags.rtr){
        masterTxBuffer[5]=(RTR | length);
        transaction.count = 6;
    }else{
        masterTxBuffer[5]=(length);
        for(int i = 0 ; i<length ; i++){
            masterTxBuffer[6+i]=(msg->data[i]);
        }
        transaction.count = 7+length;
    }

    SPI_transfer(spi, &transaction);

    GPIO_write(MCP2515_nEN, 1);

    mcp2515_RTS(1);  //only use buffer 0

    return 1;
}
#endif

