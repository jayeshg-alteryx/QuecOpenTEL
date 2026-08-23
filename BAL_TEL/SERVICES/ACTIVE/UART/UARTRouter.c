/**
 * @file          UARTRouter.c
 * @brief         Source file containing UARTRouter functionality
 * @date          04/08/26
 * @author        Yash Giramkar [YSG]
 * @copyright     Bajaj Auto Technology Limited (BATL)
 */

/******************************************************************************/
/*                                                                            */
/*                                  INCLUDES                                  */
/*                                                                            */
/******************************************************************************/
#include "UARTRouter.h"

/******************************************************************************/
/*                                                                            */
/*                                  DEFINES                                   */
/*                                                                            */
/******************************************************************************/
/**
 * @def           <Define name>
 * @brief         <Define details>.
 */

/******************************************************************************/
/*                                                                            */
/*                                   ENUMS                                    */
/*                                                                            */
/******************************************************************************/
// Definition of all the enums
/**
 * @enum          <Enum name>
 * @brief         <Enum details>.
 */

// Declarations of all the enum variables
/**
 * @var           <Variable name>
 * @brief         <Variable details>.
 */

/******************************************************************************/
/*                                                                            */
/*                                 STRUCTURES                                 */
/*                                                                            */
/******************************************************************************/
// Definition of all the structures
/**
 * @struct        <Structure name>
 * @brief         <Structure details>.
 */

// Declarations of all the structure variables
/**
 * @var           <Variable name>
 * @brief         <Variable details>.
 */

/******************************************************************************/
/*                                                                            */
/*                                   UNIONS                                   */
/*                                                                            */
/******************************************************************************/
// Definition of all the unions
/**
 * @union         <Union name>
 * @brief         <Union details>.
 */

// Declarations of all the union variables
/**
 * @var           <Variable name>
 * @brief         <Variable details>.
 */

/******************************************************************************/
/*                                                                            */
/*                       PRIVATE FUNCTION DECLARATIONS                        */
/*                                                                            */
/******************************************************************************/

/******************************************************************************/
/*                                                                            */
/*                              EXTERN VARIABLES                              */
/*                                                                            */
/******************************************************************************/

/******************************************************************************/
/*                                                                            */
/*                              PUBLIC VARIABLES                              */
/*                                                                            */
/******************************************************************************/
/**
 * @var           <Variable name>
 * @brief         <Variable details>.
 */

/******************************************************************************/
/*                                                                            */
/*                             PRIVATE VARIABLES                              */
/*                                                                            */
/******************************************************************************/
/**
 * @var           <Variable name>
 * @brief         <Variable details>.
 */

/******************************************************************************/
/*                                                                            */
/*                              EXTERN FUNCTIONS                              */
/*                                                                            */
/******************************************************************************/

/******************************************************************************/
/*                                                                            */
/*                        PRIVATE FUNCTION DEFINITIONS                        */
/*                                                                            */
/******************************************************************************/
/**
 * @private       <Function name>
 * @brief         <Function details>.
 * @param[in]     <Input parameter details>.
 * @param[out]    <Output parameter details>.
 * @param[inout]  <Input-Output parameter details>.
 * @return        <Return details>.
 */

/******************************************************************************/
/*                                                                            */
/*                        PUBLIC FUNCTION DEFINITIONS                         */
/*                                                                            */
/******************************************************************************/
/**
 * @public        gv_UARTManager_RouteToApp
 * @brief         Uart manager function to route the received UART frame to the source application.
 *                This function is called when a UART frame is received and needs to be routed to the
 *                appropriate application based on the application ID information in the IPC message.
 * @param[in]     ptst_IPCEvent : Pointer to the IPC message containing the
 *                received UART frame and routing information.
 * @param[out]    None
 * @param[inout]  None
 * @return        None
 */
