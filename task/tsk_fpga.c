#include"tsk_fpga.h"
#include "hi_common.h"
#include "his_spi.h"
#include "fpga_conf.h"
//#include "sys_env.h"
//#include "tsk_scene.h"
//#include "tsk_player.h"


/********************************************************
WV_S32  TSK_FPGA_Open();
********************************************************/

WV_S32  TSK_FPGA_Open()
{

    FPGA_CONF_ResetA();
    //设置分辨率
    FPGA_CONF_SetResolution();
     //关闭融合带
    HIS_SPI_FpgaWd(0x26,0x3);
     //配置分割参数
    FPGA_CONF_SetSplit();
    
    HIS_FB_ClrFpga();
    FPGA_CONF_ClrBuf();
    HIS_SPI_FpgaWd(0x9,0);// stop win number
	HIS_SPI_FpgaWd(0xb,0);
   FPGA_CONF_SetOutput(0xffff);   //设置FPGA输出
    return WV_SOK;
}


/********************************************************
WV_S32  TSK_FPGA_Close();
********************************************************/

WV_S32  TSK_FPGA_Close()
{
 
    return WV_SOK;
}
