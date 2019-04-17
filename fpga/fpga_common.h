#ifndef _FPGA_COMMON_H_H_
#define _FPGA_COMMON_H_H_

#include "wv_common.h"
//#define FPGA_DEBUG

#ifdef FPGA_DEBUG
#define FPGA_printf(...)                 \
    do                                   \
    {                                    \
        printf("\n\rFPGA:" __VA_ARGS__); \
        fflush(stdout);                  \
    } while (0)
#else
#define FPGA_printf(...)
#endif


/*******************************************************************
WV_S32 FPGA_COMMON_getIpInt(WV_S8 *pName,WV_U8* pIp);
*******************************************************************/
WV_S32 FPGA_COMMON_getIpInt(WV_S8 *pSrc, WV_U8 *pIp);

/*******************************************************************
WV_S32 FPGA_COMMON_getMacInt(WV_S8 *pSrc, WV_U8 *pMac);
*******************************************************************/
WV_S32 FPGA_COMMON_getMacInt(WV_S8 *pSrc, WV_U8 *pMac);

/**********************************************************
 * WV_S32 strstr_cnt(WV_S8  *string, WV_S8 *substring)
 * 查询某个字符串在另外一个字符串中出现的次数
**********************************************************/
WV_S32 strstr_cnt(WV_S8  *string, WV_S8 *substring);

#endif