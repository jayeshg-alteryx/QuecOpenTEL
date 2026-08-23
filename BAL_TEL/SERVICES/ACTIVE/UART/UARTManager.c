/**
 * @file          UARTManager.c
 * @brief         Source file containing the code for UART service manager.
 * @date          14/07/2026
 * @author        Yash Giramkar [YSG], Aditya Bhosale [ASB]
 * @copyright     Bajaj Auto Technology Limited (BATL)
 */

/******************************************************************************/
/*                                                                            */
/*                                  INCLUDES                                  */
/*                                                                            */
/******************************************************************************/
#include "UARTManager.h"

/******************************************************************************/
/*                                  DEFINES                                   */
/******************************************************************************/
#define UART_MANAGER_TASK_STACK_SIZE         4096U
#define UART_MANAGER_TASK_PRIORITY           APP_PRIORITY_ABOVE_NORMAL
#define UART_MANAGER_TASK_EVENT_COUNT        16U
#define UART_MANAGER_QUEUE_LENGTH            16U

#define UART_INIT_FAILURE                    (1<<0)
#define UART_CB_REG_FAILURE                  (1<<1)
#define UART_Q_INIT_FAILURE                  (1<<2)
#define UART_TASK_REG_FAILURE                (1<<3)

/******************************************************************************/
/*                                   ENUMS                                    */
/******************************************************************************/
typedef enum
{
   eUART_MANAGER_EVENT_RX,
   eUART_MANAGER_EVENT_RX_OVERFLOW,
   eUART_MANAGER_EVENT_TX
} UARTManagerEventType_E;


// Enum to maintain current status of UART TX Operation
typedef enum
{
   UART_TX_IDLE=0,
   UART_TX_META_DATA,
   UART_TX_META_DATA_ACK_WAIT,
   UART_TX_BULK_DATA_TX_BEGIN,
} UARTTxStatus_E;

// Enum to maintain current status of UART RX Operation
typedef enum
{
   UART_RX_IDLE=0,
   UART_RX_META_DATA,
   UART_RX_META_DATA_ACK_SEND,
   UART_RX_BULK_DATA_RX_WAIT,
} UARTRxStatus_E;

/******************************************************************************/
/*                                 STRUCTURES                                 */
/******************************************************************************/
// Create a structure to hold UART TX Context, to handle medata and data frame transmission
typedef struct
{
   uint32_t u32_txDataLength;
   uint16_t u16_txAppId;
   uint16_t u16_txServiceID;
   UARTTxStatus_E e_txStatus;
   uint8_t u8a_composedDataFrame[UART_DATA_FRAME_MAX_SIZE];
} UARTManagerTxContext_T;

// Create a structure to hold UART RX Context, to handle medata and data frame reception
typedef struct
{
   uint32_t u32_expectedRxDataLength;
   uint16_t u16_rxAppId;
   uint16_t u16_rxServiceID;
   UARTRxStatus_E e_rxStatus;
   uint8_t u8_dataFrameCksum;
} UARTManagerRxContext_T;

typedef struct
{
   ql_task_t pt_Task;
   ql_queue_t pt_Queue;
   bool b_Initialized;
   uint8_t u8_failureCode;
   UARTManagerTxContext_T st_txContext;
   UARTManagerRxContext_T st_rxContext;
} UARTManagerContext_T;

/******************************************************************************/
/*                             PRIVATE VARIABLES                              */
/******************************************************************************/
// Variable to store current context of UART Manager
static UARTManagerContext_T sst_UARTManagerContext;
// Temporary buffer to hold received UART Data before enqueuing
static uint8_t u8a_receivedDataTempBuff[UART_DATA_FRAME_MAX_SIZE];
// Offset to current write location inside UART Temp Buffer
static uint32_t u32_currRxPtr = 0;

IPCMessage_T st_InterUARTIPCMsg = {0};

/******************************************************************************/
/*                       PRIVATE FUNCTION DECLARATIONS                        */
/******************************************************************************/
static void sv_UARTManager_Callback(uint32 u32_EventType,
                                    ql_uart_port_number_e ePort,
                                    uint32 u32_Size);
static void sv_UARTManager_Task(void *vpt_Argument);
static void sv_UARTManager_ProcessRx(IPCMessage_T* ptst_IPCEvent);
static void sv_UARTManager_ProcessTx(IPCMessage_T* ptst_IPCEvent);
static void sv_UARTManager_ProcessOverflow(IPCMessage_T* ptst_IPCEvent);
static TelSysResponse_E se_UARTManager_SendDataFrame(void);
static TelSysResponse_E se_UARTManager_SendMetadataAck(IPCMessage_T *cpt_ipcData,
                                                        bool bPositiveAck);
static TelSysResponse_E se_UARTManager_SendGeneralAck(IPCMessage_T *cpt_ipcData,
                                                        bool bPositiveAck);
static void sv_UARTManager_ClearTxContext(void);
static void sv_UARTManager_ClearRxContext(void);
static void sv_UARTManager_ComposeSendGeneralFrame(IPCMessage_T *cpt_IPCEvent);
static void sv_UARTManager_ComposeDataFrameSave(IPCMessage_T *cpt_IPCEvent);
static void sv_UARTManager_ComposeSendMetadataFrame(IPCMessage_T *cpt_IPCEvent);

static bool b_ReadAndValidateFrameRx(ql_uart_port_number_e e_Port, uint32 u32_Size);
static void v_UARTRxEventPackAndEnqueue(uint32_t u32_eventType);
static uint8_t su8_UARTManager_DetermineAppID(IPCNodes_E e_sourceNode);


