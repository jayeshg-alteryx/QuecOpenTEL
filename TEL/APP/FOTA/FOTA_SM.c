#include "FOTA_SM.h"

#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

#include "AppMonitor.h"
#include "cJSON.h"
#include "FS_Manager.h"

typedef struct
{
    uint32_t u32_Magic;
    uint16_t u16_Version;
    uint16_t u16_Length;
    uint32_t u32_CalendarDay;
    uint8_t u8_PollCount;
    uint8_t au8_Reserved[3];
    uint64_t u64_LastPollEpoch;
    uint32_t u32_Crc;
} FOTAPollRecord_T;

static bool sb_FOTA_FinishVerification(FOTASMContext_T *pt_Context);
static bool sb_FOTA_ResumeRecoveredState(FOTASMContext_T *pt_Context);
static bool sb_FOTA_SelectPackage(FOTASMContext_T *pt_Context);
static FOTAStringView_T st_FOTA_JobIdView(const FOTASMContext_T *cpt_Context);

static bool sb_FOTA_VerifyStored(const FOTASMContext_T *cpt_Context,
                                 FOTAStorageFile_E e_File,
                                 const void *cvp_Data,
                                 uint32_t u32_Length)
{
    const uint8_t *cu8p_Data = (const uint8_t *)cvp_Data;
    uint8_t *pu8_Verify;
    uint32_t u32_Offset = 0U;

    if ((cpt_Context == NULL) || (cvp_Data == NULL) ||
        (cpt_Context->st_Ops.storage_read == NULL) ||
        (cpt_Context->st_Ops.storage_size == NULL) ||
        (cpt_Context->st_Ops.storage_size(e_File) != u32_Length))
    {
        return false;
    }
    pu8_Verify = (uint8_t *)cpt_Context->au8_HttpBuffer;
    while (u32_Offset < u32_Length)
    {
        uint32_t u32_Chunk = u32_Length - u32_Offset;
        uint32_t u32_Actual = 0U;
        if (u32_Chunk > FOTA_HTTP_CHUNK_BYTES)
        {
            u32_Chunk = FOTA_HTTP_CHUNK_BYTES;
        }
        if (!cpt_Context->st_Ops.storage_read(e_File, u32_Offset,
                                               pu8_Verify, u32_Chunk,
                                               &u32_Actual) ||
            (u32_Actual != u32_Chunk) ||
            (memcmp(pu8_Verify, cu8p_Data + u32_Offset, u32_Chunk) != 0))
        {
            return false;
        }
        u32_Offset += u32_Chunk;
    }
    return true;
}

static uint32_t gu32_FOTA_Crc32(const void *cvp_Data, uint32_t u32_Length)
{
    const uint8_t *cu8p_Data = (const uint8_t *)cvp_Data;
    uint32_t u32_Crc = 0xFFFFFFFFUL;
    uint32_t u32_Index;
    uint8_t u8_Bit;
    for (u32_Index = 0U; u32_Index < u32_Length; u32_Index++)
    {
        u32_Crc ^= cu8p_Data[u32_Index];
        for (u8_Bit = 0U; u8_Bit < 8U; u8_Bit++)
        {
            u32_Crc = (u32_Crc & 1U) ?
                      ((u32_Crc >> 1U) ^ 0xEDB88320UL) :
                      (u32_Crc >> 1U);
        }
    }
    return ~u32_Crc;
}

static bool sb_FOTA_ViewEqual(FOTAStringView_T st_Left,
                              FOTAStringView_T st_Right)
{
    return (st_Left.u16_Length == st_Right.u16_Length) &&
           (st_Left.u16_Length == 0U ||
            memcmp(st_Left.pc_Data, st_Right.pc_Data, st_Left.u16_Length) == 0);
}

static bool sb_FOTA_CopyView(char *pc_Destination,
                             uint16_t u16_Capacity,
                             FOTAStringView_T st_Source,
                             uint16_t *pu16_Length)
{
    if ((pc_Destination == NULL) || (pu16_Length == NULL) ||
        (st_Source.pc_Data == NULL) ||
        (st_Source.u16_Length >= u16_Capacity))
    {
        return false;
    }
    memcpy(pc_Destination, st_Source.pc_Data, st_Source.u16_Length);
    pc_Destination[st_Source.u16_Length] = '\0';
    *pu16_Length = st_Source.u16_Length;
    return true;
}

static bool sb_FOTA_GetTime(const FOTASMContext_T *cpt_Context,
                            uint64_t *pu64_Epoch,
                            uint32_t *pu32_Day)
{
    if ((cpt_Context == NULL) || (cpt_Context->st_Ops.get_time == NULL) ||
        (pu64_Epoch == NULL) || (pu32_Day == NULL))
    {
        return false;
    }
    return cpt_Context->st_Ops.get_time(pu64_Epoch, pu32_Day);
}

static FileSystemID_E se_FOTA_MapStorage(FOTAStorageFile_E e_File)
{
    switch (e_File)
    {
        case eFOTA_STORAGE_JOB_DOCUMENT_A: return eFS_FILE_FOTA_JOB_DOCUMENT_A;
        case eFOTA_STORAGE_JOB_DOCUMENT_B: return eFS_FILE_FOTA_JOB_DOCUMENT_B;
        case eFOTA_STORAGE_CONTEXT_A:      return eFS_FILE_FOTA_CONTEXT_A;
        case eFOTA_STORAGE_CONTEXT_B:      return eFS_FILE_FOTA_CONTEXT_B;
        case eFOTA_STORAGE_RESUME:         return eFS_FILE_FOTA_RESUME;
        case eFOTA_STORAGE_POLL_BUDGET:   return eFS_FILE_FOTA_POLL_BUDGET;
        case eFOTA_STORAGE_CURRENT_FILE:  return eFS_FILE_FOTA_FILE;
        case eFOTA_STORAGE_JOB_DOCUMENT:  return eFS_FILE_FOTA_JOB_DOCUMENT_A;
        default:                           return eFS_FILE_MAX;
    }
}

static bool sb_FOTA_DefaultStorageRead(FOTAStorageFile_E e_File,
                                       uint32_t u32_Offset,
                                       void *pv_Buffer,
                                       uint32_t u32_Length,
                                       uint32_t *pu32_Actual)
{
    FileSystemReq_T st_Request = {0};
    if (se_FOTA_MapStorage(e_File) == eFS_FILE_MAX)
    {
        return false;
    }
    st_Request.e_FileId = se_FOTA_MapStorage(e_File);
    st_Request.u32_Offset = u32_Offset;
    st_Request.vpt_Buffer = pv_Buffer;
    st_Request.u32_RequestedBytes = u32_Length;
    if ((ge_FS_Read(&st_Request) != eFS_OK) ||
        (st_Request.u32_ActualBytes != u32_Length))
    {
        return false;
    }
    if (pu32_Actual != NULL)
    {
        *pu32_Actual = st_Request.u32_ActualBytes;
    }
    return true;
}

static bool sb_FOTA_DefaultStorageWrite(FOTAStorageFile_E e_File,
                                         uint32_t u32_Offset,
                                         const void *cvp_Buffer,
                                         uint32_t u32_Length)
{
    FileSystemID_E e_FileId = se_FOTA_MapStorage(e_File);
    FileSystemReq_T st_Request = {0};
    if (e_FileId == eFS_FILE_MAX)
    {
        return false;
    }
    if (ge_FS_IsFilePresent(e_FileId) != eFS_OK)
    {
        if (ge_FS_Create(e_FileId) != eFS_OK)
        {
            return false;
        }
    }
    st_Request.e_FileId = e_FileId;
    st_Request.u32_Offset = u32_Offset;
    st_Request.vpt_Buffer = (void *)cvp_Buffer;
    st_Request.u32_RequestedBytes = u32_Length;
    return (ge_FS_Write(&st_Request) == eFS_OK) &&
           (st_Request.u32_ActualBytes == u32_Length);
}

static bool sb_FOTA_DefaultStorageDelete(FOTAStorageFile_E e_File)
{
    FileSystemID_E e_FileId = se_FOTA_MapStorage(e_File);
    if (e_FileId == eFS_FILE_MAX)
    {
        return false;
    }
    if (ge_FS_IsFilePresent(e_FileId) != eFS_OK)
    {
        return true;
    }
    return ge_FS_Delete(e_FileId) == eFS_OK;
}

static bool sb_FOTA_DefaultStorageExists(FOTAStorageFile_E e_File)
{
    FileSystemID_E e_FileId = se_FOTA_MapStorage(e_File);
    return (e_FileId != eFS_FILE_MAX) &&
           (ge_FS_IsFilePresent(e_FileId) == eFS_OK);
}

static uint32_t gu32_FOTA_DefaultStorageSize(FOTAStorageFile_E e_File)
{
    FileSystemID_E e_FileId = se_FOTA_MapStorage(e_File);
    return (e_FileId == eFS_FILE_MAX) ? 0U : ge_FS_GetFileSize(e_FileId);
}

static uint32_t gu32_FOTA_DefaultStorageFree(void)
{
    return UINT32_MAX;
}

static bool sb_FOTA_DefaultMonitorAcquire(void)
{
    return ge_AppMonitor_RunApplication(eAPPLICATION_MODEM_FOTA) == eTEL_OK;
}

static void gv_FOTA_DefaultMonitorRelease(void)
{
    (void)ge_AppMonitor_StopCurrentRunningApp(eAPPLICATION_MODEM_FOTA);
}

static bool sb_FOTA_DefaultInstaller(FOTAStringView_T job_id,
                                     const FOTAPackage_T *cpt_Package,
                                     FOTAStorageFile_E e_File)
{
    BAL_UNUSED(job_id);
    BAL_UNUSED(cpt_Package);
    BAL_UNUSED(e_File);
    return true;
}

