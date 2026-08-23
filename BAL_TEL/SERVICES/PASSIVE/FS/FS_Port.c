
/**
 * @file          FS_Port.c
 * @brief         QuecOpen UFS port layer for File System Manager.
 * @date          22/07/2026
 * @author        Aditya Bhosale [ASB]
 * @copyright     Bajaj Auto Technology Limited (BATL)
 */

/******************************************************************************/
/*                                                                            */
/*                                  INCLUDES                                  */
/*                                                                            */
/******************************************************************************/
#include "FS_Port.h"

/******************************************************************************/
/*                                                                            */
/*                        PUBLIC FUNCTION DEFINITIONS                         */
/*                                                                            */
/******************************************************************************/
/**
 * @public        ge_FileOpen
 * @brief         Opens a UFS file using QuecOpen API.
 * @param[in]     path Internal UFS file path.
 * @param[in]     mode File open mode.
 * @param[out]    handle Pointer updated with opened file handle.
 * @return        eFS_OK when open succeeds.
 * @return        eFS_INVALID_PARAMETER when input parameters are invalid.
 * @return        eFS_OPEN_FAILED when QuecOpen open fails.
 */
FileSystemResult_E ge_FileOpen(FS_FILE_HANDLE *pt_FileHandle,
                        const char *ccpt_Path,
                        const char *ccpt_Mode)
{
    FileSystemResult_E e_Result = eFS_OPEN_FAILED;
    // Validate open parameters.
    if ((ccpt_Path == NULL) || (ccpt_Mode == NULL) || (pt_FileHandle == NULL))
    {
        e_Result = eFS_INVALID_PARAMETER;
    }
    else{
        // This is the only layer that translates a manager request into an SDK call.
        *pt_FileHandle = ql_fopen(ccpt_Path, ccpt_Mode);

        // Check if open returned valid handle.
        if (FS_PORT_IS_INVALID_HANDLE(*pt_FileHandle))
        {
            e_Result = eFS_OPEN_FAILED;
        }
        else
        {
            e_Result = eFS_OK;
        }
    }

    // File opened successfully.
    return e_Result;
}

/**
 * @public        ge_FileClose
 * @brief         Closes a UFS file handle using QuecOpen API.
 * @param[in]     handle File handle to close.
 * @return        eFS_OK when close succeeds.
 * @return        eFS_INVALID_PARAMETER when handle is invalid.
 * @return        eFS_CLOSE_FAILED when QuecOpen close fails.
 */
FileSystemResult_E ge_FileClose(FS_FILE_HANDLE handle)
{
    int32_t i32_CloseResult = 0;
    FileSystemResult_E e_Result = eFS_CLOSE_FAILED;

    // Validate file handle.
    if (FS_PORT_IS_INVALID_HANDLE(handle))
    {
        e_Result = eFS_INVALID_PARAMETER;
    }
    else{
        // Close UFS file using QuecOpen API.
        i32_CloseResult = (int32_t)ql_fclose(handle);

        // Check if close failed.
        if (i32_CloseResult < 0)
        {
            e_Result = eFS_CLOSE_FAILED;
        }
        else
        {
            e_Result = eFS_OK;
        }
    }

    // File closed successfully.
    return e_Result;
}

/**
 * @public        ge_FileSeek
 * @brief         Moves the current UFS file position using QuecOpen API.
 * @param[in]     handle Open file handle.
 * @param[in]     offset Offset in bytes.
 * @return        eFS_OK when seek succeeds.
 * @return        eFS_INVALID_PARAMETER when handle is invalid.
 * @return        eFS_SEEK_FAILED when QuecOpen seek fails.
 */
