/**
 * @file          AppMonitor.c
 * @brief         Source file containing application monitor functionality
 * @date          04/08/26
 * @author        Yash Giramkar [YSG], Shubham Jadhav [SJ]
 * @copyright     Bajaj Auto Technology Limited (BATL)
 */

/******************************************************************************/
/*                                                                            */
/*                                  INCLUDES                                  */
/*                                                                            */
/******************************************************************************/
#include "AppMonitor.h"

/******************************************************************************/
/*                                                                            */
/*                                  DEFINES                                   */
/*                                                                            */
/******************************************************************************/
/**
 * @def           BAL_APPMON_LOG_LEVEL
 * @brief         Sets the log level for the application monitor module. 
 */
#define BAL_APPMON_LOG_LEVEL                  QL_LOG_LEVEL_INFO

/**
 * @def           BAL_APPMON_LOG
 * @brief         Macro for logging messages in the application monitor module. 
 */
#define BAL_APPMON_LOG(msg, ...)              QL_LOG(BAL_APPMON_LOG_LEVEL, "BAL_APPMON", msg, ##__VA_ARGS__)

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
 * @struct        st_AppMonitorContext
 * @brief         Structure to hold the context of application monitor module.
 */
typedef struct AppMonitor_T
{
   Application_E  e_currentRunningApp;
   Application_E  e_lastRunningApp;
   int64_t        i64_currentAppStartTime;
   int64_t        i64_lastRunAppExecutionTime;
}AppMonitor_T; 


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
 * @var           sst_AppMonitor
 * @brief         Holds the passive application ownership information.
 */
static  AppMonitor_T sst_AppMonitor;

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
 * @public        ge_AppMonitor_Init
 * @brief         Initlizes the application monitor module. This function must be
 *                called before any other function of this module is called.
 * @param[in]     <Input parameter details>.
 * @param[out]    <Output parameter details>.
 * @param[inout]  <Input-Output parameter details>.
 * @return        <Return details>.
 */
TelSysResponse_E ge_AppMonitor_Init(void)
{
   sst_AppMonitor.e_currentRunningApp = eAPPLICATION_IDLE;
   sst_AppMonitor.e_lastRunningApp    = eAPPLICATION_IDLE;
   /*0 represents no time elapsed or invalid time */
   sst_AppMonitor.i64_currentAppStartTime     = 0;
   sst_AppMonitor.i64_lastRunAppExecutionTime = 0;
   #ifdef DEBUG_ENABLED
      BAL_APPMON_LOG("APP MONITOR Init Successful!");
   #endif // DEBUG_ENABLED
   return eTEL_OK;
}

/**
 * @public        ge_AppMonitor_RunApplication
 * @brief         Registers an application as the current running application
 *                when no other application owns the monitor.
 * @param[in]     e_requestedAppID Application requesting exclusive ownership.
 * @return        eTEL_OK when ownership is registered; eTEL_NOK otherwise.
 */
TelSysResponse_E ge_AppMonitor_RunApplication(Application_E e_requestedAppID)
{
   TelSysResponse_E e_response = eTEL_NOK;

   if (sst_AppMonitor.e_currentRunningApp == eAPPLICATION_IDLE)
   {
      sst_AppMonitor.e_currentRunningApp = e_requestedAppID;
      /*Get the start time of the current application*/
      sst_AppMonitor.i64_currentAppStartTime = ql_rtos_up_time_ms();
      #ifdef DEBUG_ENABLED
         BAL_APPMON_LOG("Current Running App ID: %d", sst_AppMonitor.e_currentRunningApp);
         BAL_APPMON_LOG("Current Running App Start Time: %lld", sst_AppMonitor.i64_currentAppStartTime);
      #endif // DEBUG_ENABLED
      e_response = eTEL_OK;
   }
   else
   {
      #ifdef DEBUG_ENABLED
         BAL_APPMON_LOG("The requested App Id: %d should not run since current \
            running app is not idle but: %d",e_requestedAppID, sst_AppMonitor.e_currentRunningApp);
      #endif // DEBUG_ENABLED
   }
   return e_response;
}

/**
 * @public        ge_AppMonitor_StopCurrentRunningApp
 * @brief         Releases exclusive ownership when requested by the current
 *                running application.
 * @param[in]     e_requesterAppID Application requesting ownership release.
 * @return        eTEL_OK when ownership is released; eTEL_NOK otherwise.
 */
