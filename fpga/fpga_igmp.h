#ifndef _FPGA_IGMP_H_H_
#define _FPGA_IGMP_H_H_

#include "wv_common.h"

void FPGA_IGMP_SetSecondOfIgmpSend(WV_U32 sec);
void FPGA_IGMP_join(WV_S32 ethID, WV_S32 srcID, WV_U8 multicastAddr[], WV_U8 sourceIp[]);
void FPGA_IGMP_exit(WV_S32 ethID, WV_S32 srcID, WV_U8 multicastAddr[]);
void FPGA_IGMP_Reset();
void FPGA_IGMP_Open();
void FPGA_IGMP_Close();


void FPGA_IGMP_Lock();
void FPGA_IGMP_UnLock();
#endif