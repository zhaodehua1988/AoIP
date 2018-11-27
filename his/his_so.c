#include"his_so.h"
#include"hi_unf_so.h"

/******************************************************************
WV_S32  HIS_SO_Init();
******************************************************************/

WV_S32  HIS_SO_Init()
{
    HI_S32                  s32Ret;
    
    s32Ret = HI_UNF_SO_Init();

    if (HI_SUCCESS != s32Ret)
    {
        WV_ERROR("HI_UNF_SO_Init init fail.\n");
        return s32Ret;
    }
    
  return  WV_SOK; 
}

/******************************************************************
WV_S32  HIS_SO_Init();
******************************************************************/

WV_S32  HIS_SO_DeInit()
{
    HI_S32                  s32Ret;
    
    s32Ret = HI_UNF_SO_DeInit();

    if (HI_SUCCESS != s32Ret)
    {
        WV_ERROR("HI_UNF_SO_DeInit DeInit fail.\n");
        return s32Ret;
    }
    
  return  WV_SOK; 
}
