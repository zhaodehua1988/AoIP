#ifndef _WV_QUE_QUE_H_
#define _WV_QUE_QUE_H_
#include "wv_common.h" 

#define WV_QUE_TIME_NONE    0
typedef struct   WV_QUE_DEV_E
{

  WV_U32 curRd;
  WV_U32 curWr;
  WV_U32 len;
  WV_U32  count;

  WV_U32 *queue;

  pthread_mutex_t lock;
  pthread_cond_t  condRd;
  pthread_cond_t  condWr;
  
} WV_QUE_DEV_E; 

WV_S32  WV_QUE_Create(WV_QUE_DEV_E *pQue,WV_U32 maxLen);
WV_S32  WV_QUE_Destroy(WV_QUE_DEV_E *pQue);
WV_S32  WV_QUE_Get(WV_QUE_DEV_E *pQue,WV_U32 *pValue,WV_S32 timeout );
WV_S32  WV_QUE_Put(WV_QUE_DEV_E *pQue,WV_U32  value,WV_S32 timeout);
#endif
