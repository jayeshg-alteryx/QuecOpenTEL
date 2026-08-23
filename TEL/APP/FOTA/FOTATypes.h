/**
 * @file FOTATypes.h
 * @brief Public types shared by the FOTA download state machine.
 */
#ifndef FOTA_TYPES_H
#define FOTA_TYPES_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "SysTypes.h"

#define FOTA_MAX_JOB_DOCUMENT_BYTES   (24576U)
#define FOTA_MAX_PACKAGES             (8U)
#define FOTA_HTTP_CHUNK_BYTES         (4096U)
#define FOTA_RETRY_ATTEMPTS           (4U)
#define FOTA_DAILY_POLL_LIMIT         (4U)
#define FOTA_POLL_INTERVAL_SECONDS    (21600ULL)
#define FOTA_STATUS_BUFFER_BYTES      (1024U)

typedef struct
{
    const char *pc_Data;
    uint16_t u16_Length;
} FOTAStringView_T;

typedef enum
{
    eFOTA_STATE_WAIT_MANIFEST = 0,
    eFOTA_STATE_IDLE,
    eFOTA_STATE_POLLING,
    eFOTA_STATE_ACQUIRE_JOB_DOCUMENT,
    eFOTA_STATE_VALIDATE_JOB,
    eFOTA_STATE_SELECT_PACKAGE,
    eFOTA_STATE_REPORT_SKIPPED,
    eFOTA_STATE_DOWNLOADING,
    eFOTA_STATE_DOWNLOAD_COMPLETED,
    eFOTA_STATE_VERIFY,
    eFOTA_STATE_VERIFICATION_COMPLETED,
    eFOTA_STATE_WAITING_FOR_INSTALLER,
    eFOTA_STATE_REPORT_FAILURE,
    eFOTA_STATE_FATAL
} FOTAState_E;

typedef enum
{
    eFOTA_EVENT_MANIFEST_READY = 0,
    eFOTA_EVENT_MANIFEST_INVALID,
    eFOTA_EVENT_RTC_POLL_DUE,
    eFOTA_EVENT_POLL_RESPONSE,
    eFOTA_EVENT_JOB_VALIDATION_RESPONSE,
    eFOTA_EVENT_DOCUMENT_RESPONSE,
    eFOTA_EVENT_MQTT_RESULT,
    eFOTA_EVENT_HTTP_RANGE_RESULT,
    eFOTA_EVENT_SHUTDOWN_PREPARE,
    eFOTA_EVENT_INSTALLATION_COMPLETED,
    eFOTA_EVENT_INSTALLATION_FAILED,
    eFOTA_EVENT_INTERNAL_STEP
} FOTAEventType_E;

typedef enum
{
    eFOTA_ERROR_NONE = 0,
    eFOTA_ERROR_INVALID_ARGUMENT,
    eFOTA_ERROR_MQTT,
    eFOTA_ERROR_JOB_NOT_FOUND,
    eFOTA_ERROR_JOB_INVALID_STATE,
    eFOTA_ERROR_JOB_CANCELED,
    eFOTA_ERROR_DOCUMENT_MALFORMED,
    eFOTA_ERROR_DOCUMENT_INVALID,
    eFOTA_ERROR_DOCUMENT_EXPIRED,
    eFOTA_ERROR_MANIFEST,
    eFOTA_ERROR_VERSION_INCOMPATIBLE,
    eFOTA_ERROR_HTTP,
    eFOTA_ERROR_HTTP_RESPONSE,
    eFOTA_ERROR_FILESYSTEM,
    eFOTA_ERROR_STORAGE,
    eFOTA_ERROR_INSUFFICIENT_SPACE,
    eFOTA_ERROR_CANCELED,
    eFOTA_ERROR_INTERNAL
} FOTAError_E;

typedef struct
{
    FOTAStringView_T ecu_type;
    FOTAStringView_T ecu_id;
    FOTAStringView_T min_ver;
    FOTAStringView_T target_ver;
    FOTAStringView_T url;
    FOTAStringView_T compression;
    FOTAStringView_T package_id;
    uint32_t u32_SequenceId;
    uint32_t u32_PackageSize;
    uint64_t u64_UrlExpiry;
    uint32_t u32_ExpectedHash;
} FOTAPackage_T;

typedef struct
{
    FOTAStringView_T operation;
    FOTAStringView_T logs_url;
    FOTAPackage_T at_Packages[FOTA_MAX_PACKAGES];
    uint8_t u8_PackageCount;
    uint64_t u64_EarliestExpiry;
    const char *pc_RawDocument;
    uint32_t u32_RawDocumentLength;
    /* Opaque cJSON tree retained while the validated job is active. */
    void *pv_JsonRoot;
} FOTAJob_T;