/******************************************************************************/
/*                        PRIVATE FUNCTION DEFINITIONS                         */
/******************************************************************************/
/**
 * @private       sv_UARTManager_Callback
 * @brief         Converts Quectel UART indications into UART Manager events.
 *                The callback must remain lightweight; frame parsing and
 *                routing are performed by sv_UARTManager_Task.
 * @param[in]     u32_EventType : Quectel UART event indication.
 * @param[in]     ePort : UART port that generated the indication.
 * @param[in]     u32_Size : Number of bytes reported by the driver.
 * @return        none
 */
static void sv_UARTManager_Callback(uint32 u32_EventType,
                                    ql_uart_port_number_e e_Port,
                                    uint32 u32_Size)
{
   bool bQueueEvent = FALSE;

   if ((sst_UARTManagerContext.b_Initialized) &&
       (sst_UARTManagerContext.pt_Queue != NULL))
   {
      if (u32_EventType == QUEC_UART_RX_RECV_DATA_IND)
      {
#ifdef DEBUG_ENABLED
   BAL_UART_SERVICE_LOG("UART CB : Data RX");
#endif // DEBUG_ENABLED
         bQueueEvent = b_ReadAndValidateFrameRx(e_Port, u32_Size);
      }
      else if (u32_EventType == (uint32_t)QUEC_UART_RX_OVERFLOW_IND)
      {
#ifdef DEBUG_ENABLED
   BAL_UART_SERVICE_LOG("UART CB : Data RX Overflow");
#endif // DEBUG_ENABLED
         bQueueEvent = TRUE;
      }
      else
      {
#ifdef DEBUG_ENABLED
   BAL_UART_SERVICE_LOG("UART CB : Unrecognised Event : %d", u32_EventType);
#endif // DEBUG_ENABLED
      }

      if (bQueueEvent)
      {
         // Pack IPC and Enqueue
         v_UARTRxEventPackAndEnqueue(u32_EventType);

      }
      else
      {
         // Yet to receive complete data or an unrecognised event occured
      }
   }
}

/**
 * @private       b_ReadAndValidateFrameRx
 * @brief         The function is called on reception of QUEC_UART_RX_RECV_DATA_IND.
 *                The function is responsible for first checking the frame validity,
 *                basis presence of SOF and EOF and the current UART Manager Context.
 *                If the frame is found to be complete then the function returns TRUE,
 *                else it returns FALSE.
 * @param[in]     ePort : UART port that generated the indication.
 * @param[in]     u32_Size : Number of bytes reported by the driver.
 * @return        none
 */
static bool b_ReadAndValidateFrameRx(ql_uart_port_number_e e_Port, uint32 u32_Size)
{
   (void)e_Port;
   bool b_retVal = FALSE;
   if ((u32_Size == 0U) || (u32_Size > (UART_DATA_FRAME_MAX_SIZE - u32_currRxPtr)))
   {
      u32_currRxPtr = 0U;
      b_retVal =  FALSE;
   }
   else
   {

      // Read the UART Data Content
      gs32_UARTPort_Read((uint8_t*)(u8a_receivedDataTempBuff+u32_currRxPtr), u32_Size);
      // Update the write pointer to Temp Reception Buffer
      u32_currRxPtr += u32_Size;
      // Start Reception of new frame
      if((u32_currRxPtr == sst_UARTManagerContext.st_txContext.u32_txDataLength) ||
         (u32_currRxPtr == sst_UARTManagerContext.st_rxContext.u32_expectedRxDataLength))
      {
         //  Check for SOF and EOF characters
         if((u8a_receivedDataTempBuff[0] == UART_SOF) &&
            (u8a_receivedDataTempBuff[u32_currRxPtr - 1] == UART_EOF))
         {
            b_retVal = TRUE;
         }
         else
         {
#ifdef DEBUG_ENABLED
            // Log Error
            BAL_UART_SERVICE_LOG(" Incorrect Data Length : %s", &u8a_receivedDataTempBuff[0]);
            // TBD: Error Handling to be added
#endif
         }
      }
      else
      {
         //  Complete Frame yet to be received
#ifdef DEBUG_ENABLED
            // Log Error
            BAL_UART_SERVICE_LOG("Data Rx Length: %d Expected Rx Length: %d, Expected Tx Length: %d",
                     u32_currRxPtr,
                     sst_UARTManagerContext.st_rxContext.u32_expectedRxDataLength,
                     sst_UARTManagerContext.st_txContext.u32_txDataLength);
#endif
      }
   }

   return b_retVal;
}

/**
 * @private       v_UARTRxEventPackAndEnqueue
 * @brief         The function is called on reception of:
 *                - An RX Overflow indication on UART
 *                - Complete frame reception.
 * @param[in]     eEventType : Event Type indicating the even for IPC packing.
 * @return        none
 */
