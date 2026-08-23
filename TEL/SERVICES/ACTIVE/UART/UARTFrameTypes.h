/**
 * @file          UARTFrameTypes.h
 * @brief         UART wire-protocol types shared by the UART service.
 * @date          22/07/2026
 * @author        Yash Giramkar [YSG], Aditya Bhosale [ASB]
 * @copyright     Bajaj Auto Technology Limited (BATL)
 */

#ifndef _UART_FRAME_TYPES_H
#define _UART_FRAME_TYPES_H

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
 * @def           UART_SOF
 * @brief         Start of frame character for all UART frames, used in IPC for
 *                RH850 and Modem.
 */
#define UART_SOF                             0x55U

/**
 * @def           UART_EOF
 * @brief         End of frame character for all UART frames, used in IPC for
 *                RH850 and Modem.
 */
#define UART_EOF                             0xAAU

/**
 * @def           UART_GENERAL_FRAME_SIZE
 * @brief         General frame size for UART frames, used in IPC for RH850 and
 *                Modem.
 */
#define UART_GENERAL_FRAME_SIZE              16U

/**
 * @def           UART_GENERAL_FRAME_CKSUM_LENGTH
 * @brief         General frame data length for computing checksum
 */
#define UART_GENERAL_FRAME_CKSUM_LENGTH      13U

/**
 * @def           UART_GENERAL_FRAME_CKSUM_OFFSET
 * @brief         General frame data offset from the beginning of the frame to
 *                compute checksum.
 */
#define UART_GENERAL_FRAME_CKSUM_OFFSET      1U

/**
 * @def           UART_GENERAL_PAYLOAD_SIZE
 * @brief         General payload size for UART frames, used in IPC for RH850 and
 *                Modem.
 */
#define UART_GENERAL_PAYLOAD_SIZE            8U

/**
 * @def           UART_METADATA_PAYLOAD_SIZE
 * @brief         Metadata payload size for UART frames, used in IPC for RH850 and
 *                Modem.
 */
#define UART_METADATA_PAYLOAD_SIZE           3U

/**
 * @def           UART_DATA_FRAME_MAX_PAYLOAD_SIZE
 * @brief         Maximum payload size for UART data frames, used in IPC for
 *                RH850 and Modem.
 */
#define UART_DATA_FRAME_MAX_PAYLOAD_SIZE     256U


/**
 * @def           UART_DATA_FRAME_CKSUM_OFFSET
 * @brief         General frame data offset from the beginning of the frame to
 *                compute checksum.
 */
#define UART_DATA_FRAME_CKSUM_OFFSET      1U

/**
 * @def           UART_DATA_FRAME_MAX_SIZE
 * @brief         Maximum payload size for UART data frames, used in IPC for
 *                RH850 and Modem.
 */
#define UART_DATA_FRAME_MAX_SIZE             (UART_DATA_FRAME_MAX_PAYLOAD_SIZE + 2)

/**
 * @def           UART_CRC8_POLYNOMIAL
 * @brief         CRC8 polynomial for UART frames, used in IPC for RH850 and
 *                Modem.
 */
#define UART_CRC8_POLYNOMIAL                 0x1DU

/**
 * @def           UART_NEGATIVE_ACK_SERVICE_OFFSET
 * @brief         Service offset for negative acknowledgment in UART frames,
 *                used in IPC for RH850 and Modem.
 */
#define UART_NEGATIVE_ACK_SERVICE_OFFSET     0x80U


/******************************************************************************/
/*                                                                            */
/*                                 TYPEDEFS                                   */
/*                                                                            */
/******************************************************************************/


/******************************************************************************/
/*                                                                            */
/*                                   ENUMS                                    */
/*                                                                            */
/******************************************************************************/

/**
 * @enum          UARTFrameType_E
 * @brief         UART frame types for IPC between RH850 and Modem.
 */
