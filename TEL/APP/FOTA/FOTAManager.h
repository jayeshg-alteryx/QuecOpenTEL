#ifndef FOTA_MANAGER_H
#define FOTA_MANAGER_H

#include "FOTA_SM.h"

TelSysResponse_E ge_FOTA_Init(const FOTAPlatformOps_T *cpt_Ops);
TelSysResponse_E ge_FOTA_PostEvent(const FOTAEvent_T *cpt_Event);
FOTAState_E ge_FOTA_GetState(void);
void gv_FOTA_GetStatus(FOTAStatus_T *pt_Status);

#endif /* FOTA_MANAGER_H */