static void v_UARTRxEventPackAndEnqueue(uint32_t u32_eventType)
{
#ifdef DEBUG_ENABLED
   QlOSStatus e_status;
#endif // DEBUG_ENABLED
   if(u32_eventType == (uint32_t)QUEC_UART_RX_OVERFLOW_IND)
   {
      //  Mark an RX Overflow error and send the IPC data
      st_InterUARTIPCMsg.eSourceNode = eIPC_NODE_SVC_UART;
      st_InterUARTIPCMsg.eDestNode = eIPC_NODE_SVC_UART;
      st_InterUARTIPCMsg.eMsgType = eIPC_MSG_NOTIFICATION;
      st_InterUARTIPCMsg.eCmd = eIPC_CMD_UART_RX_OVERFLOW;
      st_InterUARTIPCMsg.tTransactionID = 0;
      st_InterUARTIPCMsg.eStatus = eIPC_STATUS_ERROR;
      st_InterUARTIPCMsg.eFlags = eIPC_FLAG_NONE;
      st_InterUARTIPCMsg.tDataLength = 0;
      st_InterUARTIPCMsg.u8a_Data[0] = '\0';
   }
   else
   {
      // Fill the IPC structure with proper information and send the IPC data to UART Manager
      //  Mark an RX Overflow error and send the IPC data
      st_InterUARTIPCMsg.eSourceNode = eIPC_NODE_SVC_UART;
      st_InterUARTIPCMsg.eDestNode = eIPC_NODE_SVC_UART;
      st_InterUARTIPCMsg.eMsgType = eIPC_MSG_NOTIFICATION;
      st_InterUARTIPCMsg.eCmd = eIPC_CMD_UART_RX_DATA_AVAILABLE;
      st_InterUARTIPCMsg.tTransactionID = 0;
      st_InterUARTIPCMsg.eStatus = eIPC_STATUS_OK;
      st_InterUARTIPCMsg.eFlags = eIPC_FLAG_NONE;
      st_InterUARTIPCMsg.tDataLength = u32_currRxPtr;
      memcpy(st_InterUARTIPCMsg.u8a_Data, u8a_receivedDataTempBuff, u32_currRxPtr);
      // Reset the current RX pointer to 0 for next frame reception and clear the temporary buffer
      u32_currRxPtr = 0;
      memset(u8a_receivedDataTempBuff, 0, sizeof(u8a_receivedDataTempBuff));
   }

   // Enqueue the message
#ifdef DEBUG_ENABLED
   e_status =
#endif // DEBUG_ENABLED
   ql_rtos_queue_release(sst_UARTManagerContext.pt_Queue, sizeof(IPCMessage_T),
                                          (uint8_t *)&st_InterUARTIPCMsg, QL_WAIT_FOREVER);
#ifdef DEBUG_ENABLED
   if(e_status == QL_OSI_SUCCESS)
   {
      BAL_UART_SERVICE_LOG("UART Message Enqueue Success");
   }
   else
   {
      BAL_UART_SERVICE_LOG("UART Message Enqueue Failed");
   }
#endif // DEBUG_ENABLED
}

static void sv_UARTManager_ClearTxContext(void)
{
   (void)memset(&sst_UARTManagerContext.st_txContext, 0,
                sizeof(sst_UARTManagerContext.st_txContext));
   sst_UARTManagerContext.st_txContext.e_txStatus = UART_TX_IDLE;
}

static void sv_UARTManager_ClearRxContext(void)
{
   (void)memset(&sst_UARTManagerContext.st_rxContext, 0,
                sizeof(sst_UARTManagerContext.st_rxContext));
   sst_UARTManagerContext.st_rxContext.e_rxStatus = UART_RX_IDLE;
   sst_UARTManagerContext.st_rxContext.u32_expectedRxDataLength = 16;
}



static TelSysResponse_E se_UARTManager_SendDataFrame(void)
{
   return ge_UARTPort_Write(sst_UARTManagerContext.st_txContext.u8a_composedDataFrame,
                            (uint16_t)sst_UARTManagerContext.st_txContext.u32_txDataLength);
}

static TelSysResponse_E se_UARTManager_SendMetadataAck(IPCMessage_T *cpt_ipcData,
                                                        bool bPositiveAck)
{
   // uint8_t u8a_tempUartBuff[UART_GENERAL_FRAME_SIZE];
   UARTFrameGenMeta_T st_uartTxFrame = {0};
   st_uartTxFrame.u8_SOF = UART_SOF;
   st_uartTxFrame.eFrameType = eUART_FRAME_TYPE_METADATA;
   UartFrame_U* upt_rxUartFrame = (UartFrame_U*) cpt_ipcData->u8a_Data;

   // Determine the application ID based on the data received in the IPC Message
   st_uartTxFrame.eApplication = upt_rxUartFrame->st_uartGenMetaFrame.eApplication;
   st_uartTxFrame.eOriginator = eUART_ORIGINATOR_MODEM;
   st_uartTxFrame.eIntent = eUART_INTENT_RESPONSE;

   if(bPositiveAck == TRUE)
   {
      st_uartTxFrame.u16_FrameId = cpt_ipcData->eServiceID;
      // Positive acknowledgement, hence the payload is empty
      st_uartTxFrame.u8_PayloadLength = 0;
      (void)memset(st_uartTxFrame.au8_Payload, 0,
                sizeof(st_uartTxFrame.au8_Payload));

   }
   else
   {
      // Negative acknowledgement
      // Set the MSB to indicate negative ack
      st_uartTxFrame.u16_FrameId = cpt_ipcData->eServiceID + 0x8000;
      // The payload should be managed by caller
      st_uartTxFrame.u8_PayloadLength = cpt_ipcData->tDataLength;
      (void)memcpy(st_uartTxFrame.au8_Payload, cpt_ipcData->u8a_Data,
                cpt_ipcData->tDataLength);
   }


   st_uartTxFrame.u8_cksum = gu8_Crypto_CalcCksm((((uint8_t*)&st_uartTxFrame)+1), 13);
   st_uartTxFrame.u8_EOF = UART_EOF;

   return ge_UARTPort_Write((uint8_t*)&st_uartTxFrame, UART_GENERAL_FRAME_SIZE);
}


