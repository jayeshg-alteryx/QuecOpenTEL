/**
 * @file          UARTConfig.h
 * @brief         Header file containing Configuration for UART Initialization.
 * @date          22/07/2026
 * @author        Yash Giramkar [YSG], Aditya Bhosale [ASB]
 * @copyright     Bajaj Auto Technology Limited (BATL)
 */

#ifndef _UART_CONFIG_H
#define _UART_CONFIG_H

/******************************************************************************/
/*                                                                            */
/*                                  INCLUDES                                  */
/*                                                                            */
/******************************************************************************/
#include "Generics.h"
#include "ql_uart.h"

/******************************************************************************/
/*                                                                            */
/*                                  DEFINES                                   */
/*                                                                            */
/******************************************************************************/
/**
 * @def           MAX_UART_CHANNEL_CONFIGURED
 * @brief         Indicates the maximum number of UART Channels configured.
 */
#define MAX_UART_CHANNEL_CONFIGURED          1U

/**
 * @def           MODEM_TO_RH850_UART_CHANNEL
 * @brief         Indicates the maximum number of UART Channels configured.
 */
#define MODEM_TO_RH850_UART_PORT             QL_UART_PORT_1

/**
 * @def           MODEM_TO_RH850_BAUDRATE
 * @brief         Indicates the maximum number of UART Channels configured.
 */
#define MODEM_TO_RH850_BAUDRATE              QL_UART_BAUD_115200

/**
 * @def           MODEM_TO_RH850_UART_FIFO_SIZE
 * @brief         Indicates the maximum number of UART Channels configured.
 */
#define MODEM_TO_RH850_UART_FIFO_SIZE        2048U


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
 * @struct        UARTConfig_T
 * @brief         Structure containg the UART initlization configuration
 *                specific to a channel.
 */
typedef struct
{
   ql_uart_port_number_e ePortNumber;
   ql_uart_config_s stPortConfig;

} UARTConfig_T;

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

/**
 * @struct        st_UARTConfig
 * @brief         Structure containg the UART initlization configuration
 *                for all channels of the system.
 */
extern UARTConfig_T st_UARTConfig[MAX_UART_CHANNEL_CONFIGURED];

/******************************************************************************/
/*                                                                            */
/*                              EXTERN FUNCTIONS                              */
/*                                                                            */
/******************************************************************************/


#endif //!_UART_CONFIG_H

/**
 * Copyright(c) Bajaj Auto Technology Limited (BATL) as an unpublished work.
 * THIS SOFTWARE AND/OR MATERIAL IS THE PROPERTY OF BATL.
 * ALL USE, DISCLOSURE, AND/OR REPRODUCTION NOT SPECIFICALLY AUTHORIZED BY
 * BATL IS PROHIBITED.
 *
 * @author: Yash Giramkar [YSG], Aditya Bhosale [ASB]
 */
