#ifndef _FPGA_CONF_JSON_H_H_
#define _FPGA_CONF_JSON_H_H_
#include "wv_common.h"
#include "fpga_conf.h"
WV_S32 FPGA_CONF_MakeJson();
WV_S32 FPGA_CONF_WinAnalysisJson(char *json,FPGA_CONF_WIN_T *gWin);
WV_S32 FPGA_CONF_EthAnalysisJson(char *json,FPGA_CONF_ETH_T *gEth);
#endif