static TelSysResponse_E se_UARTManager_SendGeneralAck(IPCMessage_T *cpt_ipcData,
                                                        bool bPositiveAck)
{
   // Implementation for sending general acknowledgment
   UARTFrameGenMeta_T st_uartTxFrame = {0};
   st_uartTxFrame.u8_SOF = UART_SOF;
   st_uartTxFrame.eFrameType = eUART_FRAME_TYPE_GENERAL;
   st_uartTxFrame.eApplication = su8_UARTManager_DetermineAppID(cpt_ipcData->eSourceNode);
   st_uartTxFrame.eOriginator = eUART_ORIGINATOR_MODEM;
   st_uartTxFrame.eIntent = eUART_INTENT_RESPONSE;
   st_uartTxFrame.u16_FrameId = cpt_ipcData->eServiceID;

   if(bPositiveAck == TRUE)
   {
      // Positive acknowledgement, hence the payload is empty
      st_uartTxFrame.u8_PayloadLength = 0;
      (void)memset(st_uartTxFrame.au8_Payload, 0,
                sizeof(st_uartTxFrame.au8_Payload));

   }
   else
   {
      // Negative acknowledgement, hence the payload should be managed by caller
      st_uartTxFrame.u8_PayloadLength = cpt_ipcData->tDataLength;
      (void)memcpy(st_uartTxFrame.au8_Payload, cpt_ipcData->u8a_Data,
                cpt_ipcData->tDataLength);
   }


   st_uartTxFrame.u8_cksum = gu8_Crypto_CalcCksm((((uint8_t*)&st_uartTxFrame)+1), 13);
   st_uartTxFrame.u8_EOF = UART_EOF;

   return ge_UARTPort_Write((uint8_t*)&st_uartTxFrame, UART_GENERAL_FRAME_SIZE);
}


static void sv_UARTManager_ComposeSendGeneralFrame(IPCMessage_T *cpt_ipcEvent)
{

   UARTFrameGenMeta_T st_uartTxFrame = {0};
   st_uartTxFrame.u8_SOF = UART_SOF;
   st_uartTxFrame.eFrameType = eUART_FRAME_TYPE_GENERAL;
   // @R : TBD : to be written
   st_uartTxFrame.eApplication = ge_Common_GetAppIDFromIPCNode(cpt_ipcEvent->eSourceNode);
   st_uartTxFrame.eOriginator = eUART_ORIGINATOR_MODEM;
   st_uartTxFrame.eIntent = (uint8_t)cpt_ipcEvent->eMsgType;
   st_uartTxFrame.u16_FrameId = cpt_ipcEvent->eServiceID;
   st_uartTxFrame.u8_PayloadLength = cpt_ipcEvent->tDataLength;
   (void)memcpy(st_uartTxFrame.au8_Payload, cpt_ipcEvent->u8a_Data,
                  cpt_ipcEvent->tDataLength);

   st_uartTxFrame.u8_cksum = gu8_Crypto_CalcCksm((((uint8_t*)&st_uartTxFrame)+1), 13);
   st_uartTxFrame.u8_EOF = UART_EOF;

   ge_UARTPort_Write((uint8_t*)&st_uartTxFrame, UART_GENERAL_FRAME_SIZE);
   return;
}


static void sv_UARTManager_ComposeDataFrameSave(IPCMessage_T *cpt_ipcEvent)
{
   UARTManagerTxContext_T *pt_context = &sst_UARTManagerContext.st_txContext;

   pt_context->u32_txDataLength = (uint32_t)cpt_ipcEvent->tDataLength + 2U;
   pt_context->u16_txServiceID = (uint16_t)cpt_ipcEvent->eServiceID;

   pt_context->u8a_composedDataFrame[0] = UART_SOF;
   (void)memcpy(&pt_context->u8a_composedDataFrame[1], cpt_ipcEvent->u8a_Data,
                cpt_ipcEvent->tDataLength);
   pt_context->u8a_composedDataFrame[pt_context->u32_txDataLength - 1U] = UART_EOF;
   pt_context->e_txStatus = UART_TX_META_DATA;
#ifdef DEBUG_ENABLED
      BAL_UART_SERVICE_LOG("Data Frame : %s", pt_context->u8a_composedDataFrame);
#endif
}

static void sv_UARTManager_ComposeSendMetadataFrame(IPCMessage_T *cpt_ipcEvent)
{
   UARTFrameGenMeta_T st_uartTxFrame = {0};
   uint16_t u16_payloadLength = cpt_ipcEvent->tDataLength;


   st_uartTxFrame.u8_SOF = UART_SOF;
   st_uartTxFrame.eFrameType = eUART_FRAME_TYPE_METADATA;
   st_uartTxFrame.eApplication = cpt_ipcEvent->eSourceNode;
   st_uartTxFrame.eOriginator = eUART_ORIGINATOR_MODEM;
   st_uartTxFrame.eIntent = eUART_INTENT_LAST_INTENT;
   st_uartTxFrame.u16_FrameId = cpt_ipcEvent->eServiceID;
   st_uartTxFrame.u8_PayloadLength = 3;
   st_uartTxFrame.au8_Payload[0] = (uint8_t)(u16_payloadLength >> 8U);
   st_uartTxFrame.au8_Payload[1] = (uint8_t)u16_payloadLength;
   st_uartTxFrame.au8_Payload[2] = (uint8_t)gu8_Crypto_CalcCksm(cpt_ipcEvent->u8a_Data, u16_payloadLength);
   st_uartTxFrame.u8_cksum = gu8_Crypto_CalcCksm((((uint8_t*)&st_uartTxFrame)+1), 13);
   st_uartTxFrame.u8_EOF = UART_EOF;


   if (ge_UARTPort_Write((uint8_t*)&st_uartTxFrame, UART_GENERAL_FRAME_SIZE) == eTEL_OK)
   {
      sst_UARTManagerContext.st_txContext.e_txStatus = UART_TX_META_DATA_ACK_WAIT;
   }
   else
   {
      sv_UARTManager_ClearTxContext();
   }

   return;
}


/**
 * @private       sv_UARTManager_Task
 * @brief         Waits for UART events and dispatches them to manager logic.
 *                Currenlty only the following commands are supported by UART Manager
 *                - eIPC_CMD_UART_RX_DATA_AVAILABLE :
 *                   Sent to UART Manager by UART Callback, the UART Manager then
 *                   hands it over to Process Rx.
 *                - eIPC_CMD_UART_RX_OVERFLOW :
 *                   Sent to UART Manager by UART Callback, the UART Manager then
 *                   performs error handling inside Process Overflow
 *                - eIPC_CMD_UART_TX_SEND_DATA :
 *                   Sent to UART Manager by a valid applciation. The UART manager
 *                   then passes it over to Process Tx.
 * @param[in]     vpt_Argument : Reserved task argument.
 * @return        none
 */
