/**
 * @file          PROVManager.c
 * @brief         Source file containing the implementation for Provisioning
 *                Manager
 * @date          10/08/26
 * @author        Yash Giramkar
 * @copyright     Bajaj Auto Technology Limited (BATL)
 */

/******************************************************************************/
/*                                                                            */
/*                                  INCLUDES                                  */
/*                                                                            */
/******************************************************************************/
#include "PROVManager.h"

/******************************************************************************/
/*                                                                            */
/*                                  DEFINES                                   */
/*                                                                            */
/******************************************************************************/
/**
 * @def           PROV_MANAGER_QUEUE_LENGTH
 * @brief         Defines the length of the queue for the Provisioning Manager task.
 */
#define PROV_MANAGER_QUEUE_LENGTH            8U

/**
 * @def           PROV_MANAGER_TASK_STACK_SIZE
 * @brief         Defines the stack size for the Provisioning Manager task.
 */
#define PROV_MANAGER_TASK_STACK_SIZE         16*1024

/**
 * @def           PROV_MANAGER_TASK_PRIORITY
 * @brief         Defines the priority for the Provisioning Manager task.
 */
#define PROV_MANAGER_TASK_PRIORITY           APP_PRIORITY_NORMAL

/**
 * @def           PROV_MANAGER_TASK_EVENT_COUNT
 * @brief         Defines the number of events for the Provisioning Manager task.
 */
#define PROV_MANAGER_TASK_EVENT_COUNT        8U

/**
 * @def           PROV_NEG_ACK_LENGTH
 * @brief         Negative Acknowledgement length.
 */
#define PROV_NEG_ACK_LENGTH                  1U

/**
 * @def           PROV_NEG_ACK_DATA
 * @brief         Negative Acknowledgement data.
 */
#define PROV_NEG_ACK_DATA                    0x7F

/**
 * @def           PROV_POS_ACK_LENGTH
 * @brief         Positive Acknowledgement length.
 */
#define PROV_POS_ACK_LENGTH                  1U

/**
 * @def           PROV_POS_ACK_WITH_ADDITIONAL_DATA_LENGTH
 * @brief         Positive Acknowledgement length with additional data.
 */
#define PROV_POS_ACK_WITH_ADDITIONAL_DATA_LENGTH     3U

/**
 * @def           PROV_POS_ACK_DATA
 * @brief         Positive Acknowledgement data.
 */
#define PROV_POS_ACK_DATA                    0x40




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
typedef enum
{
   DEV_PROV_WAIT_FOR_VIN = 0,
   DEV_PROV_WAIT_FOR_PKI_REQ,
   DEV_PROV_WAIT_FOR_CSR_BLK_REQ,
   DEV_PROV_WAIT_FOR_ROOT_CA_SIZE,
   DEV_PROV_WAIT_FOR_ROOT_CA_BLK_RECP,
   DEV_PROV_WAIT_FOR_DEV_CERT_SIZE,
   DEV_PROV_WAIT_FOR_DEV_CERT_BLK_RECP,
   DEV_PROV_WAIT_FOR_VIN_ALIAS,
   DEV_PROV_WAIT_FOR_DEVICE_ID,
   DEV_PROV_COMPLETED,
}PROVStates_E;

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

typedef struct
{
   ql_task_t pt_Task;
   ql_queue_t pt_Queue;
   PROVStatus_E e_deviceProvStatus;
   PROVStates_E e_deviceProvSMState;
   bool b_responseRequired;
} PROVManagerContext_T;

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
static void sv_PROVManager_Task(void *vpt_Argument);
static void sv_PROV_ChangeState(PROVStates_E e_assignedState);
static void sv_PROV_GeneratePositiveAck(IPCMessage_T* stpt_IPCEvent);
static void sv_PROV_GenerateNegativeAck(IPCMessage_T* stpt_IPCEvent);
static TelSysResponse_E se_PROV_ReadDevCertsAndKeys(void);
static TelSysResponse_E se_PROV_WriteDevCertsAndKeys(void);
static void sv_PROV_CheckDevProvisioningStatus(void);


static void sv_PROV_WaitForVIN(IPCMessage_T* stpt_IPCEvent);
static void sv_PROV_WaitForPKI(IPCMessage_T* stpt_IPCEvent);
static void sv_PROV_WaitForCSRBlockReq(IPCMessage_T* stpt_IPCEvent);
static void sv_PROV_WaitForRootCASize(IPCMessage_T* stpt_IPCEvent);
static void sv_PROV_WaitForRootCABlockRecp(IPCMessage_T* stpt_IPCEvent);
static void sv_PROV_WaitForDeviceCertSize(IPCMessage_T* stpt_IPCEvent);
static void sv_PROV_WaitForDeviceCertBlockRecp(IPCMessage_T* stpt_IPCEvent);
static void sv_PROV_WaitForVINAlias(IPCMessage_T* stpt_IPCEvent);
static void sv_PROV_WaitForDeviceID(IPCMessage_T* stpt_IPCEvent);
static void sv_PROV_CompletedState(IPCMessage_T* stpt_IPCEvent);

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
 * @var           sst_provManagerCtx
 * @brief         Static variable to maintain the current context of Provisioning
 *                Manager.
 */
static PROVManagerContext_T sst_provManagerCtx = {0};

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
 * @private       sv_PROVManager_Task
 * @brief         Task function for the Provisioning Manager.
 * @param[in]     vpt_Argument : Reserved task argument.
 * @param[out]    None
 * @param[inout]  None
 * @return        none
 */
