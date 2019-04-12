#include "his_sys.h"
#include "his_snd.h"
#include "his_so.h"
#include "his_demux.h"
#include "his_iic.h"
#include "his_dis.h"
#include "his_hdmi.h"
#include "his_venc.h"
#include "his_avplay.h"
#include "hi_common.h"
#include "hi_unf_video.h"
#include "his_spi.h"
#include "his_reg.h"
#include "his_fb.h"
#include "his_player.h"
#include"his_temperature.h" 
#include"his_gpio.h" 
#include "his_go.h"
 
typedef struct  HIS_SYS_CONF_S
{
HI_UNF_ENC_FMT_E  disFormat;
} 
HIS_SYS_CONF_S;

static  HIS_SYS_CONF_S   gHisConf;
 
// WV_AVPLAY_CHNL_E  gPlayChnl[2];

/*************************************************************************************
WV_S32  HIS_SYS_ConfInit();
*************************************************************************************/
WV_S32  HIS_SYS_ConfInit()
{
  //gHisConf.disFormat = HI_UNF_ENC_FMT_3840X2160_30;
  //gHisConf.disFormat = HI_UNF_ENC_FMT_1080P_60; 
  //gHisConf.disFormat = HI_UNF_ENC_FMT_VESA_2560X1600_60_RB;

    gHisConf.disFormat = HI_UNF_ENC_FMT_BUTT;
    //gHisConf.disFormat = HI_UNF_ENC_FMT_1080P_60;

    //gHisConf.disFormat = HI_UNF_ENC_FMT_1080P_30;
    return WV_SOK;
}
/*************************************************************************************
WV_S32  HIS_SYS_Init();
*************************************************************************************/
WV_S32   HIS_SYS_Init()
{
  //WV_S8  ipLocal[20] = "192.168.15.98";
  //WV_S8  ipRx[20] = "231.0.20.100";

  HI_SYS_Init(); 
  HIS_IO_Init();
  HIS_IIC_Init();
  HIS_REG_Init();
  HIS_TEMP_Init();
  HIS_SPI_Init();
  HIS_SYS_ConfInit();
  HIS_SND_Init();
  HIS_SO_Init();
  HIS_DEMUX_Init(); 
  HIS_DIS_Init(gHisConf.disFormat,0);//默认4k输出
  HIS_HDMI_Init(gHisConf.disFormat);  
  HIS_AVP_Init();
  HIS_VENC_Init();
  HIS_GO_Init();
  HIS_FB_Open(); 
     
  return WV_SOK;
}
/*************************************************************************************
WV_S32  HIS_SYS_DeInit();
*************************************************************************************/

WV_S32 HIS_SYS_DeInit()
{
   HIS_FB_Close();
   HIS_GO_DeInit();
   HIS_VENC_DeInit();
   HIS_AVP_DeInit();
   HIS_HDMI_DeInit();
   HIS_DIS_DeInit();
   HIS_SND_DeInit();
   HIS_SO_DeInit();
   HIS_DEMUX_DeInit(); 
   HIS_IIC_DeInit();
   HIS_SPI_DeInit();
   HIS_TEMP_DeInit();
   HIS_IO_DeInit();
   HIS_REG_DeInit();
   HI_SYS_DeInit();
  return WV_SOK;
}