static void sv_UARTManager_Task(void *vpt_Argument)
{
   (void)vpt_Argument;

   IPCMessage_T st_IPCEvent;

   while (TRUE)
   {
      if (ql_rtos_queue_wait(sst_UARTManagerContext.pt_Queue,
                             (uint8_t *)&st_IPCEvent,
                             sizeof(IPCMessage_T),
                             QL_WAIT_FOREVER) != QL_OSI_SUCCESS)
      {
#ifdef DEBUG_ENABLED
         BAL_UART_SERVICE_LOG("Pend on UART Event Manager Queue Failed");
#endif // DEBUG_ENABLED
      }

      switch (st_IPCEvent.eCmd)
      {
         case eIPC_CMD_UART_RX_DATA_AVAILABLE:
            sv_UARTManager_ProcessRx(&st_IPCEvent);
            break;

         case eIPC_CMD_UART_RX_OVERFLOW:
            sv_UARTManager_ProcessOverflow(&st_IPCEvent);
            break;

         case eIPC_CMD_UART_TX_SEND_DATA:
            sv_UARTManager_ProcessTx(&st_IPCEvent);
            break;

         default:
            break;
      }
   }
}

/**
 * @private       sv_UARTManager_ProcessRx
 * @brief         Stub for RX-byte acquisition, parser invocation, validation,
 *                metadata ACK handling and application routing.
 * @return        none
 */
