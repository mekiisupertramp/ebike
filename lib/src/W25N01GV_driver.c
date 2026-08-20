/*
 * W25N01GV_driver.c
 *
 *  Created on: 10 Mar 2022
 *      Author: mehmedblazevic
 */


#include "W25N01GV_driver.h"

static SPI_Handle      spi;
static SPI_Params      spiParams;
static SPI_Transaction transaction;

static enum chipModels _model;
//static int _dieSelect;



int8_t init_W25N01GV(uint32_t spi_clock){
   SPI_Params_init(&spiParams);
   spiParams.frameFormat = SPI_POL0_PHA0;
   spiParams.bitRate = spi_clock;
   spiParams.mode = SPI_MASTER;

   spi = SPI_open(CONFIG_SPI_FLASH, &spiParams);

   if(spi == NULL){
       return ERR;
   }

   reset();

    char jedec[5] = {W25N_JEDEC_ID, 0x00, 0x00, 0x00, 0x00};
    readData(jedec, jedec, sizeof(jedec));
    if(jedec[2] == WINBOND_MAN_ID){
        if((uint16_t)(jedec[3] << 8 | jedec[4]) == W25N01GV_DEV_ID){
            setStatusReg(W25N_PROT_REG, 0x00);
            _model = W25N01GV;
            return OK;
        }
        if((uint16_t)(jedec[3] << 8 | jedec[4]) == W25M02GV_DEV_ID){
            _model = W25M02GV;
            dieSelect(0);
            setStatusReg(W25N_PROT_REG, 0x00);
            dieSelect(1);
            setStatusReg(W25N_PROT_REG, 0x00);
            dieSelect(0);
            return OK;
        }
    }
    return ERR;
}
void readData(char *buf, char *readBuf, uint32_t n){
    transaction.txBuf = buf;
    transaction.rxBuf = readBuf;
    transaction.count = n;
    GPIO_write(FLASH_nCS, 0);
    SPI_transfer(spi, &transaction);
    GPIO_write(FLASH_nCS, 1);
}
void readDataNoCS(char *buf, char *readBuf, uint32_t n){
    transaction.txBuf = buf;
    transaction.rxBuf = readBuf;
    transaction.count = n;
    SPI_transfer(spi, &transaction);
}
void sendData(char *buf, uint32_t n){
    transaction.txBuf = buf;
    transaction.rxBuf = NULL;
    transaction.count = n;
    GPIO_write(FLASH_nCS, 0);
    SPI_transfer(spi, &transaction);
    GPIO_write(FLASH_nCS, 1);
}
void sendDataNoCS(char *buf, uint32_t n){
    transaction.txBuf = buf;
    transaction.rxBuf = NULL;
    transaction.count = n;
    SPI_transfer(spi, &transaction);
}

void reset(){
  //TODO check WIP in case of reset during write
  char buf[] = {W25N_RESET};
  sendData(buf, sizeof(buf));
  ClockP_usleep(600);   //Trst max time 500uS
}

int dieSelect(char die){
   if(_model!=W25M02GV)
       return OK;
  //TODO add some type of input validation
  char buf[2] = {W25M_DIE_SELECT, die};
  sendData(buf, sizeof(buf));
  //_dieSelect = die;
  return OK;
}

int dieSelectOnAdd(uint32_t pageAdd){
  if(pageAdd > getMaxPage()) return 1;
  return dieSelect(pageAdd / W25N01GV_MAX_PAGE);
}

char getStatusReg(char reg){
  char buf[3] = {W25N_READ_STATUS_REG, reg, 0x00};
  readData(buf, buf, sizeof(buf));
  return buf[2];
}

void setStatusReg(char reg, char set){
  char buf[3] = {W25N_WRITE_STATUS_REG, reg, set};
  sendData(buf, sizeof(buf));
}

uint32_t getMaxPage(){
  if (_model == W25M02GV) return W25M02GV_MAX_PAGE;
  if (_model == W25N01GV) return W25N01GV_MAX_PAGE;
  return 0;
}

void writeEnable(){
  char buf[] = {W25N_WRITE_ENABLE};
  sendData(buf, sizeof(buf));
}

void writeDisable(){
  char buf[] = {W25N_WRITE_DISABLE};
  sendData(buf, sizeof(buf));
}

int blockErase(uint32_t pageAdd){
  if(pageAdd > getMaxPage()) return ERR;
  dieSelectOnAdd(pageAdd);
  char pageHigh = (char)((pageAdd & 0xFF00) >> 8);
  char pageLow = (char)(pageAdd);
  char buf[4] = {W25N_BLOCK_ERASE, 0x00, pageHigh, pageLow};
  block_WIP();
  writeEnable();
  sendData(buf, sizeof(buf));
  return OK;
}

