#ifndef FOTA_DOWNLOAD_SM_H
#define FOTA_DOWNLOAD_SM_H

#include "FOTATypes.h"

typedef enum
{
    eFOTA_DOWNLOAD_READY = 0,
    eFOTA_DOWNLOAD_WAITING_HTTP,
    eFOTA_DOWNLOAD_PROGRESS,
    eFOTA_DOWNLOAD_COMPLETE,
    eFOTA_DOWNLOAD_EXPIRED,
    eFOTA_DOWNLOAD_FAILED,
    eFOTA_DOWNLOAD_SHUTDOWN
} FOTADownloadResult_E;

typedef struct
{
    const FOTAPlatformOps_T *cpt_Ops;
    FOTAPackage_T st_Package;
    uint8_t *pu8_Buffer;
    uint32_t u32_FileSize;
    uint32_t u32_PendingStart;
    uint32_t u32_PendingEnd;
    uint8_t u8_RangeAttempts;
    uint8_t u8_NextCheckpoint;
    bool b_Pending;
    bool b_ShutdownRequested;
} FOTADownloadContext_T;

typedef bool (*FOTADownloadCheckpointFn)(uint8_t u8_Percentage,
                                         uint32_t u32_Bytes,
                                         void *pv_User);

FOTADownloadResult_E ge_FOTA_DownloadBegin(
    FOTADownloadContext_T *pt_Context,
    const FOTAPlatformOps_T *cpt_Ops,
    const FOTAPackage_T *cpt_Package,
    uint32_t u32_ExistingSize,
    uint8_t *pu8_Buffer);

FOTADownloadResult_E ge_FOTA_DownloadIssueRange(
    FOTADownloadContext_T *pt_Context);

FOTADownloadResult_E ge_FOTA_DownloadHandleRange(
    FOTADownloadContext_T *pt_Context,
    const FOTAHttpRangeResult_T *cpt_Result,
    FOTADownloadCheckpointFn fpt_Checkpoint,
    void *pv_User);

void gv_FOTA_DownloadRequestShutdown(FOTADownloadContext_T *pt_Context);
bool gb_FOTA_DownloadIsComplete(const FOTADownloadContext_T *cpt_Context);
uint32_t gu32_FOTA_DownloadBytes(const FOTADownloadContext_T *cpt_Context);

#endif /* FOTA_DOWNLOAD_SM_H */