static void sv_UARTManager_ProcessRx(IPCMessage_T* ptst_IPCEvent)
{
   TelSysResponse_E e_status = eTEL_NOK;
   //  Parse the incoming UART Frame
   UartFrame_U* upt_rxUartFrame = (UartFrame_U*) ptst_IPCEvent->u8a_Data;
   uint8_t u8_rxDataExpectedCksum = 0;
   uint32_t u32_rxDataLength = 0U;
   uint8_t* u8pt_rxData = NULL;
   UARTFrameType_E e_FrameType = eUART_FRAME_TYPE_GENERAL;
   Application_E e_appID = eAPPLICATION_LAST_APPLICATION;
   uint32_t u32_expectedDataLength = 0U;
   UARTOriginator_E e_frameOrigin = eUART_ORIGINATOR_MODEM;
   UARTIntent_E e_frameIntent = eUART_INTENT_LAST_INTENT;
   uint16_t u16_rxServiceID = 0U;
   uint8_t u8_payloadLen = 0U;

   //  First determine the type of frame
   if(ptst_IPCEvent->tDataLength == UART_GENERAL_FRAME_SIZE)
   {
#ifdef DEBUG_ENABLED
      BAL_UART_SERVICE_LOG("General or Metadata Frame or Metadata ACK/ NACK is received");
#endif // DEBUG_ENABLED
      // General or Metadata Frame or Metadata ACK/ NACK is received
      u8_rxDataExpectedCksum = upt_rxUartFrame->st_uartGenMetaFrame.u8_cksum;
      u32_rxDataLength = UART_GENERAL_FRAME_CKSUM_LENGTH;
      u8pt_rxData = (ptst_IPCEvent->u8a_Data + UART_GENERAL_FRAME_CKSUM_OFFSET);
      e_FrameType = upt_rxUartFrame->st_uartGenMetaFrame.eFrameType;
      e_appID = upt_rxUartFrame->st_uartGenMetaFrame.eApplication;
      e_frameOrigin = upt_rxUartFrame->st_uartGenMetaFrame.eOriginator;
      e_frameIntent = upt_rxUartFrame->st_uartGenMetaFrame.eIntent;
      u16_rxServiceID = upt_rxUartFrame->st_uartGenMetaFrame.u16_FrameId;
      u8_payloadLen = upt_rxUartFrame->st_uartGenMetaFrame.u8_PayloadLength;

      if((((e_FrameType == eUART_FRAME_TYPE_GENERAL) && (u8_payloadLen <= UART_GENERAL_PAYLOAD_SIZE))
      || ((e_FrameType == eUART_FRAME_TYPE_METADATA) && (u8_payloadLen == UART_METADATA_PAYLOAD_SIZE)))
         && (e_appID < eAPPLICATION_LAST_APPLICATION)
         && (e_frameOrigin == eUART_ORIGINATOR_RH850)
         && (e_frameIntent < eUART_INTENT_LAST_INTENT)
         && (u16_rxServiceID < eSERVICE_ID_MAX_CONFIGURED_ID))
      {
         // Correct Frame received, proceed to validate the checksum and then route it to application
      }
      else
      {
#ifdef DEBUG_ENABLED
         BAL_UART_SERVICE_LOG("Invalid Frame Type received : %d, Application ID: %d, Originator: %d, Intent: %d, Payload Length: %d",
            e_FrameType, e_appID, e_frameOrigin, e_frameIntent, u8_payloadLen);
         BAL_UART_SERVICE_LOG("Or Invalid Service ID received : %d", u16_rxServiceID);
#endif // DEBUG_ENABLED
         // Invalid Frame Type received, log error and ignore the frame
         // sv_UARTManager_ClearTxContext();
         sv_UARTManager_ClearRxContext();
         return;
      }
   }
   else
   {
#ifdef DEBUG_ENABLED
      BAL_UART_SERVICE_LOG("Probable Data Frame received");
#endif // DEBUG_ENABLED
      // Data Frame is received
      u8_rxDataExpectedCksum = sst_UARTManagerContext.st_rxContext.u8_dataFrameCksum;
      u32_rxDataLength = sst_UARTManagerContext.st_rxContext.u32_expectedRxDataLength - 2U;
      u8pt_rxData = (ptst_IPCEvent->u8a_Data + UART_DATA_FRAME_CKSUM_OFFSET);
      e_FrameType = eUART_FRAME_TYPE_DATA;
   }

   // Validate the incoming frame CRC
   e_status = ge_Crypto_ValidateCksm(u8pt_rxData, u32_rxDataLength, u8_rxDataExpectedCksum);

   if(e_status == eTEL_OK)
   {
#ifdef DEBUG_ENABLED
      BAL_UART_SERVICE_LOG(" CRC Validation Passed");
#endif // DEBUG_ENABLED
      // Frame checksum valid
      if(e_FrameType == eUART_FRAME_TYPE_GENERAL)
      {
#ifdef DEBUG_ENABLED
      BAL_UART_SERVICE_LOG(" General Frame received routed to application");
#endif // DEBUG_ENABLED
         // Send it to correct application
         gv_UARTManager_RouteToApp(ptst_IPCEvent, e_FrameType, e_appID, u16_rxServiceID, u8_payloadLen);
         sv_UARTManager_ClearRxContext();
      }
      else if(e_FrameType == eUART_FRAME_TYPE_METADATA)
      {
         // Check if its Metadata Request or an ACK
         if(upt_rxUartFrame->st_uartGenMetaFrame.u16_FrameId == eSERVICE_ID_CTS)
         {
            // This is a postive CTS, proceed to send Data Frame
            // Send the Data Frame and Inform requester application that data sending was successful and then clear the UART TX Context.
            if(sst_UARTManagerContext.st_txContext.e_txStatus == UART_TX_META_DATA_ACK_WAIT)
            {
#ifdef DEBUG_ENABLED
      BAL_UART_SERVICE_LOG("ACK so send Data frame");
#endif // DEBUG_ENABLED
               sst_UARTManagerContext.st_txContext.e_txStatus = UART_TX_BULK_DATA_TX_BEGIN;
               // TBD: update this function
               e_status = se_UARTManager_SendDataFrame();

               if( e_status == eTEL_OK)
               {
                  // Data Frame sent successfully, inform the source application of this and clear the TX context
                  ptst_IPCEvent->eStatus = eIPC_STATUS_OK;
                  gv_UARTManager_RouteToApp(ptst_IPCEvent, eUART_FRAME_TYPE_METADATA, e_appID, eSERVICE_ID_DATA_SEND_ACK, u8_payloadLen);
               }
               else
               {
                  // Data Frame Send Failed, inform the source application of this and clear the TX context
                  ptst_IPCEvent->eStatus = eIPC_STATUS_ERROR;
                  gv_UARTManager_RouteToApp(ptst_IPCEvent, eUART_FRAME_TYPE_METADATA, e_appID, eSERVICE_ID_DATA_SEND_NACK, u8_payloadLen);
               }
               sv_UARTManager_ClearTxContext();
            }
            else
            {
               // Unexpected CTS received, log error and ignore it.
#ifdef DEBUG_ENABLED
      BAL_UART_SERVICE_LOG("Unexpected CTS received, log error and ignore it");
#endif // DEBUG_ENABLED
            }
         }
         else if(upt_rxUartFrame->st_uartGenMetaFrame.u16_FrameId ==
                 (uint16_t)(sst_UARTManagerContext.st_txContext.u16_txServiceID + 0x8000U))
         {
#ifdef DEBUG_ENABLED
      BAL_UART_SERVICE_LOG("NACK so no data frame");
#endif // DEBUG_ENABLED
            // This is a negative CTS, pass the payload to requester application
            ptst_IPCEvent->eStatus = eIPC_STATUS_ERROR;
            gv_UARTManager_RouteToApp(ptst_IPCEvent, eUART_FRAME_TYPE_METADATA, e_appID, u16_rxServiceID, u8_payloadLen);
            sv_UARTManager_ClearTxContext();
         }
         else
         {
#ifdef DEBUG_ENABLED
      BAL_UART_SERVICE_LOG("New Metadata frame obtained");
#endif // DEBUG_ENABLED
            // New Metadata frame obtained
            // Update UART Context and send postive ACK
            sst_UARTManagerContext.st_rxContext.e_rxStatus = UART_RX_META_DATA;
            sst_UARTManagerContext.st_rxContext.u16_rxAppId = upt_rxUartFrame->st_uartGenMetaFrame.eApplication;
            sst_UARTManagerContext.st_rxContext.u16_rxServiceID = upt_rxUartFrame->st_uartGenMetaFrame.u16_FrameId;


            // Calculate and validate the expected data length
            u32_expectedDataLength =
                  ((uint32_t)upt_rxUartFrame->st_uartGenMetaFrame.au8_Payload[0] << 8U) |
                   (uint32_t)upt_rxUartFrame->st_uartGenMetaFrame.au8_Payload[1];
            if(u32_expectedDataLength > UART_DATA_FRAME_MAX_SIZE)
            {
#ifdef DEBUG_ENABLED
      BAL_UART_SERVICE_LOG("New Metadata frame obtained with invalid data length");
#endif // DEBUG_ENABLED
               (void)se_UARTManager_SendMetadataAck(ptst_IPCEvent, FALSE);
               // Invalid data length, log error and ignore the frame
               sst_UARTManagerContext.st_rxContext.e_rxStatus = UART_RX_IDLE;
               sv_UARTManager_ClearRxContext();
            }
            else
            {
#ifdef DEBUG_ENABLED
      BAL_UART_SERVICE_LOG("Valid metadata frame obtained, expected data length: %d", u32_expectedDataLength);
#endif // DEBUG_ENABLED
               sst_UARTManagerContext.st_rxContext.u32_expectedRxDataLength = u32_expectedDataLength + 2; // Add 2 for SOF and EOF

               sst_UARTManagerContext.st_rxContext.u8_dataFrameCksum =
                                 upt_rxUartFrame->st_uartGenMetaFrame.au8_Payload[2];
               sst_UARTManagerContext.st_rxContext.u16_rxAppId = e_appID;
               sst_UARTManagerContext.st_rxContext.u16_rxServiceID = u16_rxServiceID;
               e_status = se_UARTManager_SendMetadataAck(ptst_IPCEvent, TRUE);
               if( e_status == eTEL_OK)
               {
                  sst_UARTManagerContext.st_rxContext.e_rxStatus = UART_RX_BULK_DATA_RX_WAIT;
               }
               else
               {
                  // Metadata ACK send failed, log the error, cant inform source application
                  sv_UARTManager_ClearRxContext();
               }
            }

         }
      }
      else
      {
         // Valid Data Frame received, route it to application and clear the RX context
         sst_UARTManagerContext.st_rxContext.e_rxStatus = UART_RX_IDLE;

         e_appID = sst_UARTManagerContext.st_rxContext.u16_rxAppId;
         u16_rxServiceID = sst_UARTManagerContext.st_rxContext.u16_rxServiceID;
         u8_payloadLen = sst_UARTManagerContext.st_rxContext.u32_expectedRxDataLength - 2U;

         gv_UARTManager_RouteToApp(ptst_IPCEvent, e_FrameType, e_appID, u16_rxServiceID, u8_payloadLen);
         sv_UARTManager_ClearRxContext();
      }
   }
   else
   {
      //  Invalid frame checksum
      if(e_FrameType == eUART_FRAME_TYPE_GENERAL)
      {
#ifdef DEBUG_ENABLED
      BAL_UART_SERVICE_LOG("General Frame CRC Validation Failed, send NACK to RH850");
#endif // DEBUG_ENABLED
         // Send negative response over UART to RH850
         se_UARTManager_SendGeneralAck(ptst_IPCEvent, FALSE);
      }
      else if(e_FrameType == eUART_FRAME_TYPE_METADATA)
      {
         // Check if its Metadata Request or an ACK
         if((upt_rxUartFrame->st_uartGenMetaFrame.u16_FrameId == eSERVICE_ID_CTS) ||
         (upt_rxUartFrame->st_uartGenMetaFrame.u16_FrameId == (sst_UARTManagerContext.st_rxContext.u16_rxServiceID + 0x8000)))
         {
#ifdef DEBUG_ENABLED
      BAL_UART_SERVICE_LOG("Metadata CTS Frame CRC Validation Failed, send NACK to RH850");
#endif // DEBUG_ENABLED
            // CTS reception failed, due to error in CRC, inform the source application of this.
            ptst_IPCEvent->eStatus = eIPC_STATUS_ERROR;
            gv_UARTManager_RouteToApp(ptst_IPCEvent, eUART_FRAME_TYPE_METADATA, e_appID, u16_rxServiceID, 0);
            sv_UARTManager_ClearTxContext();
         }
         else
         {
#ifdef DEBUG_ENABLED
      BAL_UART_SERVICE_LOG("New Metadata Frame CRC Validation Failed, send NACK to RH850");
#endif // DEBUG_ENABLED
            // New Metadata frame obtained with wrong CRC
            (void)se_UARTManager_SendMetadataAck(ptst_IPCEvent, FALSE);
         }
      }
      else
      {
#ifdef DEBUG_ENABLED
      BAL_UART_SERVICE_LOG("Incorrect Data Frame CRC Validation Failed, send NACK to RH850");
#endif // DEBUG_ENABLED
         // Incorrect Data Frame, send NACK to RH850
         se_UARTManager_SendMetadataAck(ptst_IPCEvent, FALSE);
         // Clear RX UART Conetxt
         sv_UARTManager_ClearRxContext();
      }

   }

}

