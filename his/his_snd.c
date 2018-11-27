#include "his_snd.h"
#include "hi_unf_sound.h"

/******************************************************************
WV_S32  HIS_SND_Init();
******************************************************************/

WV_S32  HIS_SND_Init()
{
    HI_S32                  Ret;
    HI_UNF_SND_ATTR_S       stAttr;

    Ret = HI_UNF_SND_Init();
    if (Ret != HI_SUCCESS)
    {
        WV_ERROR("call HI_UNF_SND_Init failed.\r\n");
        return Ret;
    }
    Ret = HI_UNF_SND_GetDefaultOpenAttr(HI_UNF_SND_0, &stAttr);
    if (Ret != HI_SUCCESS)
    {
        WV_ERROR("call HI_UNF_SND_GetDefaultOpenAttr failed.\r\n");
        return Ret;
    }

    Ret = HI_UNF_SND_Open(HI_UNF_SND_0, &stAttr);
    if (Ret != HI_SUCCESS)
    {
        WV_ERROR("call HI_UNF_SND_Open failed.\r\n");
        return Ret;
    }


  return  WV_SOK; 
}

/******************************************************************
WV_S32  HIS_SND_DeInit();
******************************************************************/

WV_S32  HIS_SND_DeInit()
{
    HI_S32                  Ret;

    Ret = HI_UNF_SND_Close(HI_UNF_SND_0);
    if (Ret != HI_SUCCESS )
    {
        WV_ERROR("call HI_UNF_SND_Close failed.\r\n");
        return Ret;
    }

    Ret = HI_UNF_SND_DeInit();
    if (Ret != HI_SUCCESS )
    {
        WV_ERROR("call HI_UNF_SND_DeInit failed.\r\n");
        return Ret;
    }

    return WV_SOK;
}

