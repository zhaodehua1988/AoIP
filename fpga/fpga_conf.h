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
//fpga版本号信息
typedef struct FPGA_VER
{
    WV_U16 version;
    WV_U16 type;
    WV_U16 year;
    WV_U16 month;
    WV_U16 day;

} FPGA_VER;

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
    //FPGA_VER ver;
    FPGA_CONF_WIN_T win[FPGA_CONF_WINNUM_D];
    FPGA_CONF_ETH_T eth[FPGA_CONF_ETHNUM_D];
    //FPGA_CONF_SHOW_INFO showInfo;
    //FPGA_CONF_DIS display;

} FPGA_CONF_DEV;


WV_S32 FPGA_CONF_SetWin(FPGA_CONF_WIN_T *pWin);
WV_S32 FPGA_CONF_SetETH(FPGA_CONF_ETH_T *pEth,WV_S32 ethID);
void FPGA_CONF_Init();
void FPGA_CONF_DeInit();
#if 0
typedef struct FPGA_CONF_LOCAL_ADDR
{
    WV_U8 ipv6[16];       //如果是ip4，只用前四个字节 例如 :192.168.1.10 ipv6[0] = 0x0B ,ipv6[1] = 0x10，ipv6[2] = 0xC0,ipv6[3]=0xA8
    WV_U8 mac[6];         //例如:80-9f-fb-88-88-00 mac[0]=0x00;mac[1]=0x88;mac[2]=0x80;mac[3]=0xfb;mac[4]=0x9f;mac[5]=0x80
    WV_U8 subnet_mask[4]; //子网掩码//例如:255.255.0.0，mask[0] = 0;mask[1]=0;mask[1]=0xff;mask[1]=0xff;
    WV_U8 getway[4];      //网关//
    //WV_U8 port;
} FPGA_CONF_LOCAL_ADDR_T;

//目标地址包含ip mac 端口号
typedef struct FPGA_CONF_DES_ADDR
{
    WV_U8 ipv6[16];
    WV_U8 mac[6];
    WV_U16 srcPort;//本地发送端口号
    WV_U16 desPort;
    WV_U16 txCtrl;
    WV_U16 txSel;

} FPGA_CONF_DES_ADDR_T; 
//本地网卡信息设置

typedef struct FPGA_CONF_ETH
{

    WV_U8 ipv6[16];       //如果是ip4，只用前四个字节 例如 :192.168.1.10 ipv6[0] = 0x0B ,ipv6[1] = 0x10，ipv6[2] = 0xC0,ipv6[3]=0xA8
    WV_U8 mac[6];         //例如:80-9f-fb-88-88-00 mac[0]=0x00;mac[1]=0x88;mac[2]=0x80;mac[3]=0xfb;mac[4]=0x9f;mac[5]=0x80
    WV_U8 subnet_mask[4]; //子网掩码//例如:255.255.0.0，mask[0] = 0;mask[1]=0;mask[1]=0xff;mask[1]=0xff;
    WV_U8 getway[4];      //网关//
    //WV_U8 port;
} FPGA_CONF_ETH;
//////////////////////////////////////////////////
//信息显示设置
typedef struct FPGA_CONF_SHOW_INFO
{
    WV_U8 baseInfo;
    WV_U8 errInfo;
    WV_U8 timeInfo;
    WV_U8 time_x;
    WV_U8 time_y;
    WV_U8 time_w;
    WV_U8 time_h;

} FPGA_CONF_SHOW_INFO;

////////////////////////////////////////////////////
//其他信息设置：输出分辨率/音量等
typedef struct FPGA_CONF_OTHER_CONF
{
    WV_U16 vol;        //音量
    WV_U16 resolution; //分辨率
} FPGA_CONF_OTHER_CONF;
//////////////////////////////////////////////////
//输出显示设置
typedef struct FPGA_CONF_DIS
{
    WV_U16 videoCtrl;
    WV_U16 coloKey_R;
    WV_U16 coloKey_G;
    WV_U16 coloKey_B;
    WV_U16 valid;
    WV_U16 alpha; //[15:8]保留，[7:0]alpha 值 ，范围0～128
} FPGA_CONF_DIS;
///////////////////////////////////////////////////

#endif

#if 0
/*****************************************************
* WV_S32 FPGA_CONF_GetVersion(FPGA_VER *pVer);
*查询fpga版本信息
*****************************************************/
WV_S32 FPGA_CONF_GetVersion(FPGA_VER *pVer);

/*****************************************************
*WV_S32 FPGA_CONF_NetConf(FPGA_CONF_DIS *pDis);
*设置网络
*****************************************************/
WV_S32 FPGA_CONF_NetConf(FPGA_CONF_ETH *pNet);

/*****************************************************
* WV_S32 FPGA_CONF_GetCurrentConf(FPGA_CONF_DEV *pConf);
*设置视频开窗信息
*****************************************************/
WV_S32 FPGA_CONF_SetWin(FPGA_CONF_WIN *pWin);


/*****************************************************
*WV_S32 FPGA_CONF_DisConf(FPGA_CONF_DIS *pDis);
*设置视频输出
*****************************************************/
WV_S32 FPGA_CONF_DisConf(FPGA_CONF_DIS *pDis);

/*****************************************************
* WV_S32 FPGA_CONF_PrintEth();
*打印网卡配置信息
*****************************************************/
WV_S32 FPGA_CONF_PrintEth();

/*****************************************************
* WV_S32 FPGA_CONF_PrintWin();
*打印窗口位置信息
*****************************************************/
WV_S32 FPGA_CONF_PrintWin();


/*****************************************************
* void FPGA_CONF_GetCurrentConf(FPGA_CONF_DEV *pConf);
*获取当前配置
*****************************************************/
void FPGA_CONF_GetCurrentConf(FPGA_CONF_DEV *pConf);
#endif
#endif