static void sv_PROVManager_Task(void *vpt_Argument)
{
   (void)vpt_Argument;
   IPCMessage_T st_IPCEvent;
   ql_queue_t pt_Queue = NULL;
   TelSysResponse_E eRetVal = eTEL_NOK;

   sst_provManagerCtx.b_responseRequired = TRUE;

   while(TRUE)
   {
      // Wait for Queue population event indefinitely
      if (ql_rtos_queue_wait(sst_provManagerCtx.pt_Queue,
                             (uint8_t *)&st_IPCEvent,
                             sizeof(IPCMessage_T),
                             QL_WAIT_FOREVER) != QL_OSI_SUCCESS)
      {
#ifdef DEBUG_ENABLED
         BAL_PROV_LOG("Pend on Provisioning Manager Queue Failed");
#endif // DEBUG_ENABLED
      }
      sst_provManagerCtx.b_responseRequired = TRUE;
      // Now the state machine to follow, will depend upon the current state of
      // Provisioning manager, instead of just incoming message command and service ID
      switch (sst_provManagerCtx.e_deviceProvSMState)
      {
         case DEV_PROV_WAIT_FOR_VIN:
         {
            sv_PROV_WaitForVIN(&st_IPCEvent);
         }
         break;
         case DEV_PROV_WAIT_FOR_PKI_REQ:
         {
            sv_PROV_WaitForPKI(&st_IPCEvent);
         }
         break;
         case DEV_PROV_WAIT_FOR_CSR_BLK_REQ:
         {
            sv_PROV_WaitForCSRBlockReq(&st_IPCEvent);
         }
         break;
         case DEV_PROV_WAIT_FOR_ROOT_CA_SIZE:
         {
            sv_PROV_WaitForRootCASize(&st_IPCEvent);
         }
         break;
         case DEV_PROV_WAIT_FOR_ROOT_CA_BLK_RECP:
         {
            sv_PROV_WaitForRootCABlockRecp(&st_IPCEvent);
         }
         break;
         case DEV_PROV_WAIT_FOR_DEV_CERT_SIZE:
         {
            sv_PROV_WaitForDeviceCertSize(&st_IPCEvent);
         }
         break;
         case DEV_PROV_WAIT_FOR_DEV_CERT_BLK_RECP:
         {
            sv_PROV_WaitForDeviceCertBlockRecp(&st_IPCEvent);
         }
         break;
         case DEV_PROV_WAIT_FOR_VIN_ALIAS:
         {
            sv_PROV_WaitForVINAlias(&st_IPCEvent);
         }
         break;
         case DEV_PROV_WAIT_FOR_DEVICE_ID:
         {
            sv_PROV_WaitForDeviceID(&st_IPCEvent);
         }
         break;
         case DEV_PROV_COMPLETED:
         {
            sv_PROV_CompletedState(&st_IPCEvent);
         }
         break;
         default:
         {
#ifdef DEBUG_ENABLED
            BAL_PROV_LOG("Unkown state of Provisioning manager reached");
#endif
         }
         break;
      }

      // Check if a response if required and send the response
      if(sst_provManagerCtx.b_responseRequired == TRUE)
      {
         // Send the response back to RH850
         st_IPCEvent.eSourceNode = eIPC_NODE_APP_PROVISIONING;
         st_IPCEvent.eDestNode = eIPC_NODE_SVC_UART;
         st_IPCEvent.eCmd = eIPC_CMD_UART_TX_SEND_DATA;

         /** We would be responding back with same Command,Transaction ID and
          *  Service ID as received in the request. So no need to update.*/

         st_IPCEvent.eMsgType = eIPC_MSG_RESPONSE;
         st_IPCEvent.eFlags = eIPC_FLAG_NONE;

         eRetVal = ge_Common_GetIPCQueue(eIPC_NODE_SVC_UART, &pt_Queue);
         if(eRetVal == eTEL_OK)
         {
            if (ql_rtos_queue_release(pt_Queue, sizeof(IPCMessage_T),
                     (uint8_t *)&st_IPCEvent, QL_WAIT_FOREVER) != QL_OSI_SUCCESS)
            {
   #ifdef DEBUG_ENABLED
               BAL_PROV_LOG("Send IPC Event to UART Manager Queue Failed");
   #endif // DEBUG_ENABLED
            }
            else
            {
   #ifdef DEBUG_ENABLED
               BAL_PROV_LOG("Send IPC Event to UART Manager Queue Success");
   #endif // DEBUG_ENABLED
            }
         }
         else
         {
   #ifdef DEBUG_ENABLED
            BAL_PROV_LOG("Get UART Manager Queue Failed");
   #endif // DEBUG_ENABLED
         }
      }
      else
      {
         // Response Not required
      }

   }
}


/**
 * @private       sv_PROV_ChangeState
 * @brief         This function changes the state of the provisioning state
 *                machine
 * @param[in]     e_assignedState
 * @param[out]    None
 * @param[inout]  None
 * @return        None
 * @note          The function goes ahead and updates the global context of
 *                device provisioning manager inside sst_provManagerCtx
 */
static void sv_PROV_ChangeState(PROVStates_E e_assignedState)
{
   sst_provManagerCtx.e_deviceProvSMState = e_assignedState;
}


/**
 * @private       sv_PROV_GeneratePositiveAck
 * @brief         This function generate the positive acknowledgement from
 *                Provisioning application.
 * @param[in]     None
 * @param[out]    None
 * @param[inout]  stpt_IPCEvent Pointer to IPC Event
 * @return        None
 * @note          The function goes ahead and updates the global context of
 *                device provisioning manager inside sst_provManagerCtx
 */
static void sv_PROV_GeneratePositiveAck(IPCMessage_T* stpt_IPCEvent)
{
   stpt_IPCEvent->tDataLength = PROV_POS_ACK_LENGTH;
   stpt_IPCEvent->u8a_Data[0] = PROV_POS_ACK_DATA;
}

/**
 * @private       sv_PROV_GenerateNegativeAck
 * @brief         This function generate the negative acknowledgement from
 *                Provisioning application.
 * @param[in]     None
 * @param[out]    None
 * @param[inout]  stpt_IPCEvent Pointer to IPC Event
 * @return        None
 * @note          The function goes ahead and updates the global context of
 *                device provisioning manager inside sst_provManagerCtx
 */
static void sv_PROV_GenerateNegativeAck(IPCMessage_T* stpt_IPCEvent)
{
   stpt_IPCEvent->eServiceID += 0x8000;
   stpt_IPCEvent->tDataLength = PROV_NEG_ACK_LENGTH;
   stpt_IPCEvent->u8a_Data[0] = PROV_NEG_ACK_DATA;
}

/**
 * @private       se_PROV_ReadDevCertsAndKeys
 * @brief         Reads all provisioning data from the file system into RAM.
 * @details       The files are read in the following order: VIN, VIN alias,
 *                device ID, public key, private key, CSR, Root CA, and device
 *                certificate. A short read reported as eFS_EOF is accepted.
 * @param[in]     None
 * @param[out]    None
 * @param[inout]  None
 * @return        eTEL_OK if all files are read successfully; otherwise
 *                eTEL_NOK.
 * @note          The function updates the global provisioning RAM buffers.
 *
 */
static TelSysResponse_E se_PROV_ReadDevCertsAndKeys(void)
{
   typedef struct
   {
      FileSystemID_E e_fileId;
      void *vpt_buffer;
      uint32_t u32_size;
      const char *ccpt_name;
   } PROVFileRead_T;

   static const PROVFileRead_T st_fileReads[] =
   {
      {eFS_FILE_VIN,             (void*)gca_VIN,         PROV_VIN_MAX_SIZE,         "VIN"},
      {eFS_FILE_VIN_ALIAS,       (void*)gca_vinAlias,    PROV_VIN_ALIAS_MAX_SIZE,   "VIN alias"},
      {eFS_FILE_DEV_ID,          (void*)gca_devID,       PROV_DEV_ID_MAX_SIZE,      "device ID"},
      {eFS_FILE_DEV_PUBLIC_KEY,  (void*)gca_publicKey,  PROV_PUBLIC_KEY_MAX_SIZE,  "public key"},
      {eFS_FILE_DEV_PRIVATE_KEY, (void*)gca_privateKey, PROV_PRIVATE_KEY_MAX_SIZE, "private key"},
      {eFS_FILE_DEV_CSR,         (void*)gca_CSR,         PROV_CSR_MAX_SIZE,          "CSR"},
      {eFS_FILE_ROOT_CA,         (void*)gca_rootCA,      PROV_ROOT_CA_MAX_SIZE,      "root CA"},
      {eFS_FILE_DEV_CERT,        (void*)gca_deviceCert,  PROV_DEV_CERT_MAX_SIZE,    "device certificate"}
   };
   FileSystemReq_T st_fsReq = {0};
   FileSystemResult_E e_fsResult = eFS_INTERNAL_ERROR;
   TelSysResponse_E e_retVal = eTEL_NOK;
   uint32_t u32_index = 0U;

   if(ge_FS_IsFilePresent(eFS_FILE_VIN) == eFS_OK)
   {
      for(u32_index = 0U; u32_index < (sizeof(st_fileReads) / sizeof(st_fileReads[0])); u32_index++)
      {
         st_fsReq.e_FileId = st_fileReads[u32_index].e_fileId;
         st_fsReq.u32_RequestedBytes = st_fileReads[u32_index].u32_size;
         st_fsReq.vpt_Buffer = st_fileReads[u32_index].vpt_buffer;

         e_fsResult = ge_FS_Read(&st_fsReq);
         if((e_fsResult != eFS_OK) && (e_fsResult != eFS_EOF))
         {
            #ifdef DEBUG_ENABLED
               BAL_PROV_LOG("Error reading %s file", st_fileReads[u32_index].ccpt_name);
            #endif
            break;
         }
      }

      if(u32_index == (sizeof(st_fileReads) / sizeof(st_fileReads[0])))
      {
         e_retVal = eTEL_OK;
      }
   }

   return e_retVal;
}

