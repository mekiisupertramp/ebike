/**********************************************************************************************
 * Filename:       virtualUartService.c
 *
 * Description:    This file contains the implementation of the service.
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


/*********************************************************************
 * INCLUDES
 */
#include <string.h>
#include <virtualUartService.h>
#include "bcomdef.h"
#include "linkdb.h"
#include "att.h"
#include "gatt.h"
#include "gatt_uuid.h"
#include "gattservapp.h"
#include "gapbondmgr.h"
#include "config.h"

#include "util.h"
#include "icall_ble_api.h"


/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
* GLOBAL VARIABLES
*/

// virtualUartService Service UUID
CONST uint8_t virtualUartServiceUUID[ATT_UUID_SIZE] =
{
  VIRTUALUARTSERVICE_SERV_UUID
};

// TX UUID
CONST uint8_t virtualUartService_TXUUID[ATT_UUID_SIZE] =
{
  VIRTUALUARTSERVICE_TX_UUID
};
static int virtualUartService_RX_length=0;
// RX UUID
CONST uint8_t virtualUartService_RXUUID[ATT_UUID_SIZE] =
{
  VIRTUALUARTSERVICE_RX_UUID
};

/*********************************************************************
 * LOCAL VARIABLES
 */

static virtualUartServiceCBs_t *pAppCBs = NULL;

/*********************************************************************
* Profile Attributes - variables
*/

// Service declaration
static CONST gattAttrType_t virtualUartServiceDecl = { ATT_UUID_SIZE, virtualUartServiceUUID };

// Characteristic "TX" Properties (for declaration)
static uint8_t virtualUartService_TXProps = GATT_PROP_WRITE | GATT_PROP_WRITE_NO_RSP;

// Characteristic "TX" Value variable
static uint8_t virtualUartService_TXVal[VIRTUALUARTSERVICE_TX_LEN] = {0};
// Characteristic "RX" Properties (for declaration)
static uint8_t virtualUartService_RXProps = GATT_PROP_READ | GATT_PROP_NOTIFY;

// Characteristic "RX" Value variable
static uint8_t virtualUartService_RXVal[VIRTUALUARTSERVICE_RX_LEN] = {0};

// Characteristic "RX" CCCD
static gattCharCfg_t *virtualUartService_RXConfig;

/*********************************************************************
* Profile Attributes - Table
*/

static gattAttribute_t virtualUartServiceAttrTbl[] =
{
  // virtualUartService Service Declaration
  {
    { ATT_BT_UUID_SIZE, primaryServiceUUID },
    GATT_PERMIT_READ,
    0,
    (uint8_t *)&virtualUartServiceDecl
  },
    // TX Characteristic Declaration
    {
      { ATT_BT_UUID_SIZE, characterUUID },
      GATT_PERMIT_READ,
      0,
      &virtualUartService_TXProps
    },
      // TX Characteristic Value
      {
        { ATT_UUID_SIZE, virtualUartService_TXUUID },
        GATT_PERMIT_AUTHEN_WRITE,
        0,
        virtualUartService_TXVal
      },
    // RX Characteristic Declaration
    {
      { ATT_BT_UUID_SIZE, characterUUID },
      GATT_PERMIT_READ,
      0,
      &virtualUartService_RXProps
    },
      // RX Characteristic Value
      {
        { ATT_UUID_SIZE, virtualUartService_RXUUID },
        GATT_PERMIT_AUTHEN_READ,
        0,
        virtualUartService_RXVal
      },
      // RX CCCD
      {
        { ATT_BT_UUID_SIZE, clientCharCfgUUID },
        GATT_PERMIT_AUTHEN_READ | GATT_PERMIT_AUTHEN_WRITE,
        0,
        (uint8 *)&virtualUartService_RXConfig
      },
};

/*********************************************************************
 * LOCAL FUNCTIONS
 */
static bStatus_t virtualUartService_ReadAttrCB( uint16 connHandle, gattAttribute_t *pAttr,
                                           uint8 *pValue, uint16 *pLen, uint16 offset,
                                           uint16 maxLen, uint8 method );