typedef enum
{
   eUART_FRAME_TYPE_GENERAL                  = 0x0U,
   eUART_FRAME_TYPE_METADATA                 = 0x1U,
   eUART_FRAME_TYPE_DATA                     = 0x2U,
   eUART_FRAME_TYPE_INVALID                  = 0xFU,
} UARTFrameType_E;

/**
 * @enum          UARTOriginator_E
 * @brief         UART originator types for IPC between RH850 and Modem.
 */
typedef enum
{
   eUART_ORIGINATOR_RH850                    = 0x1U,
   eUART_ORIGINATOR_MODEM                    = 0x2U
} UARTOriginator_E;


/**
 * @enum          UARTIntent_E
 * @brief         UART intent types for IPC between RH850 and Modem.
 */
typedef enum
{
   eUART_INTENT_REQUEST = 0x1U,
   eUART_INTENT_RESPONSE = 0x2U,
   eUART_INTENT_NOTIFY = 0x3U,
   eUART_INTENT_LAST_INTENT
} UARTIntent_E;

/**
 * @enum          UARTPacketType_E
 * @brief         UART packet types for IPC between RH850 and Modem.
 */
typedef enum
{
   eUART_PACKET_GENERAL,
   eUART_PACKET_DATA
} UARTPacketType_E;

/******************************************************************************/
/*                                                                            */
/*                                 STRUCTURES                                 */
/*                                                                            */
/******************************************************************************/

/* QuecOpen uses the ARM GCC toolchain.  These structures directly map the
 * UART wire format and must not acquire compiler-inserted padding. */
#pragma pack(push, 1)

/**
 * @struct        UARTFrameGenMeta_T
 * @brief         UART frame structure for General and Metadata Frame
 */
typedef struct
{
   uint8_t u8_SOF;
   uint8_t eApplication : 4;
   uint8_t eFrameType : 4;
   uint8_t eIntent: 4;
   uint8_t eOriginator : 4;
   uint16_t u16_FrameId;
   uint8_t u8_PayloadLength;
   uint8_t au8_Payload[UART_GENERAL_PAYLOAD_SIZE];
   uint8_t u8_cksum;
   uint8_t u8_EOF;
} UARTFrameGenMeta_T;

/**
 * @struct        UARTFrameData_T
 * @brief         UART frame structure for Data Frame
 */
typedef struct
{
   uint8_t u8_SOF;
   uint8_t au8_Payload[UART_DATA_FRAME_MAX_PAYLOAD_SIZE];
   uint8_t u8_EOF;
} UARTFrameData_T;

#pragma pack(pop)

_Static_assert(sizeof(UARTFrameGenMeta_T) == UART_GENERAL_FRAME_SIZE,
               "UART general/metadata frame layout must match the wire format");
_Static_assert(sizeof(UARTFrameData_T) == UART_DATA_FRAME_MAX_SIZE,
               "UART data frame layout must match the wire format");




/******************************************************************************/
/*                                                                            */
/*                                 UNION                                      */
/*                                                                            */
/******************************************************************************/

/**
 * @union         UartFrame_U
 * @brief         Union to hold all types of UART frame.
 */
typedef union
{
   uint8_t u8a_uartData[UART_DATA_FRAME_MAX_SIZE];
   UARTFrameGenMeta_T st_uartGenMetaFrame;
   UARTFrameData_T st_uartDataFrame;
} UartFrame_U;





#endif //!_UART_FRAME_TYPES_H


/**
 * Copyright(c) Bajaj Auto Technology Limited (BATL) as an unpublished work.
 * THIS SOFTWARE AND/OR MATERIAL IS THE PROPERTY OF BATL.
 * ALL USE, DISCLOSURE, AND/OR REPRODUCTION NOT SPECIFICALLY AUTHORIZED BY
 * BATL IS PROHIBITED.
 *
 * @author: Yash Giramkar [YSG], Aditya Bhosale [ASB]
 */
