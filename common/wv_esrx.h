#ifndef _WV_ESRX_PROC_H_
#define _WV_ESRX_PROC_H_
#include "wv_common.h"
#define WV_ESRX_PAG_LOAD_LEN                1316
#define WV_ESRX_PAG_HEAD_LEN                12
#define WV_ESRX_FRAME_HEAD_LEN              12
#define WV_ESRX_PAG_LEN               WV_ESRX_PAG_LOAD_LEN + WV_ESRX_PAG_HEAD_LEN

typedef struct  WV_ESRX_PACK_INFO_S
{
WV_U8 	sync; 
WV_U8 	endFlag; 
WV_U8 	cc; 
WV_U16 	pid;
WV_U32 	loadOffset;
WV_U32 	loadLen;
WV_S8   *pLoad;
WV_U64   pts;
}WV_ESRX_PACK_INFO_S;


WV_S32  WV_ESRX_Analyze(WV_S8 *pBuf,WV_ESRX_PACK_INFO_S *pInfo,WV_S32 len); 
#endif