TelSysResponse_E ge_AppMonitor_StopCurrentRunningApp(Application_E e_requesterAppID)
{
   TelSysResponse_E e_response = eTEL_NOK;

   if ((sst_AppMonitor.e_currentRunningApp != eAPPLICATION_IDLE) &&
       (sst_AppMonitor.e_currentRunningApp == e_requesterAppID))
   {
      sst_AppMonitor.e_currentRunningApp = eAPPLICATION_IDLE;
      sst_AppMonitor.e_lastRunningApp    = e_requesterAppID;
      /*Calculate the execution time the application that ran*/
      sst_AppMonitor.i64_lastRunAppExecutionTime = ql_rtos_up_time_ms() - sst_AppMonitor.i64_currentAppStartTime;
      #ifdef DEBUG_ENABLED
         BAL_APPMON_LOG(" For Stopped App: %d total execution time was: %lld", sst_AppMonitor.e_lastRunningApp,sst_AppMonitor.i64_lastRunAppExecutionTime);
      #endif // DEBUG_ENABLED
      /*Reset the start time to 0*/
      sst_AppMonitor.i64_currentAppStartTime = 0;
      e_response = eTEL_OK;
   }
   else
   {
      #ifdef DEBUG_ENABLED
         BAL_APPMON_LOG("Current running app is : %d whereas app requested to stop is %d", sst_AppMonitor.e_currentRunningApp,e_requesterAppID);
      #endif // DEBUG_ENABLED
   }

   return e_response;
}

/**
 * @public        ge_AppMonitor_GetCurrentRunningApp
 * @brief         Gets the application that currently owns the monitor.
 * @return        Currently registered application, or eAPPLICATION_IDLE.
 */
Application_E ge_AppMonitor_GetCurrentRunningApp(void)
{
   #ifdef DEBUG_ENABLED
      BAL_APPMON_LOG("Current Running App ID: %d", sst_AppMonitor.e_currentRunningApp);
   #endif // DEBUG_ENABLED
   return sst_AppMonitor.e_currentRunningApp;
}

/**
 * @public        ge_AppMonitor_GetLastRunningApp
 * @brief         Gets the application that last owned the monitor.
 * @return        Last registered application, or eAPPLICATION_IDLE.
 */
Application_E ge_AppMonitor_GetLastRunningApp(void)
{
   #ifdef DEBUG_ENABLED
      BAL_APPMON_LOG("Last Running App ID: %d", sst_AppMonitor.e_lastRunningApp);
   #endif // DEBUG_ENABLED
   return sst_AppMonitor.e_lastRunningApp;
}

/**
 * @public        gi64_AppMonitor_GetCurrentRunningTime
 * @brief         Gets the time for which the current application has been running.
 * @return        Time in milliseconds, or 0 if no application is running.
 */
int64_t gi64_AppMonitor_GetCurrentRunningTime(void)
{
   int64_t i64_CurrentRunningTime;
   i64_CurrentRunningTime = (ql_rtos_up_time_ms() - sst_AppMonitor.i64_currentAppStartTime);
   #ifdef DEBUG_ENABLED
      BAL_APPMON_LOG("Current Running App time: %lld", i64_CurrentRunningTime);
   #endif // DEBUG_ENABLED
   return i64_CurrentRunningTime;
}

/**
 * @public        gi64_AppMonitor_GetLastExecutionRuntime
 * @brief         Gets the time for which the last application ran.
 * @return        Time in milliseconds, or 0 if no application has run yet.
 */
int64_t gi64_AppMonitor_GetLastExecutionRuntime(void)
{
   #ifdef DEBUG_ENABLED
      BAL_APPMON_LOG("Last Running App Execution Time: %d", sst_AppMonitor.i64_lastRunAppExecutionTime);
   #endif // DEBUG_ENABLED
   return sst_AppMonitor.i64_lastRunAppExecutionTime;
}

/**
 * Copyright(c) Bajaj Auto Technology Limited (BATL) as an unpublished work.
 * THIS SOFTWARE AND/OR MATERIAL IS THE PROPERTY OF BATL.
 * ALL USE, DISCLOSURE, AND/OR REPRODUCTION NOT SPECIFICALLY AUTHORIZED BY
 * BATL IS PROHIBITED.
 *
 * @author:Yash Giramkar [YSG], Shubham Jadhav [SJ]
 */
