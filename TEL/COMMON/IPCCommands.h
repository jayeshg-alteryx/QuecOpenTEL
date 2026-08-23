/**
 * @file          IPCCommands.h
 * @brief         Header file containing error codes typedefs.
 * @date          14/07/2026
 * @author        Yash Giramkar [YSG], Aditya Bhosale [ASB]
 * @copyright     Bajaj Auto Technology Limited (BATL)
 */

#ifndef _IPC_COMMANDS_H
#define _IPC_COMMANDS_H

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
 * @enum          IPCCommands_E
 * @brief         Global Enum containing all the commands assocaited with all
                  the Application to Serviec and vice versas related
                  transactions.
                  Command Allocations

                  0x0000 - 0x00FF            Framework Reserved
                  0x0100 - 0x01FF            Network Service
                  0x0200 - 0x02FF            UART Service
                  0x0300 - 0x03FF            MQTT Service
                  0x0400 - 0x04FF            HTTP Service
                  0x0500 - 0x05FF            Timer Service
                  0x0600 - 0xFFFF            Reserved for future
 */

typedef enum
{
/******************************************************************************/
/*                               FRAMEWORK                                    */
/******************************************************************************/
eIPC_CMD_NONE=0x0000,


/******************************************************************************/
/*                             GENERAL APP                                    */
/******************************************************************************/
eIPC_CMD_GENERAL_GET_STATUS = 0x0001,


/******************************************************************************/
/*                               Network Service                              */
/******************************************************************************/
eIPC_CMD_NETWORK_CONNECT = 0x0100,
eIPC_CMD_NETWORK_DISCONNECT,
eIPC_CMD_NETWORK_STATUS,

/******************************************************************************/
/*                               UART Service                                 */
/******************************************************************************/
eIPC_CMD_UART_INIT = 0x0200,
eIPC_CMD_UART_TX_ENABLE,
eIPC_CMD_UART_TX_DISABLE,
eIPC_CMD_UART_RX_ENABLE,
eIPC_CMD_UART_RX_DISABLE,
eIPC_CMD_UART_TX_SEND_DATA,
eIPC_CMD_UART_RX_DATA_AVAILABLE,
eIPC_CMD_UART_RX_FLUSH,
eIPC_CMD_UART_TX_FLUSH,
eIPC_CMD_UART_REINIT,
eIPC_CMD_UART_RX_OVERFLOW,


/******************************************************************************/
/*                               MQTT Service                                 */
/******************************************************************************/
eIPC_CMD_MQTT_INIT = 0x0300,
eIPC_CMD_MQTT_CONNECT ,
eIPC_CMD_MQTT_DISCONNECT ,
eIPC_CMD_MQTT_SUBSCRIBE ,
eIPC_CMD_MQTT_UNSUBSCRIBE ,
eIPC_CMD_MQTT_PUBLISH ,

/******************************************************************************/
/*                               HTTP Service                                 */
/******************************************************************************/
eIPC_CMD_HTTP_INIT = 0x0400,
eIPC_CMD_HTTP_OPEN ,
eIPC_CMD_HTTP_CLOSE ,
eIPC_CMD_HTTP_PUT ,
eIPC_CMD_HTTP_GET ,
eIPC_CMD_HTTP_CHUNKED_PUT,
eIPC_CMD_HTTP_CHUNKED_GET,

/******************************************************************************/
/*                               Timer Service                                */
/******************************************************************************/
eIPC_CMD_TIMER_INIT = 0x0500,
eIPC_CMD_TIMER_CREATE,
eIPC_CMD_TIMER_DELETE,
eIPC_CMD_TIMER_START,
eIPC_CMD_TIMER_STOP,
eIPC_CMD_TIMER_ALARM,
eIPC_CMD_TIMER_RESTART,

}IPCCommands_E;


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

#endif //!_IPC_COMMANDS_H

/**
 * Copyright(c) Bajaj Auto Technology Limited (BATL) as an unpublished work.
 * THIS SOFTWARE AND/OR MATERIAL IS THE PROPERTY OF BATL.
 * ALL USE, DISCLOSURE, AND/OR REPRODUCTION NOT SPECIFICALLY AUTHORIZED BY
 * BATL IS PROHIBITED.
 *
 * @author:Yash Giramkar [YSG], Aditya Bhosale [ASB]
 */
