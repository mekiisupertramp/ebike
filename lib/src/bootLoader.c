/*
 * bootLoader.c
 *
 *  Created on: 14 janv. 2022
 *      Author: Mehmed Blazevic
 */
#include <ti/drivers/NVS.h>
#include "ti_drivers_config.h"
#include <driverlib/sys_ctrl.h>

#include "bootLoader.h"
#include "memoryTask.h"
#include "stdbool.h"
#include "serviceBootLoader.h"
#include "W25N01GV_driver.h"
//#include "pixxil_driver.h"
#ifdef PIXXI_UPDATE
#include "pixxil_update.h"
#else
#define PIXXI_FLASH_SIZE 0
#endif


bool newFirmwareAvailable=false;
bool requestReboot=false;


#pragma DATA_SECTION(firmwreInfo, ".version")
#pragma CLINK(firmwreInfo)
volatile const tFirmwreInfo firmwreInfo={MAGIC_NUMBER,VERSION,0,VERSIONGCF};

void bootLoaderInit()
{
    volatile tFirmwreInfo tmp=firmwreInfo; //if not present compiler optimization remove firmwreInfo

    uint8_t data[BOOT_LOADER_PAKET_SIZE]={1,2,3,4,5,6,7,8,9,10};
    loadProgData(0, (char*)data, BOOT_LOADER_PAKET_SIZE);

    /*////test
    blockErase(0);
    loadProgData(0, (char*)data, BOOT_LOADER_PAKET_SIZE);
    ProgramExecute(0);
    pageDataRead(0);
    volatile uint8_t buffertest[W25N_PAGE_SIZE];
    read(0, (char*)buffertest, W25N_PAGE_SIZE);
    buffertest[0]=0;*/
}

void bootLoaderWrite(uint8_t* data)
{
    static volatile int oldAdresse=0; //for debug only
    volatile uint32_t address=*((uint32_t*)data); //4first byte = adresse
    data+=4;//remove address

    if(address==0xFFFFFFFF)
    {
        requestReboot=true;
        return;
    }

#ifdef PIXXI_UPDATE
    if(address==0xFFFFFFFE) //request check GCF
    {
        uint32_t size[2];
        uint32_t uncompressedSize[2];
        uint32_t version[2];
        bool isAllreadyUpdate[2];
        bool haskOk[2];
        checkGcf(0,size, uncompressedSize, version, isAllreadyUpdate, haskOk);// check if new GCF is valide and update serviceBootLoader_GcfVerVal with the new version is is valide
    }
#endif

    if(address%1024==0) //every 1kB send notify (for say data is written)
    {
        ServiceBootLoader_SetParameter(SERVICEBOOTLOADER_FLOWCONTROL, SERVICEBOOTLOADER_FLOWCONTROL_LEN, (void*)&address);
    }

    if(address==0 || address==oldAdresse+BOOT_LOADER_PAKET_SIZE || address==FIRMWARE_SIZE/*pixxil gcf file*/ || address==FIRMWARE_SIZE+PIXXI_FLASH_SIZE/*pixxil gcf file in second memory*/) //if is the first paquet or the next paquet (we not missing a paquet)
    {
        oldAdresse=address;

        if(address==0)
        {
            for(int l=0;l<FIRMWARE_SIZE/W25N_BLOCK_SIZE;l++)
                blockErase((l*W25N_BLOCK_SIZE)/W25N_PAGE_SIZE);
        }

        else if((address==FIRMWARE_SIZE/*pixxil gcf file*/) || (address==FIRMWARE_SIZE+PIXXI_FLASH_SIZE/*pixxil gcf file in second memory*/))
        {
            for(int l=0;l<PIXXI_FLASH_SIZE/W25N_BLOCK_SIZE;l++)
                blockErase((l*W25N_BLOCK_SIZE+address)/W25N_PAGE_SIZE);
        }
        else if(address/W25N_PAGE_SIZE>FIRMWARE_SIZE/W25N_PAGE_SIZE && address/W25N_PAGE_SIZE!=(FIRMWARE_SIZE+PIXXI_FLASH_SIZE)/W25N_PAGE_SIZE)// if is a data for pixxil and not the header
        {
            address+=W25N_BLOCK_SIZE-W25N_PAGE_SIZE;
        }

        static uint8_t buffer[W25N_PAGE_SIZE];
        memcpy(buffer+(address%W25N_PAGE_SIZE), data, BOOT_LOADER_PAKET_SIZE);
        if((address%W25N_PAGE_SIZE)==(W25N_PAGE_SIZE-BOOT_LOADER_PAKET_SIZE))
        {
            if(address<FIRMWARE_SIZE+PIXXI_FLASH_SIZE*2)
            {
                loadProgData(0, (char*)buffer, W25N_PAGE_SIZE);
                ProgramExecute(address/W25N_PAGE_SIZE);//when a page is full program it
            }
        }

        if(address==FIRMWARE_SIZE-BOOT_LOADER_PAKET_SIZE) //if is last packet
        {
            newFirmwareAvailable=true;
            ServiceBootLoader_SetParameter(SERVICEBOOTLOADER_FLOWCONTROL, SERVICEBOOTLOADER_FLOWCONTROL_LEN, (void*)&address);
        }

    }

}

void bootLoaderReboot()
{
    SysCtrlSystemReset();
}
