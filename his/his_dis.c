#include "his_dis.h"
#include "hi_unf_disp.h"
#include "hi_unf_vo.h"
#include "fpga_conf.h"

#define  HIS_CAST_WIDTH   	1280 
#define  HIS_CAST_HEIGHT   	720  
#define  HIS_DIS_HEIGHT     2172        

//{1,   1,   2,  2160,  82,  8,  3840, 384, 176,       1,   1,  1,    88, 10, 1, 0,  0,  0}
HI_UNF_DISP_TIMING_S CustTiming = 
{
    8,//HI_U32                        VFB;            /**<vertical front blank*//**<CNcomment:垂直前消隐*/
    82,//HI_U32                        VBB;            /**<vertical back blank*//**<CNcomment:垂直后消隐*/
    2160,				//HI_U32                        VACT;           /**<vertical active area*//**<CNcomment:垂直有效区*/
    176,//HI_U32                        HFB;            /**<horizonal front blank*//**<CNcomment:水平前消隐*/
    384,//HI_U32                        HBB;            /**<horizonal back blank*//**<CNcomment:水平后消隐*/
    3840,//HI_U32                      HACT;           /**<horizonal active area*/ /**<CNcomment:水平有效区*/
    10,//HI_U32                        VPW;            /**<vertical sync pluse width*//**<CNcomment:垂直脉冲宽度*/
    88,//HI_U32                        HPW;            /**<horizonal sync pluse width*/ /**<CNcomment:水平脉冲宽度*/
    0,//HI_BOOL                       IDV;            /**< flag of data valid signal is needed flip*//**<CNcomment:有效数据信号是否翻转*/
    0,//HI_BOOL                       IHS;            /**<flag of horizonal sync pluse is needed flip*//**<CNcomment:水平同步脉冲信号是否翻转*/
    0,//HI_BOOL                       IVS;            /**<flag of vertical sync pluse is needed flip*//**<CNcomment:垂直同步脉冲信号是否翻转*/
    0,					//HI_BOOL                       ClockReversal;  /**<flag of clock is needed flip*//**<CNcomment:时钟是否翻转*/
    HI_UNF_DISP_INTF_DATA_WIDTH24,	//HI_UNF_DISP_INTF_DATA_WIDTH_E DataWidth;      /**<data width*/ /**<CNcomment:数据位宽*/
    HI_UNF_DISP_INTF_DATA_FMT_RGB888,	//HI_UNF_DISP_INTF_DATA_FMT_E   ItfFormat;      /**<data format.*//**<CNcomment:数据格式.*/

    0,//HI_BOOL DitherEnable;                           /**< flag of is enable Dither*//**<CNcomment:数据格式.*/
    0,//HI_U32  ClkPara0;                               /**<PLL  register SC_VPLL1FREQCTRL0  value *//**<CNcomment:PLL  SC_VPLL1FREQCTRL0  寄存器*/
    0,//HI_U32  ClkPara1;                               /**<PLL  register SC_VPLL1FREQCTRL1 value*//**<CNcomment:PLL   SC_VPLL1FREQCTRL1寄存器*/

    0,//HI_BOOL bInterlace;                             /**<progressive or interlace*//**<CNcomment:逐行或者隔行*/
    0,//HI_U32  PixFreq;                                /**<pixel clock*//**<CNcomment:像素时钟*/
    30,//HI_U32  VertFreq;                               /**<display rate*//**<CNcomment:刷新率*/
    0,//HI_U32  AspectRatioW;                           /**<width of screen*//**<CNcomment:屏幕宽度*/
    0,//HI_U32  AspectRatioH;                           /**<height of screen*//**<CNcomment:屏幕高度*/
    0,//HI_BOOL bUseGamma;                              /**<gamma modulation*//**<CNcomment:伽马调节*/
    0,//HI_U32  Reserve0;                               /**<reserved byte*//**<CNcomment:保留位*/
    0//HI_U32  Reserve1;                               /**<reserved byte*//**<CNcomment:保留位*/
 
}; 


