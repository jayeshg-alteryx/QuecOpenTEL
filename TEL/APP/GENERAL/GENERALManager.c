/**
 * @file          GENERALManager.c
 * @brief         Source file containing General functionality and status
 *                reporting.
 *                When enquired and commanded by Modem this application:
 *                - Enables/Disables Diagnostics and Debug Mode.
 *                - Provides device status (provisioning, network, etc)
 *                - Provides device specific information (IMEI, ICCID, NETWORK
 *                   STRENGTH, STORED VIN, etc)
 * @date          05/08/26
 * @author        Yash Giramkar [YSG]
 * @copyright     Bajaj Auto Technology Limited (BATL)
 */

/******************************************************************************/
/*                                                                            */
/*                                  INCLUDES                                  */
/*                                                                            */
/******************************************************************************/
#include "GENERALManager.h"

/******************************************************************************/
/*                                                                            */
/*                                  DEFINES                                   */
/*                                                                            */
/******************************************************************************/
/**
 * @def           <Define name>
 * @brief         <Define details>.
 */
#define GENERAL_MANAGER_QUEUE_LENGTH         8U

#define GENERAL_MANAGER_STACK_SIZE           4096U
#define GENERAL_MANAGER_TASK_PRIORITY        APP_PRIORITY_ABOVE_NORMAL
#define GENERAL_MANAGER_TASK_EVENT_COUNT     8U



 #define MAX_DEVICE_INFO_LENGTH              64U
 #define GM_ERROR_RESPONSE_LENGTH            1U
 #define GM_INVALID_UNSUPPORTED_CMD_RESPONSE 0x7F
 #define GM_IMEI_RETRIVAL_FAILED_RESPONSE    0x7E
 #define GM_ICCID_RETRIVAL_FAILED_RESPONSE   0x7D
 #define GM_SERIAL_NUMBER_RETRIVAL_FAILED_RESPONSE 0x7C
 #define GM_PRODUCT_ID_RETRIVAL_FAILED_RESPONSE 0x7B
 #define GM_MODEL_INFO_RETRIVAL_FAILED_RESPONSE 0x7A
 #define GM_FIRMWARE_VERSION_RETRIVAL_FAILED_RESPONSE 0x79
 #define GM_FIRMWARE_SUBVERSION_RETRIVAL_FAILED_RESPONSE 0x78
 #define GM_TEMPERATURE_RETRIVAL_FAILED_RESPONSE 0x77
 #define GM_SIM_STATUS_RETRIVAL_FAILED_RESPONSE 0x76
 #define GM_NW_STATUS_RETRIVAL_FAILED_RESPONSE 0x75
 #define GM_NW_STRENGTH_RETRIVAL_FAILED_RESPONSE 0x74
 #define GM_NW_TIME_RETRIVAL_FAILED_RESPONSE 0x73
 #define GM_VIN_ABSENT_RESPONSE              0x72
 #define SIZE_OF_VIN                         17U



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
 * @struct        GeneralManagerCtx_T
 * @brief         Structure to hold context of General manager application.
 */
typedef struct
{
   ql_task_t pt_Task;
   ql_queue_t pt_Queue;
   bool b_provisioningStatus;
   bool b_simCardStatus;
   bool b_networkStatus;
   bool b_vinPresent;
   bool b_debugModeEnable;
   bool b_responseRequired;
} GeneralManagerCtx_T ;

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
static void sv_GM_Task(void *vpt_Argument);
static void sv_GM_SetDebugMode(IPCMessage_T *cpt_IPCEvent);
static void sv_GM_GetIMEI(IPCMessage_T *cpt_IPCEvent);
static void sv_GM_GetICCID(IPCMessage_T *cpt_IPCEvent);
static void sv_GM_GetSerialNumber(IPCMessage_T *cpt_IPCEvent);
static void sv_GM_GetProductID(IPCMessage_T *cpt_IPCEvent);
static void sv_GM_GetModelInfo(IPCMessage_T *cpt_IPCEvent);
static void sv_GM_GetFirmwareVersion(IPCMessage_T *cpt_IPCEvent);
static void sv_GM_GetSubVersion(IPCMessage_T *cpt_IPCEvent);
static void sv_GM_GetTemperature(IPCMessage_T *cpt_IPCEvent);
static void sv_GM_GetVIN(IPCMessage_T *cpt_IPCEvent);
static void sv_GM_GetSimStatus(IPCMessage_T *cpt_IPCEvent);
static void sv_GM_GetNWStatus(IPCMessage_T *cpt_IPCEvent);
static void sv_GM_GetNWStrength(IPCMessage_T *cpt_IPCEvent);
static void sv_GM_GetNWTime(IPCMessage_T *cpt_IPCEvent);

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
 * @var           st_GeneralManagerCtx
 * @brief         Structure to hold context of General Manager Application.
 */
