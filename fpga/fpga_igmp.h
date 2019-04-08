#ifndef _FPGA_IGMP_H_H_
#define _FPGA_IGMP_H_H_

#include "wv_common.h"

// void FPGA_IGMP_join();
// void FPGA_IGMP_exit();
void FPGA_IGMP_enable();
void FPGA_IGMP_SetMode(WV_S32 mode);

void FPGA_IGMP_Open();
void FPGA_IGMP_Close();

#endif