/**
 * @file          Sample_Format.c
 * @brief         Source file containing <Details>
 * @date          <Date of generating C file - DD/MM/YY>
 * @author        <Author of C file - Name [Initials]>
 * @copyright     Bajaj Auto Technology Limited (BATL)
 */

/******************************************************************************/
/*                                                                            */
/*                                  INCLUDES                                  */
/*                                                                            */
/******************************************************************************/
#include "Generics.h"

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
 * @var           spta_IPCQueues
 * @brief         Global Array to hold the IPC queues for all the nodes.
 *                The array is indexed by IPCNodes_E.
 *                Each element of the array is a pointer to the queue associated
 *                with the corresponding application.
 */
static ql_queue_t spta_IPCQueues[eIPC_NODE_MAX] = {NULL};



/**
 * @var           ga_appIDtoIPCNodeMap
 * @brief         Global Array to map Application IDs to IPC Node IDs.
 */

static const AppIDtoIPCNodeMap_T sst_AppIDtoIPCNodeMap[eAPPLICATION_LAST_APPLICATION] =
{
   {eAPPLICATION_IDLE, eIPC_NODE_MAX},
   {eAPPLICATION_GENERAL_STATUS, eIPC_NODE_APP_GENERAL_STATUS},
   {eAPPLICATION_PROVISIONING, eIPC_NODE_APP_PROVISIONING},
   {eAPPLICATION_OFFLINE_ANALYTICS, eIPC_NODE_APP_OFFLINE_ANALYTICS},
   {eAPPLICATION_DATA_LOGGER, eIPC_NODE_APP_DATA_LOGGER},
   {eAPPLICATION_BALNET_FOTA, eIPC_NODE_APP_BALNET_FOTA},
   {eAPPLICATION_UDS_FOTA, eIPC_NODE_APP_UDS_FOTA},
   {eAPPLICATION_MODEM_FOTA, eIPC_NODE_APP_MODEM_FOTA},
   {eAPPLICATION_COMMAND_MANAGER, eIPC_NODE_APP_COMMAND_MANAGER},
   {eAPPLICATION_MANIFEST, eIPC_NODE_APP_MANIFEST}
};





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
 * @public        ge_Common_RegisterIPCQueue
 * @brief         This function registers the queue for a specific node
 *                in the global IPC queue array.
 * @param[in]     e_nodeID : Node ID for which the queue is being registered.
 * @param[in]     pt_Queue : Pointer to the queue to be registered for the
 *                specified node.
 * @return        eTEL_OK if the queue is successfully registered.
 *                eTEL_NOK if the node ID is invalid or the queue pointer is NULL.
 */
TelSysResponse_E ge_Common_RegisterIPCQueue(IPCNodes_E e_nodeID, ql_queue_t pt_Queue)
{
   TelSysResponse_E eRetVal = eTEL_NOK;

   if ((e_nodeID < eIPC_NODE_MAX) && (pt_Queue != NULL))
   {
      spta_IPCQueues[e_nodeID] = pt_Queue;
      eRetVal = eTEL_OK;
   }

   return eRetVal;
}


/**
 * @public        ge_Common_GetIPCQueue
 * @brief         This function retrieves the queue for a specific node.
 *                It checks if the node ID is valid and if the output pointer is not NULL.
 *                If both conditions are met, it assigns the corresponding queue
 *                to the output pointer and returns eTEL_OK. Otherwise,
 *                it returns eTEL_NOK.
 * @param[in]     e_nodeID : The node ID for which the queue is to be retrieved.
 * @param[out]    ppt_Queue : Pointer to the queue pointer where the node queue will be stored.
 * @param[inout]  None
 * @return        eTEL_OK if the application queue is successfully retrieved.
 *                eTEL_NOK if the application ID is invalid or the output pointer is NULL.
 */
TelSysResponse_E ge_Common_GetIPCQueue(IPCNodes_E e_nodeID, ql_queue_t* ppt_Queue)
{
   TelSysResponse_E eRetVal = eTEL_NOK;

   if ((e_nodeID < eIPC_NODE_MAX) && (ppt_Queue != NULL))
   {
      *ppt_Queue = spta_IPCQueues[e_nodeID];
      eRetVal = eTEL_OK;
   }

   return eRetVal;
}

/**
 * @public        ge_Common_GetAppQueue
 * @brief         This function retrieves the queue for a specific application.
 * @param[in]     e_appID : The application ID for which the queue is to be retrieved.
 * @param[out]    ppt_Queue : Pointer to the queue pointer where the application queue will be stored.
 * @return        The IPC node ID corresponding to the application if successful,
 *                or eIPC_NODE_MAX if the application ID is invalid or the output pointer is NULL.
 */
IPCNodes_E ge_Common_GetAppQueue(Application_E e_appID, ql_queue_t* ppt_Queue)
{
   IPCNodes_E eRetVal = eIPC_NODE_MAX;

   if ((e_appID < eAPPLICATION_LAST_APPLICATION) && (ppt_Queue != NULL))
   {
      IPCNodes_E e_nodeID = sst_AppIDtoIPCNodeMap[e_appID].e_nodeID;
      *ppt_Queue = spta_IPCQueues[e_nodeID];
      eRetVal = e_nodeID;
   }

   return eRetVal;
}

/**
 * @public        ge_Common_GetAppIDFromIPCNode
 * @brief         This function retrieves the application ID basis an IPC Node ID
 * @param[in]     e_nodeID : The IPC Node ID for which the application ID is to be retrieved.
 * @param[out]    None
 * @param[inout]    None
 * @return        Application_E App ID corresponding to the incoming IPCNodes_E Node ID.
 */
Application_E ge_Common_GetAppIDFromIPCNode(IPCNodes_E e_nodeID)
{
   Application_E eRetVal = eAPPLICATION_LAST_APPLICATION;

   for (int i = 0; i < eAPPLICATION_LAST_APPLICATION; ++i)
   {
      if (sst_AppIDtoIPCNodeMap[i].e_nodeID == e_nodeID)
      {
         eRetVal = sst_AppIDtoIPCNodeMap[i].e_appID;
         break;
      }
   }

   return eRetVal;
}


/**
 * Copyright(c) Bajaj Auto Technology Limited (BATL) as an unpublished work.
 * THIS SOFTWARE AND/OR MATERIAL IS THE PROPERTY OF BATL.
 * ALL USE, DISCLOSURE, AND/OR REPRODUCTION NOT SPECIFICALLY AUTHORIZED BY
 * BATL IS PROHIBITED.
 *
 * @author:Yash Giramkar [YSG]
 */
