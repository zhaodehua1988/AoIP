#include "fpga_common.h"
#include "sys_ip.h"

/*******************************************************************
WV_S32 FPGA_COMMON_getIpInt(WV_S8 *pName,WV_U8* pIp);
*******************************************************************/
WV_S32 FPGA_COMMON_getIpInt(WV_S8 *pSrc, WV_U8 *pIp)
{

    WV_S8 *pData = pSrc;
    WV_S32 len;
    WV_S32 i, j, k, data = 0;
    WV_S32 des;
    len = strlen(pSrc);
    if (strncmp(pData, ".", 1) == 0)
    {
        FPGA_printf("get ip error\r\n");
        return WV_EFAIL;
    }
    j = 3;
    k = 1;
    for (i = len - 1; i >= 0; i--)
    {

        if (SYS_IP_SwitchChar(&pData[i], &des) == 0)
        {

            data += des * k;
            pIp[j] = data;
            k *= 10;
        }
        else
        {

            k = 1;
            data = 0;

            j--;
            if (j < 0)
            {
                break;
            }
        }
    }

    return WV_SOK;
}
/*******************************************************************
WV_S32 FPGA_COMMON_getMacInt(WV_S8 *pSrc, WV_U8 *pMac);
*******************************************************************/
WV_S32 FPGA_COMMON_getMacInt(WV_S8 *pSrc, WV_U8 *pMac)
{

    WV_S8 *pData = pSrc;
    WV_S32 len;
    WV_S32 i, j, k, data = 0;
    WV_S32 des;

    len = strlen(pSrc);

    j = 5;
    k = 1;
    for (i = len - 1; i >= 0; i--)
    {

        if (SYS_IP_SwitchChar(&pData[i], &des) == 0)
        {

            data += des * k;
            pMac[j] = data;
            k *= 16;
        }
        else
        {
            k = 1;
            data = 0;

            j--;
            if (j < 0)
            {
                break;
            }
        }
    }

    return WV_SOK;
}

/**********************************************************
 * WV_S32 strstr_cnt(WV_S8  *string, WV_S8 *substring)
 * 查询某个字符串在另外一个字符串中出现的次数
**********************************************************/
WV_S32 strstr_cnt(WV_S8  *string, WV_S8 *substring)
{

    WV_S32 i, j, k, count = 0;
    for (i = 0; string[i]; i++)
    {
        for (j = i, k = 0; string[j] == substring[k]; j++, k++)
        {
            if (j == strlen(string))
                break;

            if (!substring[k + 1])
            {
                count++;
            }
        }
    }

    return (count);
}
