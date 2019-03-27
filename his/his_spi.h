#ifndef  _HIS_MODE_SPI_H_H_
#define  _HIS_MODE_SPI_H_H_

#include "wv_common.h" 
WV_S32  HIS_SPI_Init();
WV_S32  HIS_SPI_DeInit();
WV_S32  HIS_SPI_FpgaWd(WV_U16 addr,WV_U16 data);
WV_S32  HIS_SPI_FpgaRd(WV_U16 addr,WV_U16 *pData);
WV_S32 HIS_SPI_FpgaRdNum(WV_U16 addr, WV_U16 *pData, WV_U32 dataNum);
WV_S32 HIS_SPI_Write_then_Read(WV_U8 *pu8WtBuf,WV_U32 u32WtNum,WV_U8 *pu8RdBuf,WV_U32 u32RdNum);
WV_S32 HIS_SPI_Read(WV_U8 *pu8RdBuf,WV_U32 u32RdNum);
WV_S32 HIS_SPI_Write(WV_U8 *pu8WtBuf,WV_U32 u32WtNum);
void HIS_SPI_SetFpgaUpdateEna();
void HIS_SPI_SetFpgaUpdateDisable();
#endif