/**********************************************************************************************

WV_S32  HIS_DIS_InitCast(HI_HANDLE * pHandlCast)

**********************************************************************************************/

WV_S32  HIS_DIS_InitCast(HI_HANDLE * pHandlCast)
{
  HI_UNF_DISP_CAST_ATTR_S  castAttrs;
  WV_S32 ret =0;
  
   WV_RET_ADD (HI_UNF_DISP_GetDefaultCastAttr(HI_UNF_DISPLAY1,&castAttrs),ret);
    castAttrs.u32Width  = HIS_CAST_WIDTH;
    castAttrs.u32Height = HIS_CAST_HEIGHT;
    
   WV_RET_ADD (HI_UNF_DISP_CreateCast(HI_UNF_DISPLAY1,&castAttrs,pHandlCast),ret); 
   WV_RET_ADD ( HI_UNF_DISP_SetCastEnable(*pHandlCast, HI_TRUE),ret);
   return  ret;
}


/**********************************************************************************************

WV_S32  HIS_DIS_DeInitCast(HI_HANDLE * pHandlCast)

**********************************************************************************************/

WV_S32  HIS_DIS_DeInitCast(HI_HANDLE * pHandlCast)
{
    WV_S32 ret =0;
   WV_RET_ADD ( HI_UNF_DISP_SetCastEnable(*pHandlCast, HI_FALSE),ret); 
   WV_RET_ADD ( HI_UNF_DISP_DestroyCast (*pHandlCast),ret);
   return ret;  
}
/**********************************************************************************************

WV_S32  HIS_DIS_StartCast(HI_HANDLE handlCast)

**********************************************************************************************/

WV_S32  HIS_DIS_StartCast(HI_HANDLE handlCast)
{
   
    WV_S32 ret =0;
   WV_RET_ADD ( HI_UNF_DISP_SetCastEnable(handlCast, HI_TRUE),ret);
   return ret;  
    
}


/**********************************************************************************************

WV_S32  HIS_DIS_EndCast(HI_HANDLE handlCast)

**********************************************************************************************/

WV_S32  HIS_DIS_EndCast(HI_HANDLE handlCast)
{
   WV_S32 ret =0;
   WV_RET_ADD ( HI_UNF_DISP_SetCastEnable(handlCast, HI_FALSE),ret);
   return ret; 
}


/**********************************************************************************************

WV_S32  HIS_DIS_SetVirtualScreen(WV_U32 mode);

**********************************************************************************************/
WV_S32  HIS_DIS_SetVirtualScreen(WV_U32 mode)
{
  
	WV_S32 ret=0;
	switch(mode)
	{
		case 0:
			ret = HI_UNF_DISP_SetVirtualScreen(HI_UNF_DISPLAY1, 3840, 1092);
			WV_printf("\nset virtual screen  3840*1092\n");
			break;
		case 1:
			WV_printf("\nset virtual screen  3840*2172\n");
			ret = HI_UNF_DISP_SetVirtualScreen(HI_UNF_DISPLAY1, 3840, HIS_DIS_HEIGHT);//2172
			break;
		case 2:
			WV_printf("\nset virtual screen  1920*1092\n");
			ret = HI_UNF_DISP_SetVirtualScreen(HI_UNF_DISPLAY1, 1920, 1092);
			break;
		default:
			break; 				
	
	}

	return ret;
	
} 