void gv_UARTManager_RouteToApp(IPCMessage_T* ptst_IPCEvent, UARTFrameType_E e_FrameType,
                  Application_E e_appID, uint16_t u16_rxServiceID, uint8_t u8_payloadLen)
{
   //  Parse the incoming UART Frame
   UartFrame_U* upt_rxUartFrame = (UartFrame_U*) ptst_IPCEvent->u8a_Data;
   uint8_t* u8pt_rxData = NULL;


   IPCMessage_T st_NewIPCEvent = {0};
   ql_queue_t pt_Queue = NULL;
   IPCNodes_E e_nodeID = ge_Common_GetAppQueue(e_appID, &pt_Queue);


   if(pt_Queue != NULL)
   {
      // Compose an IPC Message to send to the application queue
      st_NewIPCEvent.eSourceNode = eIPC_NODE_SVC_UART;
      st_NewIPCEvent.eDestNode = e_nodeID;

      st_NewIPCEvent.eMsgType = eIPC_MSG_EVENT;
      st_NewIPCEvent.eCmd = eIPC_CMD_UART_RX_DATA_AVAILABLE;
      st_NewIPCEvent.tTransactionID = 0;
      st_NewIPCEvent.eServiceID = u16_rxServiceID;

      st_NewIPCEvent.eFlags = eIPC_FLAG_NONE;

      st_NewIPCEvent.tDataLength = u8_payloadLen;

      if(e_FrameType == eUART_FRAME_TYPE_GENERAL)
      {
         u8pt_rxData = upt_rxUartFrame->st_uartGenMetaFrame.au8_Payload;
         memcpy(st_NewIPCEvent.u8a_Data, u8pt_rxData, u8_payloadLen);
         st_NewIPCEvent.eStatus = eIPC_STATUS_OK;
#ifdef DEBUG_ENABLED
         BAL_UART_SERVICE_LOG("General Frame Enqueued to application ID: %d, Service ID: %d",
                                                                  e_appID, u16_rxServiceID);
#endif // DEBUG_ENABLED

      }
      else if(e_FrameType == eUART_FRAME_TYPE_METADATA)
      {
#ifdef DEBUG_ENABLED
         BAL_UART_SERVICE_LOG("CTS Status Enqueued to Application ID: %d, Service ID: %d, CTS Status: %d",
                                                   e_appID, u16_rxServiceID, ptst_IPCEvent->eStatus);
#endif // DEBUG_ENABLED
         st_NewIPCEvent.eStatus = ptst_IPCEvent->eStatus;
      }
      else
      {
         // Data Frame is received, copy the payload to IPC message
         st_NewIPCEvent.eStatus = eIPC_STATUS_OK;
         u8pt_rxData = upt_rxUartFrame->st_uartDataFrame.au8_Payload;
         memcpy(st_NewIPCEvent.u8a_Data, u8pt_rxData, u8_payloadLen);

#ifdef DEBUG_ENABLED
         BAL_UART_SERVICE_LOG("Data Frame Enqueued to application ID: %d, Service ID: %d",
                                                                     e_appID, u16_rxServiceID);
#endif // DEBUG_ENABLED
      }

#ifdef DEBUG_ENABLED
      BAL_UART_SERVICE_LOG("Enqueue operation for application ID: %d, Service ID: %d",
                                                                     e_appID, u16_rxServiceID);
#endif // DEBUG_ENABLED
      // Send the IPC message to the application queue
      (void)ql_rtos_queue_release(pt_Queue,
         sizeof(IPCMessage_T), (uint8_t *)&st_NewIPCEvent, QL_WAIT_FOREVER);
   }
   else
   {
      // No queue registered for the application, log error and ignore the frame
#ifdef DEBUG_ENABLED
      BAL_UART_SERVICE_LOG("No queue registered for application ID: %d, Service ID: %d",
                                                                     e_appID, u16_rxServiceID);
#endif // DEBUG_ENABLED
   }

   return;
}

/**
 * Copyright(c) Bajaj Auto Technology Limited (BATL) as an unpublished work.
 * THIS SOFTWARE AND/OR MATERIAL IS THE PROPERTY OF BATL.
 * ALL USE, DISCLOSURE, AND/OR REPRODUCTION NOT SPECIFICALLY AUTHORIZED BY
 * BATL IS PROHIBITED.
 *
 * @author: Yash Giramkar [YSG]
 */
