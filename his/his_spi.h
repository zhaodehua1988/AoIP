#ifndef  _HIS_MODE_SPI_H_H_
#define  _HIS_MODE_SPI_H_H_

#include "wv_common.h" 
WV_S32  HIS_SPI_Init();
WV_S32  HIS_SPI_DeInit();
WV_S32  HIS_SPI_FpgaWd(WV_U16 addr,WV_U16 data);
WV_S32  HIS_SPI_FpgaWd2(WV_U16 addr,WV_U16 data);
WV_S32  HIS_SPI_FpgaRd(WV_U16 addr,WV_U16 *pData);
WV_S32  HIS_SPI_FpgaRd2(WV_U16 addr,WV_U16 *pData);
//WV_S32  HIS_SPI_FpgaRdNum(WV_U16 addr,WV_U16 *pData,WV_U32 dataNum);
WV_S32   HIS_SPI_FpgaReset();

//WV_S32  HIS_SPI_Read(WV_S8 * pbuf,WV_S32 len);
//WV_S32  HIS_SPI_Write(WV_S8 * pbuf,WV_S32 len);

#endif
