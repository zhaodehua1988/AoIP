#include"his_demux.h"
#include"hi_unf_demux.h"

/******************************************************************
WV_S32  HIS_DEMUX_Init();
******************************************************************/

WV_S32  HIS_DEMUX_Init()
{
    HI_S32                  s32Ret;
    
    s32Ret = HI_UNF_DMX_Init();

    if (HI_SUCCESS != s32Ret)
    {
        WV_ERROR("HI_UNF_DMX_Init init fail.\n");
        return s32Ret;
    }
    
  return  WV_SOK; 
}

/******************************************************************
WV_S32  HIS_DEMUX_Init();
******************************************************************/

WV_S32  HIS_DEMUX_DeInit()
{
    HI_S32                  s32Ret;
    
    s32Ret = HI_UNF_DMX_DeInit();

    if (HI_SUCCESS != s32Ret)
    {
        WV_ERROR("HI_UNF_DMX_DeInit DeInit fail.\n");
        return s32Ret;
    }
    
  return  WV_SOK; 
}
