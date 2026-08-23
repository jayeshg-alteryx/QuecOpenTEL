/**
 * @file          UARTManager.h
 * @brief         Header file containing public APIs available and exposed
 *                by UART service. This service is created specifically to
 *                cater to communication between RH850 and Modem.
 *
 * @note          Currently the UART manager is non-configurable to cater to
 *                multiple UART channels. Additional changes would be required
 *                to handle multiple channel usecase.
 * @date          14/07/2026
 * @author        Yash Giramkar [YSG], Aditya Bhosale [ASB]
 * @copyright     Bajaj Auto Technology Limited (BATL)
 */

#ifndef _UART_MGR_H
#define _UART_MGR_H

/******************************************************************************/
/*                                                                            */
/*                                  INCLUDES                                  */
/*                                                                            */
/******************************************************************************/
#include "Generics.h"
#include "UARTPort.h"
#include "UARTFrameTypes.h"
#include "UARTRouter.h"
#include "CryptoManager.h"

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
extern TelSysResponse_E gv_UARTManager_Init(void);
extern TelSysResponse_E gv_UARTManager_EnqueueTx(IPCMessage_T* stpt_intraUARTIPCMsg);


#endif //!_UART_MGR_H

/**
 * Copyright(c) Bajaj Auto Technology Limited (BATL) as an unpublished work.
 * THIS SOFTWARE AND/OR MATERIAL IS THE PROPERTY OF BATL.
 * ALL USE, DISCLOSURE, AND/OR REPRODUCTION NOT SPECIFICALLY AUTHORIZED BY
 * BATL IS PROHIBITED.
 *
 * @author: Yash Giramkar [YSG], Aditya Bhosale [ASB]
 */
