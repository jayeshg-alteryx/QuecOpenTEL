/**
 * @file          FS_Manager.h
 * @brief         Header file containing File System Manager APIs.
 * @date          22/07/2026
 * @author        Aditya Bhosale [ASB]
 * @copyright     Bajaj Auto Technology Limited (BATL)
 */

#ifndef _FS_MANAGER_H
#define _FS_MANAGER_H

/******************************************************************************/
/*                                                                            */
/*                                  INCLUDES                                  */
/*                                                                            */
/******************************************************************************/
#include "FS_Port.h"

/******************************************************************************/
/*                                                                            */
/*                              EXTERN FUNCTIONS                              */
/*                                                                            */
/******************************************************************************/

extern FileSystemResult_E ge_FS_Init(void);

extern FileSystemResult_E ge_FS_Read(FileSystemReq_T *stpt_fsContext);

extern FileSystemResult_E ge_FS_Write(FileSystemReq_T *stpt_fsContext);

extern FileSystemResult_E ge_FS_Append(FileSystemReq_T *stpt_fsContext);

extern FileSystemResult_E ge_FS_Create(FileSystemID_E e_FileId);

extern FileSystemResult_E ge_FS_Delete(FileSystemID_E e_FileId);

extern uint32_t ge_FS_GetFileSize(FileSystemID_E e_FileId);

extern FileSystemResult_E ge_FS_IsFilePresent(FileSystemID_E e_FileId);

#endif // !_FS_MANAGER_H

/**
 * Copyright(c) Bajaj Auto Technology Limited (BATL) as an unpublished work.
 * THIS SOFTWARE AND/OR MATERIAL IS THE PROPERTY OF BATL.
 * ALL USE, DISCLOSURE, AND/OR REPRODUCTION NOT SPECIFICALLY AUTHORIZED BY
 * BATL IS PROHIBITED.
 *
 * @author: Aditya Bhosale [ASB]
 */
