/**********************************************************************************************
 * Filename:       serviceBootLoader.c
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

#include "bcomdef.h"
#include "linkdb.h"
#include "att.h"
#include "gatt.h"
#include "gatt_uuid.h"
#include "gattservapp.h"
#include "gapbondmgr.h"

#include "icall_ble_api.h"

#include "serviceBootLoader.h"

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

// serviceBootLoader Service UUID
CONST uint8_t serviceBootLoaderUUID[ATT_UUID_SIZE] =
{
  SERVICEBOOTLOADER_SERV_UUID
};

// firmware UUID
CONST uint8_t serviceBootLoader_FirmwareUUID[ATT_UUID_SIZE] =
{
  SERVICEBOOTLOADER_FIRMWARE_UUID
};
// version UUID
CONST uint8_t serviceBootLoader_VersionUUID[ATT_UUID_SIZE] =
{
  SERVICEBOOTLOADER_VERSION_UUID
};
// flowControl UUID
CONST uint8_t serviceBootLoader_FlowControlUUID[ATT_UUID_SIZE] =
{
  SERVICEBOOTLOADER_FLOWCONTROL_UUID
};
// GcfVer UUID
CONST uint8_t serviceBootLoader_GcfVerUUID[ATT_UUID_SIZE] =
{
  SERVICEBOOTLOADER_GCFVER_UUID
};

/*********************************************************************
 * LOCAL VARIABLES
 */

static serviceBootLoaderCBs_t *pAppCBs = NULL;

/*********************************************************************
* Profile Attributes - variables
*/

// Service declaration
static CONST gattAttrType_t serviceBootLoaderDecl = { ATT_UUID_SIZE, serviceBootLoaderUUID };

// Characteristic "Firmware" Properties (for declaration)
static uint8_t serviceBootLoader_FirmwareProps = GATT_PROP_WRITE | GATT_PROP_WRITE_NO_RSP;

// Characteristic "Firmware" Value variable
static uint8_t serviceBootLoader_FirmwareVal[SERVICEBOOTLOADER_FIRMWARE_LEN] = {0};
// Characteristic "Version" Properties (for declaration)
static uint8_t serviceBootLoader_VersionProps = GATT_PROP_READ;

// Characteristic "Version" Value variable
static uint8_t serviceBootLoader_VersionVal[SERVICEBOOTLOADER_VERSION_LEN] = {(uint8_t)(VERSION>>24),(uint8_t)(VERSION>>16),(uint8_t)(VERSION>>8),(uint8_t)(VERSION)};
// Characteristic "FlowControl" Properties (for declaration)
static uint8_t serviceBootLoader_FlowControlProps = GATT_PROP_READ | GATT_PROP_NOTIFY;

// Characteristic "FlowControl" Value variable
static uint8_t serviceBootLoader_FlowControlVal[SERVICEBOOTLOADER_FLOWCONTROL_LEN] = {0};

// Characteristic "FlowControl" CCCD
static gattCharCfg_t *serviceBootLoader_FlowControlConfig;
// Characteristic "GcfVer" Properties (for declaration)
static uint8_t serviceBootLoader_GcfVerProps = GATT_PROP_READ;

// Characteristic "GcfVer" Value variable
//const static uint8_t* serviceBootLoader_GcfVerVal = 0x500012E8;
uint8_t serviceBootLoader_GcfVerVal[SERVICEBOOTLOADER_GCFVER_LEN];

/*********************************************************************
* Profile Attributes - Table
*/

static gattAttribute_t serviceBootLoaderAttrTbl[] =
{
  // serviceBootLoader Service Declaration
  {
    { ATT_BT_UUID_SIZE, primaryServiceUUID },
    GATT_PERMIT_READ,
    0,
    (uint8_t *)&serviceBootLoaderDecl
  },
    // Firmware Characteristic Declaration
    {
      { ATT_BT_UUID_SIZE, characterUUID },
      GATT_PERMIT_READ,
      0,
      &serviceBootLoader_FirmwareProps
    },
      // Firmware Characteristic Value
      {
        { ATT_UUID_SIZE, serviceBootLoader_FirmwareUUID },
        GATT_PERMIT_AUTHEN_WRITE,
        0,
        serviceBootLoader_FirmwareVal
      },
    // Version Characteristic Declaration
    {
      { ATT_BT_UUID_SIZE, characterUUID },
      GATT_PERMIT_READ,
      0,
      &serviceBootLoader_VersionProps
    },
      // Version Characteristic Value
      {
        { ATT_UUID_SIZE, serviceBootLoader_VersionUUID },
        GATT_PERMIT_AUTHEN_READ,
        0,
        serviceBootLoader_VersionVal
      },
    // FlowControl Characteristic Declaration
    {
      { ATT_BT_UUID_SIZE, characterUUID },
      GATT_PERMIT_READ,
      0,
      &serviceBootLoader_FlowControlProps
    },
      // FlowControl Characteristic Value
      {
        { ATT_UUID_SIZE, serviceBootLoader_FlowControlUUID },
        GATT_PERMIT_AUTHEN_READ,
        0,
        serviceBootLoader_FlowControlVal
      },
      // FlowControl CCCD
      {
        { ATT_BT_UUID_SIZE, clientCharCfgUUID },
        GATT_PERMIT_AUTHEN_READ | GATT_PERMIT_AUTHEN_WRITE,
        0,
        (uint8 *)&serviceBootLoader_FlowControlConfig
      },
    // GcfVer Characteristic Declaration
    {
      { ATT_BT_UUID_SIZE, characterUUID },
	  GATT_PERMIT_READ,
      0,
      &serviceBootLoader_GcfVerProps
    },
      // GcfVer Characteristic Value
      {
        { ATT_UUID_SIZE, serviceBootLoader_GcfVerUUID },
        GATT_PERMIT_AUTHEN_READ,
        0,
        serviceBootLoader_GcfVerVal
      },
};