/**********************************************************************************************

WV_S32  HIS_DIS_Init(HI_UNF_ENC_FMT_E disFormat)

**********************************************************************************************/
WV_S32  HIS_DIS_Init(HI_UNF_ENC_FMT_E disFormat,WV_S32 mode)
{
    WV_S32                      Ret;
    HI_UNF_DISP_BG_COLOR_S      BgColor;
    HI_UNF_DISP_INTF_S          stIntf;
    HI_UNF_DISP_OFFSET_S        offset;
    HI_UNF_DISP_TIMING_S    custTiming;   
 
    WV_CHECK_RET(HI_UNF_DISP_Init());
   
    stIntf.enIntfType                = HI_UNF_DISP_INTF_TYPE_HDMI;
    stIntf.unIntf.enHdmi             = HI_UNF_HDMI_ID_0;
    
    Ret = HI_UNF_DISP_AttachIntf(HI_UNF_DISPLAY1, &stIntf, 1);
    if (Ret != HI_SUCCESS)
    {
        WV_printf("HI_UNF_DISP_AttachIntf failed, Ret=%#x.\n", Ret);
        HI_UNF_DISP_DeInit();
        return Ret;
    }
 if(disFormat !=  HI_UNF_ENC_FMT_BUTT)
    {
     Ret = HI_UNF_DISP_SetFormat(HI_UNF_DISPLAY1, disFormat);
    	if (Ret != HI_SUCCESS)
   	 {
   	     WV_printf("HI_UNF_DISP_SetFormat failed, Ret=%#x.\n", Ret); 
   	     HI_UNF_DISP_DeInit();
   	     return Ret;
   	 }
      }
  else
     {
       Ret = HI_UNF_DISP_SetFormat(HI_UNF_DISPLAY1, HI_UNF_ENC_FMT_VESA_2560X1600_60_RB);
    	if (Ret != HI_SUCCESS)
   	 {
   	     WV_printf("HI_UNF_DISP_SetFormat failed, Ret=%#x.\n", Ret); 
   	     HI_UNF_DISP_DeInit();
   	     return Ret;      
        }
     }
/*
	if(mode == 0){
		Ret = HI_UNF_DISP_SetVirtualScreen(HI_UNF_DISPLAY1, 3840, 1092);
		//Ret = HI_UNF_DISP_SetVirtualScreen(HI_UNF_DISPLAY1, 1920, 1092);
	}else{
  		Ret = HI_UNF_DISP_SetVirtualScreen(HI_UNF_DISPLAY1, 3840, HIS_DIS_HEIGHT);
		//Ret = HI_UNF_DISP_SetVirtualScreen(HI_UNF_DISPLAY1, 3840, 1092);
	}
*/
	Ret = HI_UNF_DISP_SetVirtualScreen(HI_UNF_DISPLAY1, 1920, 1092); 
    if (Ret != HI_SUCCESS)
    {
        WV_printf("HI_UNF_DISP_SetVirtualScreen failed, Ret=%#x.\n", Ret);
        HI_UNF_DISP_DeInit();
        return Ret;
    }

    offset.u32Left      = 0;
    offset.u32Top       = 0;
    offset.u32Right     = 0;
    offset.u32Bottom    = 0;
    Ret = HI_UNF_DISP_SetScreenOffset(HI_UNF_DISPLAY1, &offset);
    if (Ret != HI_SUCCESS)
    {
        WV_printf("HI_UNF_DISP_SetBgColor failed, Ret=%#x.\n", Ret); 
        HI_UNF_DISP_DeInit();
        return Ret;
    } 
    
    BgColor.u8Red   = 0;
    BgColor.u8Green = 0;
    BgColor.u8Blue  = 0;
    Ret = HI_UNF_DISP_SetBgColor(HI_UNF_DISPLAY1, &BgColor);
    if (Ret != HI_SUCCESS)
    {
        WV_printf("HI_UNF_DISP_SetBgColor failed, Ret=%#x.\n", Ret); 
        HI_UNF_DISP_DeInit();
        return Ret;
    }

    Ret = HI_UNF_DISP_Open(HI_UNF_DISPLAY1);
    if (Ret != HI_SUCCESS)
    {
        WV_printf("HI_UNF_DISP_Open DISPLAY1 failed, Ret=%#x.\n", Ret); 
        HI_UNF_DISP_DeInit();
        return Ret;
    }

    Ret  = HI_UNF_VO_Init(HI_UNF_VO_DEV_MODE_MOSAIC); 
    if (Ret != HI_SUCCESS)
    {
        WV_printf("HI_UNF_VO_Init DISPLAY1 failed, Ret=%#x.\n", Ret); 
        HI_UNF_DISP_Close(HI_UNF_DISPLAY1);
        HI_UNF_DISP_DeInit();
        return Ret;
    } 
   
  if(disFormat ==  HI_UNF_ENC_FMT_BUTT)
  {
/*
   memset(&custTiming,0,sizeof(HI_UNF_DISP_TIMING_S));
	mode =1;
	if(mode==0){	
	
	custTiming.VFB = 4;//4
	custTiming.VBB = 29; //29
	custTiming.VACT= 1092;//1092
	custTiming.HFB = 176;//176;
	custTiming.HBB = 384;//384;
	custTiming.HACT= 3840;
	custTiming.VPW = 5;//5
		
	}else{

  	custTiming.VFB = 4;//8
	custTiming.VBB = 29; //80
	custTiming.VACT= HIS_DIS_HEIGHT;//2172
	custTiming.HFB = 176;
	custTiming.HBB = 384;
	custTiming.HACT= 3840;
	custTiming.VPW = 10;//5	
	
	}
	custTiming.HPW = 88; 
	custTiming.DataWidth = HI_UNF_DISP_INTF_DATA_WIDTH24; 
	custTiming.ItfFormat = HI_UNF_DISP_INTF_DATA_FMT_RGB888;
        custTiming.VertFreq = 30000;
          
      Ret = HI_UNF_DISP_SetCustomTiming(HI_UNF_DISPLAY1,&custTiming);
*/
	Ret = HIS_DIS_SetCustomTiming(mode);
    	if (Ret != HI_SUCCESS)
   	 {
   	     WV_printf("HI_UNF_DISP_SetCustomTiming, Ret=%#x.\n", Ret); 
   	    // HI_UNF_DISP_DeInit();
   	     //return Ret;
   	 }

     }
    /*
     HI_UNF_DISP_TIMING_S  hdmiTims;
     Ret =  HI_UNF_DISP_GetCustomTiming (HI_UNF_DISPLAY1,&hdmiTims);
    
	printf("HI_UNF_DISP_GetCustomTiming\n");
	printf("VFB   		= %d\n",hdmiTims.VFB);
	printf("VBB   		= %d\n",hdmiTims.VBB);
	printf("VACT   		= %d\n",hdmiTims.VACT);
	printf("HFB   		= %d\n",hdmiTims.HFB);
	printf("HBB   		= %d\n",hdmiTims.HBB);
	printf("HACT   		= %d\n",hdmiTims.HACT);
	printf("VPW   		= %d\n",hdmiTims.VPW);
	printf("HPW   		= %d\n",hdmiTims.HPW);
	printf("IDV   		= %d\n",hdmiTims.IDV);
	printf("IHS   		= %d\n",hdmiTims.IHS);
	printf("IVS   		= %d\n",hdmiTims.IVS);
	printf("DataWidth   	= %d\n",hdmiTims.DataWidth);
	printf("ItfFormat   	= %d\n",hdmiTims.ItfFormat);
	printf("Dither   	= %d\n",hdmiTims.DitherEnable);
	printf("bInterlace   	= %d\n",hdmiTims.bInterlace);
	printf("PixFreq   	= %d\n",hdmiTims.PixFreq);
	printf("AspectRatioW   	= %d\n",hdmiTims.AspectRatioW);
	printf("AspectRatioH   	= %d\n",hdmiTims.AspectRatioH);
	printf("bUseGamma   	= %d\n",hdmiTims.bUseGamma);
 */
     
    return  WV_SOK; 
    
}




