#ifndef _FPGA_ARP_H_H_
#define _FPGA_ARP_H_H_
#include "wv_common.h"

/**************************************************************
 * void FPGA_ARP_Request(WV_U16 ethID,WV_S8 *desIp)
 * 函数说明：指定fpga某个网卡发送ARP协议
 * 参数说明：
 *     ethID：网卡id 取值范围【0～3】
 *     desIp：目标ip地址
 * ************************************************************/
void FPGA_ARP_Request(WV_U16 ethID,WV_S8 *desIp);


/***************************************************************
 * void FPGA_ARP_Init()
 * ************************************************************/
void FPGA_ARP_Init();

/***************************************************************
 * void FPGA_ARP_DeInit()
 * ************************************************************/
void FPGA_ARP_DeInit();
#endif