static void gv_FOTA_FillDefaultOps(FOTAPlatformOps_T *pt_Ops)
{
    if (pt_Ops->app_monitor_acquire == NULL)
    {
        pt_Ops->app_monitor_acquire = sb_FOTA_DefaultMonitorAcquire;
    }
    if (pt_Ops->app_monitor_release == NULL)
    {
        pt_Ops->app_monitor_release = gv_FOTA_DefaultMonitorRelease;
    }
    if (pt_Ops->storage_read == NULL)
    {
        pt_Ops->storage_read = sb_FOTA_DefaultStorageRead;
    }
    if (pt_Ops->storage_write == NULL)
    {
        pt_Ops->storage_write = sb_FOTA_DefaultStorageWrite;
    }
    if (pt_Ops->storage_delete == NULL)
    {
        pt_Ops->storage_delete = sb_FOTA_DefaultStorageDelete;
    }
    if (pt_Ops->storage_exists == NULL)
    {
        pt_Ops->storage_exists = sb_FOTA_DefaultStorageExists;
    }
    if (pt_Ops->storage_size == NULL)
    {
        pt_Ops->storage_size = gu32_FOTA_DefaultStorageSize;
    }
    if (pt_Ops->storage_free_bytes == NULL)
    {
        pt_Ops->storage_free_bytes = gu32_FOTA_DefaultStorageFree;
    }
    if (pt_Ops->installer_handoff == NULL)
    {
        pt_Ops->installer_handoff = sb_FOTA_DefaultInstaller;
    }
}

static bool sb_FOTA_WriteRetries(const FOTASMContext_T *cpt_Context,
                                 FOTAStorageFile_E e_File,
                                 const void *cvp_Buffer,
                                 uint32_t u32_Length)
{
    uint8_t u8_Attempt;
    for (u8_Attempt = 0U; u8_Attempt < FOTA_RETRY_ATTEMPTS; u8_Attempt++)
    {
        if (cpt_Context->st_Ops.storage_write(e_File, 0U,
                                              cvp_Buffer, u32_Length) &&
            sb_FOTA_VerifyStored(cpt_Context, e_File, cvp_Buffer,
                                 u32_Length))
        {
            return true;
        }
    }
    return false;
}

static bool sb_FOTA_DeleteRetries(const FOTASMContext_T *cpt_Context,
                                  FOTAStorageFile_E e_File)
{
    uint8_t u8_Attempt;
    for (u8_Attempt = 0U; u8_Attempt < FOTA_RETRY_ATTEMPTS; u8_Attempt++)
    {
        if (cpt_Context->st_Ops.storage_delete(e_File))
        {
            return true;
        }
    }
    return false;
}

static bool sb_FOTA_PersistPoll(FOTASMContext_T *pt_Context)
{
    FOTAPollRecord_T st_Record = {0};
    st_Record.u32_Magic = FOTA_CONTEXT_MAGIC;
    st_Record.u16_Version = FOTA_CONTEXT_VERSION;
    st_Record.u16_Length = (uint16_t)sizeof(st_Record);
    st_Record.u32_CalendarDay = pt_Context->st_Status.u32_PollCalendarDay;
    st_Record.u8_PollCount = pt_Context->st_Status.u8_PollCount;
    st_Record.u64_LastPollEpoch = pt_Context->st_Status.u64_LastPollEpoch;
    st_Record.u32_Crc = 0U;
    st_Record.u32_Crc = gu32_FOTA_Crc32(&st_Record, sizeof(st_Record));
    return sb_FOTA_WriteRetries(pt_Context, eFOTA_STORAGE_POLL_BUDGET,
                                &st_Record, sizeof(st_Record));
}

static bool sb_FOTA_PersistResume(FOTASMContext_T *pt_Context,
                                  uint32_t u32_Bytes)
{
    FOTAResumeRecord_T st_Record = {0};
    uint16_t u16_PackageIdLength = pt_Context->st_Job.u8_PackageCount > 0U ?
        pt_Context->st_Job.at_Packages[pt_Context->u8_CurrentPackage].package_id.u16_Length : 0U;
    if (u16_PackageIdLength >= FOTA_MAX_JOB_ID_BYTES)
    {
        return false;
    }
    st_Record.u32_Magic = FOTA_CONTEXT_MAGIC;
    st_Record.u16_Version = FOTA_CONTEXT_VERSION;
    st_Record.u16_Length = (uint16_t)sizeof(st_Record);
    st_Record.u32_Generation = ++pt_Context->u32_RecoveryGeneration;
    st_Record.u32_DownloadedBytes = u32_Bytes;
    st_Record.u32_ExpectedSize = pt_Context->st_Download.st_Package.u32_PackageSize;
    st_Record.u32_SequenceId = pt_Context->st_Download.st_Package.u32_SequenceId;
    st_Record.u16_PackageIdLength = u16_PackageIdLength;
    memcpy(st_Record.ac_PackageId,
           pt_Context->st_Download.st_Package.package_id.pc_Data,
           u16_PackageIdLength);
    st_Record.u32_Crc = 0U;
    st_Record.u32_Crc = gu32_FOTA_Crc32(&st_Record, sizeof(st_Record));
    return sb_FOTA_WriteRetries(pt_Context, eFOTA_STORAGE_RESUME,
                                &st_Record, sizeof(st_Record));
}

static bool sb_FOTA_PersistContext(FOTASMContext_T *pt_Context)
{
    FOTARecoveryRecord_T st_Record = {0};
    FOTAStorageFile_E e_Slot;
    if (pt_Context->u16_JobIdLength >= FOTA_MAX_JOB_ID_BYTES)
    {
        return false;
    }
    st_Record.u32_Magic = FOTA_CONTEXT_MAGIC;
    st_Record.u16_Version = FOTA_CONTEXT_VERSION;
    st_Record.u16_Length = (uint16_t)sizeof(st_Record);
    st_Record.u32_Generation = ++pt_Context->u32_RecoveryGeneration;
    st_Record.u8_State = (uint8_t)pt_Context->e_State;
    st_Record.u8_CurrentPackage = pt_Context->u8_CurrentPackage;
    st_Record.u8_DocumentSlot = pt_Context->u8_DocumentSlot;
    st_Record.u32_DownloadedBytes = gu32_FOTA_DownloadBytes(&pt_Context->st_Download);
    st_Record.u32_PackageSize = (pt_Context->u8_CurrentPackage <
                                 pt_Context->st_Job.u8_PackageCount) ?
                                pt_Context->st_Job.at_Packages[
                                    pt_Context->u8_CurrentPackage].u32_PackageSize : 0U;
    st_Record.u16_JobIdLength = pt_Context->u16_JobIdLength;
    memcpy(st_Record.ac_JobId, pt_Context->ac_JobId,
           pt_Context->u16_JobIdLength);
    st_Record.u32_Crc = 0U;
    st_Record.u32_Crc = gu32_FOTA_Crc32(&st_Record, sizeof(st_Record));
    e_Slot = (st_Record.u32_Generation & 1U) ?
             eFOTA_STORAGE_CONTEXT_B : eFOTA_STORAGE_CONTEXT_A;
    return sb_FOTA_WriteRetries(pt_Context, e_Slot,
                                &st_Record, sizeof(st_Record));
}

static bool sb_FOTA_PersistDocument(FOTASMContext_T *pt_Context)
{
    FOTAStorageFile_E e_Slot = (pt_Context->u8_DocumentSlot == 0U) ?
                               eFOTA_STORAGE_JOB_DOCUMENT_B :
                               eFOTA_STORAGE_JOB_DOCUMENT_A;
    uint8_t u8_Attempt;
    if ((pt_Context->u32_JobDocumentLength == 0U) ||
        (pt_Context->u32_JobDocumentLength > FOTA_MAX_JOB_DOCUMENT_BYTES))
    {
        return false;
    }
    (void)sb_FOTA_DeleteRetries(pt_Context, e_Slot);
    for (u8_Attempt = 0U; u8_Attempt < FOTA_RETRY_ATTEMPTS; u8_Attempt++)
    {
        if (pt_Context->st_Ops.storage_write(e_Slot, 0U,
                                              pt_Context->ac_JobDocument,
                                              pt_Context->u32_JobDocumentLength) &&
            sb_FOTA_VerifyStored(pt_Context, e_Slot,
                                 pt_Context->ac_JobDocument,
                                 pt_Context->u32_JobDocumentLength))
        {
            pt_Context->u8_DocumentSlot = (e_Slot ==
                                           eFOTA_STORAGE_JOB_DOCUMENT_A) ? 0U : 1U;
            return true;
        }
    }
    return false;
}

static bool sb_FOTA_ReadPollRecord(FOTASMContext_T *pt_Context,
                                   FOTAPollRecord_T *pt_Record)
{
    uint32_t u32_Actual = 0U;
    uint32_t u32_Crc;
    if ((pt_Context == NULL) || (pt_Record == NULL) ||
        !pt_Context->st_Ops.storage_exists(eFOTA_STORAGE_POLL_BUDGET) ||
        (pt_Context->st_Ops.storage_size(eFOTA_STORAGE_POLL_BUDGET) <
         sizeof(*pt_Record)) ||
        !pt_Context->st_Ops.storage_read(eFOTA_STORAGE_POLL_BUDGET, 0U,
                                         pt_Record, sizeof(*pt_Record),
                                         &u32_Actual) ||
        (u32_Actual != sizeof(*pt_Record)) ||
        (pt_Record->u32_Magic != FOTA_CONTEXT_MAGIC) ||
        (pt_Record->u16_Version != FOTA_CONTEXT_VERSION) ||
        (pt_Record->u16_Length != sizeof(*pt_Record)) ||
        (pt_Record->u8_PollCount > FOTA_DAILY_POLL_LIMIT))
    {
        return false;
    }
    u32_Crc = pt_Record->u32_Crc;
    pt_Record->u32_Crc = 0U;
    if (gu32_FOTA_Crc32(pt_Record, sizeof(*pt_Record)) != u32_Crc)
    {
        pt_Record->u32_Crc = u32_Crc;
        return false;
    }
    pt_Record->u32_Crc = u32_Crc;
    return true;
}

