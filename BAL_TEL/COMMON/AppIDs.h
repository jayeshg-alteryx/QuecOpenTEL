/**
 * @file          AppIDs.h
 * @brief         Header file containing application IDs for the Quectel OpenCPU
 *                TEL Modem platform.
 * @date          04/08/2026
 * @author        Yash Giramkar [YSG]
 * @copyright     Bajaj Auto Technology Limited (BATL)
 */

#ifndef _APP_IDS_H
#define _APP_IDS_H

/******************************************************************************/
/*                                                                            */
/*                                  INCLUDES                                  */
/*                                                                            */
/******************************************************************************/

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
 * @enum          Application_E
 * @brief         Application types for IPC between RH850 and Modem.
 */
typedef enum
{
   eAPPLICATION_IDLE  = 0x0U,
   eAPPLICATION_GENERAL_STATUS,
   eAPPLICATION_PROVISIONING,
   eAPPLICATION_OFFLINE_ANALYTICS,
   eAPPLICATION_DATA_LOGGER,
   eAPPLICATION_BALNET_FOTA,
   eAPPLICATION_UDS_FOTA,
   eAPPLICATION_MODEM_FOTA,
   eAPPLICATION_COMMAND_MANAGER,
   eAPPLICATION_MANIFEST,
   eAPPLICATION_LAST_APPLICATION
} Application_E;


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

#endif //!_SAMPLE_FORMAT_H

/**
 * Copyright(c) Bajaj Auto Technology Limited (BATL) as an unpublished work.
 * THIS SOFTWARE AND/OR MATERIAL IS THE PROPERTY OF BATL.
 * ALL USE, DISCLOSURE, AND/OR REPRODUCTION NOT SPECIFICALLY AUTHORIZED BY
 * BATL IS PROHIBITED.
 *
 * @author:<Author of H file - Name [Initials]>
 */
