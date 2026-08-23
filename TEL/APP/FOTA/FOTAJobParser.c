#include "FOTAJobParser.h"

#include <ctype.h>
#include <string.h>

#include "cJSON.h"

static bool sb_FOTA_GetString(const cJSON *cpt_Object,
                              const char *ccpt_Key,
                              FOTAStringView_T *pt_View)
{
    cJSON *pt_Item;
    size_t z_Length;
    if ((cpt_Object == NULL) || (ccpt_Key == NULL) || (pt_View == NULL))
    {
        return false;
    }
    pt_Item = cJSON_GetObjectItemCaseSensitive(cpt_Object, ccpt_Key);
    if (!cJSON_IsString(pt_Item) || (pt_Item->valuestring == NULL))
    {
        return false;
    }
    z_Length = strlen(pt_Item->valuestring);
    if (z_Length > UINT16_MAX)
    {
        return false;
    }
    pt_View->pc_Data = pt_Item->valuestring;
    pt_View->u16_Length = (uint16_t)z_Length;
    return true;
}

static bool sb_FOTA_ParseDecimal(FOTAStringView_T st_Value,
                                 uint64_t u64_Max,
                                 uint64_t *pu64_Result)
{
    uint16_t u16_Index;
    uint64_t u64_Value = 0U;
    if ((st_Value.pc_Data == NULL) || (st_Value.u16_Length == 0U) ||
        (pu64_Result == NULL))
    {
        return false;
    }
    for (u16_Index = 0U; u16_Index < st_Value.u16_Length; u16_Index++)
    {
        uint8_t u8_Digit;
        if ((st_Value.pc_Data[u16_Index] < '0') ||
            (st_Value.pc_Data[u16_Index] > '9'))
        {
            return false;
        }
        u8_Digit = (uint8_t)(st_Value.pc_Data[u16_Index] - '0');
        if (u64_Value > ((u64_Max - u8_Digit) / 10U))
        {
            return false;
        }
        u64_Value = (u64_Value * 10U) + u8_Digit;
    }
    *pu64_Result = u64_Value;
    return true;
}

static bool sb_FOTA_ParsePackage(const cJSON *cpt_Object,
                                 FOTAPackage_T *pt_Package)
{
    FOTAStringView_T st_Number = {0};
    uint64_t u64_Value;
    if ((cpt_Object == NULL) || !cJSON_IsObject(cpt_Object) ||
        (pt_Package == NULL) ||
        !sb_FOTA_GetString(cpt_Object, "ecu_type", &pt_Package->ecu_type) ||
        !sb_FOTA_GetString(cpt_Object, "ecu_id", &pt_Package->ecu_id) ||
        !sb_FOTA_GetString(cpt_Object, "min_ver", &pt_Package->min_ver) ||
        !sb_FOTA_GetString(cpt_Object, "target_ver", &pt_Package->target_ver) ||
        !sb_FOTA_GetString(cpt_Object, "url", &pt_Package->url) ||
        !sb_FOTA_GetString(cpt_Object, "url_expiry", &st_Number) ||
        !sb_FOTA_ParseDecimal(st_Number, UINT64_MAX, &u64_Value))
    {
        return false;
    }
    pt_Package->u64_UrlExpiry = u64_Value;
    if (!sb_FOTA_GetString(cpt_Object, "compression",
                           &pt_Package->compression) ||
        !gb_FOTA_StringEquals(pt_Package->compression, "none") ||
        !sb_FOTA_GetString(cpt_Object, "hash", &st_Number) ||
        !sb_FOTA_ParseDecimal(st_Number, UINT32_MAX, &u64_Value))
    {
        return false;
    }
    pt_Package->u32_ExpectedHash = (uint32_t)u64_Value;
    if (!sb_FOTA_GetString(cpt_Object, "package_size", &st_Number) ||
        !sb_FOTA_ParseDecimal(st_Number, UINT32_MAX, &u64_Value) ||
        (u64_Value == 0U))
    {
        return false;
    }
    pt_Package->u32_PackageSize = (uint32_t)u64_Value;
    if (!sb_FOTA_GetString(cpt_Object, "package_id",
                           &pt_Package->package_id) ||
        (pt_Package->ecu_type.u16_Length == 0U) ||
        (pt_Package->ecu_id.u16_Length == 0U) ||
        (pt_Package->min_ver.u16_Length == 0U) ||
        (pt_Package->target_ver.u16_Length == 0U) ||
        (pt_Package->package_id.u16_Length == 0U) ||
        (pt_Package->url.u16_Length < 8U) ||
        (memcmp(pt_Package->url.pc_Data, "https://", 8U) != 0) ||
        !sb_FOTA_GetString(cpt_Object, "seq_id", &st_Number) ||
        !sb_FOTA_ParseDecimal(st_Number, UINT32_MAX, &u64_Value) ||
        (u64_Value == 0U))
    {
        return false;
    }
    pt_Package->u32_SequenceId = (uint32_t)u64_Value;
    return true;
}