static bool sb_FOTA_ReadResumeRecord(FOTASMContext_T *pt_Context,
                                     FOTAResumeRecord_T *pt_Record)
{
    uint32_t u32_Actual = 0U;
    uint32_t u32_Crc;
    if ((pt_Context == NULL) || (pt_Record == NULL) ||
        !pt_Context->st_Ops.storage_exists(eFOTA_STORAGE_RESUME) ||
        (pt_Context->st_Ops.storage_size(eFOTA_STORAGE_RESUME) <
         sizeof(*pt_Record)) ||
        !pt_Context->st_Ops.storage_read(eFOTA_STORAGE_RESUME, 0U,
                                         pt_Record, sizeof(*pt_Record),
                                         &u32_Actual) ||
        (u32_Actual != sizeof(*pt_Record)) ||
        (pt_Record->u32_Magic != FOTA_CONTEXT_MAGIC) ||
        (pt_Record->u16_Version != FOTA_CONTEXT_VERSION) ||
        (pt_Record->u16_Length != sizeof(*pt_Record)) ||
        (pt_Record->u16_PackageIdLength >= FOTA_MAX_JOB_ID_BYTES))
    {
        return false;
    }
    u32_Crc = pt_Record->u32_Crc;
    pt_Record->u32_Crc = 0U;
    if (gu32_FOTA_Crc32(pt_Record, sizeof(*pt_Record)) != u32_Crc)
    {
        pt_Record->u32_Crc = u32_Crc;
        return false;
    }
    pt_Record->u32_Crc = u32_Crc;
    return true;
}

static bool sb_FOTA_ReadRecord(FOTASMContext_T *pt_Context,
                               FOTAStorageFile_E e_File,
                               FOTARecoveryRecord_T *pt_Record)
{
    uint32_t u32_Actual = 0U;
    uint32_t u32_Crc;
    if ((pt_Context->st_Ops.storage_read == NULL) ||
        !pt_Context->st_Ops.storage_exists(e_File) ||
        (pt_Context->st_Ops.storage_size(e_File) < sizeof(*pt_Record)) ||
        !pt_Context->st_Ops.storage_read(e_File, 0U, pt_Record,
                                         sizeof(*pt_Record), &u32_Actual) ||
        (u32_Actual != sizeof(*pt_Record)) ||
        (pt_Record->u32_Magic != FOTA_CONTEXT_MAGIC) ||
        (pt_Record->u16_Version != FOTA_CONTEXT_VERSION) ||
        (pt_Record->u16_Length != sizeof(*pt_Record)))
    {
        return false;
    }
    u32_Crc = pt_Record->u32_Crc;
    pt_Record->u32_Crc = 0U;
    if (gu32_FOTA_Crc32(pt_Record, sizeof(*pt_Record)) != u32_Crc)
    {
        pt_Record->u32_Crc = u32_Crc;
        return false;
    }
    pt_Record->u32_Crc = u32_Crc;
    return true;
}

static bool sb_FOTA_CopyJobIdFromEvent(FOTASMContext_T *pt_Context,
                                       const FOTAEvent_T *cpt_Event)
{
    if ((cpt_Event->job_id.pc_Data != NULL) &&
        (cpt_Event->job_id.u16_Length > 0U))
    {
        return sb_FOTA_CopyView(pt_Context->ac_JobId,
                                sizeof(pt_Context->ac_JobId),
                                cpt_Event->job_id,
                                &pt_Context->u16_JobIdLength);
    }
    if ((cpt_Event->pc_Payload != NULL) &&
        (cpt_Event->u32_PayloadLength > 0U) &&
        (cpt_Event->u32_PayloadLength <= FOTA_MAX_JOB_DOCUMENT_BYTES))
    {
        cJSON *pt_Root;
        cJSON *pt_Execution;
        cJSON *pt_JobId;
        const char *pc_ParseEnd = NULL;
        const char *pc_End = cpt_Event->pc_Payload +
                             cpt_Event->u32_PayloadLength;
        pt_Root = cJSON_ParseWithLengthOpts(cpt_Event->pc_Payload,
                                            cpt_Event->u32_PayloadLength,
                                            &pc_ParseEnd, false);
        if ((pt_Root == NULL) || (pc_ParseEnd == NULL))
        {
            cJSON_Delete(pt_Root);
            return false;
        }
        if (!cJSON_IsObject(pt_Root))
        {
            cJSON_Delete(pt_Root);
            return false;
        }
        while ((pc_ParseEnd < pc_End) &&
               (isspace((unsigned char)*pc_ParseEnd) != 0))
        {
            pc_ParseEnd++;
        }
        pt_Execution = cJSON_GetObjectItemCaseSensitive(pt_Root,
                                                         "execution");
        pt_JobId = cJSON_GetObjectItemCaseSensitive(pt_Root, "jobId");
        if (cJSON_IsObject(pt_Execution) && (pt_JobId == NULL))
        {
            pt_JobId = cJSON_GetObjectItemCaseSensitive(pt_Execution,
                                                         "jobId");
        }
        if ((pc_ParseEnd == pc_End) && cJSON_IsString(pt_JobId) &&
            (pt_JobId->valuestring != NULL) &&
            (strlen(pt_JobId->valuestring) < sizeof(pt_Context->ac_JobId)))
        {
            (void)sb_FOTA_CopyView(
                pt_Context->ac_JobId,
                sizeof(pt_Context->ac_JobId),
                (FOTAStringView_T){pt_JobId->valuestring,
                                   (uint16_t)strlen(pt_JobId->valuestring)},
                &pt_Context->u16_JobIdLength);
            cJSON_Delete(pt_Root);
            return true;
        }
        cJSON_Delete(pt_Root);
    }
    return false;
}

static bool sb_FOTA_EventJsonValid(const FOTAEvent_T *cpt_Event)
{
    cJSON *pt_Root;
    const char *pc_ParseEnd = NULL;
    const char *pc_End;
    if ((cpt_Event == NULL) || (cpt_Event->pc_Payload == NULL) ||
        (cpt_Event->u32_PayloadLength == 0U) ||
        (cpt_Event->u32_PayloadLength > FOTA_MAX_JOB_DOCUMENT_BYTES))
    {
        return false;
    }
    pc_End = cpt_Event->pc_Payload + cpt_Event->u32_PayloadLength;
    pt_Root = cJSON_ParseWithLengthOpts(cpt_Event->pc_Payload,
                                        cpt_Event->u32_PayloadLength,
                                        &pc_ParseEnd, false);
    if ((pt_Root == NULL) || (pc_ParseEnd == NULL))
    {
        cJSON_Delete(pt_Root);
        return false;
    }
    while ((pc_ParseEnd < pc_End) &&
           (isspace((unsigned char)*pc_ParseEnd) != 0))
    {
        pc_ParseEnd++;
    }
    if ((pc_ParseEnd != pc_End) || !cJSON_IsObject(pt_Root))
    {
        cJSON_Delete(pt_Root);
        return false;
    }
    cJSON_Delete(pt_Root);
    return true;
}

static bool sb_FOTA_EventIsNoJob(const FOTAEvent_T *cpt_Event)
{
    cJSON *pt_Root;
    cJSON *pt_Execution;
    bool b_NoJob;
    const char *pc_ParseEnd = NULL;
    const char *pc_End;
    if ((cpt_Event == NULL) || (cpt_Event->pc_Payload == NULL) ||
        (cpt_Event->u32_PayloadLength == 0U) ||
        (cpt_Event->u32_PayloadLength > FOTA_MAX_JOB_DOCUMENT_BYTES))
    {
        return true;
    }
    pc_End = cpt_Event->pc_Payload + cpt_Event->u32_PayloadLength;
    pt_Root = cJSON_ParseWithLengthOpts(cpt_Event->pc_Payload,
                                        cpt_Event->u32_PayloadLength,
                                        &pc_ParseEnd, false);
    if ((pt_Root == NULL) || (pc_ParseEnd == NULL))
    {
        cJSON_Delete(pt_Root);
        return false;
    }
    while ((pc_ParseEnd < pc_End) &&
           (isspace((unsigned char)*pc_ParseEnd) != 0))
    {
        pc_ParseEnd++;
    }
    if ((pc_ParseEnd != pc_End) || !cJSON_IsObject(pt_Root))
    {
        cJSON_Delete(pt_Root);
        return false;
    }
    pt_Execution = cJSON_GetObjectItemCaseSensitive(pt_Root, "execution");
    b_NoJob = cJSON_IsNull(pt_Execution);
    cJSON_Delete(pt_Root);
    return b_NoJob;
}

static bool sb_FOTA_IsAuthoritativeStop(FOTAError_E e_Error)
{
    return (e_Error == eFOTA_ERROR_JOB_CANCELED) ||
           (e_Error == eFOTA_ERROR_JOB_NOT_FOUND) ||
           (e_Error == eFOTA_ERROR_JOB_INVALID_STATE);
}

static bool sb_FOTA_EventJobMatches(const FOTASMContext_T *cpt_Context,
                                    const FOTAEvent_T *cpt_Event)
{
    return (cpt_Event->job_id.pc_Data == NULL) ||
           sb_FOTA_ViewEqual(cpt_Event->job_id,
                             st_FOTA_JobIdView(cpt_Context));
}

static FOTAStringView_T st_FOTA_JobIdView(const FOTASMContext_T *cpt_Context)
{
    FOTAStringView_T st_View = { cpt_Context->ac_JobId,
                                 cpt_Context->u16_JobIdLength };
    return st_View;
}

