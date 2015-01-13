/**************************************************************************************************
  Filename:       glucservice.c
  Revised:        $Date $
  Revision:       $Revision $

  Description:    This file contains the Glucose sample service
                  for use with the Glucose   sample application.

 Copyright 2011-2013 Texas Instruments Incorporated. All rights reserved.

  IMPORTANT: Your use of this Software is limited to those specific rights
  granted under the terms of a software license agreement between the user
  who downloaded the software, his/her employer (which must be your employer)
  and Texas Instruments Incorporated (the "License").  You may not use this
  Software unless you agree to abide by the terms of the License. The License
  limits your use, and you acknowledge, that the Software may not be modified,
  copied or distributed unless embedded on a Texas Instruments microcontroller
  or used solely and exclusively in conjunction with a Texas Instruments radio
  frequency transceiver, which is integrated into your product.  Other than for
  the foregoing purpose, you may not use, reproduce, copy, prepare derivative
  works of, modify, distribute, perform, display or sell this Software and/or
  its documentation for any purpose.

  YOU FURTHER ACKNOWLEDGE AND AGREE THAT THE SOFTWARE AND DOCUMENTATION ARE
  PROVIDED �AS IS� WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED,
  INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY, TITLE,
  NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL
  TEXAS INSTRUMENTS OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER CONTRACT,
  NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR OTHER
  LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
  INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE
  OR CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT
  OF SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
  (INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.

  Should you have any questions regarding your right to use this Software,
  contact Texas Instruments Incorporated at www.TI.com.
**************************************************************************************************/

/*********************************************************************
 * INCLUDES
 */
#include "bcomdef.h"
#include "OSAL.h"
#include "linkdb.h"
#include "att.h"
#include "gatt.h"
#include "gatt_uuid.h"
#include "gatt_profile_uuid.h"
#include "gattservapp.h"
#include "gapbondmgr.h"
#include "cgmservice.h"
#include "CGM_Service_values.h"


/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */

// Position of glucose measurement value in attribute array
#define GLUCOSE_MEAS_VALUE_POS         2
#define GLUCOSE_MEAS_CONFIG_POS        3
#define GLUCOSE_CONTEXT_VALUE_POS      5
#define GLUCOSE_CONTEXT_CONFIG_POS     6
#define GLUCOSE_CTL_PNT_VALUE_POS      10
#define GLUCOSE_CTL_PNT_CONFIG_POS     11



//NEW
#define CGM_MEAS_VALUE_POS                     2
#define CGM_MEAS_CONFIG_POS                    3
#define CGM_FEATURE_VALUE_POS                  5
#define CGM_STATUS_VALUE_POS                   7
#define CGM_SESSION_START_TIME_VALUE_POS       9
#define CGM_SESSION_RUN_TIME_VALUE_POS         11
#define CGM_RACP_VALUE_POS                     13
#define CGM_RACP_CONFIG_POS                    14   
#define CGM_CGM_OPCP_VALUE_POS                 16   
#define CGM_CGM_OPCP_CONFIG_POS                17

/*********************************************************************
 * TYPEDEFS
 */
   

/*********************************************************************
 * GLOBAL VARIABLES
 */

//NEW
// CGM service
CONST uint8 CGMServiceUUID[ATT_BT_UUID_SIZE] =
{
  LO_UINT16(CGM_SERV_UUID), HI_UINT16(CGM_SERV_UUID)
};

// CGM Measurement characteristic
CONST uint8 CGMMeasUUID[ATT_BT_UUID_SIZE] =
{
  LO_UINT16(CGM_MEAS_UUID), HI_UINT16(CGM_MEAS_UUID)
};

// CGM Feature
CONST uint8 CGMFeatureUUID[ATT_BT_UUID_SIZE] =
{
  LO_UINT16(CGM_FEATURE_UUID), HI_UINT16(CGM_FEATURE_UUID)
};

// CGM Status
CONST uint8 CGMStatusUUID[ATT_BT_UUID_SIZE] =
{
  LO_UINT16(CGM_STATUS_UUID), HI_UINT16(CGM_STATUS_UUID)
};