GeneralManagerCtx_T st_GeneralManagerCtx = {0};
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
 * @private       sv_GM_SetDebugMode
 * @brief         This command and function would help us to enable or disable
 *                the debug mode in future.
 * @param[in]     None
 * @param[out]    None
 * @param[inout]  cpt_IPCEvent - Pointer to IPC Event structure which contains
 *                the command and data received from RH850. This pointer would
 *                also be used to send the response back to RH850.
 * @return        None
 */
static void sv_GM_SetDebugMode(IPCMessage_T *cpt_IPCEvent)
{
   // @R: TBD

   cpt_IPCEvent->eStatus = eIPC_STATUS_NOT_SUPPORTED;
   cpt_IPCEvent->tDataLength = GM_ERROR_RESPONSE_LENGTH;
   cpt_IPCEvent->u8a_Data[0] = GM_INVALID_UNSUPPORTED_CMD_RESPONSE;

}

/**
 * @private       sv_GM_GetIMEI
 * @brief         This command and function helps with IMEI retrieval.
 * @param[in]     None
 * @param[out]    None
 * @param[inout]  cpt_IPCEvent - Pointer to IPC Event structure which contains
 *                the command and data received from RH850. This pointer would
 *                also be used to send the response back to RH850.
 * @return        None
 */
static void sv_GM_GetIMEI(IPCMessage_T *cpt_IPCEvent)
{
   ql_errcode_dev_e e_ret = QL_DEV_SUCCESS;
   char c_devInfo[MAX_DEVICE_INFO_LENGTH] = {0};

   e_ret = ql_dev_get_imei(c_devInfo, MAX_DEVICE_INFO_LENGTH, 0);
   if(e_ret == QL_DEV_SUCCESS)
   {
      cpt_IPCEvent->eStatus = eIPC_STATUS_OK;
      cpt_IPCEvent->tDataLength = strlen((char *)c_devInfo);
#ifdef DEBUG_ENABLED
      BAL_GENERAL_APP_LOG("IMEI: %s, Length : %d", c_devInfo, cpt_IPCEvent->tDataLength);
#endif
      memcpy(cpt_IPCEvent->u8a_Data, c_devInfo, cpt_IPCEvent->tDataLength);
   }
   else
   {

      cpt_IPCEvent->eStatus = eIPC_STATUS_ERROR;
      cpt_IPCEvent->tDataLength = GM_ERROR_RESPONSE_LENGTH;
      cpt_IPCEvent->u8a_Data[0] = GM_IMEI_RETRIVAL_FAILED_RESPONSE;
   }
}

/**
 * @private       sv_GM_GetICCID
 * @brief         This command and function helps with ICCID retrieval.
 * @param[in]     None
 * @param[out]    None
 * @param[inout]  cpt_IPCEvent - Pointer to IPC Event structure which contains
 *                the command and data received from RH850. This pointer would
 *                also be used to send the response back to RH850.
 * @return        None
 */
static void sv_GM_GetICCID(IPCMessage_T *cpt_IPCEvent)
{
   ql_sim_errcode_e e_ret = QL_SIM_SUCCESS;
   char c_devInfo[MAX_DEVICE_INFO_LENGTH] = {0};

   e_ret = ql_sim_get_iccid(0, c_devInfo, MAX_DEVICE_INFO_LENGTH);
   if(e_ret == QL_SIM_SUCCESS)
   {

      cpt_IPCEvent->eStatus = eIPC_STATUS_OK;
      cpt_IPCEvent->tDataLength = strlen((char *)c_devInfo);
      memcpy(cpt_IPCEvent->u8a_Data, c_devInfo, cpt_IPCEvent->tDataLength);
#ifdef DEBUG_ENABLED
      BAL_GENERAL_APP_LOG("ICCID: %s, Length : %d", c_devInfo, cpt_IPCEvent->tDataLength);
#endif
   }
   else
   {

      cpt_IPCEvent->eStatus = eIPC_STATUS_ERROR;
      cpt_IPCEvent->tDataLength = GM_ERROR_RESPONSE_LENGTH;
      cpt_IPCEvent->u8a_Data[0] = GM_ICCID_RETRIVAL_FAILED_RESPONSE;
   }
}

/**
 * @private       sv_GM_GetSerialNumber
 * @brief         This command and function helps with Serial Number retrieval.
 * @param[in]     None
 * @param[out]    None
 * @param[inout]  cpt_IPCEvent - Pointer to IPC Event structure which contains
 *                the command and data received from RH850. This pointer would
 *                also be used to send the response back to RH850.
 * @return        None
 */