static bStatus_t virtualUartService_WriteAttrCB( uint16 connHandle, gattAttribute_t *pAttr,
                                            uint8 *pValue, uint16 len, uint16 offset,
                                            uint8 method );

/*********************************************************************
 * PROFILE CALLBACKS
 */
// Simple Profile Service Callbacks
CONST gattServiceCBs_t virtualUartServiceCBs =
{
  virtualUartService_ReadAttrCB,  // Read callback function pointer
  virtualUartService_WriteAttrCB, // Write callback function pointer
  NULL                       // Authorization callback function pointer
};

/*********************************************************************
* PUBLIC FUNCTIONS
*/

/*
 * VirtualUartService_AddService- Initializes the VirtualUartService service by registering
 *          GATT attributes with the GATT server.
 *
 */
bStatus_t VirtualUartService_AddService( void )
{
  uint8_t status;

  // Allocate Client Characteristic Configuration table
  virtualUartService_RXConfig = (gattCharCfg_t *)ICall_malloc( sizeof(gattCharCfg_t) * linkDBNumConns );
  if ( virtualUartService_RXConfig == NULL )
  {
    return ( bleMemAllocError );
  }

  // Initialize Client Characteristic Configuration attributes
  GATTServApp_InitCharCfg( 0xFFFF, virtualUartService_RXConfig );
  // Register GATT attribute list and CBs with GATT Server App
  status = GATTServApp_RegisterService( virtualUartServiceAttrTbl,
                                        GATT_NUM_ATTRS( virtualUartServiceAttrTbl ),
                                        GATT_MAX_ENCRYPT_KEY_SIZE,
                                        &virtualUartServiceCBs );

  return ( status );
}

/*
 * VirtualUartService_RegisterAppCBs - Registers the application callback function.
 *                    Only call this function once.
 *
 *    appCallbacks - pointer to application callbacks.
 */
bStatus_t VirtualUartService_RegisterAppCBs( virtualUartServiceCBs_t *appCallbacks )
{
  if ( appCallbacks )
  {
    pAppCBs = appCallbacks;

    return ( SUCCESS );
  }
  else
  {
    return ( bleAlreadyInRequestedMode );
  }
}

/*
 * VirtualUartService_SetParameter - Set a VirtualUartService parameter.
 *
 *    param - Profile parameter ID
 *    len - length of data to right
 *    value - pointer to data to write.  This is dependent on
 *          the parameter ID and WILL be cast to the appropriate
 *          data type (example: data type of uint16 will be cast to
 *          uint16 pointer).
 */
bStatus_t VirtualUartService_SetParameter( uint8 param, uint16 len, void *value )
{
  bStatus_t ret = SUCCESS;
  switch ( param )
  {
    case VIRTUALUARTSERVICE_RX:
      if ( len <= VIRTUALUARTSERVICE_RX_LEN )
      {
        memcpy(virtualUartService_RXVal, value, len);
        virtualUartService_RX_length=len;

        // Try to send notification.
        GATTServApp_ProcessCharCfg( virtualUartService_RXConfig, (uint8_t *)&virtualUartService_RXVal, FALSE,
                                    virtualUartServiceAttrTbl, GATT_NUM_ATTRS( virtualUartServiceAttrTbl ),
                                    INVALID_TASK_ID,  virtualUartService_ReadAttrCB);
      }
      else
      {
        ret = bleInvalidRange;
      }
      break;

    default:
      ret = INVALIDPARAMETER;
      break;
  }
  return ret;
}


/*
 * VirtualUartService_GetParameter - Get a VirtualUartService parameter.
 *
 *    param - Profile parameter ID
 *    value - pointer to data to write.  This is dependent on
 *          the parameter ID and WILL be cast to the appropriate
 *          data type (example: data type of uint16 will be cast to
 *          uint16 pointer).
 */
bStatus_t VirtualUartService_GetParameter( uint8 param, void *value )
{
  bStatus_t ret = SUCCESS;
  switch ( param )
  {
    case VIRTUALUARTSERVICE_TX:
      memcpy(value, virtualUartService_TXVal, VIRTUALUARTSERVICE_TX_LEN);
      break;

    default:
      ret = INVALIDPARAMETER;
      break;
  }
  return ret;
}