static bool sb_FOTA_PublishStatus(FOTASMContext_T *pt_Context,
                                  const char *ccpt_Status,
                                  const char *ccpt_Step,
                                  const FOTAPackage_T *cpt_Package,
                                  FOTAError_E e_Error,
                                  uint8_t u8_Percentage)
{
    char ac_Payload[FOTA_STATUS_BUFFER_BYTES];
    char ac_Sequence[11];
    char ac_Percentage[4];
    char ac_Error[11];
    cJSON *pt_Root;
    cJSON *pt_Details;
    int i32_Length;
    uint8_t u8_Attempt;
    if (pt_Context->st_Ops.mqtt_publish_status == NULL)
    {
        return true;
    }
    pt_Root = cJSON_CreateObject();
    pt_Details = cJSON_CreateObject();
    if ((pt_Root == NULL) || (pt_Details == NULL))
    {
        cJSON_Delete(pt_Root);
        cJSON_Delete(pt_Details);
        return false;
    }
    cJSON_AddItemToObject(pt_Root, "statusDetails", pt_Details);
    if ((cJSON_AddStringToObject(pt_Root, "status", ccpt_Status) == NULL) ||
        (cJSON_AddStringToObject(pt_Details, "step", ccpt_Step) == NULL))
    {
        cJSON_Delete(pt_Root);
        return false;
    }
    (void)snprintf(ac_Error, sizeof(ac_Error), "%u", (unsigned)e_Error);
    if (cJSON_AddStringToObject(pt_Details, "error_code", ac_Error) == NULL)
    {
        cJSON_Delete(pt_Root);
        return false;
    }
    if (cpt_Package != NULL)
    {
        (void)snprintf(ac_Sequence, sizeof(ac_Sequence), "%lu",
                       (unsigned long)cpt_Package->u32_SequenceId);
        (void)snprintf(ac_Percentage, sizeof(ac_Percentage), "%u",
                       (unsigned)u8_Percentage);
        if ((cJSON_AddStringToObject(pt_Details, "ecu_type",
                                     cpt_Package->ecu_type.pc_Data) == NULL) ||
            (cJSON_AddStringToObject(pt_Details, "ecu_id",
                                     cpt_Package->ecu_id.pc_Data) == NULL) ||
            (cJSON_AddStringToObject(pt_Details, "package_id",
                                     cpt_Package->package_id.pc_Data) == NULL) ||
            (cJSON_AddStringToObject(pt_Details, "seq_id", ac_Sequence) == NULL) ||
            (cJSON_AddStringToObject(pt_Details, "percentage",
                                     ac_Percentage) == NULL))
        {
            cJSON_Delete(pt_Root);
            return false;
        }
    }
    if (!cJSON_PrintPreallocated(pt_Root, ac_Payload, sizeof(ac_Payload),
                                 false))
    {
        cJSON_Delete(pt_Root);
        return false;
    }
    i32_Length = (int)strlen(ac_Payload);
    cJSON_Delete(pt_Root);
    for (u8_Attempt = 0U; u8_Attempt < FOTA_RETRY_ATTEMPTS; u8_Attempt++)
    {
        if (pt_Context->st_Ops.mqtt_publish_status(
                st_FOTA_JobIdView(pt_Context), ac_Payload,
                (uint32_t)i32_Length))
        {
            return true;
        }
    }
    return false;
}

static void gv_FOTA_ClearContext(FOTASMContext_T *pt_Context)
{
    gv_FOTA_ReleaseJob(&pt_Context->st_Job);
    (void)sb_FOTA_DeleteRetries(pt_Context, eFOTA_STORAGE_CURRENT_FILE);
    (void)sb_FOTA_DeleteRetries(pt_Context, eFOTA_STORAGE_RESUME);
    (void)sb_FOTA_DeleteRetries(pt_Context, eFOTA_STORAGE_CONTEXT_A);
    (void)sb_FOTA_DeleteRetries(pt_Context, eFOTA_STORAGE_CONTEXT_B);
    (void)sb_FOTA_DeleteRetries(pt_Context, eFOTA_STORAGE_JOB_DOCUMENT_A);
    (void)sb_FOTA_DeleteRetries(pt_Context, eFOTA_STORAGE_JOB_DOCUMENT_B);
    pt_Context->b_JobActive = false;
    pt_Context->b_DocumentRefresh = false;
    pt_Context->u8_CurrentPackage = 0U;
    if (pt_Context->b_MonitorOwned)
    {
        pt_Context->st_Ops.app_monitor_release();
        pt_Context->b_MonitorOwned = false;
    }
    pt_Context->e_State = eFOTA_STATE_IDLE;
    pt_Context->st_Status.e_State = eFOTA_STATE_IDLE;
}

static void gv_FOTA_SetFatal(FOTASMContext_T *pt_Context,
                             FOTAError_E e_Error)
{
    if (pt_Context->b_MonitorOwned)
    {
        pt_Context->st_Ops.app_monitor_release();
        pt_Context->b_MonitorOwned = false;
    }
    pt_Context->b_JobActive = false;
    pt_Context->st_Status.e_LastError = e_Error;
    pt_Context->e_State = eFOTA_STATE_FATAL;
    pt_Context->st_Status.e_State = pt_Context->e_State;
}

static void gv_FOTA_FailJob(FOTASMContext_T *pt_Context, FOTAError_E e_Error)
{
    const FOTAPackage_T *cpt_Package =
        (pt_Context->u8_CurrentPackage < pt_Context->st_Job.u8_PackageCount) ?
        &pt_Context->st_Job.at_Packages[pt_Context->u8_CurrentPackage] : NULL;
    pt_Context->e_State = eFOTA_STATE_REPORT_FAILURE;
    pt_Context->st_Status.e_State = pt_Context->e_State;
    pt_Context->st_Status.e_LastError = e_Error;
    (void)sb_FOTA_PublishStatus(pt_Context, "IN_PROGRESS", "FAILED",
                                cpt_Package, e_Error, 0U);
    (void)sb_FOTA_PublishStatus(pt_Context, "FAILED", "FAILED",
                                NULL, e_Error, 0U);
    gv_FOTA_ClearContext(pt_Context);
}

static void gv_FOTA_DiscardJob(FOTASMContext_T *pt_Context)
{
    gv_FOTA_ClearContext(pt_Context);
}

static bool sb_FOTA_CheckPollDue(FOTASMContext_T *pt_Context)
{
    uint64_t u64_Now;
    uint32_t u32_Day;
    if (!sb_FOTA_GetTime(pt_Context, &u64_Now, &u32_Day))
    {
        return false;
    }
    if (pt_Context->st_Status.u32_PollCalendarDay != u32_Day)
    {
        pt_Context->st_Status.u32_PollCalendarDay = u32_Day;
        pt_Context->st_Status.u8_PollCount = 0U;
    }
    if (pt_Context->st_Status.u8_PollCount >= FOTA_DAILY_POLL_LIMIT)
    {
        return false;
    }
    return (pt_Context->st_Status.u64_LastPollEpoch == 0U) ||
           ((u64_Now >= pt_Context->st_Status.u64_LastPollEpoch) &&
            ((u64_Now - pt_Context->st_Status.u64_LastPollEpoch) >=
             FOTA_POLL_INTERVAL_SECONDS));
}

static bool sb_FOTA_IssuePoll(FOTASMContext_T *pt_Context)
{
    uint8_t u8_Attempt;
    for (u8_Attempt = pt_Context->u8_PollAttempts;
         u8_Attempt < FOTA_RETRY_ATTEMPTS;
         u8_Attempt++)
    {
        pt_Context->u8_PollAttempts = (uint8_t)(u8_Attempt + 1U);
        if ((pt_Context->st_Ops.mqtt_poll_next != NULL) &&
            pt_Context->st_Ops.mqtt_poll_next())
        {
            return true;
        }
    }
    gv_FOTA_ClearContext(pt_Context);
    return false;
}

static bool sb_FOTA_IssueDocument(FOTASMContext_T *pt_Context)
{
    uint8_t u8_Attempt;
    for (u8_Attempt = pt_Context->u8_DocumentAttempts;
         u8_Attempt < FOTA_RETRY_ATTEMPTS;
         u8_Attempt++)
    {
        pt_Context->u8_DocumentAttempts = (uint8_t)(u8_Attempt + 1U);
        if ((pt_Context->st_Ops.mqtt_request_document != NULL) &&
            pt_Context->st_Ops.mqtt_request_document(st_FOTA_JobIdView(pt_Context)))
        {
            return true;
        }
    }
    gv_FOTA_DiscardJob(pt_Context);
    return false;
}

static bool sb_FOTA_StartPoll(FOTASMContext_T *pt_Context)
{
    uint64_t u64_Now;
    uint32_t u32_Day;
    if (!sb_FOTA_CheckPollDue(pt_Context) ||
        !pt_Context->st_Ops.app_monitor_acquire())
    {
        return false;
    }
    pt_Context->b_MonitorOwned = true;
    (void)sb_FOTA_GetTime(pt_Context, &u64_Now, &u32_Day);
    pt_Context->st_Status.u64_LastPollEpoch = u64_Now;
    pt_Context->st_Status.u32_PollCalendarDay = u32_Day;
    if (!sb_FOTA_PersistPoll(pt_Context))
    {
        gv_FOTA_SetFatal(pt_Context, eFOTA_ERROR_STORAGE);
        return false;
    }
    pt_Context->e_State = eFOTA_STATE_POLLING;
    pt_Context->st_Status.e_State = pt_Context->e_State;
    pt_Context->u8_PollAttempts = 0U;
    return sb_FOTA_IssuePoll(pt_Context);
}

static bool sb_FOTA_StartDocument(FOTASMContext_T *pt_Context)
{
    pt_Context->e_State = eFOTA_STATE_ACQUIRE_JOB_DOCUMENT;
    pt_Context->st_Status.e_State = pt_Context->e_State;
    return sb_FOTA_IssueDocument(pt_Context);
}