static void sv_GM_GetSerialNumber(IPCMessage_T *cpt_IPCEvent)
{
   ql_errcode_dev_e e_ret = QL_DEV_SUCCESS;
   char c_devInfo[MAX_DEVICE_INFO_LENGTH] = {0};

   e_ret = ql_dev_get_sn(c_devInfo, MAX_DEVICE_INFO_LENGTH, 0);
   if(e_ret == QL_DEV_SUCCESS)
   {

      cpt_IPCEvent->eStatus = eIPC_STATUS_OK;
      cpt_IPCEvent->tDataLength = strlen((char *)c_devInfo);
      memcpy(cpt_IPCEvent->u8a_Data, c_devInfo, cpt_IPCEvent->tDataLength);
#ifdef DEBUG_ENABLED
      BAL_GENERAL_APP_LOG("Serial Number: %s, Length : %d", c_devInfo, cpt_IPCEvent->tDataLength);
#endif
   }
   else
   {

      cpt_IPCEvent->eStatus = eIPC_STATUS_ERROR;
      cpt_IPCEvent->tDataLength = GM_ERROR_RESPONSE_LENGTH;
      cpt_IPCEvent->u8a_Data[0] = GM_SERIAL_NUMBER_RETRIVAL_FAILED_RESPONSE;
   }
}

/**
 * @private       sv_GM_GetProductID
 * @brief         This command and function helps with Modems Product ID
 *                retrieval.
 * @param[in]     None
 * @param[out]    None
 * @param[inout]  cpt_IPCEvent - Pointer to IPC Event structure which contains
 *                the command and data received from RH850. This pointer would
 *                also be used to send the response back to RH850.
 * @return        None
 */
static void sv_GM_GetProductID(IPCMessage_T *cpt_IPCEvent)
{
   ql_errcode_dev_e e_ret = QL_DEV_SUCCESS;
   char c_devInfo[MAX_DEVICE_INFO_LENGTH] = {0};

   e_ret = ql_dev_get_product_id(c_devInfo, MAX_DEVICE_INFO_LENGTH);
   if(e_ret == QL_DEV_SUCCESS)
   {

      cpt_IPCEvent->eStatus = eIPC_STATUS_OK;
      cpt_IPCEvent->tDataLength = strlen((char *)c_devInfo);
      memcpy(cpt_IPCEvent->u8a_Data, c_devInfo, cpt_IPCEvent->tDataLength);
#ifdef DEBUG_ENABLED
      BAL_GENERAL_APP_LOG("Product ID: %s, Length : %d", c_devInfo, cpt_IPCEvent->tDataLength);
#endif
   }
   else
   {

      cpt_IPCEvent->eStatus = eIPC_STATUS_ERROR;
      cpt_IPCEvent->tDataLength = GM_ERROR_RESPONSE_LENGTH;
      cpt_IPCEvent->u8a_Data[0] = GM_PRODUCT_ID_RETRIVAL_FAILED_RESPONSE;
   }

}


/**
 * @private       sv_GM_GetModelInfo
 * @brief         This command and function helps with Modems Model Info
 *                retrieval.
 * @param[in]     None
 * @param[out]    None
 * @param[inout]  cpt_IPCEvent - Pointer to IPC Event structure which contains
 *                the command and data received from RH850. This pointer would
 *                also be used to send the response back to RH850.
 * @return        None
 */
static void sv_GM_GetModelInfo(IPCMessage_T *cpt_IPCEvent)
{
   ql_errcode_dev_e e_ret = QL_DEV_SUCCESS;
   char c_devInfo[MAX_DEVICE_INFO_LENGTH] = {0};

   e_ret = ql_dev_get_model(c_devInfo, MAX_DEVICE_INFO_LENGTH);
   if(e_ret == QL_DEV_SUCCESS)
   {

      cpt_IPCEvent->eStatus = eIPC_STATUS_OK;
      cpt_IPCEvent->tDataLength = strlen((char *)c_devInfo);
      memcpy(cpt_IPCEvent->u8a_Data, c_devInfo, cpt_IPCEvent->tDataLength);
#ifdef DEBUG_ENABLED
      BAL_GENERAL_APP_LOG("Model Info: %s, Length : %d", c_devInfo, cpt_IPCEvent->tDataLength);
#endif
   }
   else
   {

      cpt_IPCEvent->eStatus = eIPC_STATUS_ERROR;
      cpt_IPCEvent->tDataLength = GM_ERROR_RESPONSE_LENGTH;
      cpt_IPCEvent->u8a_Data[0] = GM_MODEL_INFO_RETRIVAL_FAILED_RESPONSE;
   }

}


/**
 * @private       sv_GM_GetFirmwareVersion
 * @brief         This command and function helps with Modems Firmware version
 *                retrieval.
 * @param[in]     None
 * @param[out]    None
 * @param[inout]  cpt_IPCEvent - Pointer to IPC Event structure which contains
 *                the command and data received from RH850. This pointer would
 *                also be used to send the response back to RH850.
 * @return        None
 */