/*********************************************************************
 * LOCAL FUNCTIONS
 */
static bStatus_t serviceBootLoader_ReadAttrCB( uint16 connHandle, gattAttribute_t *pAttr,
                                           uint8 *pValue, uint16 *pLen, uint16 offset,
                                           uint16 maxLen, uint8 method );
static bStatus_t serviceBootLoader_WriteAttrCB( uint16 connHandle, gattAttribute_t *pAttr,
                                            uint8 *pValue, uint16 len, uint16 offset,
                                            uint8 method );

/*********************************************************************
 * PROFILE CALLBACKS
 */
// Simple Profile Service Callbacks
CONST gattServiceCBs_t serviceBootLoaderCBs =
{
  serviceBootLoader_ReadAttrCB,  // Read callback function pointer
  serviceBootLoader_WriteAttrCB, // Write callback function pointer
  NULL                       // Authorization callback function pointer
};

/*********************************************************************
* PUBLIC FUNCTIONS
*/

/*
 * ServiceBootLoader_AddService- Initializes the ServiceBootLoader service by registering
 *          GATT attributes with the GATT server.
 *
 */
bStatus_t ServiceBootLoader_AddService( void )
{
  uint8_t status;

  // Allocate Client Characteristic Configuration table
  serviceBootLoader_FlowControlConfig = (gattCharCfg_t *)ICall_malloc( sizeof(gattCharCfg_t) * linkDBNumConns );
  if ( serviceBootLoader_FlowControlConfig == NULL )
  {
    return ( bleMemAllocError );
  }

  // Initialize Client Characteristic Configuration attributes
  GATTServApp_InitCharCfg( 0xFFFF, serviceBootLoader_FlowControlConfig );
  // Register GATT attribute list and CBs with GATT Server App
  status = GATTServApp_RegisterService( serviceBootLoaderAttrTbl,
                                        GATT_NUM_ATTRS( serviceBootLoaderAttrTbl ),
                                        GATT_MAX_ENCRYPT_KEY_SIZE,
                                        &serviceBootLoaderCBs );

  return ( status );
}

/*
 * ServiceBootLoader_RegisterAppCBs - Registers the application callback function.
 *                    Only call this function once.
 *
 *    appCallbacks - pointer to application callbacks.
 */
bStatus_t ServiceBootLoader_RegisterAppCBs( serviceBootLoaderCBs_t *appCallbacks )
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
 * ServiceBootLoader_SetParameter - Set a ServiceBootLoader parameter.
 *
 *    param - Profile parameter ID
 *    len - length of data to right
 *    value - pointer to data to write.  This is dependent on
 *          the parameter ID and WILL be cast to the appropriate
 *          data type (example: data type of uint16 will be cast to
 *          uint16 pointer).
 */
