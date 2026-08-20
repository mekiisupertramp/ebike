/*
 * bootLoader.h
 *
 *  Created on: 14 janv. 2022
 *      Author: Mehmed Blazevic
 */


#ifndef APPLICATION_BOOTLOADER_H_
#define APPLICATION_BOOTLOADER_H_

#include "cpu_variante_liste.h"
#include "appBootLoader.h"

#define LENGTH_STORE_VECT 8
#define FIRMWARE_SIZE (256*1024)


void bootLoaderInit();
void bootLoaderWrite(uint8_t* data);
void bootLoaderReboot();

typedef volatile struct
{
    uint32_t magic;
    uint32_t version;
    uint8_t sp_BootAdresse[LENGTH_STORE_VECT];
    uint32_t versionGCF;
} tFirmwreInfo;

#define BOOT_LOADER_HEADER_SIZE 4
#define BOOT_LOADER_PAKET_SIZE 128

extern const tFirmwreInfo firmwreInfo;
extern bool newFirmwareAvailable;
extern bool requestReboot;


#endif /* APPLICATION_BOOTLOADER_H_ */