static void sv_GM_GetFirmwareVersion(IPCMessage_T *cpt_IPCEvent)
{
   ql_errcode_dev_e e_ret = QL_DEV_SUCCESS;
   char c_devInfo[MAX_DEVICE_INFO_LENGTH] = {0};

   e_ret = ql_dev_get_firmware_version(c_devInfo, MAX_DEVICE_INFO_LENGTH);
   if(e_ret == QL_DEV_SUCCESS)
   {

      cpt_IPCEvent->eStatus = eIPC_STATUS_OK;
      cpt_IPCEvent->tDataLength = strlen((char *)c_devInfo);
      memcpy(cpt_IPCEvent->u8a_Data, c_devInfo, cpt_IPCEvent->tDataLength);
#ifdef DEBUG_ENABLED
      BAL_GENERAL_APP_LOG("FW Version: %s, Length : %d", c_devInfo, cpt_IPCEvent->tDataLength);
#endif
   }
   else
   {

      cpt_IPCEvent->eStatus = eIPC_STATUS_ERROR;
      cpt_IPCEvent->tDataLength = GM_ERROR_RESPONSE_LENGTH;
      cpt_IPCEvent->u8a_Data[0] = GM_FIRMWARE_VERSION_RETRIVAL_FAILED_RESPONSE;
   }

}


/**
 * @private       sv_GM_GetSubVersion
 * @brief         This command and function helps with Modems Firmware subversion
 *                retrieval.
 * @param[in]     None
 * @param[out]    None
 * @param[inout]  cpt_IPCEvent - Pointer to IPC Event structure which contains
 *                the command and data received from RH850. This pointer would
 *                also be used to send the response back to RH850.
 * @return        None
 */
static void sv_GM_GetSubVersion(IPCMessage_T *cpt_IPCEvent)
{
   ql_errcode_dev_e e_ret = QL_DEV_SUCCESS;
   char c_devInfo[MAX_DEVICE_INFO_LENGTH] = {0};

   e_ret = ql_dev_get_firmware_subversion(c_devInfo, MAX_DEVICE_INFO_LENGTH);
   if(e_ret == QL_DEV_SUCCESS)
   {

      cpt_IPCEvent->eStatus = eIPC_STATUS_OK;
      cpt_IPCEvent->tDataLength = strlen((char *)c_devInfo);
      memcpy(cpt_IPCEvent->u8a_Data, c_devInfo, cpt_IPCEvent->tDataLength);
#ifdef DEBUG_ENABLED
      BAL_GENERAL_APP_LOG("Subversion: %s, Length : %d", c_devInfo, cpt_IPCEvent->tDataLength);
#endif
   }
   else
   {

      cpt_IPCEvent->eStatus = eIPC_STATUS_ERROR;
      cpt_IPCEvent->tDataLength = GM_ERROR_RESPONSE_LENGTH;
      cpt_IPCEvent->u8a_Data[0] = GM_FIRMWARE_SUBVERSION_RETRIVAL_FAILED_RESPONSE;
   }

}


/**
 * @private       sv_GM_GetTemperature
 * @brief         This command and function helps with Modems temperature
 *                retrieval.
 * @param[in]     None
 * @param[out]    None
 * @param[inout]  cpt_IPCEvent - Pointer to IPC Event structure which contains
 *                the command and data received from RH850. This pointer would
 *                also be used to send the response back to RH850.
 * @return        None
 */
static void sv_GM_GetTemperature(IPCMessage_T *cpt_IPCEvent)
{
   ql_errcode_dev_e e_ret = QL_DEV_SUCCESS;
   int32_t i32_temp = 0;

   e_ret = ql_dev_get_temp_value(&i32_temp);
   if(e_ret == QL_DEV_SUCCESS)
   {
      cpt_IPCEvent->eStatus = eIPC_STATUS_OK;
      cpt_IPCEvent->tDataLength = sizeof(int32_t);
      memcpy(cpt_IPCEvent->u8a_Data, &i32_temp, cpt_IPCEvent->tDataLength);
#ifdef DEBUG_ENABLED
      BAL_GENERAL_APP_LOG("Temperature: %d, Length : %d", i32_temp, cpt_IPCEvent->tDataLength);
#endif
   }
   else
   {
      cpt_IPCEvent->eStatus = eIPC_STATUS_ERROR;
      cpt_IPCEvent->tDataLength = GM_ERROR_RESPONSE_LENGTH;
      cpt_IPCEvent->u8a_Data[0] = GM_TEMPERATURE_RETRIVAL_FAILED_RESPONSE;
   }

}

/**
 * @private       sv_GM_GetVIN
 * @brief         This command and function helps with Modems stored VIN
 *                retrieval.
 * @param[in]     None
 * @param[out]    None
 * @param[inout]  cpt_IPCEvent - Pointer to IPC Event structure which contains
 *                the command and data received from RH850. This pointer would
 *                also be used to send the response back to RH850.
 * @return        None
 */
