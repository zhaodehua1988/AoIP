#ifndef _FPGA_UPDATE_H_
#define _FPGA_UPDATE_H_

#include  "wv_common.h"

WV_S32 FPGA_UPDATE_Init();
WV_S32 FPGA_UPDATE_DeInit();
WV_S32 FPGA_UPDATE_Update(WV_S8 *pFpgaBin);
#endif