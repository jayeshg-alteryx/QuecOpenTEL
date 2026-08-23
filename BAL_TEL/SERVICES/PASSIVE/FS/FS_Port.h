/**
 * @file          FS_Port.h
 * @brief         Header file for the layer that isolates QuecOpen SDK APIs.
 * @date          22/07/2026
 * @author        Aditya Bhosale [ASB]
 * @copyright     Bajaj Auto Technology Limited (BATL)
 */

#ifndef _FS_PORT_H
#define _FS_PORT_H

/******************************************************************************/
/*                                                                            */
/*                                  INCLUDES                                  */
/*                                                                            */
/******************************************************************************/
#include "FS_Types.h"
#include "ql_fs.h"

/******************************************************************************/
/*                                                                            */
/*                                  DEFINES                                   */
/*                                                                            */
/******************************************************************************/

/**
 * @def           FS_PORT_OPEN_MODE_READ
 * @brief         Indicates mode in which file will be opened(Read in binary).
 */
#define FS_PORT_OPEN_MODE_READ      "rb"

/**
 * @def           FS_PORT_OPEN_MODE_UPDATE
 * @brief         Indicates mode in which file will be opened
 *                (Read+Write in binary).
 */
#define FS_PORT_OPEN_MODE_WRITE    "rb+"

/**
 * @def           FS_PORT_OPEN_MODE_CREATE
 * @brief         Indicates mode in which file will be opened
 *                (Create new file in binary).
 */
#define FS_PORT_OPEN_MODE_CREATE    "wb"

/**
 * @def           FS_PORT_OPEN_MODE_APPEND
 * @brief         Indicates mode in which file will be opened
 *                (Append to existing file in binary).
 */
#define FS_PORT_OPEN_MODE_APPEND    "ab"


/**
 * @def           FS_PORT_SEEK_SET
 * @brief         Indicates Seek from File beginning.
 */
#define FS_PORT_SEEK_SET            (0)

/**
 * @def           FS_PORT_INVALID_HANDLE
 * @brief         Indicated invalid file handle.
 */
#define FS_PORT_INVALID_HANDLE      ((QFILE)-1)

/**
 * @def           FS_PORT_NULL_HANDLE
 * @brief         Indicates handle not opened.
 */
#define FS_PORT_NULL_HANDLE         ((QFILE)0)

/**
 * @def           FS_PORT_IS_INVALID_HANDLE
 * @brief         Indicates invalid file handle.
 */
#define FS_PORT_IS_INVALID_HANDLE(handle) ((handle) < 0)

/**
 * @def           BAL_FILE_SYSTEM_LOG_LEVEL
 * @brief         Log verbosity for the File System Manager layer.
 */
#define BAL_FILE_SYSTEM_LOG_LEVEL    QL_LOG_LEVEL_INFO

/**
 * @def           BAL_FILE_SYSTEM_LOG
 * @brief         Macro for logging in the File System Manager layer.
 */
#define BAL_FILE_SYSTEM_LOG(msg, ...) \
    QL_LOG(BAL_FILE_SYSTEM_LOG_LEVEL, "BAL_FS", msg, ##__VA_ARGS__)
/******************************************************************************/
/*                                                                            */
/*                                 TYPEDEF                                    */
/*                                                                            */
/******************************************************************************/
/**
 * @typedef       FS_FILE_HANDLE
 * @brief         File handle type used by the File System Manager.
 */
typedef QFILE FS_FILE_HANDLE;

/******************************************************************************/
/*                                                                            */
/*                              EXTERN FUNCTIONS                              */
/*                                                                            */
/******************************************************************************/

extern FileSystemResult_E ge_FileOpen(FS_FILE_HANDLE *pt_FileHandle,
                        const char *ccpt_Path,
                        const char *ccpt_Mode);

extern FileSystemResult_E ge_FileClose(FS_FILE_HANDLE handle);

extern FileSystemResult_E ge_FileSeek(FS_FILE_HANDLE handle,
                        uint32_t u32_Offset, uint32_t u32_Origin);

extern FileSystemResult_E ge_FileRead(FS_FILE_HANDLE handle,
                        void *vpt_buffer,
                        uint32_t u32_RequestedBytes,
                        uint32_t *u32_ActualBytes);

extern FileSystemResult_E ge_FileWrite(FS_FILE_HANDLE handle,
                         const void *cvpt_buffer,
                         uint32_t u32_RequestedBytes,
                         uint32_t *u32_ActualBytes);

extern FileSystemResult_E ge_FileCreate(const char *ccpt_Path);

extern FileSystemResult_E ge_FileDelete(const char *ccpt_Path);

extern uint32_t ge_FileGetSize(const char *ccpt_Path);

#endif // !_FS_PORT_H

/**
 * Copyright(c) Bajaj Auto Technology Limited (BATL) as an unpublished work.
 * THIS SOFTWARE AND/OR MATERIAL IS THE PROPERTY OF BATL.
 * ALL USE, DISCLOSURE, AND/OR REPRODUCTION NOT SPECIFICALLY AUTHORIZED BY
 * BATL IS PROHIBITED.
 *
 * @author: Aditya Bhosale [ASB]
 */