static void sv_GM_GetVIN (IPCMessage_T *cpt_IPCEvent)
{
#ifdef DEBUG_ENABLED
      BAL_GENERAL_APP_LOG("VIN Requested");
#endif

   // Respond with VIN after reading from the file
   FileSystemReq_T st_fsReq = {0};
   FileSystemResult_E e_fsReturn = eFS_INTERNAL_ERROR;

   st_fsReq.e_FileId = eFS_FILE_VIN;
   st_fsReq.u32_RequestedBytes = SIZE_OF_VIN;
   st_fsReq.vpt_Buffer = (void*)cpt_IPCEvent->u8a_Data;
   e_fsReturn = ge_FS_Read(&st_fsReq);

   if((e_fsReturn == eFS_OK) && (st_fsReq.u32_ActualBytes == st_fsReq.u32_RequestedBytes))
   {
      cpt_IPCEvent->eStatus = eIPC_STATUS_OK;
      cpt_IPCEvent->tDataLength = st_fsReq.u32_ActualBytes;
   }
   else
   {
      cpt_IPCEvent->eStatus = eIPC_STATUS_ERROR;
      cpt_IPCEvent->tDataLength = GM_ERROR_RESPONSE_LENGTH;
      cpt_IPCEvent->u8a_Data[0] = GM_VIN_ABSENT_RESPONSE;
   }
}

/**
 * @private       sv_GM_GetSimStatus
 * @brief         This command and function helps with Modems SIM Status.
 * @param[in]     None
 * @param[out]    None
 * @param[inout]  cpt_IPCEvent - Pointer to IPC Event structure which contains
 *                the command and data received from RH850. This pointer would
 *                also be used to send the response back to RH850.
 * @return        None
 */
static void sv_GM_GetSimStatus(IPCMessage_T *cpt_IPCEvent)
{
   ql_sim_errcode_e e_ret = QL_SIM_SUCCESS;
   ql_sim_status_e ept_cardStatus = QL_SIM_STATUS_READY;

   e_ret = ql_sim_get_card_status(0, &ept_cardStatus);

   if(e_ret == QL_SIM_SUCCESS)
   {

      cpt_IPCEvent->eStatus = eIPC_STATUS_OK;
      cpt_IPCEvent->tDataLength = sizeof(ql_sim_status_e);
      memcpy(cpt_IPCEvent->u8a_Data, &ept_cardStatus, cpt_IPCEvent->tDataLength);
#ifdef DEBUG_ENABLED
      BAL_GENERAL_APP_LOG("Card Status: %d, Length : %d", ept_cardStatus, cpt_IPCEvent->tDataLength);
#endif
   }
   else
   {

      cpt_IPCEvent->eStatus = eIPC_STATUS_ERROR;
      cpt_IPCEvent->tDataLength = GM_ERROR_RESPONSE_LENGTH;
      cpt_IPCEvent->u8a_Data[0] = GM_SIM_STATUS_RETRIVAL_FAILED_RESPONSE;
   }

}

/**
 * @private       sv_GM_GetNWStatus
 * @brief         This command and function helps with Modems Neytwork Status
 *                Retrival.
 * @param[in]     None
 * @param[out]    None
 * @param[inout]  cpt_IPCEvent - Pointer to IPC Event structure which contains
 *                the command and data received from RH850. This pointer would
 *                also be used to send the response back to RH850.
 * @return        None
 */
static void sv_GM_GetNWStatus(IPCMessage_T *cpt_IPCEvent)
{
   ql_nw_errcode_e e_ret = QL_NW_SUCCESS;
   ql_nw_reg_status_info_s reg_info = {0};

   e_ret = ql_nw_get_reg_status(0, &reg_info);
   if(e_ret == QL_NW_SUCCESS)
   {

      cpt_IPCEvent->eStatus = eIPC_STATUS_OK;
      cpt_IPCEvent->tDataLength = sizeof(ql_nw_reg_status_info_s);
      memcpy(cpt_IPCEvent->u8a_Data, &reg_info, cpt_IPCEvent->tDataLength);
#ifdef DEBUG_ENABLED
      BAL_GENERAL_APP_LOG("Network Status Voice State: %d, Data State : %d Length : %d", reg_info.voice_reg.state, reg_info.data_reg.state, cpt_IPCEvent->tDataLength);
#endif
   }
   else
   {

      cpt_IPCEvent->eStatus = eIPC_STATUS_ERROR;
      cpt_IPCEvent->tDataLength = GM_ERROR_RESPONSE_LENGTH;
      cpt_IPCEvent->u8a_Data[0] = GM_NW_STATUS_RETRIVAL_FAILED_RESPONSE;
   }
}

/**
 * @private       sv_GM_GetSimStatus
 * @brief         This command and function helps with Modems SIM Status.
 * @param[in]     None
 * @param[out]    None
 * @param[inout]  cpt_IPCEvent - Pointer to IPC Event structure which contains
 *                the command and data received from RH850. This pointer would
 *                also be used to send the response back to RH850.
 * @return        None
 */
