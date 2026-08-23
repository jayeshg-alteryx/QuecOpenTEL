/**
 * @file          FS_Port.h
 * @brief         Header file for the layer that isolates QuecOpen SDK APIs.
 * @date          22/07/2026
 * @author        Aditya Bhosale [ASB]
 * @copyright     Bajaj Auto Technology Limited (BATL)
 */

/******************************************************************************/
/*                                                                            */
/*                                  INCLUDES                                  */
/*                                                                            */
/******************************************************************************/
#include "FS_Manager.h"

/******************************************************************************/
/*                                                                            */
/*                         PRIVATE FUNCTION DECLARATIONS                      */
/*                                                                            */
/******************************************************************************/

static const char *sccpt_FS_GetPath(FileSystemID_E e_FileId);
static FileSystemResult_E se_FS_ValidateRequest(const FileSystemReq_T *cstpt_fsContext);
static FileSystemResult_E se_FS_IsValidFile(FileSystemID_E e_FileId);

/******************************************************************************/
/*                                                                            */
/*                             PRIVATE VARIABLES                              */
/*                                                                            */
/******************************************************************************/

static const char * const sccptar_FS_PathTable[eFS_FILE_MAX] =
{
    [eFS_FILE_VIN] = "UFS:VIN",
    [eFS_FILE_VIN_ALIAS] = "UFS:VIN_ALIAS",
    [eFS_FILE_DEV_ID] = "UFS:DEV_ID",
    [eFS_FILE_ROOT_CA] = "UFS:Root_CA.crt",
    [eFS_FILE_DEV_CERT] = "UFS:Device.crt",
    [eFS_FILE_DEV_PUBLIC_KEY] = "UFS:Public.key",
    [eFS_FILE_DEV_PRIVATE_KEY] = "UFS:Private.key",
    [eFS_FILE_DEV_CSR] = "UFS:CertSignReq.csr.",
    [eFS_FILE_DEV_PROV_STATUS] = "UFS:ProvStat",


    [eFS_FILE_MANIFEST]              = "UFS:Manifest",
    [eFS_FILE_FOTA_FILE]             = "UFS:current.fota",
    [eFS_FILE_FOTA_JOB_DOCUMENT_A]   = "UFS:FOTAJobA.json",
    [eFS_FILE_FOTA_JOB_DOCUMENT_B]   = "UFS:FOTAJobB.json",
    [eFS_FILE_FOTA_CONTEXT_A]        = "UFS:FOTAContextA.bin",
    [eFS_FILE_FOTA_CONTEXT_B]        = "UFS:FOTAContextB.bin",
    [eFS_FILE_FOTA_RESUME]           = "UFS:FOTAResume.bin",
    [eFS_FILE_FOTA_POLL_BUDGET]      = "UFS:FOTAPoll.bin",
    [eFS_FILE_OFFLINE_ANALYTICS] = "UFS:OfflineAnalytics.json"
};

/******************************************************************************/
/*                                                                            */
/*                      PUBLIC FUNCTION DEFINITIONS                           */
/*                                                                            */
/******************************************************************************/
/**
 * @public        ge_FS_Init
 * @brief         Initializes the File System Manager service.
 * @param         none
 * @return        FileSystemResult_E indicating the result of the operation.
 */
FileSystemResult_E ge_FS_Init(void)
{
    return eFS_OK;
}

/**
 * @public        ge_FS_Read
 * @brief         Reads bytes from a fixed managed UFS file.
 * @details       The application supplies only a logical file ID, offset,
 *                destination buffer, and requested byte count. This API resolves
 *                the internal path, opens the file through fs_port, seeks to the
 *                requested offset, reads bytes, updates actualBytes, and closes
 *                the file.
 * @param[in,out] stpt_fsContext Pointer to read request.
 * @return        FileSystemResult_E indicating the result of the operation.
 */
