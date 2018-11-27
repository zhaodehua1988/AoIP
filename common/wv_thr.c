#include "wv_thr.h"
#include "wv_common.h"
/*******************************************************************************************
WV_S32  WV_THR_Create(WV_THR_HNDL_T *hndl, WV_THR_FUN_T func, WV_U32 pri, WV_U32 stackSize, void *prm); 
*******************************************************************************************/
WV_S32 WV_THR_Create(WV_THR_HNDL_T *hndl, WV_THR_FUN_T func, WV_U32 pri, WV_U32 stackSize, void *prm)
{
  WV_S32 ret=WV_SOK;
  pthread_attr_t threadAttr;
  struct sched_param schedPrm;

  // initialize thread attributes structure
  ret = pthread_attr_init(&threadAttr); 
  if(ret!=WV_SOK) {
     WV_printf("Could not initialize thread attributes\n");
    return ret;
  }
  
  if(stackSize!=WV_THR_STACK_SIZE_DEFAULT)
    pthread_attr_setstacksize(&threadAttr, stackSize);

  ret |= pthread_attr_setinheritsched(&threadAttr, PTHREAD_EXPLICIT_SCHED);
  ret |= pthread_attr_setschedpolicy(&threadAttr, SCHED_FIFO);
    
  if(pri>WV_THR_PRI_MAX)   
    pri=WV_THR_PRI_MAX;
  else 
  if(pri<WV_THR_PRI_MIN)   
    pri=WV_THR_PRI_MIN;
    
  schedPrm.sched_priority = pri;
  ret |= pthread_attr_setschedparam(&threadAttr, &schedPrm);

  if(ret!=WV_SOK) {
    WV_printf("Could not initialize thread attributes\n");
    goto error_exit;
  }

  ret = pthread_create(&hndl->hndl, &threadAttr, func, prm);
  
  if(ret != WV_SOK) {
     WV_printf("Could not create thread [%d]\n", ret);
  }

error_exit:  
  pthread_attr_destroy(&threadAttr);

  return ret;
}


/*******************************************************************************************
WV_S32 WV_THR_Destroy(WV_THR_HNDL_T *hndl);
*******************************************************************************************/
WV_S32 WV_THR_Destroy(WV_THR_HNDL_T *hndl)
{
   WV_S32 ret = WV_SOK;
    void *returnVal;
    
  if(&hndl->hndl != NULL)
  {
      ret = pthread_cancel(hndl->hndl); 
 	  ret = pthread_join(hndl->hndl, &returnVal); 
  }
  
  return ret;
}

