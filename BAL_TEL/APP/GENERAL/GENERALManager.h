/**
 * @file          GENERALManager.h
 * @brief         Header file containing General Application code.
 * @date          05/06/26
 * @author        Yash Giramkar
 * @copyright     Bajaj Auto Technology Limited (BATL)
 */

#ifndef _GENERAL_H
#define _GENERAL_H

/******************************************************************************/
/*                                                                            */
/*                                  INCLUDES                                  */
/*                                                                            */
/******************************************************************************/
#include "Generics.h"
#include "FS_Manager.h"

/******************************************************************************/
/*                                                                            */
/*                                  DEFINES                                   */
/*                                                                            */
/******************************************************************************/
/**
 * @def           BAL_GM_LOG_LEVEL
 * @brief         Logging Level for General Manager Application Manager.
 */
#define BAL_GM_LOG_LEVEL           QL_LOG_LEVEL_INFO

/**
 * @def           BAL_GENERAL_APP_LOG
 * @brief         Logging macro for UART Manager Service.
 */
#define BAL_GENERAL_APP_LOG(msg, ...)       QL_LOG(BAL_GM_LOG_LEVEL, "BAL_GM", msg, ##__VA_ARGS__)

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
TelSysResponse_E ge_GENERAL_Init(void);


#endif //!_GENERAL_H

/**
 * Copyright(c) Bajaj Auto Technology Limited (BATL) as an unpublished work.
 * THIS SOFTWARE AND/OR MATERIAL IS THE PROPERTY OF BATL.
 * ALL USE, DISCLOSURE, AND/OR REPRODUCTION NOT SPECIFICALLY AUTHORIZED BY
 * BATL IS PROHIBITED.
 *
 * @author: Yash Sunil Giramkar <YSG>
 */
