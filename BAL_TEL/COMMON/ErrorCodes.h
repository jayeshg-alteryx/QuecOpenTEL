/**
 * @file          ErrorCodes.h
 * @brief         Header file containing error codes typedefs.
 * @date          14/07/2026
 * @author        Yash Giramkar [YSG], Aditya Bhosale [ASB]
 * @copyright     Bajaj Auto Technology Limited (BATL)
 */

#ifndef _ERROR_CODES_H
#define _ERROR_CODES_H

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
#include <stdint.h>

/**
 * @name Global Success Code
 * @{
 */
#define BAL_SUCCESS                         (0)
/** @} */

/**
 * @name Module Error Base Offsets
 * @brief Each functional domain is allocated a unique block of 100 codes.
 * @{
 */
#define BAL_ERR_BASE_GENERIC                (0)
#define BAL_ERR_BASE_SYSTEM                (-100)
#define BAL_ERR_BASE_FRAMEWORK             (-200)
#define BAL_ERR_BASE_STORAGE               (-300)
#define BAL_ERR_BASE_IDENTITY              (-400)
#define BAL_ERR_BASE_UART                  (-500)
#define BAL_ERR_BASE_NETWORK               (-600)
#define BAL_ERR_BASE_MQTT                  (-700)
#define BAL_ERR_BASE_HTTPS                 (-800)
#define BAL_ERR_BASE_PROVISIONING          (-900)
#define BAL_ERR_BASE_HEARTBEAT             (-1000)
#define BAL_ERR_BASE_DIAGNOSTICS           (-1100)
#define BAL_ERR_BASE_FOTA                  (-1200)
/** @} */

/* ========================================================================== */
/* 1. Generic Standard Errors (0 to -99)                                      */
/* ========================================================================== */
#define BAL_ERR_GENERIC_FAILURE            (BAL_ERR_BASE_GENERIC - 1)
#define BAL_ERR_INVALID_PARAM              (BAL_ERR_BASE_GENERIC - 2)
#define BAL_ERR_NO_MEMORY                  (BAL_ERR_BASE_GENERIC - 3)
#define BAL_ERR_TIMEOUT                    (BAL_ERR_BASE_GENERIC - 4)
#define BAL_ERR_BUSY                       (BAL_ERR_BASE_GENERIC - 5)
#define BAL_ERR_NOT_SUPPORTED              (BAL_ERR_BASE_GENERIC - 6)
#define BAL_ERR_BAD_STATE                  (BAL_ERR_BASE_GENERIC - 7)
#define BAL_ERR_QUEUE_FULL                 (BAL_ERR_BASE_GENERIC - 8)
#define BAL_ERR_RESOURCE_CREATION_FAILED   (BAL_ERR_BASE_GENERIC - 9)

/* ========================================================================== */
/* 2. System Lifecycle Errors (-100 to -199)                                 */
/* ========================================================================== */
#define BAL_ERR_SYS_INIT_SEQUENCE_FAILED   (BAL_ERR_BASE_SYSTEM - 1)
#define BAL_ERR_SYS_SHUTDOWN_DENIED        (BAL_ERR_BASE_SYSTEM - 2)
#define BAL_ERR_SYS_CRITICAL_DEPENDENCY   (BAL_ERR_BASE_SYSTEM - 3)

/* ========================================================================== */
/* 3. Framework Errors (App Monitor / Watchdog) (-200 to -299)                */
/* ========================================================================== */
#define BAL_ERR_FW_TASK_HUNG               (BAL_ERR_BASE_FRAMEWORK - 1)
#define BAL_ERR_FW_WDG_REGISTER_FAILED     (BAL_ERR_BASE_FRAMEWORK - 2)

/* ========================================================================== */
/* 4. Passive Storage Errors (-300 to -399)                                   */
/* ========================================================================== */
#define BAL_ERR_STORAGE_MOUNT_FAILED       (BAL_ERR_BASE_STORAGE - 1)
#define BAL_ERR_STORAGE_READ_FAILED        (BAL_ERR_BASE_STORAGE - 2)
#define BAL_ERR_STORAGE_WRITE_FAILED       (BAL_ERR_BASE_STORAGE - 3)
#define BAL_ERR_STORAGE_FILE_NOT_FOUND     (BAL_ERR_BASE_STORAGE - 4)
#define BAL_ERR_STORAGE_CORRUPTED          (BAL_ERR_BASE_STORAGE - 5)

/* ========================================================================== */
/* 5. Passive Identity Errors (-400 to -499)                                  */
/* ========================================================================== */
#define BAL_ERR_ID_KEY_NOT_FOUND           (BAL_ERR_BASE_IDENTITY - 1)
#define BAL_ERR_ID_CERT_EXPIRED            (BAL_ERR_BASE_IDENTITY - 2)
#define BAL_ERR_ID_CRYPTO_SIGN_FAILED      (BAL_ERR_BASE_IDENTITY - 3)