/**
 * @private       se_PROV_WriteDevCertsAndKeys
 * @brief         Writes all provisioning data from RAM to the file system.
 * @details       The files are written in the following order: VIN, VIN alias,
 *                device ID, public key, private key, CSR, Root CA, and device
 *                certificate. Each file is created before it is written and
 *                the number of written bytes is verified.
 * @param[in]     None
 * @param[out]    None
 * @param[inout]  None
 * @return        eTEL_OK if all files are written successfully; otherwise
 *                eTEL_NOK.
 * @note          The function persists the global provisioning RAM buffers.
 *
 */
static TelSysResponse_E se_PROV_WriteDevCertsAndKeys(void)
{
   typedef struct
   {
      FileSystemID_E e_fileId;
      void *vpt_buffer;
      uint32_t u32_size;
   } PROVFileWrite_T;

const PROVFileWrite_T st_fileWrites[] =
{
   {eFS_FILE_VIN,             (void*)gca_VIN,         sizeof(gca_VIN)},
   {eFS_FILE_VIN_ALIAS,       (void*)gca_vinAlias,    sizeof(gca_vinAlias)},
   {eFS_FILE_DEV_ID,          (void*)gca_devID,       sizeof(gca_devID)},
   {eFS_FILE_DEV_PUBLIC_KEY,  (void*)gca_publicKey,   sizeof(gca_publicKey)},
   {eFS_FILE_DEV_PRIVATE_KEY, (void*)gca_privateKey,  sizeof(gca_privateKey)},
   {eFS_FILE_DEV_CSR,         (void*)gca_CSR,         gu16_sizeOfCSR},
   {eFS_FILE_ROOT_CA,         (void*)gca_rootCA,      gu16_sizeOfRootCA},
   {eFS_FILE_DEV_CERT,        (void*)gca_deviceCert,  gu16_sizeOfDevCert},
   {eFS_FILE_DEV_PROV_STATUS, (void*)&ge_provStatus,  sizeof(PROVStatus_E)}

};
   FileSystemReq_T st_fsReq = {0};
   TelSysResponse_E e_retVal = eTEL_NOK;
   uint32_t u32_index = 0U;

   for(u32_index = 0U; u32_index < (sizeof(st_fileWrites) / sizeof(st_fileWrites[0])); u32_index++)
   {
      st_fsReq.e_FileId = st_fileWrites[u32_index].e_fileId;
      st_fsReq.u32_RequestedBytes = st_fileWrites[u32_index].u32_size;
      st_fsReq.vpt_Buffer = st_fileWrites[u32_index].vpt_buffer;

      if((ge_FS_Create(st_fsReq.e_FileId) != eFS_OK) ||
         (ge_FS_Write(&st_fsReq) != eFS_OK) ||
         (st_fsReq.u32_ActualBytes != st_fsReq.u32_RequestedBytes))
      {
         break;
      }
   }

   if(u32_index == (sizeof(st_fileWrites) / sizeof(st_fileWrites[0])))
   {
      e_retVal = eTEL_OK;
   }

   return e_retVal;
}


/**
 * @private       se_PROV_CheckDevProvisioningStatus
 * @brief         This function checks and determines the device provisioning
 *                status.
 * @param[in]     None
 * @param[out]    None
 * @param[inout]  None
 * @return        None
 * @note          The function goes ahead and updates the global context of
 *                device provisioning manager inside sst_provManagerCtx
 */
static void sv_PROV_CheckDevProvisioningStatus(void)
{
   FileSystemResult_E e_fsReturn = eFS_INTERNAL_ERROR;
   FileSystemReq_T st_fsReq = {0};

   e_fsReturn = ge_FS_IsFilePresent(eFS_FILE_DEV_PROV_STATUS);
   if(e_fsReturn == eFS_OK)
   {
      // File present, open and read the device provisioning status
      st_fsReq.e_FileId = eFS_FILE_DEV_PROV_STATUS;
      st_fsReq.u32_RequestedBytes = sizeof(PROVStatus_E);
      st_fsReq.vpt_Buffer = (void*)&sst_provManagerCtx.e_deviceProvStatus;
      e_fsReturn = ge_FS_Read(&st_fsReq);
      // Check if the read operation was successful and the number of bytes read is as expected
      if((e_fsReturn == eFS_OK) && (st_fsReq.u32_ActualBytes == st_fsReq.u32_RequestedBytes))
      {
         // Successfully read the device provisioning status
         // Update the state machine based on the read status
         switch(sst_provManagerCtx.e_deviceProvStatus)
         {
            case eDEV_PROVISIONED:
            case eDEV_REPROVISIONED:
            {
               // Check if all the required files associated with device provisioning exists,
               //  If they do then device is correctly provisioned
               if((ge_FS_IsFilePresent(eFS_FILE_VIN) == eFS_OK) &&
                  (ge_FS_IsFilePresent(eFS_FILE_VIN_ALIAS) == eFS_OK) &&
                  (ge_FS_IsFilePresent(eFS_FILE_ROOT_CA) == eFS_OK) &&
                  (ge_FS_IsFilePresent(eFS_FILE_DEV_CERT) == eFS_OK) &&
                  (ge_FS_IsFilePresent(eFS_FILE_DEV_PUBLIC_KEY) == eFS_OK) &&
                  (ge_FS_IsFilePresent(eFS_FILE_DEV_PRIVATE_KEY) == eFS_OK) &&
                  (ge_FS_IsFilePresent(eFS_FILE_DEV_CSR) == eFS_OK) &&
                  (ge_FS_IsFilePresent(eFS_FILE_DEV_ID) == eFS_OK))
               {
                  if (se_PROV_ReadDevCertsAndKeys() == eTEL_OK)
                  {
                     #ifdef DEBUG_ENABLED
                        BAL_PROV_LOG("Device already provisioned");
                     #endif
                     sv_PROV_ChangeState(DEV_PROV_COMPLETED);
                  }
                  else
                  {
                     #ifdef DEBUG_ENABLED
                        BAL_PROV_LOG("Issue with device certificates, device un-provisioned");
                     #endif
                     sv_PROV_ChangeState(DEV_PROV_WAIT_FOR_VIN);
                  }

               }
               else
               {
                  // If any of the required files are missing, this indicates that the device provisioning is corrupt
                  sst_provManagerCtx.e_deviceProvStatus = eDEV_PROVISIONING_CORRUPT;
                  sv_PROV_ChangeState(DEV_PROV_WAIT_FOR_VIN);
               }

               // Else device provisioning is corrupted and we should start afresh


               sv_PROV_ChangeState(DEV_PROV_COMPLETED);
               break;
            }
            default:
            {
               // If the provisioning file contains anything else, this indicates file corruption.
               sst_provManagerCtx.e_deviceProvStatus = eDEV_PROVISIONING_CORRUPT;
               // At this point we should ideally delete the files associated with provisioning and
               // start fresh provisioning session, marking device as not provisioned.
               sv_PROV_ChangeState(DEV_PROV_WAIT_FOR_VIN);
               break;
            }
         }
      }
      else
      {
         // Read operation failed or unexpected number of bytes read
         // Handle error accordingly, possibly set state to not provisioned
         sv_PROV_ChangeState(DEV_PROV_WAIT_FOR_VIN);
      }

   }
   else
   {
      // File absent, hence device may be partially provisioned or is not provisioned yet
      if((ge_FS_IsFilePresent(eFS_FILE_VIN) == eFS_OK) ||
         (ge_FS_IsFilePresent(eFS_FILE_VIN_ALIAS) == eFS_OK) ||
         (ge_FS_IsFilePresent(eFS_FILE_ROOT_CA) == eFS_OK) ||
         (ge_FS_IsFilePresent(eFS_FILE_DEV_CERT) == eFS_OK) ||
         (ge_FS_IsFilePresent(eFS_FILE_DEV_PUBLIC_KEY) == eFS_OK) ||
         (ge_FS_IsFilePresent(eFS_FILE_DEV_PRIVATE_KEY) == eFS_OK) ||
         (ge_FS_IsFilePresent(eFS_FILE_DEV_CSR) == eFS_OK) ||
         (ge_FS_IsFilePresent(eFS_FILE_DEV_ID) == eFS_OK))
      {
         // Device Partially provisioned
         sst_provManagerCtx.e_deviceProvStatus = eDEV_PROVISIONING_PARTIAL;
      }
      else
      {
         // Device not yet provisioned
         sst_provManagerCtx.e_deviceProvStatus = eDEV_UNPROVISIONED;
      }
      sv_PROV_ChangeState(DEV_PROV_WAIT_FOR_VIN);
   }

}