FileSystemResult_E ge_FileSeek(FS_FILE_HANDLE handle,
                        uint32_t u32_Offset, uint32_t u32_Origin)
{
    int32_t i32_SeekResult = 0;
    FileSystemResult_E e_Result = eFS_SEEK_FAILED;

    // Validate file handle.
    if (FS_PORT_IS_INVALID_HANDLE(handle))
    {
        e_Result = eFS_INVALID_PARAMETER;
    }
    else{
        // Seek UFS file using QuecOpen API.
        i32_SeekResult = (int32_t)ql_fseek(handle, (long)u32_Offset, u32_Origin);

        // Check if seek failed.
        if (i32_SeekResult < 0)
        {
            e_Result = eFS_SEEK_FAILED;
        }
        else
        {
            e_Result = eFS_OK;
        }
    }
    // Seek completed successfully.
    return e_Result;
}

/**
 * @public        ge_FileRead
 * @brief         Reads bytes from a UFS file using QuecOpen API.
 * @param[in]     handle Open file handle.
 * @param[out]    buffer Destination buffer.
 * @param[in]     requestedBytes Number of bytes requested.
 * @param[out]    actualBytes Number of bytes actually read.
 * @return        eFS_OK when read API succeeds.
 * @return        eFS_INVALID_PARAMETER when input parameters are invalid.
 * @return        eFS_READ_FAILED when QuecOpen read fails.
 */
FileSystemResult_E ge_FileRead(FS_FILE_HANDLE handle,
                        void *vpt_buffer,
                        uint32_t u32_RequestedBytes,
                        uint32_t *u32_ActualBytes)
{
    int32_t i32_ReadResult = 0;
    FileSystemResult_E e_Result = eFS_READ_FAILED;

    // Validate read parameters.
    if ((FS_PORT_IS_INVALID_HANDLE(handle)) ||
        (vpt_buffer == NULL) ||
        (u32_ActualBytes == NULL))
    {
        e_Result = eFS_INVALID_PARAMETER;
    }
    else{
        // Reset actual read bytes.
        *u32_ActualBytes = 0U;

        // Read UFS file using QuecOpen API.
        i32_ReadResult = (int32_t)ql_fread(vpt_buffer, 1U, u32_RequestedBytes, handle);

        // Check if read failed.
        if (i32_ReadResult < 0)
        {
            e_Result = eFS_READ_FAILED;
        }
        else
        {
            e_Result = eFS_OK;
        }
        // Update actual read bytes.
        *u32_ActualBytes = (uint32_t)i32_ReadResult;
    }

    // Read completed successfully.
    return e_Result;
}

/**
 * @public        ge_FileWrite
 * @brief         Writes bytes to a UFS file using QuecOpen API.
 * @param[in]     handle Open file handle.
 * @param[in]     buffer Source buffer.
 * @param[in]     requestedBytes Number of bytes requested.
 * @param[out]    actualBytes Number of bytes actually written.
 * @return        eFS_OK when write API succeeds.
 * @return        eFS_INVALID_PARAMETER when input parameters are invalid.
 * @return        eFS_WRITE_FAILED when QuecOpen write fails.
 */
FileSystemResult_E ge_FileWrite(FS_FILE_HANDLE handle,
                         const void *vpt_buffer,
                         uint32_t u32_RequestedBytes,
                         uint32_t *u32_ActualBytes)
{
    int32_t i32_WriteResult = 0;
    FileSystemResult_E e_Result = eFS_WRITE_FAILED;
    // Validate write parameters.
    if ((FS_PORT_IS_INVALID_HANDLE(handle)) ||
        (vpt_buffer == NULL) ||
        (u32_ActualBytes == NULL))
    {
        e_Result = eFS_INVALID_PARAMETER;
    }
    else{
        // Reset actual written bytes.
        *u32_ActualBytes = 0U;

        // Write UFS file using QuecOpen API.
        i32_WriteResult = (int32_t)ql_fwrite((void *)vpt_buffer,
                                              1U,
                                              u32_RequestedBytes,
                                              handle);

        // Check if write failed.
        if (i32_WriteResult < 0)
        {
            e_Result = eFS_WRITE_FAILED;
        }
        else
        {
            e_Result = eFS_OK;
        }

        // Update actual written bytes.
        *u32_ActualBytes = (uint32_t)i32_WriteResult;
    }

    // Write completed successfully.
    return e_Result;
}

