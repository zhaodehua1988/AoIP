#include "his_hdmi.h"
#include "his_spi.h"
#include "hi_unf_hdmi.h"

typedef struct HIS_HDMI_ARGS_S
{
    HI_UNF_HDMI_ID_E  enHdmi;
}HIS_HDMI_ARGS_S;

static HIS_HDMI_ARGS_S g_stHdmiArgs;
static HI_UNF_HDMI_CALLBACK_FUNC_S g_stCallbackFunc;


/**********************************************************************************************

HI_VOID HIS_HDMI_ProPlug(HI_VOID *pPrivateData)

**********************************************************************************************/

HI_VOID HIS_HDMI_ProPlug(HI_VOID *pPrivateData)
{
    HI_S32          ret = HI_SUCCESS;
    HIS_HDMI_ARGS_S     *pArgs  = (HIS_HDMI_ARGS_S*)pPrivateData;
    HI_UNF_HDMI_ID_E       hHdmi   =  pArgs->enHdmi;

    HI_UNF_HDMI_ATTR_S             stHdmiAttr; 
    //HI_UNF_EDID_BASE_INFO_S        stSinkCap;
    HI_UNF_HDMI_STATUS_S           stHdmiStatus;
 
    HI_UNF_HDMI_GetStatus(hHdmi,&stHdmiStatus);
    if (HI_FALSE == stHdmiStatus.bConnected)
    {
        WV_printf("No Connect\r\n");
        return;
    }


    HI_UNF_HDMI_GetAttr(hHdmi, &stHdmiAttr); 
     stHdmiAttr.bEnableHdmi 			= HI_TRUE;
    stHdmiAttr.enVidOutMode 		= HI_UNF_HDMI_VIDEO_MODE_RGB444;  
    stHdmiAttr.bEnableHdmi 			= HI_TRUE;
    stHdmiAttr.bEnableAudio 		= HI_TRUE;
    stHdmiAttr.bEnableVideo 		= HI_TRUE;
    stHdmiAttr.bEnableAudInfoFrame 	= HI_TRUE;
    stHdmiAttr.bEnableAviInfoFrame 	= HI_TRUE;
    stHdmiAttr.bHDCPEnable			= HI_FALSE; 

    ret = HI_UNF_HDMI_SetAttr(hHdmi, &stHdmiAttr);
    

    /* HI_UNF_HDMI_SetAttr must before HI_UNF_HDMI_Start! */
    ret = HI_UNF_HDMI_Start(hHdmi); 
  //  HIS_SPI_FpgaReset(); 
    return;

}


/**********************************************************************************************

HI_VOID HIS_HDMI_ProUnPlug(HI_VOID *pPrivateData)

**********************************************************************************************/

HI_VOID HIS_HDMI_ProUnPlug(HI_VOID *pPrivateData)
{
    HI_S32          ret = HI_SUCCESS;
    HIS_HDMI_ARGS_S     *pArgs  = (HIS_HDMI_ARGS_S*)pPrivateData;
    HI_UNF_HDMI_ID_E       hHdmi   =  pArgs->enHdmi;
 
    ret = HI_UNF_HDMI_Stop(hHdmi); 
    return;

}
/**********************************************************************************************

HI_VOID HIS_HDMI_CallBack(HI_UNF_HDMI_EVENT_TYPE_E event, HI_VOID *pPrivateData)

**********************************************************************************************/

HI_VOID HIS_HDMI_CallBack(HI_UNF_HDMI_EVENT_TYPE_E event, HI_VOID *pPrivateData)
{

    switch ( event )
    {
        case HI_UNF_HDMI_EVENT_HOTPLUG:
             WV_printf("HI_UNF_HDMI_EVENT_HOTPLUG \r\n"); 
              HIS_HDMI_ProPlug(pPrivateData);
            break;
        case HI_UNF_HDMI_EVENT_NO_PLUG:
             WV_printf("HI_UNF_HDMI_EVENT_NO_PLUG\r\n");  
            break;
        case HI_UNF_HDMI_EVENT_EDID_FAIL:
             WV_printf("HI_UNF_HDMI_EVENT_EDID_FAI\r\n"); 
            break;
        case HI_UNF_HDMI_EVENT_HDCP_FAIL: 
             WV_printf("HI_UNF_HDMI_EVENT_HDCP_FAIL\r\n"); 
            break;
        case HI_UNF_HDMI_EVENT_HDCP_SUCCESS:
             WV_printf("HI_UNF_HDMI_EVENT_HDCP_SUCCES\r\n");  
            break;
        case HI_UNF_HDMI_EVENT_RSEN_CONNECT:
             WV_printf("HI_UNF_HDMI_EVENT_RSEN_CONNECT\r\n");  
            break;
        case HI_UNF_HDMI_EVENT_RSEN_DISCONNECT:
             WV_printf(" HI_UNF_HDMI_EVENT_RSEN_DISCONNECT\r\n");
              HIS_HDMI_ProUnPlug(pPrivateData); 
            break;
        default:
             WV_printf(" HI_UNF_HDMI_EVENT_NOT_KNOWS\r\n"); 
            break;
    }

    return;
}