/**********************************************************************************************

WV_S32  HIS_DIS_DeInit()

**********************************************************************************************/
WV_S32  HIS_DIS_DeInit()
{   
     WV_S32  Ret;
     
      Ret = HI_UNF_DISP_Close(HI_UNF_DISPLAY1);
		if (Ret != HI_SUCCESS)
		{
		    WV_printf("HI_UNF_DISP_Close failed, Ret=%#x.\n", Ret);
		    return Ret;
		}
        Ret = HI_UNF_DISP_DeInit();
       if (Ret != HI_SUCCESS)
		{
		    WV_printf("HI_UNF_DISP_DeInit failed, Ret=%#x.\n", Ret);
		    return Ret;
		}
       Ret = HI_UNF_VO_DeInit();
          if (Ret != HI_SUCCESS)
		{
		    WV_printf("HI_UNF_VO_DeInit failed, Ret=%#x.\n", Ret);
		    return Ret;
		} 
     return WV_SOK;
}

/**********************************************************************************************

WV_S32  HIS_DIS_WinCreat(HI_HANDLE *  pHndlWin,HI_RECT_S  winRect); 

**********************************************************************************************/
WV_S32  HIS_DIS_WinCreat(HI_HANDLE *  pHndlWin,HI_RECT_S  winRect)
{
  
    HI_UNF_WINDOW_ATTR_S   WinAttr;
    HI_S32  Ret; 
    
    
    memset(&WinAttr, 0, sizeof(HI_UNF_WINDOW_ATTR_S));
    WinAttr.enDisp = HI_UNF_DISPLAY1;
    WinAttr.bVirtual = HI_FALSE;
    WinAttr.stWinAspectAttr.enAspectCvrs = HI_UNF_VO_ASPECT_CVRS_IGNORE;
    WinAttr.stWinAspectAttr.bUserDefAspectRatio = HI_FALSE;
    WinAttr.stWinAspectAttr.u32UserAspectWidth  = 0;
    WinAttr.stWinAspectAttr.u32UserAspectHeight = 0;
    WinAttr.bUseCropRect = HI_FALSE;
    WinAttr.stInputRect.s32X = 0;
    WinAttr.stInputRect.s32Y = 0;
    WinAttr.stInputRect.s32Width = 0;
    WinAttr.stInputRect.s32Height = 0;
    
    WinAttr.stOutputRect.s32X 		= winRect.s32X;
    WinAttr.stOutputRect.s32Y 		= winRect.s32Y;
    WinAttr.stOutputRect.s32Width 	= winRect.s32Width ;
    WinAttr.stOutputRect.s32Height 	= winRect.s32Height;
 
    //Ret = HI_UNF_VO_CreateWindow(&WinAttr, pHndlWin);
	Ret = HI_UNF_VO_CreateWindowExt(&WinAttr,pHndlWin,HI_FALSE);
	//Ret = HI_UNF_VO_CreateWindowExt(&WinAttr,pHndlWin,HI_TRUE);
	
	if (Ret != HI_SUCCESS)
    {
        WV_printf("call HI_UNF_VO_CreateWindow failed.[%d]\n",Ret);
        return Ret;
    }
   
    return WV_SOK;
}