static bool sb_FOTA_ParsePackages(const cJSON *cpt_Array,
                                  FOTAPackage_T *at_Packages,
                                  uint8_t *pu8_Count)
{
    int i32_Count;
    int i32_Index;
    if (!cJSON_IsArray(cpt_Array) || (at_Packages == NULL) ||
        (pu8_Count == NULL))
    {
        return false;
    }
    i32_Count = cJSON_GetArraySize(cpt_Array);
    if ((i32_Count <= 0) ||
        (i32_Count > (int)FOTA_MAX_PACKAGES))
    {
        return false;
    }
    for (i32_Index = 0; i32_Index < i32_Count; i32_Index++)
    {
        if (!sb_FOTA_ParsePackage(cJSON_GetArrayItem(cpt_Array, i32_Index),
                                  &at_Packages[i32_Index]))
        {
            return false;
        }
    }
    *pu8_Count = (uint8_t)i32_Count;
    return true;
}

static bool sb_FOTA_ValidateAndSort(FOTAJob_T *pt_Job)
{
    uint8_t u8_Index;
    for (u8_Index = 0U; u8_Index < pt_Job->u8_PackageCount; u8_Index++)
    {
        uint8_t u8_Inner;
        for (u8_Inner = (uint8_t)(u8_Index + 1U);
             u8_Inner < pt_Job->u8_PackageCount;
             u8_Inner++)
        {
            if (pt_Job->at_Packages[u8_Index].u32_SequenceId ==
                pt_Job->at_Packages[u8_Inner].u32_SequenceId)
            {
                return false;
            }
            if (pt_Job->at_Packages[u8_Inner].u32_SequenceId <
                pt_Job->at_Packages[u8_Index].u32_SequenceId)
            {
                FOTAPackage_T st_Temp = pt_Job->at_Packages[u8_Index];
                pt_Job->at_Packages[u8_Index] = pt_Job->at_Packages[u8_Inner];
                pt_Job->at_Packages[u8_Inner] = st_Temp;
            }
        }
    }
    for (u8_Index = 0U; u8_Index < pt_Job->u8_PackageCount; u8_Index++)
    {
        if (pt_Job->at_Packages[u8_Index].u32_SequenceId !=
            ((uint32_t)u8_Index + 1U))
        {
            return false;
        }
    }
    return true;
}

static const char *sccpt_FOTA_SkipWhitespace(const char *pc_Data,
                                             const char *ccpt_End)
{
    while ((pc_Data < ccpt_End) &&
           (isspace((unsigned char)*pc_Data) != 0))
    {
        pc_Data++;
    }
    return pc_Data;
}

bool gb_FOTA_StringEquals(FOTAStringView_T st_Value, const char *ccpt_Literal)
{
    size_t z_Length;
    if ((ccpt_Literal == NULL) || (st_Value.pc_Data == NULL))
    {
        return false;
    }
    z_Length = strlen(ccpt_Literal);
    return (z_Length == st_Value.u16_Length) &&
           (memcmp(st_Value.pc_Data, ccpt_Literal, z_Length) == 0);
}