/**
 * @private       sv_PROV_WaitForVIN
 * @brief         This function run whenever an event is received in the wait for
 *                VIN state of device provisioning manager.
 * @param[in]     None
 * @param[out]    None
 * @param[inout]  cpt_IPCEvent - Pointer to IPC Event structure which contains
 *                the command and data received from RH850. This pointer would
 *                also be used to send the response back to RH850.
 * @return        None
 */
static void sv_PROV_WaitForVIN(IPCMessage_T* stpt_IPCEvent)
{

#ifdef DEBUG_ENABLED
   BAL_PROV_LOG("Wait for VIN");
#endif
   // First check the Service ID of the incoming message
   switch(stpt_IPCEvent->eServiceID)
   {
      case eSERVICE_ID_PROV_VIN_WRITE:
      {
#ifdef DEBUG_ENABLED
         BAL_PROV_LOG("Writing VIN");
#endif // DEBUG_ENABLED
         // Create VIN file
         memcpy(gca_VIN, stpt_IPCEvent->u8a_Data,stpt_IPCEvent->tDataLength);
         // PKI Generation Successful, setting a positive response
         sv_PROV_ChangeState(DEV_PROV_WAIT_FOR_PKI_REQ);
         sv_PROV_GeneratePositiveAck(stpt_IPCEvent);

      }
      break;
      default:
      {
#ifdef DEBUG_ENABLED
         BAL_PROV_LOG("Unhandled service ID : %d received", stpt_IPCEvent->eServiceID);
#endif
         sv_PROV_GenerateNegativeAck(stpt_IPCEvent);
      }
      break;
   }
}


/**
 * @private       sv_PROV_WaitForPKI
 * @brief         This function run during wait for PKI request state.
 * @param[in]     None
 * @param[out]    None
 * @param[inout]  cpt_IPCEvent - Pointer to IPC Event structure which contains
 *                the command and data received from RH850. This pointer would
 *                also be used to send the response back to RH850.
 * @return        None
 */
static void sv_PROV_WaitForPKI(IPCMessage_T* stpt_IPCEvent)
{
   CryptoIdentity_T st_cryptoReq={0};

#ifdef DEBUG_ENABLED
   BAL_PROV_LOG("Wait for PKI Req State");
#endif
   switch(stpt_IPCEvent->eServiceID)
   {
      case eSERVICE_ID_PROV_GEN_PKI:
      {
#ifdef DEBUG_ENABLED
         BAL_PROV_LOG("PKI Generation Service ID received");
#endif
         // @R: Call Generate PKI Function here
         // PKI Generation Successful, indicate via a positive response and change to next state
         // Copy VIN to PKI Request structure
         memcpy(st_cryptoReq.car_Vin, gca_VIN, PROV_VIN_MAX_SIZE);
         // Copy IMEI to PKI Request structure
         ql_dev_get_imei(st_cryptoReq.car_Imei, CRYPTO_IMEI_SIZE, 0);

         memset(gca_CSR,0,PROV_CSR_MAX_SIZE);

         // Set the size of certificates to PKI Request structure

         st_cryptoReq.u16_CSRGenSize       = PROV_CSR_MAX_SIZE;
         st_cryptoReq.u16_PrivateKeySize   = PROV_PRIVATE_KEY_MAX_SIZE;
         st_cryptoReq.u16_PublicKeySize    = PROV_PUBLIC_KEY_MAX_SIZE;

         // Set the pointer of array to PKI Request structure
         st_cryptoReq.u8pt_CSRGen            = (uint8_t*)gca_CSR;
         st_cryptoReq.u8pt_PrivateKey        = (uint8_t*)gca_privateKey;
         st_cryptoReq.u8pt_PublicKey         = (uint8_t*)gca_publicKey;
#ifdef DEBUG_ENABLED
         BAL_PROV_LOG("Artifacts Updated");
#endif
         if(CRYPTO_OK == ge_Crypto_GenerateArtifacts(&st_cryptoReq))
         {
            // PKI Generation Successful, setting a positive response
            sv_PROV_ChangeState(DEV_PROV_WAIT_FOR_CSR_BLK_REQ);
            sv_PROV_GeneratePositiveAck(stpt_IPCEvent);
#ifdef DEBUG_ENABLED
         BAL_PROV_LOG("PKI generation Successful, CSR Size : %d",st_cryptoReq.u16_CSRGenSize );
#endif
            // Additionally we should indicate the size of CSR generated in this response
            stpt_IPCEvent->tDataLength = PROV_POS_ACK_WITH_ADDITIONAL_DATA_LENGTH;
            gu16_sizeOfCSR = st_cryptoReq.u16_CSRGenSize;
            gu8_totalCSRBlocks = (gu16_sizeOfCSR / UART_DATA_FRAME_MAX_PAYLOAD_SIZE);

            if(gu16_sizeOfCSR % UART_DATA_FRAME_MAX_PAYLOAD_SIZE)
            {
               gu8_totalCSRBlocks+=1;
            }
            else
            {
               // The data is UART_DATA_FRAME_MAX_PAYLOAD_SIZE aligned
            }
            stpt_IPCEvent->u8a_Data[1] = (uint8_t)(gu16_sizeOfCSR & 0xFF);
            stpt_IPCEvent->u8a_Data[2] = (uint8_t)((gu16_sizeOfCSR >> 8) & 0xFF);

#ifdef DEBUG_ENABLED
            BAL_PROV_LOG("CSR generated, total size %d and total blocks %d", gu16_sizeOfCSR, gu8_totalCSRBlocks);
#endif
         }
         else
         {
            // PKI Generation Failed, setting a negative response
            sv_PROV_GenerateNegativeAck(stpt_IPCEvent);
            sv_PROV_ChangeState(DEV_PROV_WAIT_FOR_VIN);
#ifdef DEBUG_ENABLED
         BAL_PROV_LOG("PKI generation Failed");
#endif
         }
      }
      break;
      default:
      {
#ifdef DEBUG_ENABLED
         BAL_PROV_LOG("Unhandled service ID : %d received", stpt_IPCEvent->eServiceID);
#endif
         sv_PROV_ChangeState(DEV_PROV_WAIT_FOR_VIN);
         sv_PROV_GenerateNegativeAck(stpt_IPCEvent);
      }
      break;
   }
}


