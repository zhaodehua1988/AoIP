#ifndef _FPGA_CONF_H_H_
#define _FPGA_CONF_H_H_

#include "wv_common.h"
#include "fpga_sdp.h"

#define FPGA_CONF_WIN_VIDEO_ETH 0
#define FPGA_CONF_WIN_VIDEO_SDI 1
#define FPGA_CONF_WIN_VIDEO_HDMI 2
#define FPGA_CONF_WINNUM_D 16
#define FPGA_CONF_ETHNUM_D 4
#define FPGA_CONF_IPLEN  64
#define FPGA_CONF_MACLEN 24

typedef struct FPGA_CONF_WIN
{
    WV_U16 win_ena; //窗口是否有效
    WV_U16 x;
    WV_U16 y;
    WV_U16 w;
    WV_U16 h;
    WV_S8 type[16];  //"eth" /"sdi" /"hdmi"
    WV_U16 channel;  //代表第几路输入
    WV_U16 video_ipv6_ena;  //0 ipv4;  1 ipv6;
    //WV_U16 video_sel_in; //根据video_type 类别选择不同输入源的某一路输入网卡/SDI 
    WV_S8  video_ip[FPGA_CONF_IPLEN]; 
    WV_U16 video_port;   
    WV_U16 audio_ipv6_ena;//0 ipv4;1 ipv6
    //WV_U16 audio_sel_in; //根据video_type 类别选择不同输入源的某一路输入网卡/SDI 
    WV_S8  audio_ip[FPGA_CONF_IPLEN];
    WV_U16 audio_port;
    WV_U16 sdp_ena;   //sdp是否有效
    FPGA_SDP_Info sdpInfo; //sdp信息
}FPGA_CONF_WIN_T;

//源地址只包含ip和端口号
typedef struct FPGA_CONF_SRC_ADDR
{
    WV_U16 ena;
    WV_U16 ipv4OrIpv6;
    WV_S8 ipv6[FPGA_CONF_IPLEN];
    WV_U16 port;
} FPGA_CONF_SRC_ADDR_T;

//本地网卡信息设置
typedef struct FPGA_CONF_ETH_T
{
    WV_U32 ipv6Ena;
    WV_S8 ipv6[FPGA_CONF_IPLEN];       //如果是ip4，只用前四个字节 例如 :192.168.1.10 ipv6[0] = 0x0B ,ipv6[1] = 0x10，ipv6[2] = 0xC0,ipv6[3]=0xA8
    WV_S8 mac[FPGA_CONF_MACLEN];         //例如:80-9f-fb-88-88-00 mac[0]=0x00;mac[1]=0x88;mac[2]=0x80;mac[3]=0xfb;mac[4]=0x9f;mac[5]=0x80
    WV_S8 subnet_mask[FPGA_CONF_IPLEN]; //子网掩码//例如:255.255.0.0，mask[0] = 0;mask[1]=0;mask[1]=0xff;mask[1]=0xff;
    WV_S8 getway[FPGA_CONF_IPLEN];      //网关//
}FPGA_CONF_ETH_T;

typedef struct FPGA_CONF_DEV
{
    WV_S8 fpgaVer[24];
    FPGA_CONF_WIN_T win[FPGA_CONF_WINNUM_D];
    FPGA_CONF_ETH_T eth[FPGA_CONF_ETHNUM_D];
    //FPGA_CONF_SHOW_INFO showInfo;
    //FPGA_CONF_DIS display;

} FPGA_CONF_DEV;


WV_S32 FPGA_CONF_SetWin(FPGA_CONF_WIN_T *pWin);
WV_S32 FPGA_CONF_GetWin(FPGA_CONF_WIN_T *pWin);

WV_S32 FPGA_CONF_SetETH(FPGA_CONF_ETH_T *pEth,WV_S32 ethID);
WV_S32 FPGA_CONF_GetETH(FPGA_CONF_ETH_T *pEth,WV_S32 ethID);

WV_S32 FPGA_CONF_GetVersion(WV_S8 *pFpgaVer);
WV_S32 FPGA_CONF_Resolution(WV_S32 Resolution); //Resolution:0 4k60;    1 4k50;     2 4k30;    3 1080p

void FPGA_CONF_Init();
void FPGA_CONF_DeInit();

#endif