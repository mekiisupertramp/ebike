/**********************************************************************************************
 * Filename:       configService.h
 *
 * Description:    This file contains the configService service definitions and
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


#ifndef _CONFIGSERVICE_H_
#define _CONFIGSERVICE_H_

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */

#include <_hal_types.h>
#include <bcomdef.h>
#include "config.h"
/*********************************************************************
 * CONSTANTS
 */

/*********************************************************************
* CONSTANTS
*/
// Service UUID
#define CONFIGSERVICE_SERV_UUID 0x02,0x00,0x12,0xAC,0x42,0x02,0xA0,0x8E,0xEC,0x11,0xAA,0xEC,0x74,0x10,0xBD,0xC3

//  Characteristic defines
#define CONFIGSERVICE_CONFIG      0
#define CONFIGSERVICE_CONFIG_UUID 0x02,0x00,0x12,0xAC,0x42,0x02,0xA0,0x8E,0xEC,0x11,0xAB,0xEC,0x9A,0x95,0xE4,0x19
#define CONFIGSERVICE_CONFIG_LEN  512

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
typedef void (*configServiceChange_t)( uint8 paramID , uint32_t length);

typedef struct
{
  configServiceChange_t        pfnChangeCb;  // Called when characteristic value changes
} configServiceCBs_t;



/*********************************************************************
 * API FUNCTIONS
 */


/*
 * ConfigService_AddService- Initializes the ConfigService service by registering
 *          GATT attributes with the GATT server.
 *
 */
extern bStatus_t ConfigService_AddService( void );

/*
 * ConfigService_RegisterAppCBs - Registers the application callback function.
 *                    Only call this function once.
 *
 *    appCallbacks - pointer to application callbacks.
 */
extern bStatus_t ConfigService_RegisterAppCBs( configServiceCBs_t *appCallbacks );

/*
 * ConfigService_SetParameter - Set a ConfigService parameter.
 *
 *    param - Profile parameter ID
 *    len - length of data to right
 *    value - pointer to data to write.  This is dependent on
 *          the parameter ID and WILL be cast to the appropriate
 *          data type (example: data type of uint16 will be cast to
 *          uint16 pointer).
 */
extern bStatus_t ConfigService_SetParameter( uint8 param, uint8 len, void *value );

/*
 * ConfigService_GetParameter - Get a ConfigService parameter.
 *
 *    param - Profile parameter ID
 *    value - pointer to data to write.  This is dependent on
 *          the parameter ID and WILL be cast to the appropriate
 *          data type (example: data type of uint16 will be cast to
 *          uint16 pointer).
 */
extern bStatus_t ConfigService_GetParameter( uint8 param, void *value );

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* _CONFIGSERVICE_H_ */