// CGM Session Start
CONST uint8 CGMSessionStartTimeUUID[ATT_BT_UUID_SIZE] =
{
  LO_UINT16(CGM_SES_START_TIME_UUID), HI_UINT16(CGM_SES_START_TIME_UUID)
};

// CGM Session Run
CONST uint8 CGMSessionRunTimeUUID[ATT_BT_UUID_SIZE] =
{
  LO_UINT16(CGM_SES_RUN_TIME_UUID), HI_UINT16(CGM_SES_RUN_TIME_UUID)
};

// Record Control Point
CONST uint8 recordControlPointUUID[ATT_BT_UUID_SIZE] =
{
  LO_UINT16(REC_ACCESS_CTRL_PT_UUID), HI_UINT16(REC_ACCESS_CTRL_PT_UUID)
};


// CGM Specific Ops Control Point
CONST uint8 CGMSpecificOpsControlPointUUID[ATT_BT_UUID_SIZE] =
{
  LO_UINT16(CGM_SPEC_OPS_CTRL_PT_UUID), HI_UINT16(CGM_SPEC_OPS_CTRL_PT_UUID)
};



/*********************************************************************
 * EXTERNAL VARIABLES
 */

/* TRUE if record transfer in progress */
//extern bool glucoseSendAllRecords;

/*********************************************************************
 * EXTERNAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */


static CGMServiceCB_t CGMServiceCB;
 
/*********************************************************************
 * Profile Attributes - variables
 */

// CGM Service attribute
static CONST gattAttrType_t CGMService = { ATT_BT_UUID_SIZE, CGMServiceUUID };

// CGM meaaurement Characteristic
static uint8 CGMMeasProps = GATT_PROP_NOTIFY;
static gattCharCfg_t CGMMeasConfig[GATT_MAX_NUM_CONN];
static uint8 CGMMeassurementDummy=0;

// CGM feature
static uint8  CGMFeatureProps = GATT_PROP_READ;
static uint8  CGMFeatureDummy = 0;

// CGM Status
static uint8  CGMStatusProps = GATT_PROP_READ;
static uint8  CGMStatusDummy=0;

// CGM Session Start Time
static uint8  CGMSessionStartTimeProps = GATT_PROP_WRITE|GATT_PROP_READ;
static uint8  CGMSessionStartTimeDummy=0;

// CGM Session Run Time
static uint8 CGMSessionRunTimeProps = GATT_PROP_READ;
static uint16  CGMSessionRunTimeDummy=2; 

// Record Access Control Point RACP
static uint8 CGMRacpProps = GATT_PROP_WRITE|GATT_PROP_INDICATE;
static gattCharCfg_t    CGMRacpConfig[GATT_MAX_NUM_CONN];
static uint8 CGMRacpDummy=0;

// CGM Specific Control Point
static uint8 CGMControlProps = GATT_PROP_WRITE|GATT_PROP_INDICATE;
static gattCharCfg_t    CGMControlConfig[GATT_MAX_NUM_CONN];
static uint8 CGMControlDummy=0;

// Glucose Characteristic
// static uint8 glucoseProps = GATT_PROP_NOTIFY;
// static gattCharCfg_t glucoseMeasConfig[GATT_MAX_NUM_CONN];
// static uint8 glucoseMeas = 0;

// Measurement Context
//static uint8  glucoseContextProps = GATT_PROP_NOTIFY;
//static uint8  glucoseContext=0;
//static gattCharCfg_t glucoseContextConfig[GATT_MAX_NUM_CONN];

// Glucose Feature
//static uint8 glucoseFeatureProps = GATT_PROP_READ;
//static uint16 glucoseFeature = GLUCOSE_FEAT_ALL;

// Glucose Control
//static uint8  glucoseControlProps = GATT_PROP_INDICATE | GATT_PROP_WRITE;
//static uint8  glucoseControl=0;
//static gattCharCfg_t glucoseControlConfig[GATT_MAX_NUM_CONN];

/*********************************************************************
 * Profile Attributes - Table
 */