/**
 * @private       sv_PROV_WaitForCSRBlockReq
 * @brief         State function for CSR Block Request
 *                Each block is of size UART_DATA_FRAME_MAX_PAYLOAD_SIZE,
 *                last block might be of lesser size. Once last block received
 *                we transition to next state.
 * @param[in]     None
 * @param[out]    None
 * @param[inout]  cpt_IPCEvent - Pointer to IPC Event structure which contains
 *                the command and data received from RH850. This pointer would
 *                also be used to send the response back to RH850.
 * @return        None
 */
static void sv_PROV_WaitForCSRBlockReq(IPCMessage_T* stpt_IPCEvent)
{
   // bool b_isLastBlkReq = FALSE;
   uint8_t u8_currentReqBlk = 0;
   uint16_t u16_currOffset = 0;
   uint8_t u8_remainingBytes = 0;
#ifdef DEBUG_ENABLED
   BAL_PROV_LOG("CSR Block Req State");
#endif

   switch(stpt_IPCEvent->eServiceID)
   {
      case eSERVICE_ID_DATA_SEND_ACK:
      case eSERVICE_ID_DATA_SEND_NACK:
      {
         sst_provManagerCtx.b_responseRequired = FALSE;
#ifdef DEBUG_ENABLED
         BAL_PROV_LOG("Bulk Data Send Ack/Nack received, no response required");
#endif
         break;
      }
      case eSERVICE_ID_PROV_CSR_BLK_REQ:
      {
         // Block request received
         #ifdef DEBUG_ENABLED
         BAL_PROV_LOG("CSR Block Req DID");
         #endif
         // Check validity of block request
         u8_currentReqBlk = stpt_IPCEvent->u8a_Data[0];
         if(u8_currentReqBlk < (gu8_totalCSRBlocks-1))
         {
            #ifdef DEBUG_ENABLED
            BAL_PROV_LOG("Valid CSR Block Req");
            #endif
            u16_currOffset = u8_currentReqBlk * UART_DATA_FRAME_MAX_PAYLOAD_SIZE;
            // Update the CSR Data Inside the UART IPC Frame
            stpt_IPCEvent->tDataLength = UART_DATA_FRAME_MAX_PAYLOAD_SIZE;
            memcpy(&stpt_IPCEvent->u8a_Data[0], &gca_CSR[u16_currOffset], UART_DATA_FRAME_MAX_PAYLOAD_SIZE);

         }
         else if (u8_currentReqBlk == (gu8_totalCSRBlocks-1))
         {

            #ifdef DEBUG_ENABLED
            BAL_PROV_LOG("Last CSR Block Request");
            #endif
            u16_currOffset = u8_currentReqBlk * UART_DATA_FRAME_MAX_PAYLOAD_SIZE;

            // Update the last CSR Data Inside the UART IPC Frame
            u8_remainingBytes = (gu16_sizeOfCSR % UART_DATA_FRAME_MAX_PAYLOAD_SIZE);
            stpt_IPCEvent->tDataLength = u8_remainingBytes;
            memcpy(&stpt_IPCEvent->u8a_Data[0], &gca_CSR[u16_currOffset], u8_remainingBytes);
            // Change the state to wait for Root CA Size
            sv_PROV_ChangeState(DEV_PROV_WAIT_FOR_ROOT_CA_SIZE);

         }
         else
         {
            #ifdef DEBUG_ENABLED
            BAL_PROV_LOG("Invalid Block Requested %d / %d ", u8_currentReqBlk, gu8_totalCSRBlocks);
            #endif
            sv_PROV_ChangeState(DEV_PROV_WAIT_FOR_VIN);
            sv_PROV_GenerateNegativeAck(stpt_IPCEvent);
         }
      }
      break;
      default:
      {
         #ifdef DEBUG_ENABLED
         BAL_PROV_LOG("Unhandled service ID : %d received", stpt_IPCEvent->eServiceID);
         #endif
         sv_PROV_ChangeState(DEV_PROV_WAIT_FOR_VIN);
         sv_PROV_GenerateNegativeAck(stpt_IPCEvent);
      }
      break;
   }
}


/**
 * @private       sv_PROV_WaitForRootCASize
 * @brief         State function for wait for RootCA Size
 * @param[in]     None
 * @param[out]    None
 * @param[inout]  cpt_IPCEvent - Pointer to IPC Event structure which contains
 *                the command and data received from RH850. This pointer would
 *                also be used to send the response back to RH850.
 * @return        None
 */
