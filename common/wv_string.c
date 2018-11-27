#include "wv_string.h"

/*************************************************************************************
WV_S32 WV_STR_S2v(WV_S8 * pStr,WV_S32 *pValue);
*************************************************************************************/

WV_S32  WV_STR_S2v(WV_S8 * pStr,WV_U32 *pValue)
{
  WV_U32 i, d, dat, weight;

    dat = 0;
    if(pStr[0] == '0' && (pStr[1] == 'x' || pStr[1] == 'X'))
    {
        i = 2;
		weight = 16;
	}
	else
	{
        i = 0;
		weight = 10;
	}

    for(; i < 10; i++)
    {
        if(pStr[i] < 0x20)break;
		else if (weight == 16 && pStr[i] >= 'a' && pStr[i] <= 'f')
		{
            d = pStr[i] - 'a' + 10;
		}
		else if (weight == 16 && pStr[i] >= 'A' && pStr[i] <= 'F')
		{
            d = pStr[i] - 'A' + 10;
		}
		else if (pStr[i] >= '0' && pStr[i] <= '9')
		{
            d = pStr[i] - '0';
		}
		else
		{
		    return WV_EFAIL;
		}

		dat = dat * weight + d;
	}

    *pValue = dat;

	return WV_SOK;
}


 