//NEW
static gattAttribute_t CGMAttrTbl[] =
{
  // CGM Service
  {
    { ATT_BT_UUID_SIZE, primaryServiceUUID }, /* type */
    GATT_PERMIT_READ,                         /* permissions */
    0,                                        /* handle */
    (uint8 *)&CGMService                      /* pValue */
  },
    //////////////////////////////////////////////
    // CGM MEASUREMENT Charaacteristic
    //////////////////////////////////////////////
    
    // 1. Characteristic Declaration
    {
      { ATT_BT_UUID_SIZE, characterUUID },
      GATT_PERMIT_READ,
      0,
      &CGMMeasProps
    },

    // 2. Characteristic Value
    {
      { ATT_BT_UUID_SIZE, CGMMeasUUID },
      0, //return READ_NOT_PERMITTED
      0,
      &CGMMeassurementDummy
    },

    // 3.Characteristic Configuration
    {
      { ATT_BT_UUID_SIZE, clientCharCfgUUID },
      GATT_PERMIT_READ | GATT_PERMIT_WRITE,
      0,
      (uint8 *)&CGMMeasConfig
    },

    //////////////////////////////////////////////
    // CGM Feature
    //////////////////////////////////////////////

    // 4.Characteristic Declaration
    {
      { ATT_BT_UUID_SIZE, characterUUID },
      GATT_PERMIT_READ,
      0,
      &CGMFeatureProps
    },

    // 5.Characteristic Value
    {
      { ATT_BT_UUID_SIZE, CGMFeatureUUID },
      GATT_PERMIT_READ,
      0,
      &CGMFeatureDummy
    },

    //////////////////////////////////////////////
    // CGM STATUS
    //////////////////////////////////////////////

    // 6.Characteristic Declaration
    {
      { ATT_BT_UUID_SIZE, characterUUID },
      GATT_PERMIT_READ,
      0,
      &CGMStatusProps
    },

    // 7.Characteristic Value
    {
      { ATT_BT_UUID_SIZE, CGMStatusUUID },
      GATT_PERMIT_READ,
      0,
      &CGMStatusDummy
    },

    //////////////////////////////////////////////
    // CGM Session Start Time
    //////////////////////////////////////////////

    // 8.Characteristic Declaration
    {
      { ATT_BT_UUID_SIZE, characterUUID },
      GATT_PERMIT_READ |GATT_PERMIT_WRITE,
      0,
      (uint8*)&CGMSessionStartTimeProps
    },

    // 9.Characteristic Value
    {
      { ATT_BT_UUID_SIZE, CGMSessionStartTimeUUID },
      GATT_PERMIT_READ |GATT_PERMIT_WRITE,
      0,
      &CGMSessionStartTimeDummy
    },

    //////////////////////////////////////////////
    // CGM Session Run Time
    //////////////////////////////////////////////

    // 10.Characteristic Declaration
    {
      { ATT_BT_UUID_SIZE, characterUUID },
      GATT_PERMIT_READ |GATT_PERMIT_WRITE,
      0,
      &CGMSessionRunTimeProps
    },

    // 11.Characteristic Value
    {
      { ATT_BT_UUID_SIZE, CGMSessionRunTimeUUID },
      GATT_PERMIT_READ |GATT_PERMIT_WRITE,
      0,
      (uint8 *)&CGMSessionRunTimeDummy
    },

    //////////////////////////////////////////////
    // RACP Charateristic
    //////////////////////////////////////////////

    // 12.Characteristic Declaration
    {
      { ATT_BT_UUID_SIZE, characterUUID },
      GATT_PERMIT_READ |GATT_PERMIT_WRITE,
      0,
      &CGMRacpProps
    },

    // 13.Characteristic Value
    {
      { ATT_BT_UUID_SIZE, recordControlPointUUID },
      GATT_PERMIT_READ |GATT_PERMIT_WRITE,
      0,
      &CGMRacpDummy
    },
    
      // 14.Characteristic Configuration
    {
      { ATT_BT_UUID_SIZE, clientCharCfgUUID },
      GATT_PERMIT_READ | GATT_PERMIT_WRITE,
      0,
      (uint8 *)&CGMRacpConfig
    },

        //////////////////////////////////////////////
    // CGM Specific Operation Control Point Charateristic
    //////////////////////////////////////////////

    // 15.Characteristic Declaration
    {
      { ATT_BT_UUID_SIZE, characterUUID },
      GATT_PERMIT_READ |GATT_PERMIT_WRITE,
      0,
      &CGMControlProps
    },

    // 16.Characteristic Value
    {
      { ATT_BT_UUID_SIZE, CGMSpecificOpsControlPointUUID },
      GATT_PERMIT_READ |GATT_PERMIT_WRITE,
      0,
      &CGMControlDummy
    },
    
      // 17.Characteristic Configuration
    {
      { ATT_BT_UUID_SIZE, clientCharCfgUUID },
      GATT_PERMIT_READ | GATT_PERMIT_WRITE,
      0,
      (uint8 *)&CGMControlConfig
    }

};

