/**
 * @file          FS_Types.h
 * @brief         Public File System Manager type definitions.
 * @date          22/07/2026
 * @author        Aditya Bhosale [ASB]
 * @copyright     Bajaj Auto Technology Limited (BATL)
 */

#ifndef _FS_TYPES_H
#define _FS_TYPES_H

/******************************************************************************/
/*                                                                            */
/*                                  INCLUDES                                  */
/*                                                                            */
/******************************************************************************/
#include "Generics.h"

/******************************************************************************/
/*                                                                            */
/*                                   ENUMS                                    */
/*                                                                            */
/******************************************************************************/

/**
 * @enum          FileSystemID_E
 * @brief         Logical file identifiers exposed to the application layer.
 * @details       Credentials are table-managed like all other files, but
 *                write-protected.
 */
typedef enum
{
    /**************************************************************************/
    /*               Provisioning application related files                   */
    /**************************************************************************/
    eFS_FILE_VIN,                // VIN data file.
    eFS_FILE_VIN_ALIAS,          // VIN alias file.
    eFS_FILE_DEV_ID,             // Device ID file.
    eFS_FILE_ROOT_CA,            // Root CA certificate for TLS connections.
    eFS_FILE_DEV_CERT,           // Client certificate for TLS connections.
    eFS_FILE_DEV_PUBLIC_KEY,     // Client public key for TLS connections.
    eFS_FILE_DEV_PRIVATE_KEY,    // Client private key for TLS connections.
    eFS_FILE_DEV_CSR,            // Client CSR for TLS connections.
    eFS_FILE_DEV_PROV_STATUS,    // Device provisioning status file.


    /**************************************************************************/
    /*                  Manifest application related files                    */
    /**************************************************************************/
    eFS_FILE_MANIFEST,           // Manifest file for FOTA updates.

    /**************************************************************************/
    /*                  FOTA application related files                        */
    /**************************************************************************/
    eFS_FILE_FOTA_FILE,              // FOTA update file.
    eFS_FILE_FOTA_JOB_DOCUMENT_A,    // Validated job document slot A.
    eFS_FILE_FOTA_JOB_DOCUMENT_B,    // Validated job document slot B.
    eFS_FILE_FOTA_CONTEXT_A,         // Recovery context slot A.
    eFS_FILE_FOTA_CONTEXT_B,         // Recovery context slot B.
    eFS_FILE_FOTA_RESUME,             // Download resume metadata.
    eFS_FILE_FOTA_POLL_BUDGET,        // Daily cloud poll budget.

    /**************************************************************************/
    /*                    OA application related files                        */
    /**************************************************************************/
    eFS_FILE_OFFLINE_ANALYTICS,  // Offline analytics data file.

    eFS_FILE_MAX                 // Maximum number of logical files; used for validation.
} FileSystemID_E;

/**
 * @enum          FileSystemResult_E
 * @brief         File System Manager result codes.
 */
typedef enum
{
    eFS_OK               = 0, // Operation completed successfully.
    eFS_NOT_INITIALIZED  = 1, // File System Manager has not been initialized.
    eFS_INVALID_PARAMETER= 2, // One or more input parameters are invalid.
    eFS_INVALID_FILE_ID  = 3, // Logical File ID is outside the valid FileSystemID_E range.
    eFS_FILE_NOT_FOUND   = 4, // Requested file does not exist in the underlying file system.
    eFS_PERMISSION_DENIED= 5, // Requested operation is not permitted by the underlying file system.
    eFS_OPEN_FAILED      = 6, // Failed to open the requested file.
    eFS_CLOSE_FAILED     = 7, // Failed to close the opened file.
    eFS_SEEK_FAILED      = 8, // Failed to move the file pointer to the requested offset.
    eFS_READ_FAILED      = 9, // Failed to read data from the file.
    eFS_WRITE_FAILED     = 10, // Failed to write data to the file or partial write occurred.
    eFS_APPEND_FAILED    = 11, // Failed to append data to the file or partial append occurred.
    eFS_CREATE_FAILED    = 12, // Failed to create the requested file.
    eFS_DELETE_FAILED    = 13, // Failed to delete the requested file.
    eFS_EOF              = 14, // End of file reached before all requested bytes could be read.
    eFS_INTERNAL_ERROR   = 15  // Unexpected internal File System Manager error.
} FileSystemResult_E;

/******************************************************************************/
/*                                                                            */
/*                                 STRUCTURES                                 */
/*                                                                            */
/******************************************************************************/

/**
 * @struct        FileSystemReq_T
 * @brief         Structure containg the File System Manager request parameters.
 */
typedef struct
{
    FileSystemID_E e_FileId;       // Logical file identifier.
    uint32_t u32_Offset;          // Offset in bytes from the beginning of the file.
    void *vpt_Buffer;             // Pointer to the data buffer for read/write operations.
    uint32_t u32_RequestedBytes;  // Number of bytes requested to read/write.
    uint32_t u32_ActualBytes;     // Number of bytes actually read/written.
} FileSystemReq_T;

#endif // !_FS_TYPES_H

/**
 * Copyright(c) Bajaj Auto Technology Limited (BATL) as an unpublished work.
 * THIS SOFTWARE AND/OR MATERIAL IS THE PROPERTY OF BATL.
 * ALL USE, DISCLOSURE, AND/OR REPRODUCTION NOT SPECIFICALLY AUTHORIZED BY
 * BATL IS PROHIBITED.
 *
 * @author: Aditya Bhosale [ASB]
 */