int32_t gi32_FOTA_StringCompare(FOTAStringView_T st_Left,
                                FOTAStringView_T st_Right)
{
    uint16_t u16_Common = (st_Left.u16_Length < st_Right.u16_Length) ?
                          st_Left.u16_Length : st_Right.u16_Length;
    int32_t i32_Result = memcmp(st_Left.pc_Data, st_Right.pc_Data,
                                u16_Common);
    if (i32_Result == 0)
    {
        i32_Result = (st_Left.u16_Length > st_Right.u16_Length) ? 1 :
                     (st_Left.u16_Length < st_Right.u16_Length) ? -1 : 0;
    }
    return i32_Result;
}

FOTAParseResult_E ge_FOTA_ParseJobDocument(const char *ccpt_Json,
                                           uint32_t u32_Length,
                                           uint64_t u64_Now,
                                           FOTAJob_T *pt_Job)
{
    cJSON *pt_Root;
    cJSON *pt_Document;
    cJSON *pt_Files;
    const char *pc_ParseEnd = NULL;
    const char *pc_End;
    FOTAJob_T st_Parsed = {0};
    uint8_t u8_Index;

    if ((ccpt_Json == NULL) || (pt_Job == NULL) ||
        (u32_Length == 0U) ||
        (u32_Length > FOTA_MAX_JOB_DOCUMENT_BYTES))
    {
        return eFOTA_PARSE_INVALID_ARGUMENT;
    }
    pc_End = ccpt_Json + u32_Length;
    pt_Root = cJSON_ParseWithLengthOpts(ccpt_Json, u32_Length,
                                        &pc_ParseEnd, false);
    if (pt_Root == NULL)
    {
        return eFOTA_PARSE_MALFORMED;
    }
    if ((pc_ParseEnd == NULL) ||
        (sccpt_FOTA_SkipWhitespace(pc_ParseEnd, pc_End) != pc_End) ||
        !cJSON_IsObject(pt_Root))
    {
        cJSON_Delete(pt_Root);
        return eFOTA_PARSE_MALFORMED;
    }
    pt_Document = cJSON_GetObjectItemCaseSensitive(pt_Root, "jobDocument");
    pt_Files = cJSON_IsObject(pt_Document) ?
               cJSON_GetObjectItemCaseSensitive(pt_Document, "files") : NULL;
    if (!cJSON_IsObject(pt_Document) ||
        !sb_FOTA_GetString(pt_Document, "operation", &st_Parsed.operation) ||
        !gb_FOTA_StringEquals(st_Parsed.operation, "fota") ||
        !sb_FOTA_GetString(pt_Document, "logs_url", &st_Parsed.logs_url) ||
        !sb_FOTA_ParsePackages(pt_Files, st_Parsed.at_Packages,
                               &st_Parsed.u8_PackageCount) ||
        !sb_FOTA_ValidateAndSort(&st_Parsed))
    {
        cJSON_Delete(pt_Root);
        return eFOTA_PARSE_INVALID_DOCUMENT;
    }
    st_Parsed.u64_EarliestExpiry = UINT64_MAX;
    for (u8_Index = 0U; u8_Index < st_Parsed.u8_PackageCount; u8_Index++)
    {
        if (st_Parsed.at_Packages[u8_Index].u64_UrlExpiry <
            st_Parsed.u64_EarliestExpiry)
        {
            st_Parsed.u64_EarliestExpiry =
                st_Parsed.at_Packages[u8_Index].u64_UrlExpiry;
        }
    }
    if ((u64_Now != 0U) && (u64_Now >= st_Parsed.u64_EarliestExpiry))
    {
        cJSON_Delete(pt_Root);
        return eFOTA_PARSE_EXPIRED;
    }
    st_Parsed.pc_RawDocument = ccpt_Json;
    st_Parsed.u32_RawDocumentLength = u32_Length;
    st_Parsed.pv_JsonRoot = pt_Root;
    *pt_Job = st_Parsed;
    return eFOTA_PARSE_OK;
}

void gv_FOTA_ReleaseJob(FOTAJob_T *pt_Job)
{
    if (pt_Job != NULL)
    {
        if (pt_Job->pv_JsonRoot != NULL)
        {
            cJSON_Delete((cJSON *)pt_Job->pv_JsonRoot);
        }
        memset(pt_Job, 0, sizeof(*pt_Job));
    }
}
