/**
 * @file          UARTPort.c
 * @brief         Source file containing all the QuecOpen assocaited APIs.
 *                This file serves the purpose of abstracting the entire Quectel
 *                specific APIs and makes the UART service agnostic of modems
 *                SDK.
 * @date          21/07/2026
 * @author        Yash Giramkar [YSG], Aditya Bhosale [ASB]
 * @copyright     Bajaj Auto Technology Limited (BATL)
 */

/******************************************************************************/
/*                                                                            */
/*                                  INCLUDES                                  */
/*                                                                            */
/******************************************************************************/
#include "UARTPort.h"

/******************************************************************************/
/*                                                                            */
/*                                  DEFINES                                   */
/*                                                                            */
/******************************************************************************/


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

TelSysResponse_E ge_UARTPort_Init(void)
{
   uint8_t u8_LoopVar;
   ql_uart_errcode_e eReturnValue;

   for (u8_LoopVar = 0U; u8_LoopVar < MAX_UART_CHANNEL_CONFIGURED; u8_LoopVar++)
   {
      eReturnValue = ql_uart_set_dcbconfig(st_UARTConfig[u8_LoopVar].ePortNumber,
                                            &st_UARTConfig[u8_LoopVar].stPortConfig);
      if (eReturnValue != QL_UART_SUCCESS)
      {
         BAL_UART_SERVICE_LOG("UART configuration failed, port=%d error=%d",
                              st_UARTConfig[u8_LoopVar].ePortNumber, eReturnValue);
         return eTEL_NOK;
      }

      eReturnValue = ql_uart_open(st_UARTConfig[u8_LoopVar].ePortNumber);
      if (eReturnValue != QL_UART_SUCCESS)
      {
         BAL_UART_SERVICE_LOG("UART open failed, port=%d error=%d",
                              st_UARTConfig[u8_LoopVar].ePortNumber, eReturnValue);
         return eTEL_NOK;
      }
   }
   return eTEL_OK;
}

TelSysResponse_E ge_UARTPort_RegisterCallback(ql_uart_callback fpt_Callback)
{
   ql_uart_errcode_e eReturnValue;

   eReturnValue = ql_uart_register_cb(st_UARTConfig[0].ePortNumber, fpt_Callback);
   return (eReturnValue == QL_UART_SUCCESS) ? eTEL_OK : eTEL_NOK;
}

int32_t gs32_UARTPort_Read(uint8_t *u8pt_Data, uint16_t u16_DataLength)
{
   return (int32_t)ql_uart_read(st_UARTConfig[0].ePortNumber, u8pt_Data,
                                (unsigned int)u16_DataLength);
}


TelSysResponse_E ge_UARTPort_Write(const uint8_t *cu8pt_Data,
                                   uint16_t u16_DataLength)
{
   int32_t s32_BytesWritten;

   if ((cu8pt_Data == NULL) || (u16_DataLength == 0U))
   {
      return eTEL_NOK;
   }

   s32_BytesWritten = (int32_t)ql_uart_write(st_UARTConfig[0].ePortNumber,
                                              (unsigned char *)cu8pt_Data,
                                              (unsigned int)u16_DataLength);
   return (s32_BytesWritten == (int32_t)u16_DataLength) ? eTEL_OK : eTEL_NOK;
}

/**
 * Copyright(c) Bajaj Auto Technology Limited (BATL) as an unpublished work.
 * THIS SOFTWARE AND/OR MATERIAL IS THE PROPERTY OF BATL.
 * ALL USE, DISCLOSURE, AND/OR REPRODUCTION NOT SPECIFICALLY AUTHORIZED BY
 * BATL IS PROHIBITED.
 *
 * @author: Yash Giramkar [YSG], Aditya Bhosale [ASB]
 */
