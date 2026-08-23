#ifndef FOTA_JOB_PARSER_H
#define FOTA_JOB_PARSER_H

#include "FOTATypes.h"

typedef enum
{
    eFOTA_PARSE_OK = 0,
    eFOTA_PARSE_INVALID_ARGUMENT,
    eFOTA_PARSE_MALFORMED,
    eFOTA_PARSE_INVALID_DOCUMENT,
    eFOTA_PARSE_EXPIRED
} FOTAParseResult_E;

FOTAParseResult_E ge_FOTA_ParseJobDocument(const char *ccpt_Json,
                                           uint32_t u32_Length,
                                           uint64_t u64_Now,
                                           FOTAJob_T *pt_Job);

/*
 * A successful parse retains the cJSON tree because the validated field views
 * point into cJSON-owned strings. Release it before replacing or discarding
 * the job.
 */
void gv_FOTA_ReleaseJob(FOTAJob_T *pt_Job);

bool gb_FOTA_StringEquals(FOTAStringView_T st_Value, const char *ccpt_Literal);
int32_t gi32_FOTA_StringCompare(FOTAStringView_T st_Left,
                                FOTAStringView_T st_Right);

#endif /* FOTA_JOB_PARSER_H */
