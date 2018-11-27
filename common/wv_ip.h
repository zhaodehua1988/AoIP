#ifndef _WV_COMMOM_IP_H_
#define _WV_COMMOM_IP_H_
#include "wv_common.h"
WV_S32 WV_IP_Set(WV_S8 * pName ,WV_S8 * pMac ,WV_S8 * pIp ,WV_S8 * pMask ,WV_S8 * pGway);
WV_S32 WV_IP_Get(WV_S8 * pName ,WV_S8 * pMac ,WV_S8 * pIp );
WV_S32 WV_IP_CheckIp(WV_S8 * pIp); 
WV_S32 WV_IP_CheckMac(WV_S8 * pMac);
#endif
