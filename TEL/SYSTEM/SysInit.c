/**
 * @file          SysInit.c
 * @brief         Source file containing system level initialization for the BAL
 *                TEL Device.
 * @date          23-04-2026
 * @author        Yash Giramkar [YSG], Aditya Bhosale [ASB], Akash Patil [ASP],
 *                Shubham Jadhav [SSJ]
 * @copyright     Bajaj Auto Technology Limited (BATL)
 */

/******************************************************************************/
/*                                                                            */
/*                                  INCLUDES                                  */
/*                                                                            */
/******************************************************************************/
#include "SysInit.h"
/******************************************************************************/
/*                                                                            */
/*                                  DEFINES                                   */
/*                                                                            */
/******************************************************************************/
/**
 * @def           BAL_DEV_LOG_LEVEL
 * @brief         Sets  the log level for the device demo. Adjust this to control
 *                the verbosity of logs.
 */
#define BAL_DEV_LOG_LEVEL                    QL_LOG_LEVEL_INFO

/**
 * @def           BAL_DEV_LOG
 * @brief         Macro for logging messages in the device demo. It uses the
 *                defined log level and tags logs with "BAL" for easy filtering.
 */
#define BAL_DEV_LOG(msg, ...)                QL_LOG(BAL_DEV_LOG_LEVEL, "BAL", msg, ##__VA_ARGS__)

/**
 * @def           BAL_DEV_LOG_PUSH
 * @brief         Macro for pushing log messages in the device demo. It tags logs
 *                with "BAL" for easy filtering when using log push functionality.
 */
#define BAL_DEV_LOG_PUSH(msg, ...)           QL_LOG_PUSH("BAL", msg, ##__VA_ARGS__)

/**
 * @def           BAL_SAMPLE_TASK_STACK_SIZE
 * @brief         Defines the stack size for the sample task in bytes.
 */
#define BAL_SAMPLE_TASK_STACK_SIZE           2048

/**
 * @def           BAL_SAMPLE_TASK_PRIO
 * @brief         Defines the priority for the sample task.
 */
#define BAL_SAMPLE_TASK_PRIO                 APP_PRIORITY_NORMAL

/**
 * @def           BAL_SAMPLE_TASK_EVENT_CNT
 * @brief         Defines the number of events that the sample task can handle.
 */
#define BAL_SAMPLE_TASK_EVENT_CNT            5

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
 * @private       gv_APP_SampleTask
 * @brief         Task function for the sample application. This function will
 *                be executed in a separate thread and will perform the main
 *                logic of the sample application.
 * @param[in]     param - A pointer to any parameters that need to be passed to the task.
 * @param[out]    None
 * @param[inout]  None
 * @return        None
 */
static void gv_APP_SampleTask(void *param)
{

   BAL_DEV_LOG("BAL Sample Thread entry, param 0x%x", param);
   while (1)
   {
      BAL_DEV_LOG("Hello BAL TEL System Init echo");
      ql_rtos_task_sleep_ms(1000);
   }
}


/******************************************************************************/
/*                                                                            */
/*                        PUBLIC FUNCTION DEFINITIONS                         */
/*                                                                            */
/******************************************************************************/
/**
 * @public        gv_APP_SampleInit
 * @brief         Initializes the sample application task.
 * @param[in]     None
 * @param[out]    None
 * @param[inout]  None
 * @return        None
 */
void gv_SysInit(void)
{
   QlOSStatus e_errorCode = QL_OSI_SUCCESS;
   ql_task_t stpt_sampleTask = NULL;

   if (gv_UARTManager_Init() != eTEL_OK)
   {
      #ifdef DEBUG_ENABLED
         BAL_DEV_LOG("UART service initialization failed");
      #endif // DEBUG_ENABLED
   }
   else
   {
      #ifdef DEBUG_ENABLED
         BAL_DEV_LOG("UART service initialization success!");
      #endif // DEBUG_ENABLED
   }
   if (ge_AppMonitor_Init() != eTEL_OK)
   {
      #ifdef DEBUG_ENABLED
         BAL_DEV_LOG("App Monitor initialization failed");
      #endif // DEBUG_ENABLED
   }
   else
   {
      #ifdef DEBUG_ENABLED
         BAL_DEV_LOG("App Monitor initialization passed");
      #endif // DEBUG_ENABLED
   }

   e_errorCode = ql_rtos_task_create(&stpt_sampleTask, BAL_SAMPLE_TASK_STACK_SIZE, BAL_SAMPLE_TASK_PRIO, "BALSAMPLE", gv_APP_SampleTask, NULL, BAL_SAMPLE_TASK_EVENT_CNT);
   if (e_errorCode != QL_OSI_SUCCESS)
   {
      BAL_DEV_LOG("BAL Sample Task Creation failed");
   }
   else
   {
      BAL_DEV_LOG("BAL Sample Task Creation success!");
   }


   if (ge_GENERAL_Init() != eTEL_OK)
   {
      BAL_DEV_LOG("General Manager Application failed");
   }
   else
   {
      BAL_DEV_LOG("General Manager Application success!");
   }

   if (ge_Crypto_Init() != CRYPTO_OK)
   {
      BAL_DEV_LOG("Crypto Application failed");
   }
   else
   {
      BAL_DEV_LOG("Crypto Application success!");
   }


   if(ge_PROV_Init() != eTEL_OK)
   {
      BAL_DEV_LOG("Provisioning Manager Application failed");
   }
   else
   {
      BAL_DEV_LOG("Provisioning Manager Application success!");
   }

   (void)ge_FS_Init();

   if (ge_FOTA_Init(NULL) != eTEL_OK)
   {
      BAL_DEV_LOG("FOTA Manager Application failed");
   }
   else
   {
      BAL_DEV_LOG("FOTA Manager Application success!");
   }
}

/**
 * Copyright(c) Bajaj Auto Technology Limited (BATL) as an unpublished work.
 * THIS SOFTWARE AND/OR MATERIAL IS THE PROPERTY OF BATL.
 * ALL USE, DISCLOSURE, AND/OR REPRODUCTION NOT SPECIFICALLY AUTHORIZED BY
 * BATL IS PROHIBITED.
 *
 * @author: Yash Giramkar [YSG], Aditya Bhosale [ASB], Akash Patil [ASP],
 *          Shubham Jadhav [SSJ]
 */