FileSystemResult_E ge_FS_Read(FileSystemReq_T *stpt_fsContext)
{
    const char *ccpt_Path = NULL;
    FS_FILE_HANDLE handle = FS_PORT_INVALID_HANDLE;
    FileSystemResult_E e_Result;

    /* Validate request. */
    e_Result = se_FS_ValidateRequest(stpt_fsContext);

    if (e_Result == eFS_OK)
    {
        /* Reset transferred byte count. */
        stpt_fsContext->u32_ActualBytes = 0U;

        /* Nothing to read. */
        if (stpt_fsContext->u32_RequestedBytes == 0U)
        {
            e_Result = eFS_OK;
        }
        else
        {
            /* Resolve internal file path. */
            ccpt_Path = sccpt_FS_GetPath(stpt_fsContext->e_FileId);

            if (ccpt_Path == NULL)
            {
                e_Result = eFS_INVALID_FILE_ID;
            }
            else
            {
                /* Open file. */
                e_Result = ge_FileOpen(&handle,
                                       ccpt_Path,
                                       FS_PORT_OPEN_MODE_READ);

                if (e_Result == eFS_OK)
                {
                    /* Seek to requested offset. */
                    e_Result = ge_FileSeek(handle,
                                           stpt_fsContext->u32_Offset, FS_PORT_SEEK_SET);

                    if (e_Result == eFS_OK)
                    {
                        /* Read requested bytes. */
                        e_Result = ge_FileRead(handle,
                                               stpt_fsContext->vpt_Buffer,
                                               stpt_fsContext->u32_RequestedBytes,
                                               &stpt_fsContext->u32_ActualBytes);

                        /* End of file reached before satisfying request. */
                        if ((e_Result == eFS_OK) &&
                            (stpt_fsContext->u32_ActualBytes <
                             stpt_fsContext->u32_RequestedBytes))
                        {
                            e_Result = eFS_EOF;
                            BAL_FILE_SYSTEM_LOG("End of file reached before satisfying request.");
                        }
                    }

                    if (ge_FileClose(handle) != eFS_OK)
                    {
                        BAL_FILE_SYSTEM_LOG("Failed to close file.");
                    }
                }
            }
        }
    }

    return e_Result;
}

/**
 * @public        ge_FS_Write
 * @brief         Writes bytes to a fixed managed UFS file.
 * @details       This API validates the request, checks read-only policy,
 *                resolves the internal path, opens the existing file through
 *                fs_port, seeks to the requested offset, writes bytes, updates
 *                actualBytes, and closes the file. File creation is not exposed
 *                to the application layer.
 * @param[in,out] stpt_fsContext Pointer to write request.
 * @return        FileSystemResult_E indicating the result of the operation.
 */
FileSystemResult_E ge_FS_Write(FileSystemReq_T *stpt_fsContext)
{
    const char *ccpt_Path = NULL;
    FS_FILE_HANDLE handle = FS_PORT_INVALID_HANDLE;
    FileSystemResult_E e_Result;

    /* Validate request. */
    e_Result = se_FS_ValidateRequest(stpt_fsContext);

    if (e_Result == eFS_OK)
    {
        /* Reset transferred byte count. */
        stpt_fsContext->u32_ActualBytes = 0U;

        /* Nothing to write. */
        if (stpt_fsContext->u32_RequestedBytes == 0U)
        {
            e_Result = eFS_OK;
        }
        else
        {
            /* Resolve internal file path. */
            ccpt_Path = sccpt_FS_GetPath(stpt_fsContext->e_FileId);

            if (ccpt_Path == NULL)
            {
                e_Result = eFS_INVALID_FILE_ID;
            }
            else
            {
                /* Open file. */
                e_Result = ge_FileOpen(&handle,
                                       ccpt_Path,
                                       FS_PORT_OPEN_MODE_WRITE);

                if (e_Result == eFS_OK)
                {
                    /* Seek to requested offset. */
                    e_Result = ge_FileSeek(handle,
                                           stpt_fsContext->u32_Offset, FS_PORT_SEEK_SET);

                    if (e_Result == eFS_OK)
                    {
                        /* Write requested bytes. */
                        e_Result = ge_FileWrite(handle,
                                                stpt_fsContext->vpt_Buffer,
                                                stpt_fsContext->u32_RequestedBytes,
                                                &stpt_fsContext->u32_ActualBytes);

                        /* Detect partial write. */
                        if ((e_Result == eFS_OK) &&
                            (stpt_fsContext->u32_ActualBytes <
                             stpt_fsContext->u32_RequestedBytes))
                        {
                            e_Result = eFS_WRITE_FAILED;
                        }
                    }

                    if (ge_FileClose(handle) != eFS_OK)
                    {
                        BAL_FILE_SYSTEM_LOG("Failed to close file.");
                    }
                }
            }
        }
    }

    return e_Result;
}

/**
 * @public        ge_FS_Append
 * @brief         Appends bytes to a End of UFS file.
 * @details       This API validates the request, checks read-only policy,
 *                resolves the internal path, opens the existing file through
 *                fs_port in append mode, writes bytes, updates actualBytes, and
 *                closes the file. File creation is not exposed to the application layer.
 * @param[in,out] stpt_fsContext Pointer to append request.
 * @return        FileSystemResult_E indicating the result of the operation.
 */
