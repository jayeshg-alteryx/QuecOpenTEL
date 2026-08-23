#include "FOTAManager.h"

#include <string.h>

#include "Generics.h"
#include "ql_api_osi.h"

#define FOTA_MANAGER_QUEUE_LENGTH       (2U)
#define FOTA_MANAGER_TASK_STACK_SIZE    (1536U)
#define FOTA_MANAGER_TASK_PRIORITY      APP_PRIORITY_NORMAL
#define FOTA_MANAGER_TASK_EVENT_COUNT   (8U)

typedef struct
{
    ql_queue_t pt_Queue;
    ql_task_t pt_Task;
    FOTASMContext_T st_SM;
    bool b_Initialized;
} FOTAManagerContext_T;

static FOTAManagerContext_T sst_FOTA;

static void gv_FOTA_Task(void *pv_Argument)
{
    FOTAEvent_T st_Event;
    BAL_UNUSED(pv_Argument);
    for (;;)
    {
        if (ql_rtos_queue_wait(sst_FOTA.pt_Queue,
                               (uint8_t *)&st_Event,
                               sizeof(st_Event),
                               QL_WAIT_FOREVER) == QL_OSI_SUCCESS)
        {
            (void)gb_FOTA_SM_HandleEvent(&sst_FOTA.st_SM, &st_Event);
        }
    }
}

TelSysResponse_E ge_FOTA_Init(const FOTAPlatformOps_T *cpt_Ops)
{
    if (sst_FOTA.b_Initialized)
    {
        return eTEL_OK;
    }
    memset(&sst_FOTA, 0, sizeof(sst_FOTA));
    if (ql_rtos_queue_create(&sst_FOTA.pt_Queue,
                             sizeof(FOTAEvent_T),
                             FOTA_MANAGER_QUEUE_LENGTH) != QL_OSI_SUCCESS)
    {
        return eTEL_NOK;
    }
    (void)ge_Common_RegisterIPCQueue(eIPC_NODE_APP_MODEM_FOTA,
                                     sst_FOTA.pt_Queue);
    /* Recovery may submit an asynchronous AWS-execution validation request;
     * make the queue available before invoking the platform callbacks. */
    sst_FOTA.b_Initialized = true;
    gv_FOTA_SM_Init(&sst_FOTA.st_SM, cpt_Ops);
    if (ql_rtos_task_create(&sst_FOTA.pt_Task,
                            FOTA_MANAGER_TASK_STACK_SIZE,
                            FOTA_MANAGER_TASK_PRIORITY,
                            "BALFOTA",
                            gv_FOTA_Task,
                            NULL,
                            FOTA_MANAGER_TASK_EVENT_COUNT) != QL_OSI_SUCCESS)
    {
        sst_FOTA.b_Initialized = false;
        return eTEL_NOK;
    }
    return eTEL_OK;
}

TelSysResponse_E ge_FOTA_PostEvent(const FOTAEvent_T *cpt_Event)
{
    if (!sst_FOTA.b_Initialized || (cpt_Event == NULL))
    {
        return eTEL_NOK;
    }
    return (ql_rtos_queue_release(sst_FOTA.pt_Queue,
                                  sizeof(FOTAEvent_T),
                                  (uint8_t *)cpt_Event,
                                  QL_WAIT_FOREVER) == QL_OSI_SUCCESS) ?
           eTEL_OK : eTEL_NOK;
}

FOTAState_E ge_FOTA_GetState(void)
{
    return ge_FOTA_SM_GetState(&sst_FOTA.st_SM);
}

void gv_FOTA_GetStatus(FOTAStatus_T *pt_Status)
{
    gv_FOTA_SM_GetStatus(&sst_FOTA.st_SM, pt_Status);
}