static bool sb_FOTA_CheckManifestVersion(FOTASMContext_T *pt_Context,
                                         const FOTAPackage_T *cpt_Package)
{
    int32_t i32_CompareMin;
    int32_t i32_CompareTarget;
    FOTAStringView_T st_Current;
    if ((pt_Context->st_Ops.manifest_get_version == NULL) ||
        !pt_Context->st_Ops.manifest_get_version(
            cpt_Package->ecu_type, cpt_Package->ecu_id,
            pt_Context->ac_CurrentVersion,
            sizeof(pt_Context->ac_CurrentVersion),
            &pt_Context->u16_CurrentVersionLength))
    {
        gv_FOTA_FailJob(pt_Context, eFOTA_ERROR_MANIFEST);
        return false;
    }
    st_Current.pc_Data = pt_Context->ac_CurrentVersion;
    st_Current.u16_Length = pt_Context->u16_CurrentVersionLength;
    if (gb_FOTA_StringEquals(cpt_Package->min_ver, "0"))
    {
        i32_CompareMin = 1;
    }
    else
    {
        i32_CompareMin = gi32_FOTA_StringCompare(st_Current,
                                                 cpt_Package->min_ver);
    }
    i32_CompareTarget = gi32_FOTA_StringCompare(st_Current,
                                                cpt_Package->target_ver);
    if (i32_CompareMin < 0)
    {
        gv_FOTA_FailJob(pt_Context, eFOTA_ERROR_VERSION_INCOMPATIBLE);
        return false;
    }
    if (i32_CompareTarget >= 0)
    {
        pt_Context->e_State = eFOTA_STATE_REPORT_SKIPPED;
        pt_Context->st_Status.e_State = pt_Context->e_State;
        (void)sb_FOTA_PublishStatus(pt_Context, "IN_PROGRESS", "SKIPPED",
                                    cpt_Package, eFOTA_ERROR_NONE, 100U);
        pt_Context->u8_CurrentPackage++;
        pt_Context->e_State = eFOTA_STATE_SELECT_PACKAGE;
        pt_Context->st_Status.e_State = pt_Context->e_State;
        if (!sb_FOTA_PersistContext(pt_Context))
        {
            gv_FOTA_FailJob(pt_Context, eFOTA_ERROR_STORAGE);
            return false;
        }
        return true;
    }
    return true;
}

static bool sb_FOTA_CheckStorage(FOTASMContext_T *pt_Context,
                                 uint32_t u32_PackageSize,
                                 uint32_t u32_Existing)
{
    uint32_t u32_Free = pt_Context->st_Ops.storage_free_bytes();
    if ((u32_Free != UINT32_MAX) &&
        (u32_Free < (u32_PackageSize - u32_Existing)))
    {
        gv_FOTA_FailJob(pt_Context, eFOTA_ERROR_INSUFFICIENT_SPACE);
        return false;
    }
    return true;
}

static bool sb_FOTA_Checkpoint(uint8_t u8_Percentage,
                               uint32_t u32_Bytes,
                               void *pv_User)
{
    FOTASMContext_T *pt_Context = (FOTASMContext_T *)pv_User;
    const FOTAPackage_T *cpt_Package =
        &pt_Context->st_Job.at_Packages[pt_Context->u8_CurrentPackage];
    if (!sb_FOTA_PersistResume(pt_Context, u32_Bytes))
    {
        return false;
    }
    pt_Context->st_Status.u32_DownloadedBytes = u32_Bytes;
    return sb_FOTA_PublishStatus(pt_Context, "IN_PROGRESS", "DOWNLOADING",
                                 cpt_Package, eFOTA_ERROR_NONE,
                                 u8_Percentage);
}

static bool sb_FOTA_VerifyPackage(FOTASMContext_T *pt_Context,
                                  const FOTAPackage_T *cpt_Package)
{
    /* Part A deliberately bypasses verification because the hash algorithm is
     * not defined. Defining FOTA_ENABLE_CRYPTO_VERIFY enables the retained
     * seam and routes a mismatch back through package selection for a clean
     * redownload, without changing the surrounding state machine. */
#if defined(FOTA_ENABLE_CRYPTO_VERIFY)
    if ((pt_Context->st_Ops.crypto_verify != NULL) &&
        !pt_Context->st_Ops.crypto_verify(cpt_Package->u32_ExpectedHash,
                                          eFOTA_STORAGE_CURRENT_FILE,
                                          cpt_Package->u32_PackageSize))
    {
        return false;
    }
#else
    BAL_UNUSED(pt_Context);
    BAL_UNUSED(cpt_Package);
#endif
    return true;
}

static bool sb_FOTA_StartDownload(FOTASMContext_T *pt_Context)
{
    const FOTAPackage_T *cpt_Package =
        &pt_Context->st_Job.at_Packages[pt_Context->u8_CurrentPackage];
    uint32_t u32_Existing = pt_Context->st_Ops.storage_exists(
                                eFOTA_STORAGE_CURRENT_FILE) ?
                            pt_Context->st_Ops.storage_size(
                                eFOTA_STORAGE_CURRENT_FILE) : 0U;
    FOTADownloadResult_E e_Result;
    if (pt_Context->b_DocumentRefresh ||
        (u32_Existing > cpt_Package->u32_PackageSize))
    {
        if (!sb_FOTA_DeleteRetries(pt_Context, eFOTA_STORAGE_CURRENT_FILE) ||
            !sb_FOTA_DeleteRetries(pt_Context, eFOTA_STORAGE_RESUME))
        {
            gv_FOTA_FailJob(pt_Context, eFOTA_ERROR_STORAGE);
            return false;
        }
        u32_Existing = 0U;
        pt_Context->b_DocumentRefresh = false;
    }
    if (!sb_FOTA_CheckStorage(pt_Context, cpt_Package->u32_PackageSize,
                              u32_Existing))
    {
        return false;
    }
    e_Result = ge_FOTA_DownloadBegin(&pt_Context->st_Download,
                                     &pt_Context->st_Ops,
                                     cpt_Package,
                                     u32_Existing,
                                     pt_Context->au8_HttpBuffer);
    pt_Context->e_State = eFOTA_STATE_DOWNLOADING;
    pt_Context->st_Status.e_State = pt_Context->e_State;
    pt_Context->st_Status.u32_DownloadedBytes = u32_Existing;
    pt_Context->st_Status.u32_PackageSize = cpt_Package->u32_PackageSize;
    if (!sb_FOTA_PersistContext(pt_Context))
    {
        gv_FOTA_FailJob(pt_Context, eFOTA_ERROR_STORAGE);
        return false;
    }
    if (e_Result == eFOTA_DOWNLOAD_COMPLETE)
    {
        pt_Context->e_State = eFOTA_STATE_DOWNLOAD_COMPLETED;
        pt_Context->st_Status.e_State = pt_Context->e_State;
        if (!sb_FOTA_PersistContext(pt_Context))
        {
            gv_FOTA_FailJob(pt_Context, eFOTA_ERROR_STORAGE);
            return false;
        }
        return sb_FOTA_FinishVerification(pt_Context);
    }
    if (e_Result != eFOTA_DOWNLOAD_FAILED)
    {
        do
        {
            e_Result = ge_FOTA_DownloadIssueRange(&pt_Context->st_Download);
        } while (e_Result == eFOTA_DOWNLOAD_PROGRESS);
    }
    if (e_Result == eFOTA_DOWNLOAD_EXPIRED)
    {
        pt_Context->b_DocumentRefresh = true;
        pt_Context->u8_DocumentAttempts = 0U;
        return sb_FOTA_StartDocument(pt_Context);
    }
    if ((e_Result == eFOTA_DOWNLOAD_FAILED) ||
        (e_Result == eFOTA_DOWNLOAD_PROGRESS))
    {
        gv_FOTA_FailJob(pt_Context, eFOTA_ERROR_HTTP);
        return false;
    }
    return true;
}

static bool sb_FOTA_FinishVerification(FOTASMContext_T *pt_Context)
{
    const FOTAPackage_T *cpt_Package =
        &pt_Context->st_Job.at_Packages[pt_Context->u8_CurrentPackage];
    pt_Context->e_State = eFOTA_STATE_VERIFY;
    pt_Context->st_Status.e_State = pt_Context->e_State;
    if (!sb_FOTA_PersistContext(pt_Context))
    {
        gv_FOTA_FailJob(pt_Context, eFOTA_ERROR_STORAGE);
        return false;
    }
    if (!sb_FOTA_VerifyPackage(pt_Context, cpt_Package))
    {
        if (!sb_FOTA_DeleteRetries(pt_Context, eFOTA_STORAGE_CURRENT_FILE) ||
            !sb_FOTA_DeleteRetries(pt_Context, eFOTA_STORAGE_RESUME))
        {
            gv_FOTA_FailJob(pt_Context, eFOTA_ERROR_STORAGE);
            return false;
        }
        pt_Context->e_State = eFOTA_STATE_SELECT_PACKAGE;
        pt_Context->st_Status.e_State = pt_Context->e_State;
        if (!sb_FOTA_PersistContext(pt_Context))
        {
            gv_FOTA_FailJob(pt_Context, eFOTA_ERROR_STORAGE);
            return false;
        }
        return sb_FOTA_SelectPackage(pt_Context);
    }
    /* The hash algorithm is deliberately bypassed in Part A. */
    pt_Context->e_State = eFOTA_STATE_VERIFICATION_COMPLETED;
    pt_Context->st_Status.e_State = pt_Context->e_State;
    if (!sb_FOTA_PersistContext(pt_Context))
    {
        gv_FOTA_FailJob(pt_Context, eFOTA_ERROR_STORAGE);
        return false;
    }
    (void)sb_FOTA_PublishStatus(pt_Context, "IN_PROGRESS",
                                "VERIFICATION_COMPLETED", cpt_Package,
                                eFOTA_ERROR_NONE, 100U);
    if (!pt_Context->st_Ops.installer_handoff(st_FOTA_JobIdView(pt_Context),
                                              cpt_Package,
                                              eFOTA_STORAGE_CURRENT_FILE))
    {
        gv_FOTA_FailJob(pt_Context, eFOTA_ERROR_INTERNAL);
        return false;
    }
    pt_Context->e_State = eFOTA_STATE_WAITING_FOR_INSTALLER;
    pt_Context->st_Status.e_State = pt_Context->e_State;
    if (!sb_FOTA_PersistContext(pt_Context))
    {
        gv_FOTA_FailJob(pt_Context, eFOTA_ERROR_STORAGE);
        return false;
    }
    return true;
}

