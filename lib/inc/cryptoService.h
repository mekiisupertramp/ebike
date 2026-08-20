/**********************************************************************************************
 * Filename:       cryptoService.h
 *
 * Description:    This file contains the cryptoService service definitions and
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


#ifndef _CRYPTOSERVICE_H_
#define _CRYPTOSERVICE_H_

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#include <_hal_types.h>
#include <bcomdef.h>

/*********************************************************************
 * CONSTANTS
 */

/*********************************************************************
* CONSTANTS
*/
// Service UUID
#define CRYPTOSERVICE_SERV_UUID 0x32,0x72,0x82,0xA2,0x5C,0x3C,0x9E,0x86,0xFA,0x49,0xE6,0xF2,0x33,0xC5,0xFE,0x1C

//  Characteristic defines
#define CRYPTOSERVICE_KEYNUMKEY      0
#define CRYPTOSERVICE_KEYNUMKEY_UUID 0x33,0x72,0x82,0xA2,0x5C,0x3C,0x9E,0x86,0xFA,0x49,0xE6,0xF2,0x33,0xC5,0xFE,0x1C
#define CRYPTOSERVICE_KEYNUMKEY_LEN  33

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * Profile Callbacks
 */

extern uint8_t cryptoService_KeyNumKeyVal[CRYPTOSERVICE_KEYNUMKEY_LEN];

// Callback when a characteristic value has changed
typedef void (*cryptoServiceChange_t)( uint8 paramID );

typedef struct
{
  cryptoServiceChange_t        pfnChangeCb;  // Called when characteristic value changes
} cryptoServiceCBs_t;



/*********************************************************************
 * API FUNCTIONS
 */


/*
 * CryptoService_AddService- Initializes the CryptoService service by registering
 *          GATT attributes with the GATT server.
 *
 */
extern bStatus_t CryptoService_AddService( void );

/*
 * CryptoService_RegisterAppCBs - Registers the application callback function.
 *                    Only call this function once.
 *
 *    appCallbacks - pointer to application callbacks.
 */
extern bStatus_t CryptoService_RegisterAppCBs( cryptoServiceCBs_t *appCallbacks );

/*
 * CryptoService_SetParameter - Set a CryptoService parameter.
 *
 *    param - Profile parameter ID
 *    len - length of data to right
 *    value - pointer to data to write.  This is dependent on
 *          the parameter ID and WILL be cast to the appropriate
 *          data type (example: data type of uint16 will be cast to
 *          uint16 pointer).
 */
extern bStatus_t CryptoService_SetParameter( uint8 param, uint8 len, void *value );

/*
 * CryptoService_GetParameter - Get a CryptoService parameter.
 *
 *    param - Profile parameter ID
 *    value - pointer to data to write.  This is dependent on
 *          the parameter ID and WILL be cast to the appropriate
 *          data type (example: data type of uint16 will be cast to
 *          uint16 pointer).
 */
extern bStatus_t CryptoService_GetParameter( uint8 param, void *value );

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* _CRYPTOSERVICE_H_ */
