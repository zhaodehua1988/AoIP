#ifndef _WV_COMM_THR_H_
#define _WV_COMM_THR_H_
#include "wv_type.h"


#define WV_THR_PRI_MAX                 sched_get_priority_max(SCHED_FIFO)
#define WV_THR_PRI_MIN                 sched_get_priority_min(SCHED_FIFO)

#define WV_THR_PRI_DEFAULT             (WV_THR_PRI_MIN + (WV_THR_PRI_MAX-WV_THR_PRI_MIN)/2 )

#define WV_THR_STACK_SIZE_DEFAULT      0

typedef void * (*WV_THR_FUN_T)(void *);

typedef struct WV_THR_HNDL_T
{
  pthread_t      hndl;  
} WV_THR_HNDL_T;

WV_S32 WV_THR_Create(WV_THR_HNDL_T *hndl, WV_THR_FUN_T func, WV_U32 pri, WV_U32 stackSize, void *prm); 
WV_S32 WV_THR_Destroy(WV_THR_HNDL_T *hndl); 
#endif 