/*********************************************************************
 * LOCAL FUNCTIONS
 */
static uint8 CGM_ReadAttrCB( uint16 connHandle, gattAttribute_t *pAttr,
                            uint8 *pValue, uint8 *pLen, uint16 offset, uint8 maxLen );
static bStatus_t CGM_WriteAttrCB( uint16 connHandle, gattAttribute_t *pAttr,
                                 uint8 *pValue, uint8 len, uint16 offset );

static void CGM_HandleConnStatusCB( uint16 connHandle, uint8 changeType );

/*********************************************************************
 * PROFILE CALLBACKS
 */
// Service Callbacks
CONST gattServiceCBs_t  CGMCBs =
{
  CGM_ReadAttrCB,   // Read callback function pointer
  CGM_WriteAttrCB,  // Write callback function pointer
  NULL                  // Authorization callback function pointer
};


/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/*********************************************************************
 * @fn      CGM_AddService
 *
 * @brief   Initializes the CGM service by registering
 *          GATT attributes with the GATT server.
 *
 * @param   services - services to add. This is a bit map and can
 *                     contain more than one service.
 *
 * @return  Success or Failure
 */
bStatus_t CGM_AddService( uint32 services )
{
  uint8 status = SUCCESS;

  // Initialize Client Characteristic Configuration attributes
  GATTServApp_InitCharCfg( INVALID_CONNHANDLE, CGMMeasConfig );
  GATTServApp_InitCharCfg( INVALID_CONNHANDLE, CGMRacpConfig );
  GATTServApp_InitCharCfg( INVALID_CONNHANDLE, CGMControlConfig );

  // Register with Link DB to receive link status change callback
  VOID linkDB_Register( CGM_HandleConnStatusCB );

  if ( services & CGM_SERVICE )
  {
    // Register GATT attribute list and CBs with GATT Server App
    status = GATTServApp_RegisterService( CGMAttrTbl, GATT_NUM_ATTRS( CGMAttrTbl ),
                                          &CGMCBs );
  }
  return ( status );
}

/*********************************************************************
 * @fn      CGM_Register
 *
 * @brief   Register a callback function with the CGM Service.
 *
 * @param   pfnServiceCB - Callback function.
 *          pfnCtlPntCB - Callback for control point
 *
 * @return  None.
 */
extern void CGM_Register( CGMServiceCB_t pfnServiceCB)
{
  CGMServiceCB = pfnServiceCB;
}

/*********************************************************************
 * @fn      CGM_SetParameter
 *
 * @brief   Set a CGM parameter.
 *
 * @param   param - Profile parameter ID
 * @param   len - length of data to right
 * @param   value - pointer to data to write.  This is dependent on
 *          the parameter ID and WILL be cast to the appropriate
 *          data type (example: data type of uint16 will be cast to
 *          uint16 pointer).
 *
 * @return  bStatus_t
 */
bStatus_t CGM_SetParameter( uint8 param, uint8 len, void *value )
{
  bStatus_t ret = SUCCESS;
  switch ( param )
  {
    case CGM_FEATURE_PARAM:
      osal_memcpy(&CGMFeatureDummy,value,len);
      break;
    
    case CGM_STATUS_PARAM:
      osal_memcpy(&CGMStatusDummy,value,len);
      break;
    
    case CGM_SESSION_START_TIME_PARAM:
      osal_memcpy(&CGMSessionStartTimeDummy,value,len);
      break;
      
    
    default:
      ret = INVALIDPARAMETER;
      break;
  }

  return ( ret );
}

