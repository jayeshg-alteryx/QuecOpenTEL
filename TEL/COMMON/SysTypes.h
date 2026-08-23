/**
 * @file          SysTypes.h
 * @brief
 * @date          14/07/2026
 * @author        Yash Giramkar [YSG], Aditya Bhosale [ASB]
 * @copyright     Bajaj Auto Technology Limited (BATL)
 */

#ifndef _BAL_SYS_TYPES_H
#define _BAL_SYS_TYPES_H

/******************************************************************************/
/*                                                                            */
/*                                  INCLUDES                                  */
/*                                                                            */
/******************************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/******************************************************************************/
/*                                                                            */
/*                                  DEFINES                                   */
/*                                                                            */
/******************************************************************************/

#ifndef TRUE
#define TRUE  1
#endif

#ifndef FALSE
#define FALSE 0
#endif

/******************************************************************************/
/*                                                                            */
/*                                TYPEDEFS                                    */
/*                                                                            */
/******************************************************************************/

/* Standardized byte type for buffer representations */
typedef uint8_t  bal_byte_t;
typedef uint32_t bal_time_ms_t;  /* System uptime/tick timing in milliseconds */

/******************************************************************************/
/*                                                                            */
/*                                   ENUMS                                    */
/*                                                                            */
/******************************************************************************/

/**
 * @enum          TelSysResponse_E
 * @brief         Response enum from a function, could be used generically
 *                across system.
 */
typedef enum {
   eTEL_OK = 0,
   eTEL_NOK = -1,
} TelSysResponse_E;

/* ========================================================================== */
/* 2. Global System States (Used by System Manager & App Monitor)             */
/* ========================================================================== */

/**
 * @brief Core Lifecycle States of the Telematics System.
 */
typedef enum {
   eSYS_STATE_BOOTING = 0,      /**< System is powered and running initial tasks */
   eSYS_STATE_INITIALIZING,    /**< Core hardware, memory, and drivers are setting up */
   eSYS_STATE_READY,           /**< Normal operation; all active managers online */
   eSYS_STATE_DEGRADED,        /**< Operational, but with non-critical failures (e.g., SIM missing) */
   eSYS_STATE_SAFE_MODE,       /**< Critical recovery state; minimal services running */
   eSYS_STATE_SHUTDOWN,        /**< Graceful tear-down in progress */
   eSYS_STATE_REBOOTING,       /**< Preparation completed; awaiting watchdog/hw reset trigger */
   eSYS_STATE_MAX
} TelSysState_E;

/**
 * @brief Common Active Manager Operational States.
 */
typedef enum {
   eMGR_STATE_UNINITIALIZED = 0, /**< Initial state prior to allocation */
   eMGR_STATE_INIT,             /**< Initialization task sequence running */
   eMGR_STATE_IDLE,             /**< Task running, waiting for IPC event stimulation */
   eMGR_STATE_BUSY,             /**< Processing critical transactional data */
   eMGR_STATE_SUSPENDED,        /**< Paused by System Manager (e.g., in sleep/low-power mode) */
   eMGR_STATE_FAULT,            /**< Recoverable failure detected; waiting for restart/re-init */
   eMGR_STATE_MAX
} TelManagerState_E;

/* ========================================================================== */
/* 3. Helper and Defensive Programming Macros                                */
/* ========================================================================== */

/**
 * @brief Prevent compiler warnings for intentionally unused variables/arguments.
 */
#define BAL_UNUSED(x) ((void)(x))

/**
 * @brief Safely determine array sizes at compile time.
 */
#define BAL_ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))


/**
 * @brief Min/Max evaluations without double-evaluation side effects.
 */
#define BAL_MIN(a, b) ({ __typeof__(a) _a = (a); __typeof__(b) _b = (b); _a < _b ? _a : _b; })
#define BAL_MAX(a, b) ({ __typeof__(a) _a = (a); __typeof__(b) _b = (b); _a > _b ? _a : _b; })

#endif //!_BAL_SYS_TYPES_H

/**
 * Copyright(c) Bajaj Auto Technology Limited (BATL) as an unpublished work.
 * THIS SOFTWARE AND/OR MATERIAL IS THE PROPERTY OF BATL.
 * ALL USE, DISCLOSURE, AND/OR REPRODUCTION NOT SPECIFICALLY AUTHORIZED BY
 * BATL IS PROHIBITED.
 *
 * @author:<Author of H file - Name [Initials]>
 */
