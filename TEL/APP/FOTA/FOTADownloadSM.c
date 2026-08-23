#include "FOTADownloadSM.h"

#include <string.h>

static bool sb_FOTA_DownloadExpired(const FOTADownloadContext_T *cpt_Context)
{
    uint64_t u64_Now;
    uint32_t u32_Day;
    if ((cpt_Context == NULL) || (cpt_Context->cpt_Ops == NULL) ||
        (cpt_Context->cpt_Ops->get_time == NULL))
    {
        return false;
    }
    if (!cpt_Context->cpt_Ops->get_time(&u64_Now, &u32_Day))
    {
        return false;
    }
    return u64_Now >= cpt_Context->st_Package.u64_UrlExpiry;
}

static bool sb_FOTA_DownloadWrite(
    FOTADownloadContext_T *pt_Context,
    uint32_t u32_Length)
{
    uint8_t u8_Attempt;
    if ((pt_Context == NULL) || (pt_Context->cpt_Ops == NULL) ||
        (pt_Context->cpt_Ops->storage_write == NULL))
    {
        return false;
    }
    for (u8_Attempt = 0U; u8_Attempt < FOTA_RETRY_ATTEMPTS; u8_Attempt++)
    {
        if (pt_Context->cpt_Ops->storage_write(
                eFOTA_STORAGE_CURRENT_FILE,
                pt_Context->u32_PendingStart,
                pt_Context->pu8_Buffer,
                u32_Length))
        {
            return true;
        }
    }
    return false;
}

FOTADownloadResult_E ge_FOTA_DownloadBegin(
    FOTADownloadContext_T *pt_Context,
    const FOTAPlatformOps_T *cpt_Ops,
    const FOTAPackage_T *cpt_Package,
    uint32_t u32_ExistingSize,
    uint8_t *pu8_Buffer)
{
    if ((pt_Context == NULL) || (cpt_Ops == NULL) ||
        (cpt_Package == NULL) || (pu8_Buffer == NULL) ||
        (cpt_Ops->storage_size == NULL) ||
        (u32_ExistingSize > cpt_Package->u32_PackageSize))
    {
        return eFOTA_DOWNLOAD_FAILED;
    }
    memset(pt_Context, 0, sizeof(*pt_Context));
    pt_Context->cpt_Ops = cpt_Ops;
    pt_Context->st_Package = *cpt_Package;
    pt_Context->pu8_Buffer = pu8_Buffer;
    pt_Context->u32_FileSize = u32_ExistingSize;
    pt_Context->u8_NextCheckpoint = 25U;
    while ((pt_Context->u8_NextCheckpoint < 100U) &&
           ((uint64_t)u32_ExistingSize * 100U >=
            ((uint64_t)cpt_Package->u32_PackageSize *
             pt_Context->u8_NextCheckpoint)))
    {
        pt_Context->u8_NextCheckpoint =
            (uint8_t)(pt_Context->u8_NextCheckpoint + 25U);
    }
    if (u32_ExistingSize == cpt_Package->u32_PackageSize)
    {
        return eFOTA_DOWNLOAD_COMPLETE;
    }
    return eFOTA_DOWNLOAD_READY;
}

FOTADownloadResult_E ge_FOTA_DownloadIssueRange(
    FOTADownloadContext_T *pt_Context)
{
    uint32_t u32_Remaining;
    uint32_t u32_Length;
    if ((pt_Context == NULL) || (pt_Context->cpt_Ops == NULL) ||
        (pt_Context->cpt_Ops->https_get_range == NULL))
    {
        return eFOTA_DOWNLOAD_FAILED;
    }
    if (pt_Context->b_ShutdownRequested)
    {
        return eFOTA_DOWNLOAD_SHUTDOWN;
    }
    if (sb_FOTA_DownloadExpired(pt_Context))
    {
        return eFOTA_DOWNLOAD_EXPIRED;
    }
    if (pt_Context->u32_FileSize >= pt_Context->st_Package.u32_PackageSize)
    {
        return eFOTA_DOWNLOAD_COMPLETE;
    }
    u32_Remaining = pt_Context->st_Package.u32_PackageSize -
                    pt_Context->u32_FileSize;
    u32_Length = (u32_Remaining > FOTA_HTTP_CHUNK_BYTES) ?
                 FOTA_HTTP_CHUNK_BYTES : u32_Remaining;
    pt_Context->u32_PendingStart = pt_Context->u32_FileSize;
    pt_Context->u32_PendingEnd = pt_Context->u32_FileSize + u32_Length - 1U;
    if (pt_Context->u8_RangeAttempts >= FOTA_RETRY_ATTEMPTS)
    {
        return eFOTA_DOWNLOAD_FAILED;
    }
    pt_Context->u8_RangeAttempts++;
    if (pt_Context->cpt_Ops->https_get_range(
            pt_Context->st_Package.url,
            pt_Context->u32_PendingStart,
            pt_Context->u32_PendingEnd,
            pt_Context->pu8_Buffer,
            FOTA_HTTP_CHUNK_BYTES))
    {
        pt_Context->b_Pending = true;
        return eFOTA_DOWNLOAD_WAITING_HTTP;
    }
    if (pt_Context->u8_RangeAttempts >= FOTA_RETRY_ATTEMPTS)
    {
        return eFOTA_DOWNLOAD_FAILED;
    }
    return eFOTA_DOWNLOAD_PROGRESS;
}