/*********************************************************************
 * @fn      CGM_GetParameter
 *
 * @brief   Get a CGM parameter.
 *
 * @param   param - Profile parameter ID
 * @param   value - pointer to data to get.  This is dependent on
 *          the parameter ID and WILL be cast to the appropriate
 *          data type (example: data type of uint16 will be cast to
 *          uint16 pointer).
 *
 * @return  bStatus_t
 */
bStatus_t CGM_GetParameter( uint8 param, void *value )
{
  bStatus_t ret = SUCCESS;
  switch ( param )
  {
    /*case CGM_FEATURE_PARAM:
      osal_memcpy(value,&CGMFeatureDummy,sizeof(cgmFeature_t));
      break;

    case CGM_STATUS_PARAM:
      osal_memcpy(value,&CGMStatusDummy,sizeof(cgmStatus_t));
      break;
      
    case CGM_SESSION_START_TIME_PARAM:
      osal_memcpy(value,&CGMSessionStartTimeDummy,sizeof(cgmSessionStartTime_t));
      break;
    */
    default:
      ret = INVALIDPARAMETER;
      break;
  }

  return ( ret );
}

/*********************************************************************
 * @fn          CGM_MeasSend
 *
 * @brief       Send a CGM measurement.
 *
 * @param       connHandle - connection handle
 * @param       pNoti - pointer to notification structure
 *
 * @return      Success or Failure
 */

//NEW
bStatus_t CGM_MeasSend( uint16 connHandle, attHandleValueNoti_t *pNoti, uint8 taskId )
{
  uint16 value = GATTServApp_ReadCharCfg( connHandle, CGMMeasConfig );

  // If notifications enabled
  if ( value & GATT_CLIENT_CFG_NOTIFY )
  {
    // Set the handle
    pNoti->handle = CGMAttrTbl[CGM_MEAS_VALUE_POS].handle;

    // Send the Indication
    return GATT_Notification( connHandle, pNoti, FALSE );
  }

  return bleNotReady;
}


/*********************************************************************
 * @fn          Glucose_ContextSend
 *
 * @brief       Send a glucose measurement context.
 *
 * @param       connHandle - connection handle
 * @param       pNoti - pointer to notification structure
 *
 * @return      Success or Failure
 */
/*bStatus_t Glucose_ContextSend( uint16 connHandle, attHandleValueNoti_t *pNoti, uint8 taskId )
{
  uint16 value = GATTServApp_ReadCharCfg( connHandle, glucoseContextConfig );

  // If notifications enabled
  if ( value & GATT_CLIENT_CFG_NOTIFY )
  {
    // Set the handle
    pNoti->handle = glucoseAttrTbl[GLUCOSE_CONTEXT_VALUE_POS].handle;

    // Send the Indication
    return GATT_Notification( connHandle, pNoti, FALSE );
  }

  return bleNotReady;
}*/

/*********************************************************************
 * @fn          Glucose_CtlPntIndicate
 *
 * @brief       Send an indication containing a control point
 *              message.
 *
 * @param       connHandle - connection handle
 * @param       pInd - pointer to indication structure
 *
 * @return      Success or Failure
 */
//NEW
bStatus_t Glucose_CtlPntIndicate( uint16 connHandle, attHandleValueInd_t *pInd, uint8 taskId )
{
  uint16 value = GATTServApp_ReadCharCfg( connHandle, CGMControlConfig );

  // If indications enabled
  if ( value & GATT_CLIENT_CFG_INDICATE )
  {
    // Set the handle
    pInd->handle = CGMAttrTbl[CGM_CGM_OPCP_VALUE_POS].handle;

    // Send the Indication
    return GATT_Indication( connHandle, pInd, FALSE, taskId );
  }

  return bleNotReady;
}

