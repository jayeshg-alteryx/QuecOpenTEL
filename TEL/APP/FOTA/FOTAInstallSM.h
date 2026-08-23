#ifndef FOTA_INSTALL_SM_H
#define FOTA_INSTALL_SM_H

#include "FOTATypes.h"

/* Part B owns the implementation. Part A only publishes this handoff shape. */
typedef struct
{
    FOTAStringView_T job_id;
    FOTAStringView_T package_id;
    FOTAStringView_T ecu_type;
    FOTAStringView_T ecu_id;
    uint32_t u32_SequenceId;
    uint32_t u32_PackageSize;
    FOTAStorageFile_E e_File;
} FOTAInstallRequest_T;

typedef enum
{
    eFOTA_INSTALL_RESULT_COMPLETED = 0,
    eFOTA_INSTALL_RESULT_FAILED
} FOTAInstallResult_E;

#endif /* FOTA_INSTALL_SM_H */
