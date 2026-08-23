/**
 * @file          IPCTypes.h
 * @brief         Header file containing IPC related of typedefs.
 * @date          20/07/2026
 * @author        Yash Giramkar [YSG], Aditya Bhosale [ASB]
 * @copyright     Bajaj Auto Technology Limited (BATL)
 */

#ifndef _IPC_TYPES_H
#define _IPC_TYPES_H

/******************************************************************************/
/*                                                                            */
/*                                  INCLUDES                                  */
/*                                                                            */
/******************************************************************************/
#include <stdint.h>
#include <stdbool.h>

#include "IPCCommands.h"
#include "ServiceIDs.h"

/******************************************************************************/
/*                                                                            */
/*                                  DEFINES                                   */
/*                                                                            */
/******************************************************************************/
/**
 * @def           IPC_MAX_DATA_LENGTH
 * @brief         Maximum data length for IPC messages.
 */
#define IPC_MAX_DATA_LENGTH                  256U

/******************************************************************************/
/*                                                                            */
/*                                 TYPEDEFS                                   */
/*                                                                            */
/******************************************************************************/
/**
 * @typedef       IPCTransactionID_T
 * @brief         Maintains the transaction ID in an IPC message.
 */
typedef uint32_t  IPCTransactionID_T;

/**
 * @typedef       IPCDataLength_T
 * @brief         Maintains the length of data in current IPC transaction.
 */
typedef uint16_t  IPCDataLength_T;


/******************************************************************************/
/*                                                                            */
/*                                   ENUMS                                    */
/*                                                                            */
/******************************************************************************/
/**
 * @enum          IPCNodes_E
 * @brief         Global Enum containing all the nodes that can generate or
                  receive an IPC based request.
 * @note          Please note that after addition of any active service or
                  application the node name should be updated here in this list
                  of nodes.
 */
typedef enum
{
    /**************************************************************************
     * Applications
     **************************************************************************/
      eIPC_NODE_APP_GENERAL_STATUS,
      eIPC_NODE_APP_PROVISIONING,
      eIPC_NODE_APP_OFFLINE_ANALYTICS,
      eIPC_NODE_APP_DATA_LOGGER,
      eIPC_NODE_APP_BALNET_FOTA,
      eIPC_NODE_APP_UDS_FOTA,
      eIPC_NODE_APP_MODEM_FOTA,
      eIPC_NODE_APP_COMMAND_MANAGER,
      eIPC_NODE_APP_MANIFEST,

     /**************************************************************************
     * Active Services
     **************************************************************************/
      eIPC_NODE_SVC_NETWORK,
      eIPC_NODE_SVC_UART,
      eIPC_NODE_SVC_MQTT,
      eIPC_NODE_SVC_HTTPS,
      eIPC_NODE_SVC_TIMER,

      eIPC_NODE_MAX

} IPCNodes_E;

/**
 * @enum          IPCMsgType_E
 * @brief         Global Enum indicating type of message passed using IPC.
 */
typedef enum
{
   eIPC_MSG_REQUEST=1,
   eIPC_MSG_RESPONSE,
   eIPC_MSG_NOTIFICATION,
   eIPC_MSG_EVENT

}IPCMsgType_E;

/**
 * @enum          IPCStatus_E
 * @brief         Global Enum indicating the status of IPC. This field is
                  normally only to be used in the reponse message type, but
                  can be used interleaved with other message types as well.
 */
typedef enum
{
   eIPC_STATUS_OK=0,
   eIPC_STATUS_ERROR,
   eIPC_STATUS_BUSY,
   eIPC_STATUS_TIMEOUT,
   eIPC_STATUS_INVALID_PARAM,
   eIPC_STATUS_NOT_SUPPORTED,
   eIPC_STATUS_DENIED

}IPCStatus_E;

/**
 * @enum          IPCFlag_E
 * @brief         Global Enum indicating the flags that could be transmitted/
                  received during an IPC based message transaction.
 */
typedef enum
{
   eIPC_FLAG_NONE=0,
   eIPC_FLAG_RESPONSE_REQUIRED,
   eIPC_FLAG_HIGH_PRIORITY,
   eIPC_FLAG_RESERVED,

}IPCFlag_E;

/******************************************************************************/
/*                                                                            */
/*                                 STRUCTURES                                 */
/*                                                                            */
/******************************************************************************/
/**
 * @struct        IPCMessage_T
 * @brief         Structure conatains encapsulation of all the data containts of
 *                an IPC message.
 */
typedef struct
{
   /**************************************************************************
    * Routing Information
    **************************************************************************/
   IPCNodes_E eSourceNode;
   IPCNodes_E eDestNode;

   /**************************************************************************
    * Message Information
    **************************************************************************/
   IPCMsgType_E eMsgType;
   IPCCommands_E eCmd;
   IPCTransactionID_T tTransactionID;
   ServiceIDs_E eServiceID; // (This field is optional)


   /**************************************************************************
    * Response Information
    **************************************************************************/
   IPCStatus_E eStatus;
   IPCFlag_E eFlags;

   /**************************************************************************
    * Payload
    **************************************************************************/
   IPCDataLength_T tDataLength;
   uint8_t u8a_Data[IPC_MAX_DATA_LENGTH];

   /**************************************************************************
    * Service Specific Parameters
    **************************************************************************/
   uint32_t u32Param1;
   uint32_t u32Param2;
   uint32_t u32Param3;

}IPCMessage_T;


/******************************************************************************/
/*                                                                            */
/*                                   UNIONS                                   */
/*                                                                            */
/******************************************************************************/
/**
 * @union         <Union name>
 * @brief         <Union details>.
 */

/******************************************************************************/
/*                                                                            */
/*                              EXTERN VARIABLES                              */
/*                                                                            */
/******************************************************************************/

/******************************************************************************/
/*                                                                            */
/*                              EXTERN FUNCTIONS                              */
/*                                                                            */
/******************************************************************************/

#endif //!_IPC_MESSAGES_H

/**
 * Copyright(c) Bajaj Auto Technology Limited (BATL) as an unpublished work.
 * THIS SOFTWARE AND/OR MATERIAL IS THE PROPERTY OF BATL.
 * ALL USE, DISCLOSURE, AND/OR REPRODUCTION NOT SPECIFICALLY AUTHORIZED BY
 * BATL IS PROHIBITED.
 *
 * @author:Yash Giramkar [YSG], Aditya Bhosale [ASB]
 */