/*********************************************************************
 * @fn          CGM_ReadAttrCB
 *
 * @brief       Read an attribute.
 *
 * @param       connHandle - connection message was received on
 * @param       pAttr - pointer to attribute
 * @param       pValue - pointer to data to be read
 * @param       pLen - length of data to be read
 * @param       offset - offset of the first octet to be read
 * @param       maxLen - maximum length of data to be read
 *
 * @return      Success or Failure
 */
   
   //NEW
static uint8 CGM_ReadAttrCB( uint16 connHandle, gattAttribute_t *pAttr,
                            uint8 *pValue, uint8 *pLen, uint16 offset, uint8 maxLen )
{
  bStatus_t status = SUCCESS;
  // Make sure it's not a blob operation (no attributes in the profile are long)
  if ( offset > 0 )
  {
    return ( ATT_ERR_ATTR_NOT_LONG );
  }

  if ( pAttr->type.len == ATT_BT_UUID_SIZE )
  {
    // 16-bit UUID
    uint16 uuid = BUILD_UINT16( pAttr->type.uuid[0], pAttr->type.uuid[1]);
    switch ( uuid )
    {
      // No need for "GATT_SERVICE_UUID" or "GATT_CLIENT_CHAR_CFG_UUID" cases;
      // gattserverapp handles those types for reads

      case CGM_FEATURE_UUID:       
        *pLen = CGM_CHAR_VAL_SIZE_FEATURE;
         (*CGMServiceCB)(CGM_FEATURE_READ_REQUEST, pValue, *pLen);
      break;

     case CGM_STATUS_UUID:
        *pLen = CGM_CHAR_VAL_SIZE_STATUS;
         (*CGMServiceCB)(CGM_STATUS_READ_REQUEST, pValue, *pLen);
         break;
     
      case CGM_SES_START_TIME_UUID:
        *pLen = CGM_CHAR_VAL_SIZE_START_TIME;
        (*CGMServiceCB)(CGM_START_TIME_READ_REQUEST, pValue, *pLen);
      break;
    
      case CGM_SES_RUN_TIME_UUID:
        *pLen = CGM_CHAR_VAL_SIZE_RUN_TIME;
        (*CGMServiceCB)(CGM_RUN_TIME_READ_REQUEST, pValue, *pLen);
      break;
      
      
      default:
        // Should never get here! (characteristics 3 and 4 do not have read permissions)
        *pLen = 0;
        status = ATT_ERR_ATTR_NOT_FOUND;
        break;
    }
  }

  return ( status );
}

/*********************************************************************
 * @fn      glucose_WriteAttrCB
 *
 * @brief   Validate attribute data prior to a write operation
 *
 * @param   connHandle - connection message was received on
 * @param   pAttr - pointer to attribute
 * @param   pValue - pointer to data to be written
 * @param   len - length of data
 * @param   offset - offset of the first octet to be written
 *
 * @return  Success or Failure
 */