/**
 * @private       sv_UARTManager_ProcessTx
 * @brief         Stub for TX queue processing and frame serialization.
 * @return        none
 */
static void sv_UARTManager_ProcessTx(IPCMessage_T* ptst_IPCEvent)
{

#ifdef DEBUG_ENABLED
      BAL_UART_SERVICE_LOG("UART TX Processing...");
#endif
   uint16_t u16_txDataLength = ptst_IPCEvent->tDataLength;


#ifdef DEBUG_ENABLED
      BAL_UART_SERVICE_LOG("Data Length Received:%d and Data : %s", ptst_IPCEvent->tDataLength, ptst_IPCEvent->u8a_Data);
#endif
   //  First determine the type of frame
   if(u16_txDataLength <= UART_GENERAL_PAYLOAD_SIZE)
   {
#ifdef DEBUG_ENABLED
      BAL_UART_SERVICE_LOG("General Frame Tx");
#endif
      // General frame, compose the frame and send out immediately
      sv_UARTManager_ComposeSendGeneralFrame(ptst_IPCEvent);
   }
   else
   {
#ifdef DEBUG_ENABLED
      BAL_UART_SERVICE_LOG("Metdata Data Frame Tx");
#endif
      // Compose and send metadata
      sv_UARTManager_ComposeDataFrameSave(ptst_IPCEvent); // Update UART Tx Conext Inside of this
      sv_UARTManager_ComposeSendMetadataFrame(ptst_IPCEvent);
   }
}

/**
 * @private       sv_UARTManager_ProcessOverflow
 * @brief         Stub for RX overflow recovery and error reporting.
 * @return        none
 */
static void sv_UARTManager_ProcessOverflow(IPCMessage_T* ptst_IPCEvent)
{
   (void)ptst_IPCEvent;
   u32_currRxPtr = 0U;
   (void)memset(u8a_receivedDataTempBuff, 0, sizeof(u8a_receivedDataTempBuff));
   sv_UARTManager_ClearRxContext();
   // TBD: Log error event
}



