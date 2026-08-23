/**
 * @file          SysInit.h
 * @brief         Header file containing system level initialization for the BAL
 *                TEL Device.
 * @date          14-06-2026
 * @author        Yash Giramkar [YSG], Aditya Bhosale [ASB], Akash Patil [ASP],
 *                Shubham Jadhav [SSJ]
 * @copyright     Bajaj Auto Technology Limited (BATL)
 */


#ifndef _SYSINIT_H
#define _SYSINIT_H
#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
/*                                                                            */
/*                                  INCLUDES                                  */
/*                                                                            */
/******************************************************************************/
#include "Generics.h"

#include "AppMonitor.h"
#include "UARTManager.h"
#include "GENERALManager.h"
#include "PROVManager.h"
#include "FOTAManager.h"
#include "FS_Manager.h"

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
void gv_SysInit(void);


#ifdef __cplusplus
} /*"C" */
#endif

#endif // _SYSINIT_H

/**
 * Copyright(c) Bajaj Auto Technology Limited (BATL) as an unpublished work.
 * THIS SOFTWARE AND/OR MATERIAL IS THE PROPERTY OF BATL.
 * ALL USE, DISCLOSURE, AND/OR REPRODUCTION NOT SPECIFICALLY AUTHORIZED BY
 * BATL IS PROHIBITED.
 *
 * @author: Yash Giramkar [YSG], Aditya Bhosale [ASB], Akash Patil [ASP],
 *          Shubham Jadhav [SSJ]
 */
