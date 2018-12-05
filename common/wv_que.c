#include"wv_que.h"


/***********************************************************

WV_S32  WV_QUE_Create(WV_QUE_DEV_E *pQue,WV_U32 maxLen);

***********************************************************/

WV_S32  WV_QUE_Create(WV_QUE_DEV_E *pQue,WV_U32 maxLen)
{
  WV_S32 ret  = WV_SOK;

  pthread_mutexattr_t mutex_attr;
  pthread_condattr_t cond_attr;
  

  pQue->curRd = pQue->curWr = 0;
  pQue->count = 0;
  pQue->len   = maxLen;
  pQue->queue = (WV_U32 *)malloc(sizeof(WV_U32) * pQue->len);
  
  if(pQue->queue==NULL) {
    WV_ERROR("WV_QUE_Create()\r\n");
    return WV_EFAIL;
  }
 
  ret |= pthread_mutexattr_init(&mutex_attr);
  ret |= pthread_condattr_init(&cond_attr);  
  
  ret |= pthread_mutex_init(&pQue->lock, &mutex_attr);
  ret |= pthread_cond_init(&pQue->condRd, &cond_attr);    
  ret |= pthread_cond_init(&pQue->condWr, &cond_attr);  

  if(ret!=WV_SOK)
    WV_ERROR("WV_QUE_Create() = %d \r\n", ret);
    
  pthread_condattr_destroy(&cond_attr);
  pthread_mutexattr_destroy(&mutex_attr);
    
  return ret;
}

/***********************************************************

WV_S32  WV_QUE_Destroy(WV_QUE_DEV_E *pQue);

***********************************************************/

WV_S32  WV_QUE_Destroy(WV_QUE_DEV_E *pQue)
{

  if(pQue->queue!=NULL)
    free(pQue->queue);
    
  pthread_cond_destroy(&pQue->condRd);
  pthread_cond_destroy(&pQue->condWr);
  pthread_mutex_destroy(&pQue->lock);  
  
  return WV_SOK;
}

/***********************************************************

WV_S32  WV_QUE_Put(WV_QUE_DEV_E *pQue,WV_U32 value,WV_S32 timeout);

***********************************************************/

WV_S32  WV_QUE_Put(WV_QUE_DEV_E *pQue,WV_U32 value,WV_S32 timeout )
{

   WV_S32  ret  = WV_EFAIL;

  pthread_mutex_lock(&pQue->lock);

  while(1) 
	  {
			if( pQue->count < pQue->len ) {
				  pQue->queue[pQue->curWr] = value;
				  pQue->curWr = (pQue->curWr+1)%pQue->len;
				  pQue->count++;
				  ret  = WV_SOK;
				  //pthread_cond_signal(&pQue->condRd);
				  break;
			} else {
				  if(timeout == WV_QUE_TIME_NONE)
					break;

				  //ret = pthread_cond_wait(&pQue->condWr, &pQue->lock);
			}
			break;
	  }

  pthread_mutex_unlock(&pQue->lock);

 
  return ret;
}



/***********************************************************

WV_S32  WV_QUE_Get(WV_QUE_DEV_E *pQue,WV_U32 *pValue,WV_S32 timeout);

***********************************************************/

WV_S32  WV_QUE_Get(WV_QUE_DEV_E *pQue,WV_U32 *pValue,WV_S32 timeout )
{

   WV_S32  ret  = WV_EFAIL;

  pthread_mutex_lock(&pQue->lock);

	while(1) 
	{
		if(pQue->count > 0 ) {

		  if(pValue!=NULL) {
			*pValue = pQue->queue[pQue->curRd];
		  }
		  
		  pQue->curRd = (pQue->curRd+1) % pQue->len;
		  pQue->count--;
		  ret = WV_SOK;
		  //pthread_cond_signal(&pQue->condWr);
		  break;
		} else {
		  if(timeout == WV_QUE_TIME_NONE)
			break;

		  //ret = pthread_cond_wait(&pQue->condRd, &pQue->lock);
		}
		break;
	}
  pthread_mutex_unlock(&pQue->lock);

 
  return ret;
}