static uint8_t su8_UARTManager_DetermineAppID(IPCNodes_E e_sourceNode)
{
   return 1; // @R: TBD: This is a stub, to be updated with actual application ID mapping
}

/******************************************************************************/
/*                        PUBLIC FUNCTION DEFINITIONS                         */
/******************************************************************************/
/**
 * @public        gv_UARTManager_Init
 * @brief         Initializes the UART port, manager queue, manager task and
 *                driver callback in that order.
 * @param[in]     none
 * @return        none
 */
TelSysResponse_E gv_UARTManager_Init(void)
{
   // Local Variable to indicate the UART Init Failure
   uint8_t u8_errorInit = 0;
   // Reset the entire UART Manager Context
   (void)memset(&sst_UARTManagerContext, 0, sizeof(sst_UARTManagerContext));

   // UART Port Initialization
   if (ge_UARTPort_Init() == eTEL_OK)
   {
#ifdef DEBUG_ENABLED
      BAL_UART_SERVICE_LOG("UART Init Successful!");
#endif // DEBUG_ENABLED
   }
else
   {
#ifdef DEBUG_ENABLED
      BAL_UART_SERVICE_LOG("UART Init Failed!");
#endif // DEBUG_ENABLED
      u8_errorInit |= (UART_INIT_FAILURE);
   }

   // Register the UART Callback
   if(ge_UARTPort_RegisterCallback(sv_UARTManager_Callback) == eTEL_OK)
   {
#ifdef DEBUG_ENABLED
      BAL_UART_SERVICE_LOG("UART Callback Regsiteration Success");
#endif // DEBUG_ENABLED
   }
else
   {
#ifdef DEBUG_ENABLED
      BAL_UART_SERVICE_LOG("UART Callback Regsiteration Fail");
#endif // DEBUG_ENABLED
      u8_errorInit |= (UART_CB_REG_FAILURE);
   }


   // Create a queue for UART Manager Task
   if(ql_rtos_queue_create(&sst_UARTManagerContext.pt_Queue, sizeof(IPCMessage_T),
                              UART_MANAGER_QUEUE_LENGTH) == QL_OSI_SUCCESS)
   {
#ifdef DEBUG_ENABLED
      BAL_UART_SERVICE_LOG("UART Manager Queue Init Success");
#endif // DEBUG_ENABLED
      ge_Common_RegisterIPCQueue(eIPC_NODE_SVC_UART, sst_UARTManagerContext.pt_Queue);
   }
else
   {
#ifdef DEBUG_ENABLED
      BAL_UART_SERVICE_LOG("UART Manager Queue Init Fail");
#endif // DEBUG_ENABLED
      u8_errorInit |= (UART_Q_INIT_FAILURE);
   }

   // Register the UART Manager Task
   if(ql_rtos_task_create(&sst_UARTManagerContext.pt_Task, UART_MANAGER_TASK_STACK_SIZE,
         UART_MANAGER_TASK_PRIORITY, "BALUARTMGR", sv_UARTManager_Task, NULL,
         UART_MANAGER_TASK_EVENT_COUNT) == QL_OSI_SUCCESS)
   {
#ifdef DEBUG_ENABLED
      BAL_UART_SERVICE_LOG("UART Manager Task Init Success");
#endif // DEBUG_ENABLED
   }
else
   {
#ifdef DEBUG_ENABLED
      BAL_UART_SERVICE_LOG("UART Manager Task Init Fail");
#endif // DEBUG_ENABLED
      u8_errorInit |= (UART_TASK_REG_FAILURE);
   }

   if(u8_errorInit)
   {
      sst_UARTManagerContext.b_Initialized = FALSE;
      sst_UARTManagerContext.u8_failureCode = u8_errorInit;
   }
   else
   {
      sst_UARTManagerContext.b_Initialized = TRUE;
      sst_UARTManagerContext.u8_failureCode = 0;
      sst_UARTManagerContext.st_rxContext.u32_expectedRxDataLength = UART_GENERAL_FRAME_SIZE;
   }

   return sst_UARTManagerContext.b_Initialized ? eTEL_OK : eTEL_NOK;
}


/**
 * @public        ge_UART_EnqueueTx
 * @brief         Public function to enqueue request to UART Manager
 * @return        eTEL_OK if data enqueue successful.
 *                eTEL_NOK if data enqueue failed.
 */
TelSysResponse_E ge_UART_EnqueueTx(IPCMessage_T* stpt_intraUARTIPCMsg)
{
   TelSysResponse_E eRetVal = eTEL_NOK;
   QlOSStatus e_status;

   if ((!sst_UARTManagerContext.b_Initialized) || (stpt_intraUARTIPCMsg == NULL) ||
       (stpt_intraUARTIPCMsg->tDataLength > IPC_MAX_DATA_LENGTH))
   {
      e_status =  eTEL_NOK;
   }
   else
   {
      e_status = ql_rtos_queue_release(sst_UARTManagerContext.pt_Queue,
         sizeof(IPCMessage_T), (uint8_t *)stpt_intraUARTIPCMsg, QL_WAIT_FOREVER);

      if(e_status == QL_OSI_SUCCESS)
      {
#ifdef DEBUG_ENABLED
         BAL_UART_SERVICE_LOG("UART Message Enqueue Success");
#endif
         eRetVal = eTEL_OK;
      }
      else
      {
#ifdef DEBUG_ENABLED
         BAL_UART_SERVICE_LOG("UART Message Enqueue Failed");
      }
#endif
   }

   return eRetVal;
}

/**
 * Copyright(c) Bajaj Auto Technology Limited (BATL) as an unpublished work.
 * THIS SOFTWARE AND/OR MATERIAL IS THE PROPERTY OF BATL.
 */