FileSystemResult_E ge_FS_Append(FileSystemReq_T *stpt_fsContext)
{
    const char *ccpt_Path = NULL;
    FS_FILE_HANDLE handle = FS_PORT_INVALID_HANDLE;
    FileSystemResult_E e_Result;

    /* Validate request. */
    e_Result = se_FS_ValidateRequest(stpt_fsContext);

    if (e_Result == eFS_OK)
    {
        /* Reset transferred byte count. */
        stpt_fsContext->u32_ActualBytes = 0U;

        /* Nothing to append. */
        if (stpt_fsContext->u32_RequestedBytes == 0U)
        {
            e_Result = eFS_OK;
        }
        else
        {
            /* Resolve internal file path. */
            ccpt_Path = sccpt_FS_GetPath(stpt_fsContext->e_FileId);

            if (ccpt_Path == NULL)
            {
                e_Result = eFS_INVALID_FILE_ID;
            }
            else
            {
                /* Open file in append mode. */
                e_Result = ge_FileOpen(&handle,
                                       ccpt_Path,
                                       FS_PORT_OPEN_MODE_APPEND);

                if (e_Result == eFS_OK)
                {
                    /* Append requested bytes. */
                    e_Result = ge_FileWrite(handle,
                                            stpt_fsContext->vpt_Buffer,
                                            stpt_fsContext->u32_RequestedBytes,
                                            &stpt_fsContext->u32_ActualBytes);

                    /* Detect partial append. */
                    if ((e_Result == eFS_OK) &&
                        (stpt_fsContext->u32_ActualBytes <
                         stpt_fsContext->u32_RequestedBytes))
                    {
                        e_Result = eFS_APPEND_FAILED;
                    }

                    if (ge_FileClose(handle) != eFS_OK)
                    {
                        BAL_FILE_SYSTEM_LOG("Failed to close appended file.");
                    }
                }
            }
        }
    }

    return e_Result;
}

/**
 * @public        ge_FS_Create
 * @brief         Creates a fixed managed UFS file.
 * @param[in]     file Logical file identifier.
 * @return        eFS_OK when creation completes.
 */
FileSystemResult_E ge_FS_Create(FileSystemID_E e_FileId)
{
    const char *ccpt_Path = NULL;
    FileSystemResult_E e_Result = eFS_OK;

    /* Validate file ID. */
    if (se_FS_IsValidFile(e_FileId) != eFS_OK)
    {
        e_Result = eFS_INVALID_FILE_ID;
    }
    else
    {
        /* Resolve internal file path. */
        ccpt_Path = sccpt_FS_GetPath(e_FileId);

        if (ccpt_Path == NULL)
        {
            e_Result = eFS_INVALID_FILE_ID;
        }
        else
        {
            /* Create file. */
            e_Result = ge_FileCreate(ccpt_Path);
        }
    }

    return e_Result;
}

/**
 * @public        ge_FS_Delete
 * @brief         Deletes a fixed managed UFS file.
 * @param[in]     file Logical file identifier.
 * @return        eFS_OK when deletion completes.
 */
FileSystemResult_E ge_FS_Delete(FileSystemID_E e_FileId)
{
    const char *ccpt_Path = NULL;
    FileSystemResult_E e_Result = eFS_OK;

    /* Validate file ID. */
    if (se_FS_IsValidFile(e_FileId) != eFS_OK)
    {
        e_Result = eFS_INVALID_FILE_ID;
    }
    else
    {
        /* Resolve internal file path. */
        ccpt_Path = sccpt_FS_GetPath(e_FileId);

        if (ccpt_Path == NULL)
        {
            e_Result = eFS_INVALID_FILE_ID;
        }
        else
        {
            /* Delete file. */
            e_Result = ge_FileDelete(ccpt_Path);
        }
    }

    return e_Result;
}

/**
 * @public        ge_FS_GetFileSize
 * @brief         Gets the size of a fixed managed UFS file.
 * @param[in]     file Logical file identifier.
 * @param[out]    fileSize Pointer to updated file size.
 * @return        eFS_OK when operation completes.
 */
