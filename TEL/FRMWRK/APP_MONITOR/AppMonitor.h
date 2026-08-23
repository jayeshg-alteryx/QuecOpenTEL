/**
 * @file          AppMonitor.h
 * @brief         Header file containing the application monitor module.
 *                This module is responsible for monitoring currently running
 *                applicattions and provide access to run an application, if
 *                in case not application is currenlty running.
 *                This module also exposes the APIs to get the current running
 *                application.
 * @date          04/08/26
 * @author        Yash Giramkar [YSG], Shubham Jadhav [SJ]
 * @copyright     Bajaj Auto Technology Limited (BATL)
 */

#ifndef _APP_MONITOR_H
#define _APP_MONITOR_H

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
 * @struct        <Structure name>
 * @brief         <Structure details>.
 */

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
extern TelSysResponse_E ge_AppMonitor_Init(void);
extern TelSysResponse_E ge_AppMonitor_RunApplication(Application_E e_requestedAppID);
extern TelSysResponse_E ge_AppMonitor_StopCurrentRunningApp(Application_E e_requesterAppID);

extern Application_E ge_AppMonitor_GetCurrentRunningApp(void);
extern Application_E ge_AppMonitor_GetLastRunningApp(void);
extern int64_t       gi64_AppMonitor_GetCurrentRunningTime(void);
extern int64_t       gi64_AppMonitor_GetLastExecutionRuntime(void);

#endif //!_APP_MONITOR_H

/**
 * Copyright(c) Bajaj Auto Technology Limited (BATL) as an unpublished work.
 * THIS SOFTWARE AND/OR MATERIAL IS THE PROPERTY OF BATL.
 * ALL USE, DISCLOSURE, AND/OR REPRODUCTION NOT SPECIFICALLY AUTHORIZED BY
 * BATL IS PROHIBITED.
 *
 * @author:Yash Giramkar [YSG], Shubham Jadhav [SJ]
 */