/*********************************************************************
 * @fn          virtualUartService_ReadAttrCB
 *
 * @brief       Read an attribute.
 *
 * @param       connHandle - connection message was received on
 * @param       pAttr - pointer to attribute
 * @param       pValue - pointer to data to be read
 * @param       pLen - length of data to be read
 * @param       offset - offset of the first octet to be read
 * @param       maxLen - maximum length of data to be read
 * @param       method - type of read message
 *
 * @return      SUCCESS, blePending or Failure
 */
static bStatus_t virtualUartService_ReadAttrCB( uint16 connHandle, gattAttribute_t *pAttr,
                                       uint8 *pValue, uint16 *pLen, uint16 offset,
                                       uint16 maxLen, uint8 method )
{
  bStatus_t status = SUCCESS;

  // See if request is regarding the RX Characteristic Value
if ( ! memcmp(pAttr->type.uuid, virtualUartService_RXUUID, pAttr->type.len) )
  {
    if ( offset > VIRTUALUARTSERVICE_RX_LEN )  // Prevent malicious ATT ReadBlob offsets.
    {
      status = ATT_ERR_INVALID_OFFSET;
    }
    else
    {
      *pLen = MIN(maxLen, virtualUartService_RX_length - offset);  // Transmit as much as possible //ici
      memcpy(pValue, pAttr->pValue + offset, *pLen);
    }
  }
  else
  {
    // If we get here, that means you've forgotten to add an if clause for a
    // characteristic value attribute in the attribute table that has READ permissions.
    *pLen = 0;
    status = ATT_ERR_ATTR_NOT_FOUND;
  }

  return status;
}


/*********************************************************************
 * @fn      virtualUartService_WriteAttrCB
 *
 * @brief   Validate attribute data prior to a write operation
 *
 * @param   connHandle - connection message was received on
 * @param   pAttr - pointer to attribute
 * @param   pValue - pointer to data to be written
 * @param   len - length of data
 * @param   offset - offset of the first octet to be written
 * @param   method - type of write message
 *
 * @return  SUCCESS, blePending or Failure
 */
static bStatus_t virtualUartService_WriteAttrCB( uint16 connHandle, gattAttribute_t *pAttr,
                                        uint8 *pValue, uint16 len, uint16 offset,
                                        uint8 method )
{
  bStatus_t status  = SUCCESS;
  //uint8_t   paramID = 0xFF;

  // See if request is regarding a Client Characterisic Configuration
  if ( ! memcmp(pAttr->type.uuid, clientCharCfgUUID, pAttr->type.len) )
  {
    // Allow only notifications.
    status = GATTServApp_ProcessCCCWriteReq( connHandle, pAttr, pValue, len,
                                             offset, GATT_CLIENT_CFG_NOTIFY);
  }
  // See if request is regarding the TX Characteristic Value
  else if ( ! memcmp(pAttr->type.uuid, virtualUartService_TXUUID, pAttr->type.len) )
  {
    /*if ( offset + len > VIRTUALUARTSERVICE_TX_LEN )
    {
      status = ATT_ERR_INVALID_OFFSET;
    }
    else
    {
      // Copy pValue into the variable we point to from the attribute table.
      memcpy(pAttr->pValue + offset, pValue, len);

      // Only notify application if entire expected value is written
      //if ( offset + len == VIRTUALUARTSERVICE_TX_LEN)
      paramID = VIRTUALUARTSERVICE_TX;
    }*/
      pAppCBs->pfnChangeCb( pValue,len );
  }
  else
  {
    // If we get here, that means you've forgotten to add an if clause for a
    // characteristic value attribute in the attribute table that has WRITE permissions.
    status = ATT_ERR_ATTR_NOT_FOUND;
  }

  // Let the application know something changed (if it did) by using the
  // callback it registered earlier (if it did).
  /*if (paramID != 0xFF)
    if ( pAppCBs && pAppCBs->pfnChangeCb )
      pAppCBs->pfnChangeCb( paramID ); // Call app function from stack task context.*/

  return status;
}
