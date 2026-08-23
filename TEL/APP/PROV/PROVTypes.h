/**
 * @file          PROVTypes.h
 * @brief         Header file containing user defined data types for device
 *                provisioning manager.
 * @date          10/08/26
 * @author        Yash Sunil Giramkar [YSG]
 * @copyright     Bajaj Auto Technology Limited (BATL)
 */

#ifndef _PROV_TYPES_H
#define _PROV_TYPES_H

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
 * @def           BAL_PROV_LOG_LEVEL
 * @brief         Logging Level for device provisioning application.
 */
#define BAL_PROV_LOG_LEVEL                   QL_LOG_LEVEL_INFO

/**
 * @def           BAL_PROV_LOG
 * @brief         Logging macro for device provisioning application.
 */
#define BAL_PROV_LOG(msg, ...)       QL_LOG(BAL_PROV_LOG_LEVEL, "BAL_PROV", msg, ##__VA_ARGS__)
/******************************************************************************/
/*                                                                            */
/*                                   ENUMS                                    */
/*                                                                            */
/******************************************************************************/
/**
 * @enum          PROVStatus_E
 * @brief         This enum helps with determining the current status of device
 *                provisioning.
 */
typedef enum
{
 eDEV_UNPROVISIONED,
 eDEV_PROVISIONING_PARTIAL,
 eDEV_PROVISIONING_CORRUPT,
 eDEV_PROVISIONED,
 eDEV_REPROVISIONED,
}PROVStatus_E;

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

#endif //!_PROV_TYPES_H

/**
 * Copyright(c) Bajaj Auto Technology Limited (BATL) as an unpublished work.
 * THIS SOFTWARE AND/OR MATERIAL IS THE PROPERTY OF BATL.
 * ALL USE, DISCLOSURE, AND/OR REPRODUCTION NOT SPECIFICALLY AUTHORIZED BY
 * BATL IS PROHIBITED.
 *
 * @author:Yash Sunil Giramkar [YSG]
 */
