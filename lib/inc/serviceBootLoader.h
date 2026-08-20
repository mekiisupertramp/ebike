/**********************************************************************************************
 * Filename:       serviceBootLoader.h
 *
 * Description:    This file contains the serviceBootLoader service definitions and
 *                 prototypes.
 *
 * Copyright (c) 2015-2016, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *************************************************************************************************/


#ifndef _SERVICEBOOTLOADER_H_
#define _SERVICEBOOTLOADER_H_

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#include <_hal_types.h>
#include <bcomdef.h>
#include "bootLoader.h"


/*********************************************************************
 * CONSTANTS
 */

/*********************************************************************
* CONSTANTS
*/
// Service UUID
#define SERVICEBOOTLOADER_SERV_UUID 0xb4,0x16,0x60,0xc3,0xed,0x87 ,0x7c,0x95 ,0xac,0x41 ,0xfc,0x7a, 0x23,0x0c,0x76,0x87// 0000ffe0-0000-1000-8000-00805f9b34fb


//  Characteristic defines
#define SERVICEBOOTLOADER_FIRMWARE      0
#define SERVICEBOOTLOADER_FIRMWARE_UUID 0x97,0x89,0xC3,0x68,0xB6,0x17,0x98,0x81,0xC7,0x43,0xB8,0x1C,0x22,0x29,0xB4,0xE0

#define SERVICEBOOTLOADER_FIRMWARE_LEN  BOOT_LOADER_PAKET_SIZE+BOOT_LOADER_HEADER_SIZE

//  Characteristic defines
#define SERVICEBOOTLOADER_VERSION      1
#define SERVICEBOOTLOADER_VERSION_UUID 0x29,0x93,0xEF,0xB3,0x8A,0x60,0x4B,0xA3,0x5D,0x40,0xE2,0xFA,0x2C,0x6C,0x9A,0x62
#define SERVICEBOOTLOADER_VERSION_LEN  4

//  Characteristic defines
#define SERVICEBOOTLOADER_FLOWCONTROL      2
#define SERVICEBOOTLOADER_FLOWCONTROL_UUID 0x2A,0x93,0xEF,0xB3,0x8A,0x60,0x4B,0xA3,0x5D,0x40,0xE2,0xFA,0x2C,0x6C,0x9A,0x62
#define SERVICEBOOTLOADER_FLOWCONTROL_LEN  4

//  Characteristic defines
#define SERVICEBOOTLOADER_GCFVER      3
#define SERVICEBOOTLOADER_GCFVER_UUID 0x2B,0x93,0xEF,0xB3,0x8A,0x60,0x4B,0xA3,0x5D,0x40,0xE2,0xFA,0x2C,0x6C,0x9A,0x62
#define SERVICEBOOTLOADER_GCFVER_LEN  8

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * Profile Callbacks
 */

// Callback when a characteristic value has changed
typedef void (*serviceBootLoaderChange_t)( uint8 paramID );

typedef struct
{
  serviceBootLoaderChange_t        pfnChangeCb;  // Called when characteristic value changes
} serviceBootLoaderCBs_t;



/*********************************************************************
 * API FUNCTIONS
 */


/*
 * ServiceBootLoader_AddService- Initializes the ServiceBootLoader service by registering
 *          GATT attributes with the GATT server.
 *
 */
extern bStatus_t ServiceBootLoader_AddService( void );

/*
 * ServiceBootLoader_RegisterAppCBs - Registers the application callback function.
 *                    Only call this function once.
 *
 *    appCallbacks - pointer to application callbacks.
 */
extern bStatus_t ServiceBootLoader_RegisterAppCBs( serviceBootLoaderCBs_t *appCallbacks );

/*
 * ServiceBootLoader_SetParameter - Set a ServiceBootLoader parameter.
 *
 *    param - Profile parameter ID
 *    len - length of data to right
 *    value - pointer to data to write.  This is dependent on
 *          the parameter ID and WILL be cast to the appropriate
 *          data type (example: data type of uint16 will be cast to
 *          uint16 pointer).
 */
extern bStatus_t ServiceBootLoader_SetParameter( uint8 param, uint8 len, void *value );

/*
 * ServiceBootLoader_GetParameter - Get a ServiceBootLoader parameter.
 *
 *    param - Profile parameter ID
 *    value - pointer to data to write.  This is dependent on
 *          the parameter ID and WILL be cast to the appropriate
 *          data type (example: data type of uint16 will be cast to
 *          uint16 pointer).
 */
extern bStatus_t ServiceBootLoader_GetParameter( uint8 param, void *value );

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* _SERVICEBOOTLOADER_H_ */