//NEW
static bStatus_t CGM_WriteAttrCB( uint16 connHandle, gattAttribute_t *pAttr,
                                 uint8 *pValue, uint8 len, uint16 offset )
{
  bStatus_t status = SUCCESS;

  // Make sure it's not a blob operation (no attributes in the profile are long)
  if ( offset > 0 )
  {
    return ( ATT_ERR_ATTR_NOT_LONG );
  }

  uint16 uuid = BUILD_UINT16( pAttr->type.uuid[0], pAttr->type.uuid[1]);

  switch ( uuid )
  {

  case  GATT_CLIENT_CHAR_CFG_UUID:
      // CGM Notifications
      if ((pAttr->handle == CGMAttrTbl[CGM_MEAS_CONFIG_POS].handle /*||
           pAttr->handle == CGMAttrTbl[CGM_RACP_CONFIG_POS].handle ||
             pAttr->handle == CGMAttrTbl[CGM_CGM_OPCP_CONFIG_POS].handle*/))
      {
        status = GATTServApp_ProcessCCCWriteReq( connHandle, pAttr, pValue, len,
                                                 offset, GATT_CLIENT_CFG_NOTIFY );
        if ( status == SUCCESS )
        {
          uint16 charCfg = BUILD_UINT16( pValue[0], pValue[1] );

          if(pAttr->handle == CGMAttrTbl[CGM_MEAS_CONFIG_POS].handle)
          {
            (*CGMServiceCB)((charCfg == 0) ? CGM_MEAS_NTF_DISABLED :
                                                 CGM_MEAS_NTF_ENABLED, NULL, NULL);
          }
        }
      }
      //NEW
      // Indications
      else if ( pAttr->handle == CGMAttrTbl[CGM_RACP_CONFIG_POS].handle ||
              pAttr->handle == CGMAttrTbl[CGM_CGM_OPCP_CONFIG_POS ].handle )
      {
        status = GATTServApp_ProcessCCCWriteReq( connHandle, pAttr, pValue, len,
                                                 offset, GATT_CLIENT_CFG_INDICATE );
        if ( status == SUCCESS )
        {
            uint16 charCfg = BUILD_UINT16( pValue[0], pValue[1] );

            (*CGMServiceCB)((charCfg == 0) ? CGM_CTL_PNT_IND_DISABLED :
                                                 CGM_CTL_PNT_IND_ENABLED, NULL, NULL);
        }
      }
      else
      {
        status = ATT_ERR_INVALID_VALUE;
      }
    break;

    case CGM_SES_START_TIME_UUID:
       (*CGMServiceCB)(CGM_SESSION_START_TIME_CHANGED, pValue, len);
    break;
   //NEW 
    case CGM_SPEC_OPS_CTRL_PT_UUID: //if CGM specific control point is written
      if(len >= CGM_CTL_PNT_MIN_SIZE  && len <= CGM_CTL_PNT_MAX_SIZE)
      { //do preliminary filtering of the Op code, to filter out invalid codes
        (*CGMServiceCB)(CGM_CTL_PNT_CMD, pValue, len); //call back to APP2SERV to process the command received.
      }
      else
      {
        status = ATT_ERR_INVALID_VALUE_SIZE;
      }
    break;
    
    /*
    case  RECORD_CTRL_PT_UUID:
      if(len >= GLUCOSE_CTL_PNT_MIN_SIZE  && len <= GLUCOSE_CTL_PNT_MAX_SIZE)
      {
        uint8 opcode = pValue[0];

        // if transfer in progress
        if (opcode != CTL_PNT_OP_ABORT && glucoseSendAllRecords)
        {
          status = GLUCOSE_ERR_IN_PROGRESS;
        }
        // if CCC not configured for glucose measurement
        else if ( opcode == CTL_PNT_OP_REQ &&
                 !( GATTServApp_ReadCharCfg( connHandle, glucoseMeasConfig ) & GATT_CLIENT_CFG_NOTIFY ) )
        {
          status = GLUCOSE_ERR_CCC_CONFIG;
        }
        else
        {
          (*glucoseServiceCB)(GLUCOSE_CTL_PNT_CMD, pValue, len);
        }
      }
      else
      {
        status = ATT_ERR_INVALID_VALUE_SIZE;
      }
    break;
*/
    
    default:
      status = ATT_ERR_ATTR_NOT_FOUND;
      break;
  }

  return ( status );
}

/*********************************************************************
 * @fn          glucose_HandleConnStatusCB
 *
 * @brief       Simple Profile link status change handler function.
 *
 * @param       connHandle - connection handle
 * @param       changeType - type of change
 *
 * @return      none
 */
static void CGM_HandleConnStatusCB( uint16 connHandle, uint8 changeType )
{
  // Make sure this is not loopback connection
  if ( connHandle != LOOPBACK_CONNHANDLE )
  {
    // Reset Client Char Config if connection has dropped
    if ( ( changeType == LINKDB_STATUS_UPDATE_REMOVED )      ||
         ( ( changeType == LINKDB_STATUS_UPDATE_STATEFLAGS ) &&
           ( !linkDB_Up( connHandle ) ) ) )
    {
      GATTServApp_InitCharCfg( connHandle, CGMControlConfig );
      GATTServApp_InitCharCfg( connHandle, CGMRacpConfig );
      GATTServApp_InitCharCfg( connHandle, CGMMeasConfig );
    }
  }
}

/*********************************************************************
*********************************************************************/