static void sv_PROV_WaitForRootCASize(IPCMessage_T* stpt_IPCEvent)
{

#ifdef DEBUG_ENABLED
   BAL_PROV_LOG("Wait for Root CA Size State");
#endif

   switch(stpt_IPCEvent->eServiceID)
   {

      case eSERVICE_ID_DATA_SEND_ACK:
      {
         // Ignore this for now
         sst_provManagerCtx.b_responseRequired = FALSE;
      }
      break;
      case eSERVICE_ID_PROV_ROOT_CA_SIZE_INFO:
      {
         #ifdef DEBUG_ENABLED
         BAL_PROV_LOG("Wait for Root CA Service ID received");
         #endif
         gu16_sizeOfRootCA = (((uint16_t)stpt_IPCEvent->u8a_Data[0]<< 8) + stpt_IPCEvent->u8a_Data[1]);

         if((gu16_sizeOfRootCA > PROV_ROOT_CA_MAX_SIZE) || (gu16_sizeOfRootCA == 0))
         {
            #ifdef DEBUG_ENABLED
            BAL_PROV_LOG("Invalid Root CA Size");
            #endif
            // Revert to first step and send negative acknowledgement
            sv_PROV_ChangeState(DEV_PROV_WAIT_FOR_VIN);
            sv_PROV_GenerateNegativeAck(stpt_IPCEvent);
         }
         else
         {
            #ifdef DEBUG_ENABLED
            BAL_PROV_LOG("Valid Root CA Size");
            #endif
            // Compute the number of blocks required for reception
            gu8_totalRootCABlocks = gu16_sizeOfRootCA/UART_DATA_FRAME_MAX_PAYLOAD_SIZE;
            if(gu16_sizeOfRootCA % UART_DATA_FRAME_MAX_PAYLOAD_SIZE)
            {
               gu8_totalRootCABlocks+=1;
            }
            else
            {
               // Do nothing
            }
            gu8_currRootCABlock = 0;
            #ifdef DEBUG_ENABLED
            BAL_PROV_LOG("Size received : %d number of blocks calculated: %d", gu16_sizeOfRootCA, gu8_totalRootCABlocks);
            #endif
            // Change to next state and send positive acknowledgement
            sv_PROV_ChangeState(DEV_PROV_WAIT_FOR_ROOT_CA_BLK_RECP);
            sv_PROV_GeneratePositiveAck(stpt_IPCEvent);
         }
      }
      break;
      default:
      {
#ifdef DEBUG_ENABLED
         BAL_PROV_LOG("Unhandled service ID : %d received", stpt_IPCEvent->eServiceID);
#endif
         sv_PROV_ChangeState(DEV_PROV_WAIT_FOR_VIN);
         sv_PROV_GenerateNegativeAck(stpt_IPCEvent);
      }
      break;
   }
}

/**
 * @private       sv_PROV_WaitForRootCABlockRecp
 * @brief         State function for Root CA Block Reception
 *                Each block is of size UART_DATA_FRAME_MAX_PAYLOAD_SIZE,
 *                last block might be of lesser size. Once last block received
 *                we transition to next state.
 * @param[in]     None
 * @param[out]    None
 * @param[inout]  cpt_IPCEvent - Pointer to IPC Event structure which contains
 *                the command and data received from RH850. This pointer would
 *                also be used to send the response back to RH850.
 * @return        None
 */
static void sv_PROV_WaitForRootCABlockRecp(IPCMessage_T* stpt_IPCEvent)
{
   uint16_t u16_currOffset = 0;
   uint16_t u16_bytesToCopy = 0;

#ifdef DEBUG_ENABLED
   BAL_PROV_LOG("Root CA Block Reception State");
#endif

   switch(stpt_IPCEvent->eServiceID)
   {
      case eSERVICE_ID_PROV_ROOT_CA_BLK_RECP:
      {
#ifdef DEBUG_ENABLED
   BAL_PROV_LOG("Correct DID for Root CA Block Reception");
#endif
         u16_currOffset = gu8_currRootCABlock * UART_DATA_FRAME_MAX_PAYLOAD_SIZE;

         if(gu8_currRootCABlock == (gu8_totalRootCABlocks-1))
         {
            u16_bytesToCopy = stpt_IPCEvent->tDataLength;
            if(u16_bytesToCopy != (gu16_sizeOfRootCA % UART_DATA_FRAME_MAX_PAYLOAD_SIZE))
            {
               #ifdef DEBUG_ENABLED
                  BAL_PROV_LOG("Partial last block of Root CA received, error provisioning");
               #endif
               sv_PROV_ChangeState(DEV_PROV_WAIT_FOR_VIN);
               sv_PROV_GenerateNegativeAck(stpt_IPCEvent);
            }
            else
            {
               #ifdef DEBUG_ENABLED
                  BAL_PROV_LOG("Correct last block of Root CA received");
               #endif
               memcpy(&gca_rootCA[u16_currOffset], &stpt_IPCEvent->u8a_Data[0], u16_bytesToCopy);
               sv_PROV_ChangeState(DEV_PROV_WAIT_FOR_DEV_CERT_SIZE);
               sv_PROV_GeneratePositiveAck(stpt_IPCEvent);
            }
         }
         else
         {
            gu8_currRootCABlock++;
            u16_bytesToCopy = stpt_IPCEvent->tDataLength;
            if(u16_bytesToCopy != UART_DATA_FRAME_MAX_PAYLOAD_SIZE)
            {
               #ifdef DEBUG_ENABLED
                  BAL_PROV_LOG("Partial Root CA frame sent midway, error provisioning");
               #endif
               sv_PROV_ChangeState(DEV_PROV_WAIT_FOR_VIN);
               sv_PROV_GenerateNegativeAck(stpt_IPCEvent);
            }
            else
            {
               #ifdef DEBUG_ENABLED
                  BAL_PROV_LOG("Correct block of Root CA received");
               #endif
               memcpy(&gca_rootCA[u16_currOffset], &stpt_IPCEvent->u8a_Data[0], u16_bytesToCopy);
            }
         }
      }
      break;
      default:
      {
#ifdef DEBUG_ENABLED
         BAL_PROV_LOG("Unhandled service ID : %d received", stpt_IPCEvent->eServiceID);
#endif
         sv_PROV_ChangeState(DEV_PROV_WAIT_FOR_VIN);
         sv_PROV_GenerateNegativeAck(stpt_IPCEvent);
      }
      break;
   }
}

/**
 * @private       sv_PROV_WaitForDeviceCertSize
 * @brief         State function for wait for Device Certificate Size
 * @param[in]     None
 * @param[out]    None
 * @param[inout]  cpt_IPCEvent - Pointer to IPC Event structure which contains
 *                the command and data received from RH850. This pointer would
 *                also be used to send the response back to RH850.
 * @return        None
 */
static void sv_PROV_WaitForDeviceCertSize(IPCMessage_T* stpt_IPCEvent)
{
#ifdef DEBUG_ENABLED
   BAL_PROV_LOG("Wait for Device Certificate Size State");
#endif

   switch(stpt_IPCEvent->eServiceID)
   {
      case eSERVICE_ID_PROV_DEV_CERT_SIZE_INFO:
      {
         #ifdef DEBUG_ENABLED
         BAL_PROV_LOG("Wait for Device Cert Service ID received");
         #endif
         gu16_sizeOfDevCert = (((uint16_t)stpt_IPCEvent->u8a_Data[0]<< 8) + stpt_IPCEvent->u8a_Data[1]);

         if((gu16_sizeOfDevCert > PROV_ROOT_CA_MAX_SIZE) || (gu16_sizeOfDevCert == 0))
         {
            #ifdef DEBUG_ENABLED
            BAL_PROV_LOG("Invalid Device Cert Size");
            #endif
            // Revert to first step and send negative acknowledgement
            sv_PROV_ChangeState(DEV_PROV_WAIT_FOR_VIN);
            sv_PROV_GenerateNegativeAck(stpt_IPCEvent);
         }
         else
         {
            #ifdef DEBUG_ENABLED
            BAL_PROV_LOG("Valid Device Cert Size");
            #endif
            // Compute the number of blocks required for reception
            gu8_totalDevCertBlocks = gu16_sizeOfDevCert/UART_DATA_FRAME_MAX_PAYLOAD_SIZE;
            if(gu16_sizeOfDevCert % UART_DATA_FRAME_MAX_PAYLOAD_SIZE)
            {
               gu8_totalDevCertBlocks+=1;
            }
            else
            {
               // Do nothing
            }
            gu8_currDevCertBlock = 0;
            #ifdef DEBUG_ENABLED
            BAL_PROV_LOG("Size received : %d number of blocks calculated: %d", gu16_sizeOfDevCert, gu8_totalDevCertBlocks);
            #endif
            // Change to next state and send positive acknowledgement
            sv_PROV_ChangeState(DEV_PROV_WAIT_FOR_DEV_CERT_BLK_RECP);
            sv_PROV_GeneratePositiveAck(stpt_IPCEvent);
         }
      }
      break;
      default:
      {
#ifdef DEBUG_ENABLED
         BAL_PROV_LOG("Unhandled service ID : %d received", stpt_IPCEvent->eServiceID);
#endif
         sv_PROV_ChangeState(DEV_PROV_WAIT_FOR_VIN);
         sv_PROV_GenerateNegativeAck(stpt_IPCEvent);
      }
      break;
   }
}

