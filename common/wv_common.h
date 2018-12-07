#ifndef _WV_COMMOM_COMMON_H_
#define _WV_COMMOM_COMMON_H_

#include "wv_type.h"
#include "wv_string.h"
#include "wv_thr.h"
#include "wv_cmd.h"
#include "wv_file.h"
#include "wv_conf.h"
#include "wv_ip.h"
#include "wv_telnet.h"

#define WV_DEBUG_MODE 


#ifdef  WV_DEBUG_MODE
	#define WV_RET_ADD(func,ret)\
		do{\
		    WV_S32 ret_b;\
		    ret_b = func;\
		    if (WV_SOK != ret_b)\
		    {\
		        printf("\033[0;31mfailed at %s: LINE: %d with %#x!\033[0;39m\n",__FUNCTION__, __LINE__, ret_b);\
		        ret ++;\
		    }\
		    }while(0)
#else
	#define WV_RET_ADD(func,ret)\
		do{\
		    WV_S32 ret_b;\
		    ret_b = func;\
		    if (WV_SOK != ret_b)\
		    {\
		        ret ++;\
		    }\
		    }while(0)  
#endif
 
#ifdef  WV_DEBUG_MODE
	#define WV_ASSERT_RET(func)\
		do{\
		    WV_S32 ret;\
		    ret = func;\
		    if (WV_SOK != ret)\
		    {\
		        printf("\033[0;31mfailed at %s: LINE: %d with %#x!\033[0;39m\n",__FUNCTION__, __LINE__, ret);\
		    }\
		}while(0)
#else
	#define WV_ASSERT_RET(func)  
#endif

#ifdef  WV_DEBUG_MODE
	#define WV_CHECK_RET(func)\
		do{\
		    WV_S32 ret;\
		    ret = func;\
		    if (WV_SOK != ret)\
		    {\
		        printf("\033[0;31mfailed at %s: LINE: %d with %#x!\033[0;39m\n",__FUNCTION__, __LINE__, ret);\
		        return ret;\
		    }\
		}while(0)
#else
	#define WV_CHECK_RET(func)\
		do{\
		    WV_S32 ret;\
		    ret = func;\
		    if (WV_SOK != ret)\
		    {\
		        return ret;\
		    }\
		}while(0)
#endif		


#ifdef  WV_DEBUG_MODE
	#define WV_CHECK_FAIL(func)\
		do{\
		    WV_S32 ret;\
		    ret = func;\
		    if (WV_EFAIL == ret)\
		    {\
		        printf("\033[0;31mfailed at %s: LINE: %d with %#x!\033[0;39m\n",__FUNCTION__, __LINE__, ret);\
		        return ret;\
		    }\
		}while(0)
#else
	#define WV_CHECK_FAIL(func)\
		do{\
		    WV_S32 ret;\
		    ret = func;\
		    if (WV_EFAIL == ret)\
		    {\
		        return ret;\
		    }\
		}while(0)
#endif		
    
    
// printf wrapper that can be turned on and off by defining/undefining


#ifdef  WV_DEBUG_MODE 
  #define WV_printf(...)  do {printf(__VA_ARGS__); fflush(stdout); } while(0)
#else 
  #define WV_printf(...) 
#endif  


#ifdef  WV_DEBUG_MODE 
  #define WV_ERROR(...)  do { printf("\033[0;31m");printf("[%s]-%d:",__FUNCTION__, __LINE__);printf(__VA_ARGS__); printf("\033[0;39m");fflush(stdout); } while(0)
#else 
  #define WV_ERROR(...) 
#endif 


#define WV_COMM_UDPRX_BUF_MAXLEN	 200*2048
#define WV_COMM_UDPTX_BUF_MAXLEN	 200*2048

#define WV_COMM_CMDPRT_MAXLEN	 	 100*2048
#define WV_COMM_ENVPASS_MAXLEN	 	 16

#ifndef WV_COMM_CMD_TITLE
	#define WV_COMM_CMD_TITLE 		  	"HELLO"
#endif

 
#endif /* _COMMOM_COMMOM_H_*/