/**
 * @public        ge_FileCreate
 * @brief         Creates a UFS file using QuecOpen API.
 * @param[in]     ccpt_Path Internal UFS file path.
 * @return        eFS_OK when create API succeeds.
 * @return        eFS_INVALID_PARAMETER when input parameters are invalid.
 * @return        eFS_CREATE_FAILED when QuecOpen create fails.
 */
FileSystemResult_E ge_FileCreate(const char *ccpt_Path)
{
    FS_FILE_HANDLE handle = FS_PORT_INVALID_HANDLE;
    FileSystemResult_E e_Result = eFS_CREATE_FAILED;

    /* Validate path. */
    if (ccpt_Path == NULL)
    {
        e_Result = eFS_INVALID_PARAMETER;
    }
    else
    {
        /* Create file. */
        handle = ql_fopen(ccpt_Path, FS_PORT_OPEN_MODE_CREATE);

        if (FS_PORT_IS_INVALID_HANDLE(handle))
        {
            e_Result = eFS_CREATE_FAILED;
        }
        else
        {
            e_Result = ge_FileClose(handle);
        }
    }

    return e_Result;
}

/**
 * @public        ge_FileDelete
 * @brief         Deletes a UFS file using QuecOpen API.
 * @param[in]     ccpt_Path Internal UFS file path.
 * @return        eFS_OK when delete API succeeds.
 * @return        eFS_INVALID_PARAMETER when input parameters are invalid.
 * @return        eFS_DELETE_FAILED when QuecOpen delete fails.
 */
FileSystemResult_E ge_FileDelete(const char *ccpt_Path)
{
    int32_t i32_Result;
    FileSystemResult_E e_Result = eFS_DELETE_FAILED;

    /* Validate path. */
    if (ccpt_Path == NULL)
    {
        e_Result = eFS_INVALID_PARAMETER;
    }
    else
    {
        /* Delete UFS file using QuecOpen API. */
        i32_Result = (int32_t)ql_remove(ccpt_Path);

        /* Check delete status. */
        if (i32_Result == 0)
        {
            e_Result = eFS_OK;
            BAL_FILE_SYSTEM_LOG("File deleted successfully: %s", ccpt_Path);
        }
        else
        {
            e_Result = eFS_DELETE_FAILED;
            BAL_FILE_SYSTEM_LOG("Failed to delete file: %s", ccpt_Path);
        }
    }

    return e_Result;
}

/**
 * @public        ge_FileGetSize
 * @brief         Gets the size of a UFS file using QuecOpen API.
 * @param[in]     ccpt_Path Internal UFS file path.
 * @return        The size of the file in bytes.
 */
uint32_t ge_FileGetSize(const char *ccpt_Path)
{
    FS_FILE_HANDLE handle = FS_PORT_INVALID_HANDLE;
    int32_t i32_FileSize = 0;

    /* Validate path. */
    if (ccpt_Path != NULL)
    {
        /* Open file. */
        if (ge_FileOpen(&handle,
                        ccpt_Path,
                        FS_PORT_OPEN_MODE_READ) == eFS_OK)
        {
            /* Query the SDK for the file size. */
            i32_FileSize = (int32_t)ql_fsize(handle);

            if (i32_FileSize < 0)
            {
                i32_FileSize = 0;
            }

            ge_FileClose(handle);
        }
    }
    else
    {
        BAL_FILE_SYSTEM_LOG("Invalid path provided for file size query.");
    }

    return (uint32_t)i32_FileSize;
}

/**
 * Copyright(c) Bajaj Auto Technology Limited (BATL) as an unpublished work.
 * THIS SOFTWARE AND/OR MATERIAL IS THE PROPERTY OF BATL.
 * ALL USE, DISCLOSURE, AND/OR REPRODUCTION NOT SPECIFICALLY AUTHORIZED BY
 * BATL IS PROHIBITED.
 *
 * @author: Aditya Bhosale [ASB]
 */