/**
 * @private       sv_PROV_WaitForDeviceCertBlockRecp
 * @brief         State function for Device Certificate Reception
 *                Each block is of size UART_DATA_FRAME_MAX_PAYLOAD_SIZE,
 *                last block might be of lesser size. Once last block received
 *                we transition to next state.
 * @param[in]     None
 * @param[out]    None
 * @param[inout]  cpt_IPCEvent - Pointer to IPC Event structure which contains
 *                the command and data received from RH850. This pointer would
 *                also be used to send the response back to RH850.
 * @return        None
 */
static void sv_PROV_WaitForDeviceCertBlockRecp(IPCMessage_T* stpt_IPCEvent)
{
   uint16_t u16_currOffset = 0;
   uint16_t u16_bytesToCopy = 0;

#ifdef DEBUG_ENABLED
   BAL_PROV_LOG("Device Cert Block Reception State");
#endif

   switch(stpt_IPCEvent->eServiceID)
   {
      case eSERVICE_ID_PROV_DEV_CERT_BLK_RECP:
      {
#ifdef DEBUG_ENABLED
   BAL_PROV_LOG("Correct DID for Device Cert Block Reception");
#endif
         u16_currOffset = gu8_currDevCertBlock * UART_DATA_FRAME_MAX_PAYLOAD_SIZE;

         if(gu8_currDevCertBlock == (gu8_totalDevCertBlocks-1))
         {
            u16_bytesToCopy = stpt_IPCEvent->tDataLength;
            if(u16_bytesToCopy != (gu16_sizeOfDevCert % UART_DATA_FRAME_MAX_PAYLOAD_SIZE))
            {
               #ifdef DEBUG_ENABLED
                  BAL_PROV_LOG("Partial last block of Device Cert received, error provisioning");
               #endif
               sv_PROV_ChangeState(DEV_PROV_WAIT_FOR_VIN);
               sv_PROV_GenerateNegativeAck(stpt_IPCEvent);
            }
            else
            {
               #ifdef DEBUG_ENABLED
                  BAL_PROV_LOG("Correct last block of Device Cert received");
               #endif
               memcpy(&gca_deviceCert[u16_currOffset], &stpt_IPCEvent->u8a_Data[0], u16_bytesToCopy);
               sv_PROV_ChangeState(DEV_PROV_WAIT_FOR_VIN_ALIAS);
               sv_PROV_GeneratePositiveAck(stpt_IPCEvent);
            }
         }
         else
         {
            gu8_currDevCertBlock++;
            u16_bytesToCopy = stpt_IPCEvent->tDataLength;
            if(u16_bytesToCopy != UART_DATA_FRAME_MAX_PAYLOAD_SIZE)
            {
               #ifdef DEBUG_ENABLED
                  BAL_PROV_LOG("Partial Device Cert frame sent midway, error provisioning");
               #endif
               sv_PROV_ChangeState(DEV_PROV_WAIT_FOR_VIN);
               sv_PROV_GenerateNegativeAck(stpt_IPCEvent);
            }
            else
            {
               #ifdef DEBUG_ENABLED
                  BAL_PROV_LOG("Correct block of Device Cert received");
               #endif
               memcpy(&gca_deviceCert[u16_currOffset], &stpt_IPCEvent->u8a_Data[0], u16_bytesToCopy);
            }
         }
      }
      break;
      default:
      {
#ifdef DEBUG_ENABLED
         BAL_PROV_LOG("Unhandled service ID : %d received", stpt_IPCEvent->eServiceID);
#endif
         sv_PROV_ChangeState(DEV_PROV_WAIT_FOR_VIN);
         sv_PROV_GenerateNegativeAck(stpt_IPCEvent);
      }
      break;
   }
}

/**
 * @private       sv_PROV_WaitForVINAlias
 * @brief         State function to receive VIN Alias
 * @param[in]     None
 * @param[out]    None
 * @param[inout]  cpt_IPCEvent - Pointer to IPC Event structure which contains
 *                the command and data received from RH850. This pointer would
 *                also be used to send the response back to RH850.
 * @return        None
 */
static void sv_PROV_WaitForVINAlias(IPCMessage_T* stpt_IPCEvent)
{
   uint8_t u8_vinAliasSize = 0;
#ifdef DEBUG_ENABLED
   BAL_PROV_LOG("Wait for VIN Alias State");
#endif

   switch(stpt_IPCEvent->eServiceID)
   {
      case eSERVICE_ID_PROV_VIN_ALIAS_WRITE:
      {
#ifdef DEBUG_ENABLED
         BAL_PROV_LOG("Correct VIN Alias Write request received");
#endif // DEBUG_ENABLED
         u8_vinAliasSize = stpt_IPCEvent->tDataLength;
         // Check and validate VIN Alias Size
         if(u8_vinAliasSize <= PROV_VIN_ALIAS_MAX_SIZE)
         {
            // Copy VIN Alias to RAM Buffer
            memcpy(&gca_vinAlias[0], &stpt_IPCEvent->u8a_Data[0], u8_vinAliasSize);
            #ifdef DEBUG_ENABLED
               BAL_PROV_LOG("New VIN Alias received size: %d Alias : %s", u8_vinAliasSize, gca_vinAlias);
            #endif
            sv_PROV_ChangeState(DEV_PROV_WAIT_FOR_DEVICE_ID);
            sv_PROV_GeneratePositiveAck(stpt_IPCEvent);
         }
         else
         {
            #ifdef DEBUG_ENABLED
               BAL_PROV_LOG("Unsupported length of VIN Alias Received Length : %d", u8_vinAliasSize);
            #endif
            sv_PROV_ChangeState(DEV_PROV_WAIT_FOR_VIN);
            sv_PROV_GenerateNegativeAck(stpt_IPCEvent);
         }
      }
      break;
      default:
      {
#ifdef DEBUG_ENABLED
         BAL_PROV_LOG("Unhandled service ID : %d received", stpt_IPCEvent->eServiceID);
#endif
         sv_PROV_ChangeState(DEV_PROV_WAIT_FOR_VIN);
         sv_PROV_GenerateNegativeAck(stpt_IPCEvent);
      }
      break;
   }
}

