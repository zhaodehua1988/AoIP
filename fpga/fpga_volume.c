
#include <math.h>
#include "fpga_volume.h"
#include "his_spi.h"

#define _D_FPGA_VOLUME_NUM (128)
#define _D_FPGA_VOLUME_READVOLUME_ADDR 0x20




typedef struct _S_FPGA_VOLUME_CONF
{
    WV_THR_HNDL_T thrHndl;
    WV_U32 open;
    WV_U32 close;
    WV_U16 volume[_D_FPGA_VOLUME_NUM];    //fpga获取到的音量值
    WV_U16 volumeOut[_D_FPGA_VOLUME_NUM]; //计算以后输出的音量值

}_S_FPGA_VOLUME_CONF;
static _S_FPGA_VOLUME_CONF   gFpgaVolume;


/**************************************************************************
 * void FPGA_VOLUME_Get(WV_U16 ethID,WV_U16 ipID,WV_U8 volume )
 * ***********************************************************************/
void FPGA_VOLUME_Get(WV_U16 ethID,WV_U16 ipID,WV_U16 volume[] )
{

    WV_U16 winID;
    winID = ethID*4 + ipID;
    memcpy(volume,&gFpgaVolume.volumeOut[winID*_D_FPGA_VOLUME_CHLNUM],_D_FPGA_VOLUME_CHLNUM*2);
}
/**************************************************************************
 * void fpga_volume_readVolume()
 * ************************************************************************/
void fpga_volume_readVolume()
{
    volatile double dVolume;
    WV_S32 j;
    memset(gFpgaVolume.volume,0x00,_D_FPGA_VOLUME_NUM);
    HIS_SPI_FpgaWd(_D_FPGA_VOLUME_READVOLUME_ADDR, 0x00);
    HIS_SPI_FpgaRdNum(_D_FPGA_VOLUME_READVOLUME_ADDR, gFpgaVolume.volume, _D_FPGA_VOLUME_NUM);

    for(j = 0;j< _D_FPGA_VOLUME_NUM;j++)
    {
         dVolume = (gFpgaVolume.volume[j] / 32768.0);
         dVolume = fabs(dVolume);
         dVolume = log10(dVolume);
         if(dVolume < -4.0)
         {
             dVolume = -4.0;
         }
         dVolume = dVolume * 20.0;
         dVolume = dVolume + 60.0;
         gFpgaVolume.volumeOut[j] = dVolume * 1.25;
    }
}
/**************************************************************************
 * void *fpga_read_volume(void *prm)
 * ************************************************************************/
void *fpga_read_volume(void *prm)
{
    _S_FPGA_VOLUME_CONF *pDev = (_S_FPGA_VOLUME_CONF *)prm;
    pDev->open = 1;
    pDev->close = 0;

    while (pDev->open == 1)
    {
        fpga_volume_readVolume();
        usleep(30000);
    }
    pDev->open = 0;
    pDev->close = 1;
    return NULL;
}
/**************************************************************************
 * void FPGA_VOLUME_Open()
 * ************************************************************************/
void FPGA_VOLUME_Open()
{
    memset(&gFpgaVolume,0,sizeof(_S_FPGA_VOLUME_CONF));
    WV_THR_Create(&gFpgaVolume.thrHndl, fpga_read_volume, WV_THR_PRI_DEFAULT, WV_THR_STACK_SIZE_DEFAULT, &gFpgaVolume);
}
/**************************************************************************
 * void FPGA_VOLUME_Close()
 * ************************************************************************/
void FPGA_VOLUME_Close()
{

    if (gFpgaVolume.open == 1)
    {
        gFpgaVolume.open = 0;
        while (gFpgaVolume.close == 1)
            ;
        WV_THR_Destroy(&gFpgaVolume.thrHndl);
    }
    WV_printf("FPGA_VOLUME_Close end\n");
}