static bool sb_FOTA_SelectPackage(FOTASMContext_T *pt_Context)
{
    const FOTAPackage_T *cpt_Package;
    uint64_t u64_Now;
    if (pt_Context->u8_CurrentPackage >= pt_Context->st_Job.u8_PackageCount)
    {
        (void)sb_FOTA_PublishStatus(pt_Context, "SUCCEEDED", "SUCCESS",
                                    NULL, eFOTA_ERROR_NONE, 100U);
        gv_FOTA_ClearContext(pt_Context);
        return true;
    }
    cpt_Package = &pt_Context->st_Job.at_Packages[pt_Context->u8_CurrentPackage];
    if (!sb_FOTA_CheckManifestVersion(pt_Context, cpt_Package))
    {
        return false;
    }
    if (pt_Context->e_State == eFOTA_STATE_REPORT_SKIPPED)
    {
        if (pt_Context->u8_CurrentPackage >= pt_Context->st_Job.u8_PackageCount)
        {
            return sb_FOTA_SelectPackage(pt_Context);
        }
        return sb_FOTA_SelectPackage(pt_Context);
    }
    if (sb_FOTA_GetTime(pt_Context, &u64_Now, &pt_Context->st_Status.u32_PollCalendarDay) &&
        (u64_Now >= cpt_Package->u64_UrlExpiry))
    {
        pt_Context->b_DocumentRefresh = true;
        pt_Context->u8_DocumentAttempts = 0U;
        return sb_FOTA_StartDocument(pt_Context);
    }
    return sb_FOTA_StartDownload(pt_Context);
}

static bool sb_FOTA_HandleDocument(FOTASMContext_T *pt_Context,
                                   const FOTAEvent_T *cpt_Event)
{
    uint64_t u64_Now = 0U;
    FOTAParseResult_E e_Parse;
    if (!cpt_Event->b_Accepted)
    {
        if (sb_FOTA_IsAuthoritativeStop(cpt_Event->e_Error))
        {
            gv_FOTA_DiscardJob(pt_Context);
            return false;
        }
        return sb_FOTA_IssueDocument(pt_Context);
    }
    if ((cpt_Event->pc_Payload == NULL) ||
        (cpt_Event->u32_PayloadLength == 0U) ||
        (cpt_Event->u32_PayloadLength > FOTA_MAX_JOB_DOCUMENT_BYTES))
    {
        e_Parse = eFOTA_PARSE_INVALID_DOCUMENT;
    }
    else
    {
        memcpy(pt_Context->ac_JobDocument, cpt_Event->pc_Payload,
               cpt_Event->u32_PayloadLength);
        pt_Context->u32_JobDocumentLength = cpt_Event->u32_PayloadLength;
        (void)sb_FOTA_GetTime(pt_Context, &u64_Now,
                              &pt_Context->st_Status.u32_PollCalendarDay);
        gv_FOTA_ReleaseJob(&pt_Context->st_Job);
        e_Parse = ge_FOTA_ParseJobDocument(pt_Context->ac_JobDocument,
                                           pt_Context->u32_JobDocumentLength,
                                           u64_Now, &pt_Context->st_Job);
    }
    if (e_Parse != eFOTA_PARSE_OK)
    {
        if (e_Parse == eFOTA_PARSE_EXPIRED)
        {
            pt_Context->b_DocumentRefresh = true;
        }
        return sb_FOTA_IssueDocument(pt_Context);
    }
    if (!sb_FOTA_PersistDocument(pt_Context) ||
        !sb_FOTA_PersistContext(pt_Context))
    {
        gv_FOTA_FailJob(pt_Context, eFOTA_ERROR_STORAGE);
        return false;
    }
    pt_Context->e_State = eFOTA_STATE_VALIDATE_JOB;
    pt_Context->st_Status.e_State = pt_Context->e_State;
    (void)sb_FOTA_PublishStatus(pt_Context, "IN_PROGRESS",
                                "JOB_DOCUMENT_RECEIVED", NULL,
                                eFOTA_ERROR_NONE, 0U);
    pt_Context->e_State = eFOTA_STATE_SELECT_PACKAGE;
    pt_Context->st_Status.e_State = pt_Context->e_State;
    return sb_FOTA_SelectPackage(pt_Context);
}

static bool sb_FOTA_HandlePoll(FOTASMContext_T *pt_Context,
                               const FOTAEvent_T *cpt_Event)
{
    if (!cpt_Event->b_Accepted)
    {
        if (sb_FOTA_IsAuthoritativeStop(cpt_Event->e_Error))
        {
            gv_FOTA_DiscardJob(pt_Context);
            return false;
        }
        return sb_FOTA_IssuePoll(pt_Context);
    }
    if ((cpt_Event->pc_Payload != NULL) &&
        (cpt_Event->u32_PayloadLength > 0U) &&
        !sb_FOTA_EventJsonValid(cpt_Event))
    {
        return sb_FOTA_IssuePoll(pt_Context);
    }
    pt_Context->st_Status.u8_PollCount++;
    if (!sb_FOTA_PersistPoll(pt_Context))
    {
        gv_FOTA_SetFatal(pt_Context, eFOTA_ERROR_STORAGE);
        return false;
    }
    if (sb_FOTA_EventIsNoJob(cpt_Event))
    {
        gv_FOTA_ClearContext(pt_Context);
        return true;
    }
    if (!sb_FOTA_CopyJobIdFromEvent(pt_Context, cpt_Event))
    {
        /* A valid empty response has no execution/jobId. */
        gv_FOTA_ClearContext(pt_Context);
        return true;
    }
    pt_Context->b_JobActive = true;
    pt_Context->u8_DocumentAttempts = 0U;
    (void)sb_FOTA_PublishStatus(pt_Context, "IN_PROGRESS",
                                "JOB_NOTIFICATION_RECEIVED", NULL,
                                eFOTA_ERROR_NONE, 0U);
    return sb_FOTA_StartDocument(pt_Context);
}

static bool sb_FOTA_HandleMqttResult(FOTASMContext_T *pt_Context,
                                     const FOTAEvent_T *cpt_Event)
{
    if (pt_Context->b_RecoveryValidationPending)
    {
        if (!cpt_Event->b_Accepted ||
            !sb_FOTA_EventJobMatches(pt_Context, cpt_Event))
        {
            gv_FOTA_DiscardJob(pt_Context);
            return false;
        }
        pt_Context->b_RecoveryValidationPending = false;
        pt_Context->e_State = pt_Context->b_ManifestReady ?
                              eFOTA_STATE_SELECT_PACKAGE :
                              eFOTA_STATE_WAIT_MANIFEST;
        pt_Context->st_Status.e_State = pt_Context->e_State;
        return pt_Context->b_ManifestReady ?
               sb_FOTA_ResumeRecoveredState(pt_Context) : true;
    }
    if (pt_Context->e_State == eFOTA_STATE_POLLING)
    {
        return sb_FOTA_HandlePoll(pt_Context, cpt_Event);
    }
    if (pt_Context->e_State == eFOTA_STATE_ACQUIRE_JOB_DOCUMENT)
    {
        return sb_FOTA_HandleDocument(pt_Context, cpt_Event);
    }
    return true;
}

static bool sb_FOTA_HandleHttp(FOTASMContext_T *pt_Context,
                               const FOTAEvent_T *cpt_Event)
{
    FOTADownloadResult_E e_Result = ge_FOTA_DownloadHandleRange(
        &pt_Context->st_Download, &cpt_Event->st_Http,
        sb_FOTA_Checkpoint, pt_Context);
    if (e_Result == eFOTA_DOWNLOAD_PROGRESS)
    {
        if (!sb_FOTA_PersistResume(
                pt_Context,
                gu32_FOTA_DownloadBytes(&pt_Context->st_Download)))
        {
            gv_FOTA_FailJob(pt_Context, eFOTA_ERROR_STORAGE);
            return false;
        }
        pt_Context->st_Status.u32_DownloadedBytes =
            gu32_FOTA_DownloadBytes(&pt_Context->st_Download);
        do
        {
            e_Result = ge_FOTA_DownloadIssueRange(&pt_Context->st_Download);
        } while (e_Result == eFOTA_DOWNLOAD_PROGRESS);
    }
    if (e_Result == eFOTA_DOWNLOAD_EXPIRED)
    {
        pt_Context->b_DocumentRefresh = true;
        pt_Context->u8_DocumentAttempts = 0U;
        if (!sb_FOTA_DeleteRetries(pt_Context, eFOTA_STORAGE_CURRENT_FILE) ||
            !sb_FOTA_DeleteRetries(pt_Context, eFOTA_STORAGE_RESUME))
        {
            gv_FOTA_FailJob(pt_Context, eFOTA_ERROR_STORAGE);
            return false;
        }
        return sb_FOTA_StartDocument(pt_Context);
    }
    if (e_Result == eFOTA_DOWNLOAD_SHUTDOWN)
    {
        if (!sb_FOTA_PersistResume(
                pt_Context,
                gu32_FOTA_DownloadBytes(&pt_Context->st_Download)) ||
            !sb_FOTA_PersistContext(pt_Context))
        {
            gv_FOTA_FailJob(pt_Context, eFOTA_ERROR_STORAGE);
            return false;
        }
        return true;
    }
    if (e_Result == eFOTA_DOWNLOAD_COMPLETE)
    {
        pt_Context->e_State = eFOTA_STATE_DOWNLOAD_COMPLETED;
        pt_Context->st_Status.e_State = pt_Context->e_State;
        if (!sb_FOTA_PersistContext(pt_Context))
        {
            gv_FOTA_FailJob(pt_Context, eFOTA_ERROR_STORAGE);
            return false;
        }
        (void)sb_FOTA_PublishStatus(pt_Context, "IN_PROGRESS",
                                    "DOWNLOAD_COMPLETED",
                                    &pt_Context->st_Job.at_Packages[
                                        pt_Context->u8_CurrentPackage],
                                    eFOTA_ERROR_NONE, 100U);
        return sb_FOTA_FinishVerification(pt_Context);
    }
    if (e_Result == eFOTA_DOWNLOAD_FAILED)
    {
        gv_FOTA_FailJob(pt_Context, eFOTA_ERROR_HTTP_RESPONSE);
        return false;
    }
    return true;
}