/**********************************************************************************************

WV_S32  HIS_DIS_WinDestory(HI_HANDLE *  pHndlWin); 

**********************************************************************************************/
WV_S32  HIS_DIS_WinDestroy(HI_HANDLE *  pHndlWin)
{
 	return HI_UNF_VO_DestroyWindow(*pHndlWin);
}


/**********************************************************************************************

WV_S32  HIS_DIS_WinStart(HI_HANDLE *  pHndlWin);

**********************************************************************************************/
WV_S32  HIS_DIS_WinStart(HI_HANDLE *  pHndlWin)
{
 	
   return HI_UNF_VO_SetWindowEnable(*pHndlWin,HI_TRUE); 
 
}


/**********************************************************************************************

WV_S32  HIS_DIS_WinStop(HI_HANDLE *  pHndlWin);

**********************************************************************************************/
WV_S32  HIS_DIS_WinStop(HI_HANDLE *  pHndlWin)
{
 	
   return HI_UNF_VO_SetWindowEnable(*pHndlWin,HI_FALSE); 
 
}



/**********************************************************************************************

WV_S32  HIS_DIS_WinAttach(HI_HANDLE *  pHndlWin,HI_HANDLE *  pHndlSrc);

**********************************************************************************************/

WV_S32  HIS_DIS_WinAttach(HI_HANDLE *  pHndlWin,HI_HANDLE *  pHndlSrc)
{
 
 return HI_UNF_VO_AttachWindow  (*pHndlWin,*pHndlSrc);

}