uint32_t ge_FS_GetFileSize(FileSystemID_E e_FileId)
{
    const char *ccpt_Path = NULL;
    uint32_t u32_FileSize = 0U;

    /* Validate file ID. */
    if (se_FS_IsValidFile(e_FileId) == eFS_OK)
    {
        /* Resolve internal file path. */
        ccpt_Path = sccpt_FS_GetPath(e_FileId);

        if (ccpt_Path != NULL)
        {
            u32_FileSize = ge_FileGetSize(ccpt_Path);
        }
    }

    return u32_FileSize;
}

/**
 * @public        ge_FS_IsFilePresent
 * @brief         Checks whether a fixed managed UFS file exists.
 * @param[in]     file Logical file identifier.
 * @param[out]    present Pointer updated with presence status.
 * @return        eFS_OK when check completes.
 */
FileSystemResult_E ge_FS_IsFilePresent(FileSystemID_E e_FileId)
{
    const char *ccpt_Path                   = NULL;
    FS_FILE_HANDLE handle                   = FS_PORT_INVALID_HANDLE;
    FileSystemResult_E e_Result             = eFS_FILE_NOT_FOUND;

    /* Validate file ID. */
    if (se_FS_IsValidFile(e_FileId) != eFS_OK)
    {
        e_Result = eFS_INVALID_FILE_ID;
    }
    else
    {
        /* Resolve internal file path. */
        ccpt_Path = sccpt_FS_GetPath(e_FileId);

        if (ccpt_Path != NULL)
        {
            /* Probe file by opening it. */
            e_Result = ge_FileOpen(&handle,
                                    ccpt_Path,
                                    FS_PORT_OPEN_MODE_READ);

            if (e_Result == eFS_OK)
            {
                e_Result = ge_FileClose(handle);
            }
            else if (e_Result == eFS_OPEN_FAILED)
            {
                e_Result = eFS_FILE_NOT_FOUND;
            }
            else
            {
                /* Do nothing. */
            }
        }
        else
        {
            e_Result = eFS_INVALID_FILE_ID;
        }
    }
    return e_Result;
}

/******************************************************************************/
/*                                                                            */
/*                            PRIVATE FUNCTIONS                               */
/*                                                                            */
/******************************************************************************/
/**
 * @private       sccpt_FS_GetPath
 * @brief         Resolves the internal path for a logical file ID.
 * @param[in]     e_FileId : Logical file identifier.
 * @return        Pointer to the internal file path or NULL if invalid.
 */
static const char *sccpt_FS_GetPath(FileSystemID_E e_FileId)
{
    // Validate file ID and return the corresponding path from the table.
    return sccptar_FS_PathTable[e_FileId];
}

/**
 * @private       se_FS_ValidateRequest
 * @brief         Validates the file system request.
 * @param[in]     cstpt_fsContext : Pointer to the file system request.
 * @return        FileSystemResult_E indicating the result of the validation.
 */
static FileSystemResult_E se_FS_ValidateRequest(const FileSystemReq_T *cstpt_fsContext)
{
    FileSystemResult_E e_Result = eFS_OK;

    if (cstpt_fsContext == NULL)
    {
        e_Result = eFS_INVALID_PARAMETER;
    }
    else{
        // A zero-byte read or write should generally return eFS_OK without requiring a buffer.
        if ((cstpt_fsContext->u32_RequestedBytes > 0U) && (cstpt_fsContext->vpt_Buffer == NULL))
        {
            e_Result = eFS_INVALID_PARAMETER;
        }
        else{
            if (se_FS_IsValidFile(cstpt_fsContext->e_FileId) != eFS_OK)
            {
                e_Result = eFS_INVALID_FILE_ID;
            }
            else{
                e_Result = eFS_OK;
            }
        }
    }
    return e_Result;
}

/**
 * @private       se_FS_IsValidFile
 * @brief         Validates a logical file ID.
 * @param[in]     e_FileId : Logical file identifier.
 * @return        true if the file ID is valid, false otherwise.
 */
static FileSystemResult_E se_FS_IsValidFile(FileSystemID_E e_FileId)
{
    return ((uint32_t)e_FileId < (uint32_t)eFS_FILE_MAX) ? eFS_OK : eFS_INVALID_FILE_ID;
}

/**
 * Copyright(c) Bajaj Auto Technology Limited (BATL) as an unpublished work.
 * THIS SOFTWARE AND/OR MATERIAL IS THE PROPERTY OF BATL.
 * ALL USE, DISCLOSURE, AND/OR REPRODUCTION NOT SPECIFICALLY AUTHORIZED BY
 * BATL IS PROHIBITED.
 *
 * @author: Aditya Bhosale [ASB]
 */
