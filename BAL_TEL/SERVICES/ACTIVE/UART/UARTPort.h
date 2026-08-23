/**
 * @file          UARTPort.h
 * @brief         Header file containing all the QuecOpen assocaited APIs.
 *                This file serves the purpose of abstracting the entire Quectel
 *                specific APIs and makes the UART service agnostic of modems
 *                SDK.
 * @date          21/07/2026
 * @author        Yash Giramkar [YSG], Aditya Bhosale [ASB]
 * @copyright     Bajaj Auto Technology Limited (BATL)
 */

#ifndef _UART_PORT_H
#define _UART_PORT_H

/******************************************************************************/
/*                                                                            */
/*                                  INCLUDES                                  */
/*                                                                            */
/******************************************************************************/
#include "Generics.h"
#include "ql_uart.h"
#include "UARTConfig.h"
#include <string.h>

/******************************************************************************/
/*                                                                            */
/*                                  DEFINES                                   */
/*                                                                            */
/******************************************************************************/
/**
 * @def           BAL_UART_SERVICE_LOG_LEVEL
 * @brief         Logging Level for UART Service.
 */
#define BAL_UART_SERVICE_LOG_LEVEL           QL_LOG_LEVEL_INFO

/**
 * @def           BAL_UART_SERVICE_LOG
 * @brief         Logging macro for UART Manager Service.
 */
#define BAL_UART_SERVICE_LOG(msg, ...)       QL_LOG(BAL_UART_SERVICE_LOG_LEVEL, "BAL_UART", msg, ##__VA_ARGS__)

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
TelSysResponse_E ge_UARTPort_Init(void);
TelSysResponse_E ge_UARTPort_RegisterCallback(ql_uart_callback fpt_Callback);
int32_t gs32_UARTPort_Read(uint8_t *u8pt_Data, uint16_t u16_DataLength);
TelSysResponse_E ge_UARTPort_Write(const uint8_t *cu8pt_Data,
                                   uint16_t u16_DataLength);

#endif //!_UART_PORT_H

/**
 * Copyright(c) Bajaj Auto Technology Limited (BATL) as an unpublished work.
 * THIS SOFTWARE AND/OR MATERIAL IS THE PROPERTY OF BATL.
 * ALL USE, DISCLOSURE, AND/OR REPRODUCTION NOT SPECIFICALLY AUTHORIZED BY
 * BATL IS PROHIBITED.
 *
 * @author: Yash Giramkar [YSG], Aditya Bhosale [ASB]
 */