static bool sb_FOTA_HandleInstallComplete(FOTASMContext_T *pt_Context,
                                          const FOTAEvent_T *cpt_Event)
{
    const FOTAPackage_T *cpt_Package;
    if (pt_Context->e_State != eFOTA_STATE_WAITING_FOR_INSTALLER)
    {
        return false;
    }
    if (pt_Context->u8_CurrentPackage >= pt_Context->st_Job.u8_PackageCount)
    {
        return false;
    }
    cpt_Package = &pt_Context->st_Job.at_Packages[pt_Context->u8_CurrentPackage];
    if ((cpt_Event->job_id.pc_Data == NULL) ||
        !sb_FOTA_ViewEqual(cpt_Event->job_id, st_FOTA_JobIdView(pt_Context)) ||
        (cpt_Event->package_id.pc_Data == NULL) ||
        !sb_FOTA_ViewEqual(cpt_Event->package_id, cpt_Package->package_id))
    {
        return false;
    }
    if (!sb_FOTA_DeleteRetries(pt_Context, eFOTA_STORAGE_CURRENT_FILE) ||
        !sb_FOTA_DeleteRetries(pt_Context, eFOTA_STORAGE_RESUME))
    {
        gv_FOTA_FailJob(pt_Context, eFOTA_ERROR_STORAGE);
        return false;
    }
    pt_Context->u8_CurrentPackage++;
    pt_Context->e_State = eFOTA_STATE_SELECT_PACKAGE;
    pt_Context->st_Status.e_State = pt_Context->e_State;
    if (!sb_FOTA_PersistContext(pt_Context))
    {
        gv_FOTA_FailJob(pt_Context, eFOTA_ERROR_STORAGE);
        return false;
    }
    return sb_FOTA_SelectPackage(pt_Context);
}

static void gv_FOTA_LoadPollBudget(FOTASMContext_T *pt_Context)
{
    FOTAPollRecord_T st_Record = {0};
    uint64_t u64_Now;
    uint32_t u32_Day;
    if (sb_FOTA_ReadPollRecord(pt_Context, &st_Record))
    {
        pt_Context->st_Status.u32_PollCalendarDay = st_Record.u32_CalendarDay;
        pt_Context->st_Status.u8_PollCount = st_Record.u8_PollCount;
        pt_Context->st_Status.u64_LastPollEpoch = st_Record.u64_LastPollEpoch;
    }
    if (sb_FOTA_GetTime(pt_Context, &u64_Now, &u32_Day))
    {
        if (pt_Context->st_Status.u32_PollCalendarDay != u32_Day)
        {
            pt_Context->st_Status.u32_PollCalendarDay = u32_Day;
            pt_Context->st_Status.u8_PollCount = 0U;
            (void)sb_FOTA_PersistPoll(pt_Context);
        }
    }
}

static bool sb_FOTA_ValidateRecoveredResume(FOTASMContext_T *pt_Context)
{
    const FOTAPackage_T *cpt_Package;
    FOTAResumeRecord_T st_Resume = {0};
    bool b_FileExists;
    bool b_ResumeExists;
    uint32_t u32_FileSize = 0U;
    bool b_Valid;

    if ((pt_Context == NULL) ||
        (pt_Context->u8_CurrentPackage >= pt_Context->st_Job.u8_PackageCount))
    {
        return false;
    }
    cpt_Package = &pt_Context->st_Job.at_Packages[
        pt_Context->u8_CurrentPackage];
    b_FileExists = pt_Context->st_Ops.storage_exists(
        eFOTA_STORAGE_CURRENT_FILE);
    b_ResumeExists = pt_Context->st_Ops.storage_exists(eFOTA_STORAGE_RESUME);
    if (b_FileExists)
    {
        u32_FileSize = pt_Context->st_Ops.storage_size(
            eFOTA_STORAGE_CURRENT_FILE);
    }
    b_Valid = b_ResumeExists && sb_FOTA_ReadResumeRecord(pt_Context,
                                                          &st_Resume) &&
              (st_Resume.u32_ExpectedSize == cpt_Package->u32_PackageSize) &&
              (st_Resume.u32_SequenceId == cpt_Package->u32_SequenceId) &&
              (st_Resume.u16_PackageIdLength ==
               cpt_Package->package_id.u16_Length) &&
              (memcmp(st_Resume.ac_PackageId,
                      cpt_Package->package_id.pc_Data,
                      st_Resume.u16_PackageIdLength) == 0) &&
              (st_Resume.u32_DownloadedBytes == u32_FileSize) &&
              (u32_FileSize <= cpt_Package->u32_PackageSize);
    if (!b_FileExists && !b_ResumeExists)
    {
        return true;
    }
    if (!b_Valid)
    {
        if (!sb_FOTA_DeleteRetries(pt_Context, eFOTA_STORAGE_CURRENT_FILE) ||
            !sb_FOTA_DeleteRetries(pt_Context, eFOTA_STORAGE_RESUME))
        {
            return false;
        }
    }
    return true;
}

static void gv_FOTA_LoadRecovery(FOTASMContext_T *pt_Context)
{
    FOTARecoveryRecord_T st_A = {0};
    FOTARecoveryRecord_T st_B = {0};
    FOTARecoveryRecord_T *pt_Selected = NULL;
    uint32_t u32_Actual = 0U;
    uint64_t u64_Now = 0U;
    uint32_t u32_Day = 0U;
    if (sb_FOTA_ReadRecord(pt_Context, eFOTA_STORAGE_CONTEXT_A, &st_A))
    {
        pt_Selected = &st_A;
    }
    if (sb_FOTA_ReadRecord(pt_Context, eFOTA_STORAGE_CONTEXT_B, &st_B) &&
        ((pt_Selected == NULL) || (st_B.u32_Generation > pt_Selected->u32_Generation)))
    {
        pt_Selected = &st_B;
    }
    if (pt_Selected == NULL)
    {
        /* A partial artifact without a valid recovery record is not
         * resumable; remove it before accepting new work. */
        gv_FOTA_ClearContext(pt_Context);
        pt_Context->e_State = eFOTA_STATE_WAIT_MANIFEST;
        pt_Context->st_Status.e_State = pt_Context->e_State;
        return;
    }
    pt_Context->u32_RecoveryGeneration = pt_Selected->u32_Generation;
    pt_Context->u8_RecoveredState = pt_Selected->u8_State;
    if (pt_Selected->u8_State >= (uint8_t)eFOTA_STATE_FATAL)
    {
        gv_FOTA_ClearContext(pt_Context);
        pt_Context->e_State = eFOTA_STATE_WAIT_MANIFEST;
        pt_Context->st_Status.e_State = pt_Context->e_State;
        return;
    }
    pt_Context->e_State = (FOTAState_E)pt_Selected->u8_State;
    pt_Context->u8_CurrentPackage = pt_Selected->u8_CurrentPackage;
    pt_Context->u8_DocumentSlot = pt_Selected->u8_DocumentSlot;
    if ((pt_Selected->u16_JobIdLength >= FOTA_MAX_JOB_ID_BYTES) ||
        !sb_FOTA_CopyView(pt_Context->ac_JobId,
                          sizeof(pt_Context->ac_JobId),
                          (FOTAStringView_T){pt_Selected->ac_JobId,
                                              pt_Selected->u16_JobIdLength},
                          &pt_Context->u16_JobIdLength))
    {
        gv_FOTA_ClearContext(pt_Context);
        pt_Context->e_State = eFOTA_STATE_WAIT_MANIFEST;
        pt_Context->st_Status.e_State = pt_Context->e_State;
        return;
    }
    if ((pt_Context->u8_DocumentSlot > 1U) ||
        !pt_Context->st_Ops.storage_exists(pt_Context->u8_DocumentSlot == 0U ?
                                           eFOTA_STORAGE_JOB_DOCUMENT_A :
                                           eFOTA_STORAGE_JOB_DOCUMENT_B) ||
        (pt_Context->st_Ops.storage_size(pt_Context->u8_DocumentSlot == 0U ?
                                         eFOTA_STORAGE_JOB_DOCUMENT_A :
                                         eFOTA_STORAGE_JOB_DOCUMENT_B) >
         FOTA_MAX_JOB_DOCUMENT_BYTES) ||
        !pt_Context->st_Ops.storage_read(
             pt_Context->u8_DocumentSlot == 0U ? eFOTA_STORAGE_JOB_DOCUMENT_A :
                                                 eFOTA_STORAGE_JOB_DOCUMENT_B,
             0U, pt_Context->ac_JobDocument,
             pt_Context->st_Ops.storage_size(pt_Context->u8_DocumentSlot == 0U ?
                                             eFOTA_STORAGE_JOB_DOCUMENT_A :
                                             eFOTA_STORAGE_JOB_DOCUMENT_B),
             &u32_Actual))
    {
        gv_FOTA_ClearContext(pt_Context);
        pt_Context->e_State = eFOTA_STATE_WAIT_MANIFEST;
        pt_Context->st_Status.e_State = pt_Context->e_State;
        return;
    }
    pt_Context->u32_JobDocumentLength = u32_Actual;
    (void)sb_FOTA_GetTime(pt_Context, &u64_Now, &u32_Day);
    {
        FOTAParseResult_E e_Parse = ge_FOTA_ParseJobDocument(
            pt_Context->ac_JobDocument,
            pt_Context->u32_JobDocumentLength,
            u64_Now,
            &pt_Context->st_Job);
        if (e_Parse != eFOTA_PARSE_OK)
        {
            bool b_Expired = (e_Parse == eFOTA_PARSE_EXPIRED);
            gv_FOTA_ClearContext(pt_Context);
            pt_Context->e_State = eFOTA_STATE_WAIT_MANIFEST;
            pt_Context->st_Status.e_State = pt_Context->e_State;
            if (b_Expired)
            {
                pt_Context->b_ImmediatePoll = true;
            }
            return;
        }
    }
    if ((pt_Context->e_State == eFOTA_STATE_DOWNLOADING) &&
        !sb_FOTA_ValidateRecoveredResume(pt_Context))
    {
        gv_FOTA_ClearContext(pt_Context);
        pt_Context->e_State = eFOTA_STATE_WAIT_MANIFEST;
        pt_Context->st_Status.e_State = pt_Context->e_State;
        return;
    }
    pt_Context->b_JobActive = true;
    pt_Context->b_MonitorOwned = pt_Context->st_Ops.app_monitor_acquire();
    if (!pt_Context->b_MonitorOwned)
    {
        pt_Context->e_State = eFOTA_STATE_WAIT_MANIFEST;
        return;
    }
    if (pt_Context->st_Ops.mqtt_validate_job != NULL)
    {
        if (!pt_Context->st_Ops.mqtt_validate_job(st_FOTA_JobIdView(pt_Context)))
        {
            gv_FOTA_DiscardJob(pt_Context);
            return;
        }
        pt_Context->b_RecoveryValidationPending = true;
        pt_Context->e_State = eFOTA_STATE_POLLING;
        pt_Context->st_Status.e_State = pt_Context->e_State;
        return;
    }
    pt_Context->e_State = eFOTA_STATE_WAIT_MANIFEST;
    pt_Context->st_Status.e_State = pt_Context->e_State;
}

