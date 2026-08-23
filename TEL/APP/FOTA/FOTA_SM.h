#ifndef FOTA_SM_H
#define FOTA_SM_H

#include "FOTAJobParser.h"
#include "FOTADownloadSM.h"

#define FOTA_MAX_JOB_ID_BYTES       (128U)
#define FOTA_MAX_VERSION_BYTES     (64U)
#define FOTA_CONTEXT_MAGIC          (0x464F5441UL)
#define FOTA_CONTEXT_VERSION        (1U)

typedef struct
{
    uint32_t u32_Magic;
    uint16_t u16_Version;
    uint16_t u16_Length;
    uint32_t u32_Generation;
    uint32_t u32_Crc;
    uint8_t u8_State;
    uint8_t u8_CurrentPackage;
    uint8_t u8_DocumentSlot;
    uint8_t u8_Reserved;
    uint32_t u32_DownloadedBytes;
    uint32_t u32_PackageSize;
    uint16_t u16_JobIdLength;
    char ac_JobId[FOTA_MAX_JOB_ID_BYTES];
} FOTARecoveryRecord_T;

typedef struct
{
    uint32_t u32_Magic;
    uint16_t u16_Version;
    uint16_t u16_Length;
    uint32_t u32_Generation;
    uint32_t u32_Crc;
    uint32_t u32_DownloadedBytes;
    uint32_t u32_ExpectedSize;
    uint32_t u32_SequenceId;
    uint16_t u16_PackageIdLength;
    char ac_PackageId[FOTA_MAX_JOB_ID_BYTES];
} FOTAResumeRecord_T;

typedef struct
{
    FOTAPlatformOps_T st_Ops;
    FOTAState_E e_State;
    FOTAJob_T st_Job;
    FOTADownloadContext_T st_Download;
    FOTAStatus_T st_Status;
    uint32_t u32_RecoveryGeneration;
    uint8_t u8_RecoveredState;
    uint8_t u8_DocumentSlot;
    uint8_t u8_CurrentPackage;
    uint8_t u8_PollAttempts;
    uint8_t u8_DocumentAttempts;
    uint8_t u8_StatusAttempts;
    bool b_ManifestReady;
    bool b_ManifestFailed;
    bool b_MonitorOwned;
    bool b_JobActive;
    bool b_RecoveryValidationPending;
    bool b_DocumentRefresh;
    bool b_ImmediatePoll;
    char ac_JobId[FOTA_MAX_JOB_ID_BYTES];
    uint16_t u16_JobIdLength;
    char ac_JobDocument[FOTA_MAX_JOB_DOCUMENT_BYTES];
    uint32_t u32_JobDocumentLength;
    uint8_t au8_HttpBuffer[FOTA_HTTP_CHUNK_BYTES];
    char ac_CurrentVersion[FOTA_MAX_VERSION_BYTES];
    uint16_t u16_CurrentVersionLength;
} FOTASMContext_T;

void gv_FOTA_SM_Init(FOTASMContext_T *pt_Context,
                     const FOTAPlatformOps_T *cpt_Ops);
bool gb_FOTA_SM_HandleEvent(FOTASMContext_T *pt_Context,
                            const FOTAEvent_T *cpt_Event);
FOTAState_E ge_FOTA_SM_GetState(const FOTASMContext_T *cpt_Context);
void gv_FOTA_SM_GetStatus(const FOTASMContext_T *cpt_Context,
                          FOTAStatus_T *pt_Status);

#endif /* FOTA_SM_H */