/**********************************************************************************************

WV_S32  HIS_DIS_WinDetach(HI_HANDLE *  pHndlWin,HI_HANDLE *  pHndlSrc);

**********************************************************************************************/

WV_S32  HIS_DIS_WinDetach(HI_HANDLE *  pHndlWin,HI_HANDLE *  pHndlSrc)
{ 
 return HI_UNF_VO_DetachWindow  ( *pHndlWin,*pHndlSrc);

}


/**********************************************************************************************

WV_S32  HIS_DIS_WinFreeze(HI_HANDLE *  pHndlWin,HI_BOOL bEnable,WV_S32 mode);

**********************************************************************************************/

WV_S32  HIS_DIS_WinFreeze(HI_HANDLE *  pHndlWin,HI_BOOL bEnable,WV_S32 mode)
{ 


	WV_S32 ret=0;

	if(mode == 0)
	{
	ret = HI_UNF_VO_FreezeWindow(* pHndlWin, bEnable,HI_UNF_WINDOW_FREEZE_MODE_LAST);
	}else if(mode == 1)
	{
	ret = HI_UNF_VO_FreezeWindow(* pHndlWin, bEnable,HI_UNF_WINDOW_FREEZE_MODE_BLACK);
	
	}	
	//sleep(1);
	 return ret;

}

/**********************************************************************************************

WV_S32  HIS_DIS_GetWinFreezeStatus(HI_HANDLE * pHndlWin,HI_BOOL *pbEnable);

**********************************************************************************************/

WV_S32  HIS_DIS_GetWinFreezeStatus(HI_HANDLE * pHndlWin,HI_BOOL *pbEnable)
{ 
	HI_UNF_WINDOW_FREEZE_MODE_E enWinFreezeMode;
	return HI_UNF_VO_GetWindowFreezeStatus(* pHndlWin, pbEnable, &enWinFreezeMode);
}
/**********************************************************************************************

WV_S32 HIS_DIS_SetCustomTiming(WV_S32 mode)

**********************************************************************************************/
WV_S32  HIS_DIS_SetCustomTiming(WV_U32 mode)
{ 
	HI_UNF_DISP_TIMING_S    custTiming;
	WV_S32 ret=-1;
	memset(&custTiming,0,sizeof(HI_UNF_DISP_TIMING_S));

	switch(mode)
	{
		case 0:
			custTiming.VFB = 4;//4
			custTiming.VBB = 29; //29
			custTiming.VACT= 1092;//1092
			custTiming.HFB = 176;//176;
			custTiming.HBB = 384;//384;
			custTiming.HACT= 3840;
			custTiming.VPW = 5;//5
			break;
		case 1:
		  	custTiming.VFB = 4;//8
			custTiming.VBB = 29; //80
			custTiming.VACT= 2172;//HIS_DIS_HEIGHT;//2172
			custTiming.HFB = 176;
			custTiming.HBB = 384;
			custTiming.HACT= 3840;
			custTiming.VPW = 10;//5	
			break;
		case 2:
		  	custTiming.VFB = 4;//8
			custTiming.VBB = 29; //80
			custTiming.VACT= 2172;//HIS_DIS_HEIGHT;//2172
			custTiming.HFB = 176;
			custTiming.HBB = 384;
			custTiming.HACT= 3840;
			custTiming.VPW = 10;//5	
			break;
		default:
			return -1;	
	
	}
	custTiming.HPW = 88; 
	custTiming.DataWidth = HI_UNF_DISP_INTF_DATA_WIDTH24; 
	custTiming.ItfFormat = HI_UNF_DISP_INTF_DATA_FMT_RGB888;
   custTiming.VertFreq = 30000;
          
	ret = HI_UNF_DISP_SetCustomTiming(HI_UNF_DISPLAY1,&custTiming);
	if (ret != HI_SUCCESS)
	 {
	     WV_printf("HI_UNF_DISP_SetCustomTiming, Ret=%#x.\n", ret); 

	 }
	return ret;

}