/* ========================================================================== */
/* 6. Active UART Manager Errors (-500 to -559)                               */
/* ========================================================================== */
#define BAL_ERR_UART_OPEN_FAILED           (BAL_ERR_BASE_UART - 1)
#define BAL_ERR_UART_TX_FAILED             (BAL_ERR_BASE_UART - 2)
#define BAL_ERR_UART_PARSER_CRC_INVALID    (BAL_ERR_BASE_UART - 3)
#define BAL_ERR_UART_FRAME_INCOMPLETE      (BAL_ERR_BASE_UART - 4)

/* ========================================================================== */
/* 7. Active Network Manager Errors (-600 to -699)                           */
/* ========================================================================== */
#define BAL_ERR_NET_SIM_NOT_READY          (BAL_ERR_BASE_NETWORK - 1)
#define BAL_ERR_NET_REGISTRATION_FAILED    (BAL_ERR_BASE_NETWORK - 2)
#define BAL_ERR_NET_PDP_DEACTIVATED        (BAL_ERR_BASE_NETWORK - 3)

/* ========================================================================== */
/* 8. Active MQTT Manager Errors (-700 to -799)                              */
/* ========================================================================== */
#define BAL_ERR_MQTT_CONN_REFUSED          (BAL_ERR_BASE_MQTT - 1)
#define BAL_ERR_MQTT_PUB_FAILED            (BAL_ERR_BASE_MQTT - 2)
#define BAL_ERR_MQTT_SUB_FAILED            (BAL_ERR_BASE_MQTT - 3)
#define BAL_ERR_MQTT_DISCONNECTED          (BAL_ERR_BASE_MQTT - 4)

/* ========================================================================== */
/* 9. Active HTTPS Manager Errors (-800 to -899)                             */
/* ========================================================================== */
#define BAL_ERR_HTTP_TLS_HANDSHAKE_FAILED  (BAL_ERR_BASE_HTTPS - 1)
#define BAL_ERR_HTTP_AUTH_UNAUTHORIZED     (BAL_ERR_BASE_HTTPS - 2)
#define BAL_ERR_HTTP_RESPONSE_404          (BAL_ERR_BASE_HTTPS - 3)

/* ========================================================================== */
/* 10. Active Provisioning Manager Errors (-900 to -999)                     */
/* ========================================================================== */
#define BAL_ERR_PROV_CSR_GEN_FAILED        (BAL_ERR_BASE_PROVISIONING - 1)
#define BAL_ERR_PROV_CHALLENGE_FAILED      (BAL_ERR_BASE_PROVISIONING - 2)

/* ========================================================================== */
/* 11. Active Heartbeat Manager Errors (-1000 to -1099)                      */
/* ========================================================================== */
#define BAL_ERR_HB_METRIC_COLLECT_FAILED   (BAL_ERR_BASE_HEARTBEAT - 1)

/* ========================================================================== */
/* 12. Active Diagnostics Manager Errors (-1100 to -1199)                    */
/* ========================================================================== */
#define BAL_ERR_DIAG_UDS_RESP_TIMEOUT      (BAL_ERR_BASE_DIAGNOSTICS - 1)
#define BAL_ERR_DIAG_INVALID_NRC           (BAL_ERR_BASE_DIAGNOSTICS - 2)

/* ========================================================================== */
/* 13. Active FOTA Manager Errors (-1200 to -1299)                           */
/* ========================================================================== */
#define BAL_ERR_FOTA_VERIFICATION_FAILED   (BAL_ERR_BASE_FOTA - 1)
#define BAL_ERR_FOTA_SIGNATURE_MISMATCH    (BAL_ERR_BASE_FOTA - 2)
#define BAL_ERR_FOTA_INSUFFICIENT_SPACE    (BAL_ERR_BASE_FOTA - 3)
#define BAL_ERR_FOTA_ROLLBACK_TRIGGERED    (BAL_ERR_BASE_FOTA - 4)


/******************************************************************************/
/*                                                                            */
/*                                   ENUMS                                    */
/*                                                                            */
/******************************************************************************/


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

#endif //!_ERROR_CODES_H

/**
 * Copyright(c) Bajaj Auto Technology Limited (BATL) as an unpublished work.
 * THIS SOFTWARE AND/OR MATERIAL IS THE PROPERTY OF BATL.
 * ALL USE, DISCLOSURE, AND/OR REPRODUCTION NOT SPECIFICALLY AUTHORIZED BY
 * BATL IS PROHIBITED.
 *
 * @author:<Author of H file - Name [Initials]>
 */