typedef enum
{
    eFOTA_HTTP_RESULT_OK = 0,
    eFOTA_HTTP_RESULT_ERROR,
    eFOTA_HTTP_RESULT_CANCELED
} FOTAHttpResult_E;

typedef enum
{
    eFOTA_STORAGE_JOB_DOCUMENT = 0,
    eFOTA_STORAGE_JOB_DOCUMENT_A,
    eFOTA_STORAGE_JOB_DOCUMENT_B,
    eFOTA_STORAGE_CONTEXT_A,
    eFOTA_STORAGE_CONTEXT_B,
    eFOTA_STORAGE_RESUME,
    eFOTA_STORAGE_POLL_BUDGET,
    eFOTA_STORAGE_CURRENT_FILE
} FOTAStorageFile_E;

typedef struct
{
    uint32_t u32_Offset;
    uint32_t u32_Length;
    uint32_t u32_ActualLength;
    uint16_t u16_HttpStatus;
    uint32_t u32_ContentRangeStart;
    uint32_t u32_ContentRangeEnd;
    uint32_t u32_ContentRangeTotal;
    FOTAHttpResult_E e_Result;
} FOTAHttpRangeResult_T;

typedef struct
{
    FOTAEventType_E e_Type;
    bool b_Accepted;
    FOTAError_E e_Error;
    uint32_t u32_Correlation;
    const char *pc_Payload;
    uint32_t u32_PayloadLength;
    FOTAHttpRangeResult_T st_Http;
    FOTAStringView_T job_id;
    FOTAStringView_T package_id;
} FOTAEvent_T;

typedef struct
{
    bool (*get_time)(uint64_t *pu64_EpochSeconds, uint32_t *pu32_CalendarDay);
    bool (*app_monitor_acquire)(void);
    void (*app_monitor_release)(void);

    /* MQTT adapters create on-demand QoS-1 sessions. Poll/document callbacks
     * complete only after their application response; status callbacks return
     * after broker PUBACK and must not wait for AWS update/accepted. */
    bool (*mqtt_poll_next)(void);
    bool (*mqtt_validate_job)(FOTAStringView_T job_id);
    bool (*mqtt_request_document)(FOTAStringView_T job_id);
    bool (*mqtt_publish_status)(FOTAStringView_T job_id,
                                const char *pc_Payload,
                                uint32_t u32_Length);

    bool (*https_get_range)(FOTAStringView_T url,
                            uint32_t u32_Start,
                            uint32_t u32_End,
                            uint8_t *pu8_Buffer,
                            uint32_t u32_BufferLength);

    bool (*manifest_get_version)(FOTAStringView_T ecu_type,
                                 FOTAStringView_T ecu_id,
                                 char *pc_Buffer,
                                 uint16_t u16_BufferLength,
                                 uint16_t *pu16_VersionLength);

    bool (*storage_read)(FOTAStorageFile_E e_File,
                         uint32_t u32_Offset,
                         void *pv_Buffer,
                         uint32_t u32_Length,
                         uint32_t *pu32_Actual);
    bool (*storage_write)(FOTAStorageFile_E e_File,
                          uint32_t u32_Offset,
                          const void *cvp_Buffer,
                          uint32_t u32_Length);
    bool (*storage_delete)(FOTAStorageFile_E e_File);
    bool (*storage_exists)(FOTAStorageFile_E e_File);
    uint32_t (*storage_size)(FOTAStorageFile_E e_File);
    uint32_t (*storage_free_bytes)(void);

    /* Reserved for the Part B/production hash implementation. Part A keeps
     * the expected 32-bit value but deliberately bypasses this callback. */
    bool (*crypto_verify)(uint32_t u32_ExpectedHash,
                          FOTAStorageFile_E e_File,
                          uint32_t u32_FileSize);

    bool (*installer_handoff)(FOTAStringView_T job_id,
                              const FOTAPackage_T *cpt_Package,
                              FOTAStorageFile_E e_File);
} FOTAPlatformOps_T;

typedef struct
{
    FOTAState_E e_State;
    uint8_t u8_CurrentPackage;
    uint32_t u32_DownloadedBytes;
    uint32_t u32_PackageSize;
    uint8_t u8_PollCount;
    uint32_t u32_PollCalendarDay;
    uint64_t u64_LastPollEpoch;
    FOTAError_E e_LastError;
} FOTAStatus_T;

#endif /* FOTA_TYPES_H */
