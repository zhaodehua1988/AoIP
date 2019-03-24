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
WV_S32 SYS_IP_SwitchChar(WV_S8 *pStr,WV_S32 *pValue);

/**********************************************************************
 * WV_S32 SYS_IP_SetETH(WV_S8 *pIp,WV_S8 *pMask,WV_S8 *pGetWay)
 * 设置ip，网关等
 * ********************************************************************/
WV_S32 SYS_IP_SetEthIpConf(WV_S8 *pIp,WV_S8 *pMask,WV_S8 *pGetWay);

/**********************************************************************
 * WV_S32 SYS_IP_SetETH(WV_S8 *pIp,WV_S8 *pMask,WV_S8 *pGetWay)
 * 设置mac地址，因为mac地址不经常改变，所以mac地址单独设置，跟ip网关掩码分开
 * ********************************************************************/
WV_S32 SYS_IP_SetEthMacConf(WV_S8 *pMac);
#endif