FOTADownloadResult_E ge_FOTA_DownloadHandleRange(
    FOTADownloadContext_T *pt_Context,
    const FOTAHttpRangeResult_T *cpt_Result,
    FOTADownloadCheckpointFn fpt_Checkpoint,
    void *pv_User)
{
    uint32_t u32_Length;
    if ((pt_Context == NULL) || (cpt_Result == NULL) ||
        !pt_Context->b_Pending)
    {
        return eFOTA_DOWNLOAD_FAILED;
    }
    pt_Context->b_Pending = false;
    if (sb_FOTA_DownloadExpired(pt_Context))
    {
        return eFOTA_DOWNLOAD_EXPIRED;
    }
    u32_Length = pt_Context->u32_PendingEnd -
                 pt_Context->u32_PendingStart + 1U;
    if ((cpt_Result->e_Result != eFOTA_HTTP_RESULT_OK) ||
        (cpt_Result->u16_HttpStatus != 206U) ||
        (cpt_Result->u32_Offset != pt_Context->u32_PendingStart) ||
        (cpt_Result->u32_Length != u32_Length) ||
        (cpt_Result->u32_ContentRangeStart != pt_Context->u32_PendingStart) ||
        (cpt_Result->u32_ContentRangeEnd != pt_Context->u32_PendingEnd) ||
        (cpt_Result->u32_ContentRangeTotal != pt_Context->st_Package.u32_PackageSize) ||
        (cpt_Result->u32_ActualLength != u32_Length))
    {
        if (pt_Context->b_ShutdownRequested ||
            (pt_Context->u8_RangeAttempts >= FOTA_RETRY_ATTEMPTS))
        {
            return pt_Context->b_ShutdownRequested ?
                   eFOTA_DOWNLOAD_SHUTDOWN : eFOTA_DOWNLOAD_FAILED;
        }
        return eFOTA_DOWNLOAD_PROGRESS;
    }
    if (!sb_FOTA_DownloadWrite(pt_Context, u32_Length))
    {
        return eFOTA_DOWNLOAD_FAILED;
    }
    pt_Context->u32_FileSize = pt_Context->cpt_Ops->storage_size(
        eFOTA_STORAGE_CURRENT_FILE);
    if (pt_Context->u32_FileSize !=
        (pt_Context->u32_PendingStart + u32_Length))
    {
        return eFOTA_DOWNLOAD_FAILED;
    }
    pt_Context->u8_RangeAttempts = 0U;

    while ((pt_Context->u8_NextCheckpoint <= 100U) &&
           ((uint64_t)pt_Context->u32_FileSize * 100U >=
            ((uint64_t)pt_Context->st_Package.u32_PackageSize *
             pt_Context->u8_NextCheckpoint)))
    {
        if ((fpt_Checkpoint != NULL) &&
            !fpt_Checkpoint(pt_Context->u8_NextCheckpoint,
                            pt_Context->u32_FileSize,
                            pv_User))
        {
            return eFOTA_DOWNLOAD_FAILED;
        }
        if (pt_Context->u8_NextCheckpoint == 100U)
        {
            break;
        }
        pt_Context->u8_NextCheckpoint =
            (uint8_t)(pt_Context->u8_NextCheckpoint + 25U);
    }
    if (pt_Context->b_ShutdownRequested)
    {
        return eFOTA_DOWNLOAD_SHUTDOWN;
    }
    if (pt_Context->u32_FileSize == pt_Context->st_Package.u32_PackageSize)
    {
        return eFOTA_DOWNLOAD_COMPLETE;
    }
    return eFOTA_DOWNLOAD_PROGRESS;
}

void gv_FOTA_DownloadRequestShutdown(FOTADownloadContext_T *pt_Context)
{
    if (pt_Context != NULL)
    {
        pt_Context->b_ShutdownRequested = true;
    }
}

bool gb_FOTA_DownloadIsComplete(const FOTADownloadContext_T *cpt_Context)
{
    return (cpt_Context != NULL) &&
           (cpt_Context->u32_FileSize == cpt_Context->st_Package.u32_PackageSize);
}

uint32_t gu32_FOTA_DownloadBytes(const FOTADownloadContext_T *cpt_Context)
{
    return (cpt_Context != NULL) ? cpt_Context->u32_FileSize : 0U;
}