/**
 * @private       sv_PROV_WaitForDeviceID
 * @brief         State function to receive Device ID
 * @param[in]     None
 * @param[out]    None
 * @param[inout]  cpt_IPCEvent - Pointer to IPC Event structure which contains
 *                the command and data received from RH850. This pointer would
 *                also be used to send the response back to RH850.
 * @return        None
 */
static void sv_PROV_WaitForDeviceID(IPCMessage_T* stpt_IPCEvent)
{
   uint8_t u8_devIdSize = 0;
#ifdef DEBUG_ENABLED
   BAL_PROV_LOG("Wait for Device ID State");
#endif

   switch(stpt_IPCEvent->eServiceID)
   {
      case eSERVICE_ID_PROV_DEV_ID_WRITE:
      {
#ifdef DEBUG_ENABLED
         BAL_PROV_LOG("Correct Device ID Write request received");
#endif // DEBUG_ENABLED
         u8_devIdSize = stpt_IPCEvent->tDataLength;
         // Check and validate Device ID size
         if(u8_devIdSize <= PROV_DEV_ID_MAX_SIZE)
         {
            // Copy Device ID to RAM Buffer
            memcpy(&gca_devID[0], &stpt_IPCEvent->u8a_Data[0], u8_devIdSize);
            // Mark device provisioning as completed
            ge_provStatus = eDEV_PROVISIONED;
            #ifdef DEBUG_ENABLED
               BAL_PROV_LOG("New Device ID received size :%d  id: %s ", u8_devIdSize, gca_devID);
            #endif

            // Copy the entire RAM context of device provisioning to ROM
            if(se_PROV_WriteDevCertsAndKeys() == eTEL_OK)
            {
               #ifdef DEBUG_ENABLED
                  BAL_PROV_LOG("Device provisioning completed successfully!");
               #endif
               sv_PROV_ChangeState(DEV_PROV_COMPLETED);
               sv_PROV_GeneratePositiveAck(stpt_IPCEvent);
            }
            else
            {
               #ifdef DEBUG_ENABLED
                  BAL_PROV_LOG("Writing certificates to memory failed, restart device provisioning");
               #endif
               sv_PROV_ChangeState(DEV_PROV_WAIT_FOR_VIN);
               sv_PROV_GenerateNegativeAck(stpt_IPCEvent);
            }

         }
         else
         {
            #ifdef DEBUG_ENABLED
               BAL_PROV_LOG("Unsupported length of Device ID Received Length : %d", u8_devIdSize);
            #endif
            sv_PROV_ChangeState(DEV_PROV_WAIT_FOR_VIN);
            sv_PROV_GenerateNegativeAck(stpt_IPCEvent);
         }
      }
      break;
      default:
      {
#ifdef DEBUG_ENABLED
         BAL_PROV_LOG("Unhandled service ID : %d received", stpt_IPCEvent->eServiceID);
#endif
         sv_PROV_ChangeState(DEV_PROV_WAIT_FOR_VIN);
         sv_PROV_GenerateNegativeAck(stpt_IPCEvent);
      }
      break;
   }
}


/**
 * @private       sv_PROV_CompletedState
 * @brief         State function reached post successful device provisioning.
 *                This state can only entertain a device re-provisioning request.
 * @param[in]     None
 * @param[out]    None
 * @param[inout]  cpt_IPCEvent - Pointer to IPC Event structure which contains
 *                the command and data received from RH850. This pointer would
 *                also be used to send the response back to RH850.
 * @return        None
 */
static void sv_PROV_CompletedState(IPCMessage_T* stpt_IPCEvent)
{
#ifdef DEBUG_ENABLED
   BAL_PROV_LOG("Provisioning Completed State");
#endif

   switch(stpt_IPCEvent->eServiceID)
   {
      case eSERVICE_ID_PROV_DEV_ID_REPROVISION:
      {
         sv_PROV_ChangeState(DEV_PROV_WAIT_FOR_VIN);
         sv_PROV_GeneratePositiveAck(stpt_IPCEvent);
#ifdef DEBUG_ENABLED
         BAL_PROV_LOG("Re-provisioning request received, device ready to re-provision.");
#endif
      }
      break;
      default:
      {
#ifdef DEBUG_ENABLED
         BAL_PROV_LOG("Unhandled service ID : %d received, device already provisioned", stpt_IPCEvent->eServiceID);
#endif
         sv_PROV_GenerateNegativeAck(stpt_IPCEvent);
      }
      break;
   }
}


/******************************************************************************/
/*                                                                            */
/*                        PUBLIC FUNCTION DEFINITIONS                         */
/*                                                                            */
/******************************************************************************/
/**
 * @public        ge_PROV_Init
 * @brief         Initializes the device provisioning manager application.
 * @param[in]     None
 * @param[out]    None
 * @param[inout]  None
 * @return        Returns the status of device provisioning initialization.
 *                eTEL_OK - If device provisioning initialization succeeded.
 *                eTEL_NOK - If device provisioning initialization fails.
 */
TelSysResponse_E ge_PROV_Init(void)
{
   TelSysResponse_E eRetVal = eTEL_NOK;
   // Initialize the context
   memset(&sst_provManagerCtx, 0, sizeof(PROVManagerContext_T));
   // Check for device provisioning status from persistent storage
   sv_PROV_CheckDevProvisioningStatus();

   // Register the device provisioning manager task and queue here
   if(ql_rtos_queue_create(&sst_provManagerCtx.pt_Queue, sizeof(IPCMessage_T),
                           PROV_MANAGER_QUEUE_LENGTH) == QL_OSI_SUCCESS)
   {
#ifdef DEBUG_ENABLED
      BAL_PROV_LOG("Provisioning Manager Queue Init Success");
#endif // DEBUG_ENABLED
      // Register the Queue for IPC Message Routing
      ge_Common_RegisterIPCQueue(eIPC_NODE_APP_PROVISIONING, sst_provManagerCtx.pt_Queue);

      // Register the Provisioning Manager Task
      if(ql_rtos_task_create(&sst_provManagerCtx.pt_Task, PROV_MANAGER_TASK_STACK_SIZE,
            PROV_MANAGER_TASK_PRIORITY, "BALPROVMGR", sv_PROVManager_Task, NULL,
            PROV_MANAGER_TASK_EVENT_COUNT) == QL_OSI_SUCCESS)
      {
#ifdef DEBUG_ENABLED
         BAL_PROV_LOG("Provisioning Manager Task registration Success");
#endif // DEBUG_ENABLED
         eRetVal = eTEL_OK;
      }
      else
      {
#ifdef DEBUG_ENABLED
         BAL_PROV_LOG("Provisioning Manager Task registration Failed");
#endif // DEBUG_ENABLED
      }
   }
   else
   {
#ifdef DEBUG_ENABLED
      BAL_PROV_LOG("Provisioning Manager Queue Init Fail");
#endif // DEBUG_ENABLED
   }

   return eRetVal;
}

/**
 * Copyright(c) Bajaj Auto Technology Limited (BATL) as an unpublished work.
 * THIS SOFTWARE AND/OR MATERIAL IS THE PROPERTY OF BATL.
 * ALL USE, DISCLOSURE, AND/OR REPRODUCTION NOT SPECIFICALLY AUTHORIZED BY
 * BATL IS PROHIBITED.
 *
 * @author:<Author of C file - Name [Initials]>
 */