static void sv_GM_GetNWStrength (IPCMessage_T *cpt_IPCEvent)
{
   ql_nw_errcode_e e_ret = QL_NW_SUCCESS;
   ql_nw_signal_strength_info_s signal_info = {0};

   e_ret = ql_nw_get_signal_strength(0, &signal_info);
   if(e_ret == QL_NW_SUCCESS)
   {

      cpt_IPCEvent->eStatus = eIPC_STATUS_OK;
      cpt_IPCEvent->tDataLength = sizeof(ql_nw_signal_strength_info_s);
      memcpy(cpt_IPCEvent->u8a_Data, &signal_info, cpt_IPCEvent->tDataLength);
#ifdef DEBUG_ENABLED
      BAL_GENERAL_APP_LOG("Network Strength RSSI: %d, Length : %d", signal_info.rssi, cpt_IPCEvent->tDataLength);
#endif
   }
   else
   {

      cpt_IPCEvent->eStatus = eIPC_STATUS_ERROR;
      cpt_IPCEvent->tDataLength = GM_ERROR_RESPONSE_LENGTH;
      cpt_IPCEvent->u8a_Data[0] = GM_NW_STRENGTH_RETRIVAL_FAILED_RESPONSE;
   }

}


/**
 * @private       sv_GM_GetNWTime
 * @brief         This command and function helps with Modems Network Time retrival.
 * @param[in]     None
 * @param[out]    None
 * @param[inout]  cpt_IPCEvent - Pointer to IPC Event structure which contains
 *                the command and data received from RH850. This pointer would
 *                also be used to send the response back to RH850.
 * @return        None
 */
static void sv_GM_GetNWTime (IPCMessage_T *cpt_IPCEvent)
{
   // @R : TBD: NTP time retrieval from modem and return to RH850
   // For now we are implemneting with NITZ
   ql_nw_errcode_e e_ret = QL_NW_SUCCESS;
   ql_nw_nitz_time_info_s nitz_info = {0};
   e_ret = ql_nw_get_nitz_time_info(&nitz_info);
   // Pack and send the absolute time to RH850
   if(e_ret == QL_NW_SUCCESS)
   {

      cpt_IPCEvent->eStatus = eIPC_STATUS_OK;
      cpt_IPCEvent->tDataLength = sizeof(long);
      memcpy(cpt_IPCEvent->u8a_Data, &nitz_info.abs_time, cpt_IPCEvent->tDataLength);
#ifdef DEBUG_ENABLED
      BAL_GENERAL_APP_LOG("Network Time ABS: %d, NITZ : %s, Length : %d", nitz_info.abs_time, nitz_info.nitz_time, cpt_IPCEvent->tDataLength);
#endif
   }
   else
   {

      cpt_IPCEvent->eStatus = eIPC_STATUS_ERROR;
      cpt_IPCEvent->tDataLength = GM_ERROR_RESPONSE_LENGTH;
      cpt_IPCEvent->u8a_Data[0] = GM_NW_TIME_RETRIVAL_FAILED_RESPONSE;
   }

}


/**
 * @private       sv_GM_Task
 * @brief         This is the main task of General manager Application.
 *                The task is responsible for handling all thh eevents and
 *                commands received from RH850 and respond to them.
 * @param[in]     None
 * @param[out]    None
 * @param[inout]  None
 * @return        None
 * @note          This is the one of the Application Manager which does not take
 *                explicite permission from the Application Monitor to run.
 */