/**********************************************************************************************

WV_S32  HIS_HDMI_Init(HI_UNF_ENC_FMT_E disFormat)

**********************************************************************************************/

WV_S32  HIS_HDMI_Init(HI_UNF_ENC_FMT_E hdmiFormat)
{
    
    HI_S32 					Ret;
    HI_UNF_HDMI_OPEN_PARA_S stOpenParam;
    HI_UNF_HDMI_DELAY_S  	stDelay;
    HI_U32   				enDefaultMode;
    HI_UNF_HDMI_ID_E 		enHDMIId;
    HI_UNF_HDMI_ATTR_S      stHdmiAttr;
//
    enDefaultMode    = HI_UNF_HDMI_DEFAULT_ACTION_HDMI;
    enHDMIId         = HI_UNF_HDMI_ID_0;
// 
     Ret = HI_UNF_HDMI_Init();
    if (HI_SUCCESS != Ret)
    {
        WV_printf("HI_UNF_HDMI_Init failed:%#x\n",Ret);
        return HI_FAILURE;
    }
//
    HI_UNF_HDMI_GetDelay(0,&stDelay);
    stDelay.bForceFmtDelay = HI_TRUE;
    stDelay.bForceMuteDelay = HI_TRUE;
    stDelay.u32FmtDelay = 500;
    stDelay.u32MuteDelay = 120;
    HI_UNF_HDMI_SetDelay(0,&stDelay);
    
//  

   g_stHdmiArgs.enHdmi =  enHDMIId;
    g_stCallbackFunc.pfnHdmiEventCallback = HIS_HDMI_CallBack;
	g_stCallbackFunc.pPrivateData = &g_stHdmiArgs;

	Ret = HI_UNF_HDMI_RegCallbackFunc(enHDMIId, &g_stCallbackFunc);
	if (Ret != HI_SUCCESS)
    {
        WV_printf("hdmi reg failed:%#x\n",Ret);
        HI_UNF_HDMI_DeInit();
        return HI_FAILURE;
    }
//
   stOpenParam.enDefaultMode = HI_UNF_HDMI_DEFAULT_ACTION_NULL; //HI_UNF_HDMI_DEFAULT_ACTION_HDMI;
    Ret = HI_UNF_HDMI_Open(enHDMIId, &stOpenParam);
    if (Ret != HI_SUCCESS)
    {
        WV_printf("HI_UNF_HDMI_Open failed:%#x\n",Ret);
        HI_UNF_HDMI_DeInit();
        return HI_FAILURE;
    }
    
//
	HI_UNF_HDMI_GetAttr(enHDMIId, &stHdmiAttr);
	
    stHdmiAttr.bEnableHdmi 			= HI_TRUE;
    stHdmiAttr.enVidOutMode 		= HI_UNF_HDMI_VIDEO_MODE_RGB444;  
    stHdmiAttr.bEnableHdmi 			= HI_TRUE;
    stHdmiAttr.bEnableAudio 		= HI_FALSE;
    stHdmiAttr.bEnableVideo 		= HI_TRUE;
    stHdmiAttr.bEnableAudInfoFrame 	= HI_FALSE;
    stHdmiAttr.bEnableAviInfoFrame 	= HI_FALSE;
    stHdmiAttr.bHDCPEnable			= HI_FALSE; 
    
    Ret = HI_UNF_HDMI_SetAttr(enHDMIId, &stHdmiAttr);
    if (Ret != HI_SUCCESS)
    {
        WV_printf("HI_UNF_HDMI_SetAttr failed:%#x\n",Ret);
        HI_UNF_HDMI_Close(enHDMIId);
	    HI_UNF_HDMI_UnRegCallbackFunc(enHDMIId, &g_stCallbackFunc);;
        HI_UNF_HDMI_DeInit();
        return HI_FAILURE;
    }
     
    /* HI_UNF_HDMI_SetAttr must before HI_UNF_HDMI_Start! */
    Ret = HI_UNF_HDMI_Start(enHDMIId);
    if (Ret != HI_SUCCESS)
    {
        WV_printf("HI_UNF_HDMI_Start:%#x\n",Ret);
        HI_UNF_HDMI_Close(enHDMIId);
	HI_UNF_HDMI_UnRegCallbackFunc(enHDMIId, &g_stCallbackFunc);;
        HI_UNF_HDMI_DeInit();
        return HI_FAILURE;
    } 
    
   return WV_SOK; 
}


/**********************************************************************************************

WV_S32  HIS_HDMI_DeInit()

**********************************************************************************************/

WV_S32  HIS_HDMI_DeInit()
{
 
    HI_UNF_HDMI_ID_E 		enHDMIId; 
   
    enHDMIId         = HI_UNF_HDMI_ID_0;
    HI_UNF_HDMI_Stop(enHDMIId);

    HI_UNF_HDMI_Close(enHDMIId);

	 HI_UNF_HDMI_UnRegCallbackFunc(enHDMIId, &g_stCallbackFunc);

    HI_UNF_HDMI_DeInit();

    return WV_SOK;
 
}