void gv_FOTA_SM_Init(FOTASMContext_T *pt_Context,
                     const FOTAPlatformOps_T *cpt_Ops)
{
    memset(pt_Context, 0, sizeof(*pt_Context));
    if (cpt_Ops != NULL)
    {
        pt_Context->st_Ops = *cpt_Ops;
    }
    gv_FOTA_FillDefaultOps(&pt_Context->st_Ops);
    pt_Context->e_State = eFOTA_STATE_WAIT_MANIFEST;
    pt_Context->st_Status.e_State = pt_Context->e_State;
    gv_FOTA_LoadPollBudget(pt_Context);
    gv_FOTA_LoadRecovery(pt_Context);
}

static bool sb_FOTA_ResumeRecoveredState(FOTASMContext_T *pt_Context)
{
    switch (pt_Context->e_State)
    {
        case eFOTA_STATE_WAITING_FOR_INSTALLER:
            return true;
        case eFOTA_STATE_DOWNLOAD_COMPLETED:
        case eFOTA_STATE_VERIFY:
        case eFOTA_STATE_VERIFICATION_COMPLETED:
            return sb_FOTA_FinishVerification(pt_Context);
        case eFOTA_STATE_DOWNLOADING:
        case eFOTA_STATE_SELECT_PACKAGE:
        case eFOTA_STATE_REPORT_SKIPPED:
        case eFOTA_STATE_VALIDATE_JOB:
            pt_Context->e_State = eFOTA_STATE_SELECT_PACKAGE;
            pt_Context->st_Status.e_State = pt_Context->e_State;
            return sb_FOTA_SelectPackage(pt_Context);
        case eFOTA_STATE_IDLE:
            pt_Context->b_JobActive = false;
            return true;
        default:
            pt_Context->e_State = eFOTA_STATE_SELECT_PACKAGE;
            pt_Context->st_Status.e_State = pt_Context->e_State;
            return sb_FOTA_SelectPackage(pt_Context);
    }
}

bool gb_FOTA_SM_HandleEvent(FOTASMContext_T *pt_Context,
                            const FOTAEvent_T *cpt_Event)
{
    if ((pt_Context == NULL) || (cpt_Event == NULL))
    {
        return false;
    }
    switch (cpt_Event->e_Type)
    {
        case eFOTA_EVENT_MANIFEST_READY:
            if (pt_Context->b_ManifestFailed)
            {
                return true;
            }
            pt_Context->b_ManifestReady = true;
            if (pt_Context->e_State == eFOTA_STATE_WAIT_MANIFEST)
            {
                if (pt_Context->b_JobActive && !pt_Context->b_MonitorOwned)
                {
                    if (!pt_Context->st_Ops.app_monitor_acquire())
                    {
                        return true;
                    }
                    pt_Context->b_MonitorOwned = true;
                }
                pt_Context->e_State = pt_Context->b_JobActive ?
                                      pt_Context->u8_RecoveredState <
                                      (uint8_t)eFOTA_STATE_FATAL ?
                                      (FOTAState_E)pt_Context->u8_RecoveredState :
                                      eFOTA_STATE_SELECT_PACKAGE :
                                      eFOTA_STATE_IDLE;
                pt_Context->st_Status.e_State = pt_Context->e_State;
                if (pt_Context->b_JobActive)
                {
                    return sb_FOTA_ResumeRecoveredState(pt_Context);
                }
                if (pt_Context->b_ImmediatePoll)
                {
                    pt_Context->b_ImmediatePoll = false;
                    return sb_FOTA_StartPoll(pt_Context);
                }
            }
            return true;

        case eFOTA_EVENT_MANIFEST_INVALID:
            pt_Context->b_ManifestReady = false;
            pt_Context->b_ManifestFailed = true;
            if (pt_Context->b_MonitorOwned)
            {
                pt_Context->st_Ops.app_monitor_release();
                pt_Context->b_MonitorOwned = false;
            }
            pt_Context->e_State = eFOTA_STATE_WAIT_MANIFEST;
            pt_Context->st_Status.e_State = pt_Context->e_State;
            return true;

        case eFOTA_EVENT_RTC_POLL_DUE:
            if (!pt_Context->b_ManifestReady ||
                (pt_Context->e_State != eFOTA_STATE_IDLE))
            {
                return true;
            }
            return sb_FOTA_StartPoll(pt_Context);

        case eFOTA_EVENT_POLL_RESPONSE:
            if (pt_Context->b_RecoveryValidationPending)
            {
                if (!cpt_Event->b_Accepted ||
                    !sb_FOTA_EventJobMatches(pt_Context, cpt_Event))
                {
                    gv_FOTA_DiscardJob(pt_Context);
                    return false;
                }
                pt_Context->b_RecoveryValidationPending = false;
                pt_Context->e_State = pt_Context->b_ManifestReady ?
                                      eFOTA_STATE_SELECT_PACKAGE :
                                      eFOTA_STATE_WAIT_MANIFEST;
                pt_Context->st_Status.e_State = pt_Context->e_State;
                return pt_Context->b_ManifestReady ?
                       sb_FOTA_ResumeRecoveredState(pt_Context) : true;
            }
            return sb_FOTA_HandlePoll(pt_Context, cpt_Event);

        case eFOTA_EVENT_JOB_VALIDATION_RESPONSE:
            if (!cpt_Event->b_Accepted ||
                !sb_FOTA_EventJobMatches(pt_Context, cpt_Event))
            {
                gv_FOTA_DiscardJob(pt_Context);
                return false;
            }
            pt_Context->b_RecoveryValidationPending = false;
            pt_Context->e_State = pt_Context->b_ManifestReady ?
                                  eFOTA_STATE_SELECT_PACKAGE :
                                  eFOTA_STATE_WAIT_MANIFEST;
            pt_Context->st_Status.e_State = pt_Context->e_State;
            return pt_Context->b_ManifestReady ?
                   sb_FOTA_ResumeRecoveredState(pt_Context) : true;

        case eFOTA_EVENT_MQTT_RESULT:
            return sb_FOTA_HandleMqttResult(pt_Context, cpt_Event);

        case eFOTA_EVENT_DOCUMENT_RESPONSE:
            return sb_FOTA_HandleDocument(pt_Context, cpt_Event);

        case eFOTA_EVENT_HTTP_RANGE_RESULT:
            return sb_FOTA_HandleHttp(pt_Context, cpt_Event);

        case eFOTA_EVENT_INSTALLATION_COMPLETED:
            return sb_FOTA_HandleInstallComplete(pt_Context, cpt_Event);

        case eFOTA_EVENT_INSTALLATION_FAILED:
            gv_FOTA_FailJob(pt_Context, eFOTA_ERROR_INTERNAL);
            return false;

        case eFOTA_EVENT_SHUTDOWN_PREPARE:
            if (pt_Context->e_State == eFOTA_STATE_DOWNLOADING)
            {
                gv_FOTA_DownloadRequestShutdown(&pt_Context->st_Download);
                if (!pt_Context->st_Download.b_Pending)
                {
                    if (!sb_FOTA_PersistResume(
                            pt_Context,
                            gu32_FOTA_DownloadBytes(&pt_Context->st_Download)) ||
                        !sb_FOTA_PersistContext(pt_Context))
                    {
                        gv_FOTA_FailJob(pt_Context, eFOTA_ERROR_STORAGE);
                        return false;
                    }
                }
            }
            else
            {
                if (!sb_FOTA_PersistContext(pt_Context))
                {
                    gv_FOTA_FailJob(pt_Context, eFOTA_ERROR_STORAGE);
                    return false;
                }
            }
            return true;

        default:
            return false;
    }
}

FOTAState_E ge_FOTA_SM_GetState(const FOTASMContext_T *cpt_Context)
{
    return (cpt_Context != NULL) ? cpt_Context->e_State : eFOTA_STATE_FATAL;
}

void gv_FOTA_SM_GetStatus(const FOTASMContext_T *cpt_Context,
                          FOTAStatus_T *pt_Status)
{
    if ((cpt_Context != NULL) && (pt_Status != NULL))
    {
        *pt_Status = cpt_Context->st_Status;
    }
}
