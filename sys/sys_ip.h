#ifndef  _SYS_IP_ENV_H_H_
#define  _SYS_IP_ENV_H_H_
#include "wv_common.h"  
WV_S32 SYS_IP_Init();
WV_S32 SYS_IP_Deint(); 
WV_S32 SYS_IP_SetMac(WV_S8 * pMac);
WV_S32 SYS_IP_SetIp(WV_S8 * pIp);
WV_S32 SYS_IP_SetMask(WV_S8 * pMsk);
WV_S32 SYS_IP_SetGw(WV_S8 * pGw);
WV_S32 SYS_IP_GetMac(WV_S8 * pMac);
WV_S32 SYS_IP_GetIp(WV_S8 * pIp);
WV_S32 SYS_IP_GetMask(WV_S8 * pMsk);
WV_S32 SYS_IP_GetGw(WV_S8 * pGw);
WV_S32 SYS_IP_SaveConf();
WV_S32 SYS_IP_SaveUpdate();
WV_S32 SYS_IP_getIpInt(WV_S8 *pName,WV_U8* pIp);
WV_S32 SYS_IP_getMacInt(WV_U8* pMac);
#endif