static void sv_GM_Task(void *vpt_Argument)
{
   (void)vpt_Argument;
   IPCMessage_T st_IPCEvent;
   ql_queue_t pt_Queue = NULL;
   TelSysResponse_E eRetVal = eTEL_NOK;

   while (TRUE)
   {
      if (ql_rtos_queue_wait(st_GeneralManagerCtx.pt_Queue,
                             (uint8_t *)&st_IPCEvent,
                             sizeof(IPCMessage_T),
                             QL_WAIT_FOREVER) != QL_OSI_SUCCESS)
      {
#ifdef DEBUG_ENABLED
         BAL_GENERAL_APP_LOG("Pend on General Manager Queue Failed");
#endif // DEBUG_ENABLED
      }

      switch (st_IPCEvent.eServiceID)
      {
         case eSERVICE_ID_DATA_SEND_ACK:
         case eSERVICE_ID_DATA_SEND_NACK:
         {
            st_GeneralManagerCtx.b_responseRequired = FALSE;
#ifdef DEBUG_ENABLED
            BAL_GENERAL_APP_LOG("ACK or NACK on Data Send Received");
#endif
            break;
         }
         case eSERVICE_ID_GENERAL_SET_DEBUG_MODE:
         {
#ifdef DEBUG_ENABLED
            BAL_GENERAL_APP_LOG("Debug Mode CMD");
#endif // DEBUG_ENABLED
            st_GeneralManagerCtx.b_responseRequired = TRUE;
            sv_GM_SetDebugMode(&st_IPCEvent);
            break;
         }
         case eSERVICE_ID_GENERAL_GET_IMEI:
         {
            st_GeneralManagerCtx.b_responseRequired = TRUE;
#ifdef DEBUG_ENABLED
            BAL_GENERAL_APP_LOG("IMEI Request");
#endif // DEBUG_ENABLED
            sv_GM_GetIMEI(&st_IPCEvent);
            break;
         }
         case eSERVICE_ID_GENERAL_GET_ICCID:
         {
#ifdef DEBUG_ENABLED
            BAL_GENERAL_APP_LOG("ICCID Request");
#endif // DEBUG_ENABLED
            st_GeneralManagerCtx.b_responseRequired = TRUE;
            sv_GM_GetICCID(&st_IPCEvent);
            break;
         }
         case eSERVICE_ID_GENERAL_GET_SERIAL_NUMBER:
         {
#ifdef DEBUG_ENABLED
            BAL_GENERAL_APP_LOG("Serial Number Request");
#endif // DEBUG_ENABLED
            st_GeneralManagerCtx.b_responseRequired = TRUE;
            sv_GM_GetSerialNumber(&st_IPCEvent);

            break;
         }
         case eSERVICE_ID_GENERAL_GET_PRODUCT_ID:
         {
#ifdef DEBUG_ENABLED
            BAL_GENERAL_APP_LOG("Product ID Request");
#endif // DEBUG_ENABLED
            st_GeneralManagerCtx.b_responseRequired = TRUE;
            sv_GM_GetProductID(&st_IPCEvent);
            break;
         }
         case eSERVICE_ID_GENERAL_GET_MODEL_INFO:
         {
#ifdef DEBUG_ENABLED
            BAL_GENERAL_APP_LOG("Model ID Request");
#endif // DEBUG_ENABLED
            st_GeneralManagerCtx.b_responseRequired = TRUE;
            sv_GM_GetModelInfo(&st_IPCEvent);
            break;
         }
         case eSERVICE_ID_GENERAL_GET_FIRMWARE_VERSION:
         {
#ifdef DEBUG_ENABLED
            BAL_GENERAL_APP_LOG("Firmware Version Request");
#endif // DEBUG_ENABLED
            st_GeneralManagerCtx.b_responseRequired = TRUE;
            sv_GM_GetFirmwareVersion(&st_IPCEvent);
            break;
         }
         case eSERVICE_ID_GENERAL_GET_SUBVERSION:
         {
#ifdef DEBUG_ENABLED
            BAL_GENERAL_APP_LOG("Sub Version Request");
#endif // DEBUG_ENABLED
            st_GeneralManagerCtx.b_responseRequired = TRUE;
            sv_GM_GetSubVersion(&st_IPCEvent);
            break;
         }
         case eSERVICE_ID_GENERAL_GET_TEMPERATURE:
         {
#ifdef DEBUG_ENABLED
            BAL_GENERAL_APP_LOG("Temperature Request");
#endif // DEBUG_ENABLED
            st_GeneralManagerCtx.b_responseRequired = TRUE;
            sv_GM_GetTemperature(&st_IPCEvent);
            break;
         }
         case eSERVICE_ID_GENERAL_GET_VIN:
         {
#ifdef DEBUG_ENABLED
            BAL_GENERAL_APP_LOG("VIN Request");
#endif // DEBUG_ENABLED
            st_GeneralManagerCtx.b_responseRequired = TRUE;
            sv_GM_GetVIN(&st_IPCEvent);
            break;
         }
         case eSERVICE_ID_GENERAL_GET_SIM_STATUS:
         {
#ifdef DEBUG_ENABLED
            BAL_GENERAL_APP_LOG("SIM Status Request");
#endif // DEBUG_ENABLED
            st_GeneralManagerCtx.b_responseRequired = TRUE;
            sv_GM_GetSimStatus(&st_IPCEvent);
            break;
         }
         case eSERVICE_ID_GENERAL_GET_NW_STATUS:
         {
#ifdef DEBUG_ENABLED
            BAL_GENERAL_APP_LOG("NW Status Request");
#endif // DEBUG_ENABLED
            st_GeneralManagerCtx.b_responseRequired = TRUE;
            sv_GM_GetNWStatus(&st_IPCEvent);
            break;
         }
         case eSERVICE_ID_GENERAL_GET_NW_STRENGTH:
         {
#ifdef DEBUG_ENABLED
            BAL_GENERAL_APP_LOG("NW Strength Request");
#endif // DEBUG_ENABLED
            st_GeneralManagerCtx.b_responseRequired = TRUE;
            sv_GM_GetNWStrength(&st_IPCEvent);
            break;
         }
         case eSERVICE_ID_GENERAL_GET_NW_TIME:
         {
#ifdef DEBUG_ENABLED
            BAL_GENERAL_APP_LOG("NW Time Request");
#endif // DEBUG_ENABLED
            st_GeneralManagerCtx.b_responseRequired = TRUE;
            sv_GM_GetNWTime(&st_IPCEvent);
            break;
         }

         case eSERVICE_ID_GENERAL_GET_STATUS:
         default:
         {
#ifdef DEBUG_ENABLED
            BAL_GENERAL_APP_LOG("General Status/ Unrecognised Request");
#endif // DEBUG_ENABLED
            st_GeneralManagerCtx.b_responseRequired = TRUE;
            st_IPCEvent.eStatus = eIPC_STATUS_NOT_SUPPORTED;
            st_IPCEvent.tDataLength = GM_ERROR_RESPONSE_LENGTH;
            st_IPCEvent.u8a_Data[0] = GM_INVALID_UNSUPPORTED_CMD_RESPONSE;
         }
            break;
      }

      if(st_GeneralManagerCtx.b_responseRequired == TRUE)
      {
         // Send the response back to RH850
         st_IPCEvent.eSourceNode = eIPC_NODE_APP_GENERAL_STATUS;
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
               BAL_GENERAL_APP_LOG("Send IPC Event to UART Manager Queue Failed");
   #endif // DEBUG_ENABLED
            }
            else
            {
   #ifdef DEBUG_ENABLED
               BAL_GENERAL_APP_LOG("Send IPC Event to UART Manager Queue Success");
   #endif // DEBUG_ENABLED
            }
         }
         else
         {
   #ifdef DEBUG_ENABLED
            BAL_GENERAL_APP_LOG("Get UART Manager Queue Failed");
   #endif // DEBUG_ENABLED
         }
      }
      else
      {
         // Response Not required
      }

   }

   // @R: TBD, check if theres a way to utilize the following API :
   //  Try tinkering around with this
   // ql_log_set_port(QL_LOG_PORT_UART);//set the log port as USB AP port
}