bStatus_t ServiceBootLoader_SetParameter( uint8 param, uint8 len, void *value )
{
  bStatus_t ret = SUCCESS;
  switch ( param )
  {
    case SERVICEBOOTLOADER_VERSION:
      if ( len == SERVICEBOOTLOADER_VERSION_LEN )
      {
        memcpy(serviceBootLoader_VersionVal, value, len);
      }
      else
      {
        ret = bleInvalidRange;
      }
      break;

    case SERVICEBOOTLOADER_FLOWCONTROL:
      if ( len == SERVICEBOOTLOADER_FLOWCONTROL_LEN )
      {
        memcpy(serviceBootLoader_FlowControlVal, value, len);

        // Try to send notification.
        GATTServApp_ProcessCharCfg( serviceBootLoader_FlowControlConfig, (uint8_t *)&serviceBootLoader_FlowControlVal, 0,
                                    serviceBootLoaderAttrTbl, GATT_NUM_ATTRS( serviceBootLoaderAttrTbl ),
                                    INVALID_TASK_ID,  serviceBootLoader_ReadAttrCB);
      }
      else
      {
        ret = bleInvalidRange;
      }
      break;

    case SERVICEBOOTLOADER_GCFVER:
      if ( len == SERVICEBOOTLOADER_GCFVER_LEN )
      {
        memcpy(serviceBootLoader_GcfVerVal, value, len);
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
 * ServiceBootLoader_GetParameter - Get a ServiceBootLoader parameter.
 *
 *    param - Profile parameter ID
 *    value - pointer to data to write.  This is dependent on
 *          the parameter ID and WILL be cast to the appropriate
 *          data type (example: data type of uint16 will be cast to
 *          uint16 pointer).
 */
bStatus_t ServiceBootLoader_GetParameter( uint8 param, void *value )
{
  bStatus_t ret = SUCCESS;
  switch ( param )
  {
    case SERVICEBOOTLOADER_FIRMWARE:
      memcpy(value, serviceBootLoader_FirmwareVal, SERVICEBOOTLOADER_FIRMWARE_LEN);
      break;

    default:
      ret = INVALIDPARAMETER;
      break;
  }
  return ret;
}


/*********************************************************************
 * @fn          serviceBootLoader_ReadAttrCB
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
static bStatus_t serviceBootLoader_ReadAttrCB( uint16 connHandle, gattAttribute_t *pAttr,
                                       uint8 *pValue, uint16 *pLen, uint16 offset,
                                       uint16 maxLen, uint8 method )
{
  bStatus_t status = SUCCESS;

  // See if request is regarding the Version Characteristic Value
if ( ! memcmp(pAttr->type.uuid, serviceBootLoader_VersionUUID, pAttr->type.len) )
  {
    if ( offset > SERVICEBOOTLOADER_VERSION_LEN )  // Prevent malicious ATT ReadBlob offsets.
    {
      status = ATT_ERR_INVALID_OFFSET;
    }
    else
    {
      *pLen = MIN(maxLen, SERVICEBOOTLOADER_VERSION_LEN - offset);  // Transmit as much as possible
      memcpy(pValue, pAttr->pValue + offset, *pLen);
    }
  }
  // See if request is regarding the FlowControl Characteristic Value
else if ( ! memcmp(pAttr->type.uuid, serviceBootLoader_FlowControlUUID, pAttr->type.len) )
  {
    if ( offset > SERVICEBOOTLOADER_FLOWCONTROL_LEN )  // Prevent malicious ATT ReadBlob offsets.
    {
      status = ATT_ERR_INVALID_OFFSET;
    }
    else
    {
      *pLen = MIN(maxLen, SERVICEBOOTLOADER_FLOWCONTROL_LEN - offset);  // Transmit as much as possible
      memcpy(pValue, pAttr->pValue + offset, *pLen);
    }
  }
  // See if request is regarding the GcfVer Characteristic Value
else if ( ! memcmp(pAttr->type.uuid, serviceBootLoader_GcfVerUUID, pAttr->type.len) )
  {
    if ( offset > SERVICEBOOTLOADER_GCFVER_LEN )  // Prevent malicious ATT ReadBlob offsets.
    {
      status = ATT_ERR_INVALID_OFFSET;
    }
    else
    {
      *pLen = MIN(maxLen, SERVICEBOOTLOADER_GCFVER_LEN - offset);  // Transmit as much as possible
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
 * @fn      serviceBootLoader_WriteAttrCB
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
static bStatus_t serviceBootLoader_WriteAttrCB( uint16 connHandle, gattAttribute_t *pAttr,
                                        uint8 *pValue, uint16 len, uint16 offset,
                                        uint8 method )
{
  bStatus_t status  = SUCCESS;
  uint8_t   paramID = 0xFF;

  // See if request is regarding a Client Characterisic Configuration
  if ( ! memcmp(pAttr->type.uuid, clientCharCfgUUID, pAttr->type.len) )
  {
    // Allow only notifications.
    status = GATTServApp_ProcessCCCWriteReq( connHandle, pAttr, pValue, len,
                                             offset, GATT_CLIENT_CFG_NOTIFY);
  }
  // See if request is regarding the Firmware Characteristic Value
  else if ( ! memcmp(pAttr->type.uuid, serviceBootLoader_FirmwareUUID, pAttr->type.len) )
  {
    if ( offset + len > SERVICEBOOTLOADER_FIRMWARE_LEN )
    {
      status = ATT_ERR_INVALID_OFFSET;
    }
    else
    {
      // Copy pValue into the variable we point to from the attribute table.
      memcpy(pAttr->pValue + offset, pValue, len);

      // Only notify application if entire expected value is written
      if ( offset + len == SERVICEBOOTLOADER_FIRMWARE_LEN)
        paramID = SERVICEBOOTLOADER_FIRMWARE;
    }
  }
  else
  {
    // If we get here, that means you've forgotten to add an if clause for a
    // characteristic value attribute in the attribute table that has WRITE permissions.
    status = ATT_ERR_ATTR_NOT_FOUND;
  }

  // Let the application know something changed (if it did) by using the
  // callback it registered earlier (if it did).
  if (paramID != 0xFF)
    if ( pAppCBs && pAppCBs->pfnChangeCb )
      pAppCBs->pfnChangeCb( paramID ); // Call app function from stack task context.

  return status;
}