int bulkErase(){
  int error = 0;
  uint32_t i=0;
  for(i = 0; i < getMaxPage(); i+=64){
    if((error = blockErase(i)) != 0) return error;
  }
  return 0;
}

volatile int debug2=-1;

int loadProgData(uint16_t columnAdd, char* buf, uint32_t dataLen){
  if(columnAdd > (uint32_t)W25N_MAX_COLUMN) return 1;
  if(dataLen > (uint32_t)W25N_MAX_COLUMN - columnAdd) return 1;
  char columnHigh = (columnAdd & 0xFF00) >> 8;
  char columnLow = columnAdd & 0xff;
  char cmdbuf[3] = {W25N_PROG_DATA_LOAD, columnHigh, columnLow};
  block_WIP();
  writeEnable();
  GPIO_write(FLASH_nCS, 0);
  debug2=0;
  sendDataNoCS(cmdbuf, sizeof(cmdbuf));
  debug2=1;
  sendDataNoCS(buf, dataLen);
  debug2=2;
  GPIO_write(FLASH_nCS, 1);
  return 0;
}

//for use with W25N02GV
int loadProgData2(uint16_t columnAdd, char* buf, uint32_t dataLen, uint32_t pageAdd){
  if(dieSelectOnAdd(pageAdd)) return 1;
  return loadProgData(columnAdd, buf, dataLen);
}

int loadRandProgData(uint16_t columnAdd, char* buf, uint32_t dataLen){
  if(columnAdd > (uint32_t)W25N_MAX_COLUMN) return 1;
  if(dataLen > (uint32_t)W25N_MAX_COLUMN - columnAdd) return 1;
  char columnHigh = (columnAdd & 0xFF00) >> 8;
  char columnLow = columnAdd & 0xff;
  char cmdbuf[3] = {W25N_RAND_PROG_DATA_LOAD, columnHigh, columnLow};
  block_WIP();
  writeEnable();
  GPIO_write(FLASH_nCS, 0);
  sendDataNoCS(cmdbuf, sizeof(cmdbuf));
  sendDataNoCS(buf, dataLen);
  GPIO_write(FLASH_nCS, 1);
  return 0;
}

//for use with W25N02GV
int loadRandProgData2(uint16_t columnAdd, char* buf, uint32_t dataLen, uint32_t pageAdd){
  if(dieSelectOnAdd(pageAdd)) return 1;
  return loadRandProgData(columnAdd, buf, dataLen);
}

int ProgramExecute(uint32_t pageAdd){
  if(pageAdd > getMaxPage()) return ERR;
  dieSelectOnAdd(pageAdd);
  char pageHigh = (char)((pageAdd & 0xFF00) >> 8);
  char pageLow = (char)(pageAdd);
  debug2=5;
  writeEnable();
  debug2=6;
  char buf[4] = {W25N_PROG_EXECUTE, 0x00, pageHigh, pageLow};
  sendData(buf, sizeof(buf));
  debug2=7;
  return OK;
}

int pageDataRead(uint32_t pageAdd){
  if(pageAdd > getMaxPage()) return 1;
  dieSelectOnAdd(pageAdd);
  char pageHigh = (char)((pageAdd & 0xFF00) >> 8);
  char pageLow = (char)(pageAdd);
  char buf[4] = {W25N_PAGE_DATA_READ, 0x00, pageHigh, pageLow};
  block_WIP();
  sendData(buf, sizeof(buf));
  return 0;

}

int read(uint16_t columnAdd, char* buf, uint32_t dataLen){
  if(columnAdd > (uint32_t)W25N_MAX_COLUMN) return 1;
  if(dataLen > (uint32_t)W25N_MAX_COLUMN - columnAdd) return 1;
  char columnHigh = (columnAdd & 0xFF00) >> 8;
  char columnLow = columnAdd & 0xff;
  char cmdbuf[4] = {W25N_READ, columnHigh, columnLow, 0x00};
  block_WIP();
  GPIO_write(FLASH_nCS, 0);
  sendDataNoCS(cmdbuf, sizeof(cmdbuf));
  readDataNoCS(buf, buf, dataLen);
  GPIO_write(FLASH_nCS, 1);
  return 0;
}

//Returns the Write In Progress bit from flash.
int check_WIP(){
  char status = getStatusReg(W25N_STAT_REG);
  if(status & 0x01){
    return 1;
  }
  return 0;
}

int block_WIP(){
  //Max WIP time is 10ms for block erase so 15 should be a max.
  int cpt = 0;
  while(check_WIP()){
    ClockP_usleep(1000);
    cpt++;
    if (cpt > 15) return 1;
  }
  return 0;
}

int check_status(){
  return(getStatusReg(W25N_STAT_REG));
}