/******************************************************************************/
/*                                                                            */
/*                        PUBLIC FUNCTION DEFINITIONS                         */
/*                                                                            */
/******************************************************************************/
/**
 * @public        ge_GENERAL_Init
 * @brief         Function initializes the context of General Application
 *                Manager.
 * @param[in]     None
 * @param[out]    None
 * @param[inout]  None
 * @return        Return the status of General manager initialization.
 */
TelSysResponse_E ge_GENERAL_Init(void)
{
   TelSysResponse_E eRetVal = eTEL_NOK;
   // Reste the General Manager Context at every reset
   memset(&st_GeneralManagerCtx, 0, sizeof(GeneralManagerCtx_T));
   // Create a queue for General Manager Task
   if(ql_rtos_queue_create(&st_GeneralManagerCtx.pt_Queue, sizeof(IPCMessage_T),
                              GENERAL_MANAGER_QUEUE_LENGTH) == QL_OSI_SUCCESS)
   {
#ifdef DEBUG_ENABLED
      BAL_GENERAL_APP_LOG("General Manager Queue Init Success");
#endif // DEBUUG_ENABLED
      // Register the Queue for IPC Message Routing
      ge_Common_RegisterIPCQueue(eIPC_NODE_APP_GENERAL_STATUS, st_GeneralManagerCtx.pt_Queue);
      // Register the General Manager Task
      if(ql_rtos_task_create(&st_GeneralManagerCtx.pt_Task, GENERAL_MANAGER_STACK_SIZE,
            GENERAL_MANAGER_TASK_PRIORITY, "BALGENERALMGR", sv_GM_Task, NULL,
            GENERAL_MANAGER_TASK_EVENT_COUNT) == QL_OSI_SUCCESS)
      {
#ifdef DEBUG_ENABLED
         BAL_GENERAL_APP_LOG("General Manager Task Registration Success");
#endif // DEBUG_ENABLED
         eRetVal = eTEL_OK;
      }
      else
      {
#ifdef DEBUG_ENABLED
         BAL_GENERAL_APP_LOG("General Manager Task Registration Failed");
#endif // DEBUG_ENABLED
      }
   }
   else
   {
#ifdef DEBUG_ENABLED
      BAL_GENERAL_APP_LOG("General Manager Init Failed");
#endif // DEBUUG_ENABLED
   }

   return eRetVal;
}

/**
 * Copyright(c) Bajaj Auto Technology Limited (BATL) as an unpublished work.
 * THIS SOFTWARE AND/OR MATERIAL IS THE PROPERTY OF BATL.
 * ALL USE, DISCLOSURE, AND/OR REPRODUCTION NOT SPECIFICALLY AUTHORIZED BY
 * BATL IS PROHIBITED.
 *
 * @author: Yash Giramkar [YSG]
 */
