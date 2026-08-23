/**
 * @file          ErrorCodes.h
 * @brief         Header file containing error codes typedefs.
 * @date          14/07/2026
 * @author        Yash Giramkar [YSG]
 * @copyright     Bajaj Auto Technology Limited (BATL)
 */

#ifndef _ERROR_CODES_H
#define _ERROR_CODES_H

/******************************************************************************/
/*                                                                            */
/*                                  INCLUDES                                  */
/*                                                                            */
/******************************************************************************/
// Standard Libraries
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

// OS Libraries
#include "ql_api_osi.h"
#include "ql_api_dev.h"
#include "ql_log.h"
#include "ql_api_sim.h"
#include "ql_api_nw.h"

// User defined Libraries
#include "ErrorCodes.h"
#include "IPCCommands.h"
#include "IPCTypes.h"
#include "AppIDs.h"
#include "ServiceIDs.h"
#include "SysTypes.h"

/******************************************************************************/
/*                                                                            */
/*                                  DEFINES                                   */
/*                                                                            */
/******************************************************************************/
/**
 * @def           DEBUG_ENABLED
 * @brief         Flag to ernable or disable system level debug logging.
 * @note          Comment this flag when you want to disable debug.
 */
#define DEBUG_ENABLED

/******************************************************************************/
/*                                                                            */
/*                                   ENUMS                                    */
/*                                                                            */
/******************************************************************************/
/**
 * @enum          <Enum name>
 * @brief         <Enum details>.
 */
/******************************************************************************/
/*                                                                            */
/*                                 STRUCTURES                                 */
/*                                                                            */
/******************************************************************************/
/**
 * @struct        AppIDtoIPCNodeMap_T
 * @brief         Structure to map Application IDs to IPC Node IDs.
 *                This mapping is used to route IPC messages between applications.
 */
typedef struct
{
   Application_E e_appID;
   IPCNodes_E e_nodeID;
} AppIDtoIPCNodeMap_T;

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
extern TelSysResponse_E ge_Common_RegisterIPCQueue(IPCNodes_E e_nodeID, ql_queue_t pt_Queue);
extern TelSysResponse_E ge_Common_GetIPCQueue(IPCNodes_E e_nodeID, ql_queue_t* ppt_Queue);
extern IPCNodes_E ge_Common_GetAppQueue(Application_E e_appID, ql_queue_t* ppt_Queue);
extern Application_E ge_Common_GetAppIDFromIPCNode(IPCNodes_E e_nodeID);
#endif //!_ERROR_CODES_H

/**
 * Copyright(c) Bajaj Auto Technology Limited (BATL) as an unpublished work.
 * THIS SOFTWARE AND/OR MATERIAL IS THE PROPERTY OF BATL.
 * ALL USE, DISCLOSURE, AND/OR REPRODUCTION NOT SPECIFICALLY AUTHORIZED BY
 * BATL IS PROHIBITED.
 *
 * @author: Yash Giramkar [YSG]
 */

