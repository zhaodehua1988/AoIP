#include "fpga_conf.h"
#include "fpga_common.h"
#include "his_spi.h"
#include "sys_env.h"
#include "sys_ip.h"
#include "fpga_conf_json.h"
#include "fpga_sdp.h"
#include "PCA9555.h"
#include "his_dis.h"
#include "iTE6615_Init.h"
#include "fpga_igmp.h"
#include "fpga_update.h"
#define _FPGA_CONF_FILEPATH_WIN_D "./env/win.ini"
#define _FPGA_CONF_FILEPATH_ETH_D "./env/eth.ini"
#define _FPGA_CONF_FILEPATH_ALPHA_D "./env/volAlpha.ini"

#define _FPGA_MUTEX_ENA_D  (1)

#define FPGA_DEBUG

#if _FPGA_MUTEX_ENA_D
static pthread_mutex_t gMutexSetWin;
#endif

FPGA_CONF_DEV *gpFpgaConfDev;

typedef struct _FPGA_CONF_WIN_INFO_T
{
    // WV_U16 type;   //0:eth 1:sdi 2:hdmi
    // WV_U16 ethChl; //第几路输入,例如第几路网卡，第几路sdi
    // WV_U16 ipSel;  //这个只针对网卡，当前窗口属于第几个ip
    WV_U16 winID;
    FPGA_CONF_WIN_T win;
} _FPGA_CONF_WIN_INFO_T;
//FPGA_CONF_WIN_T
//static _FPGA_CONF_WIN_INFO_T gWinInfo[FPGA_CONF_WINNUM_D];                    //这里主要保存由ip地址转换为第几路网卡第几个ip用
//static FPGA_CONF_SRC_ADDR_T gSrcAddr[FPGA_CONF_ETHNUM_D][FPGA_CONF_SRCNUM_D]; //4个网卡当前4个源ip地址，用于作比较来派断是否重新加入组播

static _FPGA_CONF_WIN_INFO_T gEthWinInfo[FPGA_CONF_ETHNUM_D][FPGA_CONF_SRCNUM_D];
static _FPGA_CONF_WIN_INFO_T gEthWinInfoNew[FPGA_CONF_ETHNUM_D][FPGA_CONF_SRCNUM_D];

/*******************************************************************
WV_S32 fpga_conf_getIpInt(WV_S8 *pName,WV_U8* pIp);
*******************************************************************/
WV_S32 fpga_conf_getIpInt(WV_S8 *pSrc, WV_U8 *pIp)
{

    WV_S8 *pData = pSrc;
    WV_S32 len;
    WV_S32 i, j, k, data = 0;
    WV_S32 des;
    len = strlen(pSrc);
    if (strncmp(pData, ".", 1) == 0)
    {
        WV_printf("get ip error\r\n");
        return WV_EFAIL;
    }
    j = 3;
    k = 1;
    for (i = len - 1; i >= 0; i--)
    {

        if (SYS_IP_SwitchChar(&pData[i], &des) == 0)
        {

            data += des * k;
            pIp[j] = data;
            k *= 10;
        }
        else
        {

            k = 1;
            data = 0;

            j--;
            if (j < 0)
            {
                break;
            }
        }
    }

    return WV_SOK;
}
/*******************************************************************
WV_S32 fpga_conf_getMacInt(WV_S8 *pSrc,WV_U8* pMac);
*******************************************************************/
WV_S32 fpga_conf_getMacInt(WV_S8 *pSrc, WV_U8 *pMac)
{

    WV_S8 *pData = pSrc;
    WV_S32 len;
    WV_S32 i, j, k, data = 0;
    WV_S32 des;

    len = strlen(pSrc);

    j = 5;
    k = 1;
    for (i = len - 1; i >= 0; i--)
    {

        if (SYS_IP_SwitchChar(&pData[i], &des) == 0)
        {

            data += des * k;
            pMac[j] = data;
            k *= 16;
        }
        else
        {
            k = 1;
            data = 0;

            j--;
            if (j < 0)
            {
                break;
            }
        }
    }

    return WV_SOK;
}

/******************************************************************
 * WV_S32 fpga_conf_setIgmp(_FPGA_CONF_WIN_INFO_T (*newAddr)[FPGA_CONF_SRCNUM_D])
 * 组播设置
 * ****************************************************************/

WV_S32 fpga_conf_setIgmp(_FPGA_CONF_WIN_INFO_T (*newAddr)[FPGA_CONF_SRCNUM_D])
{

    WV_S32 i, j;
    WV_U8 multicastAddrInt[5] = {0};
    WV_U8 srcAddrInt[5] = {0};
    
    for (i = 0; i < FPGA_CONF_ETHNUM_D; i++)
    {
        for (j = 0; j < FPGA_CONF_SRCNUM_D; j++)
        {
            if(gEthWinInfo[i][j].win.win_ena == 0) continue;
            if(gEthWinInfo[i][j].win.video_ipv6_ena != newAddr[i][j].win.video_ipv6_ena || \
                strncmp(gEthWinInfo[i][j].win.video_ip,newAddr[i][j].win.video_ip,FPGA_CONF_IPLEN) != 0 || \
                gEthWinInfo[i][j].win.video_port != newAddr[i][j].win.video_port)
            {
                //退出组播gSrcAddr[i][j]
                //WV_printf("new addr[%d][%d] ip=%s \n",i,j,newAddr[i][j].ipv6);
                //WV_printf("eth[%d][%d]退出组播地址%s\n",i,j,gEthWinInfo[i][j].win.video_ip);
                fpga_conf_getIpInt(gEthWinInfo[i][j].win.video_ip, multicastAddrInt);
                FPGA_IGMP_exit(i,j,multicastAddrInt);
            }
        }
    } 
    
    //memcpy(gEthWinInfo, newAddr, sizeof(gEthWinInfo));
    //加入组播
    for (i = 0; i < FPGA_CONF_ETHNUM_D; i++)
    {
        for (j = 0; j < FPGA_CONF_SRCNUM_D; j++)
        {
            if (newAddr[i][j].win.win_ena == 0)
                continue;
            fpga_conf_getIpInt(newAddr[i][j].win.video_ip, multicastAddrInt);
            //WV_printf("eth[%d][%d]加入组播地址%s\n",i,j,newAddr[i][j].win.video_ip);
            FPGA_IGMP_join(i,j, multicastAddrInt, srcAddrInt);
        }
    }

    return WV_SOK;
}
/****************************************************
 * void FPGA_CONF_SetIgmpSendSecond(WV_U32 sec)
 * 函数功能：设置组播发送时间间隔
 * 参说说明：sec 时间，单位 秒
 * *************************************************/
void FPGA_CONF_SetIgmpSendSecond(WV_U32 sec)
{
    FPGA_IGMP_SetSecondOfIgmpSend(sec);
}

/****************************************************
 * WV_S32 FPGA_CONF_GetWinFreezeVal(WV_S32 winID)
 * 查询窗口的视频值
 * 返回值 视频R值的叠加
 * *************************************************/
WV_U32 FPGA_CONF_GetWinFreezeVal(WV_S32 winID)
{
    if (winID < 0 || winID > 15)
        return 0;
    


    WV_U16 baseAddr, regAddr, data = 0,id;
    WV_U32 video_r_sum = 0;
    WV_S32 i,j;

    for(i=0;i<FPGA_CONF_WINNUM_D;i++){
        for(j=0;j<FPGA_CONF_SRCNUM_D;j++){
            if(gEthWinInfo[i][j].winID == winID)
            {
                id = j+i*4;
                baseAddr = 0x500;
                regAddr = 0x5;

                baseAddr = ((baseAddr >> 4) | id) << 4;
                regAddr |= baseAddr;
                HIS_SPI_FpgaRd(regAddr, &data);
                WV_printf("GetWinFreezeVal:: reg[0x%X] = [0x%X]\n",regAddr,data);
                video_r_sum |= data << 16;
                HIS_SPI_FpgaRd(regAddr + 1, &data);
                WV_printf("GetWinFreezeVal:: reg[0x%X] = [0x%X]\n",regAddr+1,data);
                video_r_sum |= data;
                
                return video_r_sum;
            }
        }
    }

    //WV_printf("video_r_sum=0x%X\n",video_r_sum);
    /*
    if(video_r_sum == gWarnInfo.video_r_sum[winID]){//如果查询到的video_r_sum值跟之前保存的值一样，则说明当前画面静帧了
        freeze = 1;
        
    }else if(video_r_sum > gWarnInfo.video_r_sum[winID]){
        //(video_r_sum -_FPGA_VIDEO_FREEZE_LIMIT) <= 
    }else if(video_r_sum < gWarnInfo.video_r_sum[winID]){

    }else{
        gWarnInfo.video_r_sum[winID] = video_r_sum;
        freeze = 0;
    }
*/
    return video_r_sum;
}

/****************************************************
 * WV_S32 FPGA_CONF_GetWinStream(WV_S32 winID)
 * 查询窗口是否有视频流
 * 返回值 0:无视频流
 *       1:有视频流
 *      -1:查询错误，包括输入窗口id超出范围，id范围[0~15]
 * *************************************************/
WV_S32 FPGA_CONF_GetWinStream(WV_S32 winID)
{
    if (winID < 0 || winID > 15)
        return -1;


    WV_U16 regAddr, data = 0;
    WV_S32 videoStreamEffective; //视频流是否有效
    WV_S32 i,j;
    regAddr = 0x18;

    HIS_SPI_FpgaRd(regAddr, &data);

    for(i=0;i<FPGA_CONF_WINNUM_D;i++){
        for(j=0;j<FPGA_CONF_SRCNUM_D;j++){
            if(gEthWinInfo[i][j].winID == winID)
            {
                if((1 << (j+i*4) & data) != 0){
                    videoStreamEffective = 1;
                }else{
                    videoStreamEffective = 0;
                }
                return videoStreamEffective;
            }
        }
    }
    return videoStreamEffective;
}

/*****************************************************
WV_S32 fpga_conf_DisChangeEna(WV_U16 winena)
*设置视频输出更新（包括窗口坐标更新生效）
*****************************************************/
WV_S32 fpga_conf_DisChangeEna(WV_U16 winena)
{

    WV_S32 ret = 0;
    WV_printf("winena = 0x%X\n",winena);
    ret += HIS_SPI_FpgaWd(0x604, winena);
    return ret;
}
/*******************************************************************
static void fpga_conf_SetEthIpAsinterlaceOrder(FPGA_CONF_SRC_ADDR_T (*srcAddr)[FPGA_CONF_SRCNUM_D])
    调整网卡ip去隔行顺序，即把带有隔行的放到前面窗口
*******************************************************************/
static void fpga_conf_SetEthIpAsinterlaceOrder(_FPGA_CONF_WIN_INFO_T (*srcAddr)[FPGA_CONF_SRCNUM_D])
{
    _FPGA_CONF_WIN_INFO_T temp;
    WV_S32 i,j,k;

    for(i=0;i<FPGA_CONF_ETHNUM_D;i++){

        for(j=0;j<FPGA_CONF_SRCNUM_D-1;j++){
            for(k=0;k<FPGA_CONF_SRCNUM_D -1-j;k++){
                if(srcAddr[i][j].win.win_ena == 1 && srcAddr[i][k].win.sdpInfo.video_interlace < srcAddr[i][k+1].win.sdpInfo.video_interlace)
                {
                    temp = srcAddr[i][k];
                    srcAddr[i][k] = srcAddr[i][k+1];
                    srcAddr[i][k+1] = temp;
                }
            }

        }
    }
    
    //设置去隔行
    WV_U16 data=0;
    for(i=0;i<FPGA_CONF_ETHNUM_D;i++){
        if(srcAddr[i][0].win.sdpInfo.video_interlace == 1){
            data |= 1<< i;
        }
    }
    WV_printf("去隔行设置 0x11 = 0x%04X \n",data);
    HIS_SPI_FpgaWd(0x11,data);

}
/******************************************************************
 * void fpga_conf_SetWinIpAndSdp(FPGA_CONF_WIN_T pWin[])
 * //把设置的窗口信息，转换为网卡信息，解析窗口ip,sdp并设置
 * ****************************************************************/
WV_S32 fpga_conf_SetWinIpAndSdp(FPGA_CONF_WIN_T pWin[])
{
    //_FPGA_CONF_WIN_INFO_T winInfo[FPGA_CONF_ETHNUM_D][FPGA_CONF_SRCNUM_D]={0};

    memset(gEthWinInfoNew,0,sizeof(gEthWinInfoNew));
    WV_S32 i,j;
    for(i=0;i<FPGA_CONF_WINNUM_D;i++){
        if(pWin[i].win_ena == 0 || pWin[i].channel >3) continue;
        for(j=0;j<FPGA_CONF_SRCNUM_D;j++){
            if(gEthWinInfoNew[pWin[i].channel][j].win.win_ena == 0 ){
                gEthWinInfoNew[pWin[i].channel][j].win = pWin[i];
                gEthWinInfoNew[pWin[i].channel][j].winID = i;
                break;
            }
        }
    }
#ifdef FPGA_DEBUG
    for (i = 0; i < 4; i++)
    {
        for (j = 0; j < 4; j++)
        {
            if (gEthWinInfoNew[i][j].win.win_ena == 1)
                printf("eth_ip[%d][%d].winid=%d,x=%d,y=%d,w=%d,h=%d,ip=%s\n", i, j, \
                gEthWinInfoNew[i][j].winID,gEthWinInfoNew[i][j].win.x,gEthWinInfoNew[i][j].win.y,gEthWinInfoNew[i][j].win.w,gEthWinInfoNew[i][j].win.h,gEthWinInfoNew[i][j].win.video_ip);
        }
    }
    printf("\r\n");
#endif
    //加入组播
    fpga_conf_setIgmp(gEthWinInfoNew);

    //设置视频源ip和端口号
    WV_S32 ret = 0;
    WV_U16 baseAddr ;
    WV_U16 regAddr;
    for (i = 0; i < FPGA_CONF_ETHNUM_D; i++)
    {
        baseAddr = 0x0100;
        baseAddr = ((baseAddr >> 8) + i) << 8;
        regAddr = baseAddr;
        //设置为ipv4因为本版本不支持ipv6
        ret += HIS_SPI_FpgaWd(regAddr + 0x1f, 0); //设置接收源ipv4Oripv6
        WV_U8 ip[16] = {0};
        for (j = 0; j < 4; j++)
        {
            if (gEthWinInfoNew[i][j].win.win_ena == 0)
            {
                ret += HIS_SPI_FpgaWd(regAddr + 0x20 + j * 9, 0);
                ret += HIS_SPI_FpgaWd(regAddr + 0x21 + j * 9, 0);
                //端口号
                ret += HIS_SPI_FpgaWd(regAddr + 0x28 + j * 9,0);
                continue;
            }
            if (gEthWinInfoNew[i][j].win.video_ipv6_ena == 0)
            { //ipv4
                memset(ip, 0, sizeof(ip));     
                fpga_conf_getIpInt(gEthWinInfoNew[i][j].win.video_ip, ip);
                //ip
                //WV_printf("设置eth_ip[%d][%d] = %d.%d.%d.%d port=%d[0x04x] \n",i,j,ip[0],ip[1],ip[2],ip[3],gEthWinInfoNew[i][j].win.video_port);
                ret += HIS_SPI_FpgaWd(regAddr + 0x20 + j * 9, (ip[2] << 8) | ip[3]);
                //WV_printf("reg[0x%04x] = [0x%04x] \n",regAddr + 0x20 + j * 9,(ip[2] << 8) | ip[3]);
                ret += HIS_SPI_FpgaWd(regAddr + 0x21 + j * 9, (ip[0] << 8) | ip[1]);
                //WV_printf("reg[0x%04x] = [0x%04x] \n",regAddr + 0x21 + j * 9,(ip[0] << 8) | ip[1]);
                //端口号
                ret += HIS_SPI_FpgaWd(regAddr + 0x28 + j * 9,gEthWinInfoNew[i][j].win.video_port);
                //WV_printf("reg[0x%04x] = [0x%04x] \n",regAddr + 0x28 + j * 9,gEthWinInfoNew[i][j].win.video_port);

            }
            else
            { //ipv6
                //测试阶段暂不处理
            }
        }
    }
    
    //设置sdp信息
    for (i = 0; i < FPGA_CONF_ETHNUM_D; i++)
    {
        for (j = 0; j < 4; j++)
        {
            if (gEthWinInfoNew[i][j].win.win_ena == 0)
                continue;
           
            //设置视频sdp信息
            //WV_printf("sdp:eth[%d]ip[%d],window[%d]\n", i, j,gEthWinInfoNew[i][j].winID);
            ret = FPGA_SDP_SetInfo(&gEthWinInfoNew[i][j].win.sdpInfo, (WV_U16)i, (WV_U16)j);
            if (ret != 0)
            {
                WV_ERROR("sdp:eth[%d]ip[%d],window[%d]::set sdp info error\n", i, j,gEthWinInfoNew[i][j].winID);
            }
                
        }
    }


    return WV_SOK;

}

/*******************************************************************
WV_S32 FPGA_CONF_SetWin(FPGA_CONF_WIN_T *pWin);
fpga窗口设置
*******************************************************************/
WV_S32 FPGA_CONF_SetWin(FPGA_CONF_WIN_T winArray[])
{

    WV_S32 i, j, ret = 0;
    WV_U16 baseAddr = 0x0500;
    WV_U16 regAddr, winEna = 0;

    //
    WV_S32 mode = 0; //1080
    for (i = 0; i < FPGA_CONF_WINNUM_D; i++)
    {
        if (winArray[i].win_ena == 1)
        {
            if (winArray[i].sdpInfo.video_width > 1920 || winArray[i].sdpInfo.video_height > 1080)
            {
                mode = 1; // 4k
                break;
            }
        }
    }

    //设置ip和组播地址
    ret += fpga_conf_SetWinIpAndSdp(winArray);

    //设置窗口坐标
    for (i = 0; i < FPGA_CONF_ETHNUM_D; i++)
    {
        for (j = 0; j < FPGA_CONF_SRCNUM_D; j++)
        {
            if(gEthWinInfoNew[i][j].win.win_ena == 0 ) continue;
            regAddr = ((baseAddr >> 4) + i * 4) << 4;
            regAddr = ((regAddr>>4) + j)<<4;
            //WV_printf("11 win[%d][%d]regAddr = %X, x=%d,y=%d,w=%d,h=%d\n",i,j, regAddr, gEthWinInfoNew[i][j].win.x, gEthWinInfoNew[i][j].win.y, gEthWinInfoNew[i][j].win.w, gEthWinInfoNew[i][j].win.h);
            ret += HIS_SPI_FpgaWd(regAddr, gEthWinInfoNew[i][j].win.x);
            ret += HIS_SPI_FpgaWd(regAddr + 1, gEthWinInfoNew[i][j].win.y);
            ret += HIS_SPI_FpgaWd(regAddr + 2, gEthWinInfoNew[i][j].win.w);
            ret += HIS_SPI_FpgaWd(regAddr + 3, gEthWinInfoNew[i][j].win.h);
            winEna |= 1 <<(j+i*4);
        }
    }

    //设置窗口使能
    ret += fpga_conf_DisChangeEna(winEna);

    if (ret == 0)
    {
        FPGA_printf("set windows ok \n");
        memcpy(&gEthWinInfo,&gEthWinInfoNew,sizeof(gEthWinInfo));
    }
    else
    {
        WV_ERROR("FPGA_CONF_SetWin err\n");
    }
    return ret;
}

/*******************************************************************
WV_S32 FPGA_CONF_SetETH(FPGA_CONF_ETH_T *pEth,WV_S32 ethID);
配置fpga网卡
pEth:网卡信息
ethID:网卡id[0~3]
*******************************************************************/
WV_S32 FPGA_CONF_SetETH(FPGA_CONF_ETH_T *pEth, WV_S32 ethID)
{

    WV_U16 baseAddr;
    WV_U16 regAddr;
    WV_S32 ret = 0;
    baseAddr = 0x100;
    regAddr = ((baseAddr >> 8) + ethID) << 8;
    WV_U8 ipInt[16] = {0};
    WV_U8 mac[6] = {0};
    WV_U8 netMask[4] = {0};
    WV_U8 getWay[4] = {0};
    ret += fpga_conf_getIpInt(pEth->ipv6, ipInt);
    ret += fpga_conf_getMacInt(pEth->mac, mac);
    ret += fpga_conf_getIpInt(pEth->subnet_mask, netMask);
    ret += fpga_conf_getIpInt(pEth->getway, getWay);
#ifdef FPGA_DEBUG
    if (ret == 0)
    {
        printf("\n--------------eth[%d]------------------\n", ethID);
        printf("ip:%d.%d.%d.%d\n", ipInt[0], ipInt[1], ipInt[2], ipInt[3]);                            //192.168.1.100
        printf("netmask:%d.%d.%d.%d\n", netMask[0], netMask[1], netMask[2], netMask[3]);               //255.255.255.0
        printf("getway:%d.%d.%d.%d\n", getWay[0], getWay[1], getWay[2], getWay[3]);                    //192.168.1.1
        printf("mac:%02x:%02x:%02x:%02x:%02x:%02x\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]); //80-9f-fb-88-88-00
        printf("----------------------------------------\n");
    }
#endif
    //mac
    ret += HIS_SPI_FpgaWd(regAddr + 0x2, mac[4] << 8 | mac[5]);
    ret += HIS_SPI_FpgaWd(regAddr + 0x3, mac[2] << 8 | mac[3]);
    ret += HIS_SPI_FpgaWd(regAddr + 0x4, mac[0] << 8 | mac[1]);
    //ip
    if (pEth->ipv6Ena == 0)
    { //ipv4
        ret += HIS_SPI_FpgaWd(regAddr + 0x5, ipInt[2] << 8 | ipInt[3]);
        ret += HIS_SPI_FpgaWd(regAddr + 0x6, ipInt[0] << 8 | ipInt[1]);
    }
    else
    {
    }
    //net_mask
    ret += HIS_SPI_FpgaWd(regAddr + 0xd, netMask[2] << 8 | netMask[3]);
    ret += HIS_SPI_FpgaWd(regAddr + 0xe, netMask[0] << 8 | netMask[1]);
    //getway

    ret += HIS_SPI_FpgaWd(regAddr + 0xf, getWay[2] << 8 | getWay[3]);
    ret += HIS_SPI_FpgaWd(regAddr + 0x10, getWay[0] << 8 | getWay[1]);
    if (ret != 0)
    {
        WV_ERROR("set eth[%d] err\n", ethID);
    }
    else
    {
        memcpy((WV_S8 *)&gpFpgaConfDev->eth[ethID], (WV_S8 *)pEth, sizeof(FPGA_CONF_ETH_T));
        //fpga_conf_SaveEthCfgToFile();
    }
    //复位组播
    FPGA_IGMP_Reset();
    return ret;
}
/*******************************************************************
WV_S32 FPGA_CONF_GetETH(FPGA_CONF_ETH_T *pEth,WV_S32 ethID);
配置fpga网卡
pEth:网卡信息
ethID:网卡id[0~3]
*******************************************************************/
WV_S32 FPGA_CONF_GetETH(FPGA_CONF_ETH_T *pEth, WV_S32 ethID)
{
    memcpy((WV_S8 *)pEth, (WV_S8 *)&gpFpgaConfDev->eth[ethID], sizeof(FPGA_CONF_ETH_T));
    return WV_SOK;
}

WV_S32 FPGA_CONF_GetEthInt(WV_U8 ip[], WV_U8 mac[], WV_S32 ethID)
{
    if (ethID > 3 || ethID < 0)
        return WV_EFAIL;
    WV_U8 ipInt[16] = {0};
    WV_U8 macInt[6] = {0};
    WV_U8 netMask[4] = {0};
    WV_U8 getWay[4] = {0};
    fpga_conf_getIpInt(gpFpgaConfDev->eth[ethID].ipv6, ipInt);
    fpga_conf_getMacInt(gpFpgaConfDev->eth[ethID].mac, macInt);
    fpga_conf_getIpInt(gpFpgaConfDev->eth[ethID].subnet_mask, netMask);
    fpga_conf_getIpInt(gpFpgaConfDev->eth[ethID].getway, getWay);
    WV_S32 i;
    for (i = 0; i < 4; i++)
    {
        ip[i] = ipInt[i];
    }
    for (i = 0; i < 6; i++)
    {
        mac[i] = macInt[i];
    }

    return WV_SOK;
}
/**************************************************************
* WV_S32 FPGA_CONF_GetVersion(WV_S8 *pFpgaVer);
*查询版本信息
*************************************************************/
WV_S32 FPGA_CONF_GetVersion(FPGA_CONF_VER *pFpgaVer)
{

    WV_U16 temp;
    HIS_SPI_FpgaRd(0, &pFpgaVer->ver);
    HIS_SPI_FpgaRd(0x2, &pFpgaVer->year);
    HIS_SPI_FpgaRd(0x3, &temp);
    pFpgaVer->month = (temp & 0xf00) >> 8;
    pFpgaVer->day = temp & 0xff;

    return WV_SOK;
}

/******************************************************************************
 * WV_S32 FPGA_CONF_SetResolution(WV_S32 resolution)
 * ***************************************************************************/
WV_S32 FPGA_CONF_Resolution(WV_S32 resolution)
{
    //resolution
    WV_U16 data = 0;
    WV_U16 winEna = 0;
    HIS_SPI_FpgaRd(0x600, &data);
    HIS_SPI_FpgaRd(0x604, &winEna);
    WV_printf("resolution = %d \n", resolution);
    switch (resolution)
    {
    case 0: //3840*2160 p60
        data &= 0xff;
        data |= 0xc00; //60HZ
        HIS_SPI_FpgaWd(0x15, 0xcccc);
        HIS_SPI_FpgaWd(0x600, data);
        HIS_SPI_FpgaWd(0x15, 0xcccc);
        HIS_SPI_FpgaWd(0x12, 0x20b);
        break;
    case 1: //3840*2160 p50
        data = data & 0xff;
        data = data | 0xc00; //60HZ
        HIS_SPI_FpgaWd(0x600, data);
        HIS_SPI_FpgaWd(0x15, 0xf0f0);
        HIS_SPI_FpgaWd(0x12, 0x20b);
        break;
    case 2: //3840*2160 p30
        data = data & 0xff;
        data = data | 0x800;
        HIS_SPI_FpgaWd(0x600, data);
        HIS_SPI_FpgaWd(0x15, 0xf0f0);
        HIS_SPI_FpgaWd(0x12, 0x0b);
        break;
    case 3: //1920*1080 p60 VSEA
        data = data & 0xff;
        data = data | 0x900; //60HZ
        HIS_SPI_FpgaWd(0x600, data);
        HIS_SPI_FpgaWd(0x15, 0xf0f0);
        HIS_SPI_FpgaWd(0x12, 0x0b);
        break;
    case 4: //1920*1080 i60
        data = data | 0x900;  //60HZ
        data = data & 0xfdff; //1080p
        HIS_SPI_FpgaWd(0x600, data);
        HIS_SPI_FpgaWd(0x15, 0xf0f0);
        HIS_SPI_FpgaWd(0x12, 0x0b);
        break;
    case 5:                   //1920*1080 i50
        data = data | 0x900;  //60HZ
        data = data & 0xfdff; //1080p
        HIS_SPI_FpgaWd(0x600, data);
        HIS_SPI_FpgaWd(0x15, 0xf0f0);
        HIS_SPI_FpgaWd(0x12, 0x0b);
        break;
    default:
        break;
    }
    return HIS_DIS_SetCustomTiming((WV_U32)resolution);
}
/*****************************************************************************
 * WV_S32 FPGA_CONF_SetOutPutDisColorDepth(WV_S32 colorDepth)
 * //设置输出颜色位宽
 * **************************************************************************/
void FPGA_CONF_SetOutPutDisColorDepth(WV_S32 colorDepth)
{
    //设置colorDepth
    WV_U16 data = 0;
    HIS_SPI_FpgaRd(0x600, &data);
    colorDepth &= 0x1;
    data |= colorDepth << 9;
    HIS_SPI_FpgaRd(0x600, &data);
}

/*****************************************************************************
 * WV_S32 FPGA_CONF_SetOutPutDisColorDepth(WV_S32 colorDepth)
 * //设置输出色彩空间
 * **************************************************************************/
void FPGA_CONF_SetOutPutDisColorSpace(WV_S32 colorSpace)
{
    //设置colorSpace
}

/****************************************************************************

void FPGA_CONF_SetDisAlpha(WV_S32 alpha)
//设置边框叠加显示透明度，取值范围[0~128],0代表透明，128代表不透明
****************************************************************************/
void FPGA_CONF_SetDisAlpha(WV_S32 alpha)
{
    if (alpha >= 0 && alpha <= 128)
    {
        HIS_SPI_FpgaWd(0x605, (WV_U16)alpha);
    }
    else if (alpha > 128)
    {
        HIS_SPI_FpgaWd(0x605, 128);
    }
}
/****************************************************************************

void FPGA_CONF_SetOutPutAudioSel(WV_U16 winID,WV_U16 audioChl) 
//设置输出声音来自那个窗口,第几个声道
****************************************************************************/
void FPGA_CONF_SetOutPutAudioSel(WV_U16 winID,WV_U16 audioChl) 
{
    if (winID > 15 || audioChl < 0)
        return; //audioSel取值范围[0~15]
    WV_U16 data;
    WV_S32 i,j;
    
    for(i=0;i<FPGA_CONF_ETHNUM_D;i++){
        for(j=0;j<FPGA_CONF_SRCNUM_D;j++){
            if(gEthWinInfo[i][j].winID == winID){
                
                data = (audioChl & 0xf) | (i << 6) | (j << 4);
                WV_printf("eth_IP[%d][%d]set audio output 0x08 = 0x%04x\n",i,j,data);
                HIS_SPI_FpgaWd(0x08,data);
                return;
            }
        }
    }

}
/****************************************************************************

void FPGA_CONF_SetOutPutVolumeMultiple(WV_U16 multipl)
//设置输出音量的放大倍数 【0～15】
****************************************************************************/
void FPGA_CONF_SetOutPutVolumeMultiple(WV_U16 multipl)
{
    HIS_SPI_FpgaWd(0x09,multipl);
}

/****************************************************************************

WV_S32 FPGA_CONF_GetSdpInfo(WV_S32 winID,FPGA_SDP_Info *pSdpInfoOut);
//查询窗口sdp信息
****************************************************************************/
WV_S32 FPGA_CONF_GetSdpInfo(WV_S32 winID, FPGA_SDP_Info *pSdpInfoOut)
{
    WV_S32 i,j,ret=-1;
    for(i=0;i<FPGA_CONF_ETHNUM_D;i++){
        for(j=0;j<FPGA_CONF_SRCNUM_D;i++){
            if(gEthWinInfo[i][j].winID == winID)
            {
                ret = FPGA_SDP_ReadFromFpga(i,j, pSdpInfoOut);
            }
            return ret;
        }
    }

    return ret;
}
/****************************************************************************

WV_S32 FPGA_CONF_SetSdpInfo(WV_S32 winID,FPGA_SDP_Info *pSdpInfoOut);
//设置窗口sdp信息
****************************************************************************/
WV_S32 FPGA_CONF_SetSdpInfo(WV_S32 winID, FPGA_SDP_Info *pSdpInfoIn)
{

    WV_S32 i,j,ret=-1;
    for(i=0;i<FPGA_CONF_ETHNUM_D;i++){
        for(j=0;j<FPGA_CONF_SRCNUM_D;i++){
            if(gEthWinInfo[i][j].winID == winID)
            {
                ret = FPGA_SDP_SetInfo(pSdpInfoIn,i,j);
            }
            return ret;
        }
    }
    return ret;
}

/****************************************************************************

WV_S32 FPGA_CONF_SetCmd(WV_S32 argc, WV_S8 **argv,WV_S8 *prfBuff)

****************************************************************************/
WV_S32 FPGA_CONF_SetCmd(WV_S32 argc, WV_S8 **argv, WV_S8 *prfBuff)
{
    WV_U32 data, id;
    WV_U16 x = 0, y = 0, w = 0, h = 0;
    WV_S32 ret = 0;
    if (argc < 1)
    {
        prfBuff += sprintf(prfBuff, "set fpga <cmd>;//cmd like: wincfg/win/dis/alpha/reset/vol/volchl\r\n");
        return 0;
    }

    if (strcmp(argv[0], "wincfg") == 0)
    {

        if (argc < 2)
        {

            prfBuff += sprintf(prfBuff, "set fpga wincfg <win.json>\r\n");
        }
        if (strstr(argv[1], "win") == NULL || strstr(argv[1], "json") == NULL)
        {
            prfBuff += sprintf(prfBuff, "err!cmd like: set fpga wincfg <win.json>\r\n");
            return 0;
        }
        FPGA_CONF_WIN_T win[16];
        memset(win, 0, sizeof(FPGA_CONF_WIN_T) * 16);

        if (FPGA_CONF_WinAnalysisJson(argv[1], win) == 0)
        {
            FPGA_CONF_SetWin(win);
        }
        return 0;
    }
    else if (strcmp(argv[0], "win") == 0)
    {

        if (argc < 6)
        {
            prfBuff += sprintf(prfBuff, "set fpga win <id> <x> <y> <w> <h> \r\n");
            return WV_SOK;
        }
        //get win.id
        ret = WV_STR_S2v(argv[1], &id);
        if (ret != WV_SOK)
        {
            prfBuff += sprintf(prfBuff, "input erro!\r\n");
            return WV_SOK;
        }
        if (id >= 16)
        {
            prfBuff += sprintf(prfBuff, "input id err!\r\n");
            return WV_SOK;
        }
        //get win x
        ret = WV_STR_S2v(argv[2], &data);
        if (ret != WV_SOK)
        {
            prfBuff += sprintf(prfBuff, "input erro!\r\n");
            return WV_SOK;
        }
        x = data;
        //get win y
        ret = WV_STR_S2v(argv[3], &data);
        if (ret != WV_SOK)
        {
            prfBuff += sprintf(prfBuff, "input erro!\r\n");
            return WV_SOK;
        }
        y = data;
        //get win w
        ret = WV_STR_S2v(argv[4], &data);
        if (ret != WV_SOK)
        {
            prfBuff += sprintf(prfBuff, "input erro!\r\n");
            return WV_SOK;
        }
        w = data;
        //get win h
        ret = WV_STR_S2v(argv[5], &data);
        if (ret != WV_SOK)
        {
            prfBuff += sprintf(prfBuff, "input erro!\r\n");
            return WV_SOK;
        }
        h = data;

        prfBuff += sprintf(prfBuff, "set win[%d],x=%d,y=%d,w=%d,h=%d\n", id, x, y, w, h);
        WV_U16 baseAddr = 0x0500;
        WV_U16 regAddr = 0;

        regAddr = ((baseAddr >> 4) + id) << 4;

        prfBuff += sprintf(prfBuff, "ret 0x%04x=0x%04x[%d]\r\n", regAddr, x, x);
        prfBuff += sprintf(prfBuff, "ret 0x%04x=0x%04x[%d]\r\n", regAddr + 1, y, y);
        prfBuff += sprintf(prfBuff, "ret 0x%04x=0x%04x[%d]\r\n", regAddr + 2, w, w);
        prfBuff += sprintf(prfBuff, "ret 0x%04x=0x%04x[%d]\r\n", regAddr + 3, h, h);

        ret += HIS_SPI_FpgaWd(regAddr, x);
        ret += HIS_SPI_FpgaWd(regAddr + 1, y);
        ret += HIS_SPI_FpgaWd(regAddr + 2, w);
        ret += HIS_SPI_FpgaWd(regAddr + 3, h);

        WV_U16 winEna = 0;
        HIS_SPI_FpgaRd(0x604, &winEna);
        winEna |= 1 << id;
        HIS_SPI_FpgaWd(0x604, winEna);
    }
    else if (strcmp(argv[0], "dis") == 0)
    {

        if (argc < 2)
        {

            prfBuff += sprintf(prfBuff, "set fpga dis <num>//num(0)4k60 1<4k50>  2<4k30> 3<1080p60> 4<1080i60> 5<1080i50> 6<1080i30> 7<1080P30>\r\n");
            return WV_SOK;
        }
        ret = WV_STR_S2v(argv[1], &data);
        if (ret != WV_SOK)
        {
            prfBuff += sprintf(prfBuff, "input erro!\r\n");
            return WV_SOK;
        }
        prfBuff += sprintf(prfBuff, "set fpga dis =%d\r\n", data);
        //FPGA_CONF_SetResolution(data);
        FPGA_CONF_Resolution(data);
    }
    else if (strcmp(argv[0], "alpha") == 0)
    {

        if (argc < 2)
        {

            prfBuff += sprintf(prfBuff, "set fpga alpha <val>//val取值范围0～128\r\n");
            return WV_SOK;
        }
        ret = WV_STR_S2v(argv[1], &data);
        if (ret != WV_SOK)
        {
            prfBuff += sprintf(prfBuff, "input erro!\r\n");
            return WV_SOK;
        }
        FPGA_CONF_SetDisAlpha((WV_S32)data);
    }
    else if (strcmp(argv[0], "reset") == 0)
    {
        PCA9555_Clr(0x3, 0x2);
        usleep(500000);
        PCA9555_Set(0x3, 0x2);
    }
    else if (strcmp(argv[0], "vol") == 0)
    {
        if (argc < 2)
        {

            prfBuff += sprintf(prfBuff, "set fpga vol <val>//val取值范围0～15,设置音量输出放大倍数\r\n");
            return WV_SOK;
        }
        WV_U32 multiple;
        ret = WV_STR_S2v(argv[1], &multiple);
        if (ret != WV_SOK)
        {
            prfBuff += sprintf(prfBuff, "\r\ninput erro!set fpga vol <val>//val取值范围0～15,设置音量输出放大倍数\r\n");
            return WV_SOK;
        }

        FPGA_CONF_SetOutPutVolumeMultiple(multiple);
        prfBuff += sprintf(prfBuff, "设置音量输出放大倍数 %d 倍\r\n",multiple);
    }
    else if (strcmp(argv[0], "volchl") == 0)
    {
        if (argc < 3)
        {

            prfBuff += sprintf(prfBuff, "set fpga volchl <winID> <chl> \r\n");
            return WV_SOK;
        }
        WV_U32 winID;
        WV_U32 chl;
        ret = WV_STR_S2v(argv[1], &winID);
        if (ret != WV_SOK)
        {
            prfBuff += sprintf(prfBuff, "\r\ninput erro!! [set fpga volchl <winID> <chl>]\r\n");
            return WV_SOK;
        }
        ret = WV_STR_S2v(argv[2], &chl);
        if (ret != WV_SOK)
        {
            prfBuff += sprintf(prfBuff, "\r\ninput erro![set fpga volchl <winID> <chl>]\r\n");
            return WV_SOK;
        }
        FPGA_CONF_SetOutPutAudioSel((WV_U16)winID,(WV_U16)chl );

    }else {
        prfBuff += sprintf(prfBuff, "set fpga <cmd>;//cmd like: wincfg/win/dis/alpha/reset/vol/volchl\r\n");
        return 0;        
    }
    
    return 0;
}

/****************************************************************************

WV_S32 FPGA_CONF_GetCmd(WV_S32 argc, WV_S8 **argv,WV_S8 *prfBuff)

****************************************************************************/
WV_S32 FPGA_CONF_GetCmd(WV_S32 argc, WV_S8 **argv, WV_S8 *prfBuff)
{

    WV_S32 ret=-1;
    WV_U32 winID;
    if (argc < 1)
    {

        prfBuff += sprintf(prfBuff, "get fpga <cmd>;//cmd like: win\r\n");
        return 0;
    }
    //设置寄存器
    if (strcmp(argv[0], "win") == 0)
    {
        if (argc < 2)
        {

            prfBuff += sprintf(prfBuff, "get fpga win <cmd>;//cmd like:stream/\r\n");
            return 0;
        }

        if (strcmp(argv[1], "stream") == 0)
        {
            if(argc < 3){

                prfBuff += sprintf(prfBuff, "get fpga win stream <winID>;\r\n");
                return 0;                
            }
            ret = WV_STR_S2v(argv[2], &winID);
            if (ret != WV_SOK)
            {
                prfBuff += sprintf(prfBuff, "\r\ninput erro!! get fpga win stream <winID>;\r\n");
                return WV_SOK;
            }
            ret = FPGA_CONF_GetWinStream(winID);
            if(ret == 0){
                prfBuff += sprintf(prfBuff, "\r\n窗口[%d]有视频流\r\n",winID);
               
            }else{
                prfBuff += sprintf(prfBuff, "\r\n窗口[%d]无视频流\r\n",winID);
            }
        }else if(strcmp(argv[1], "freeze") == 0)
        {
            if(argc < 3){

                prfBuff += sprintf(prfBuff, "get fpga win freeze <winID>;\r\n");
                return 0;                
            }
            ret = WV_STR_S2v(argv[2], &winID);
            if (ret != WV_SOK)
            {
                prfBuff += sprintf(prfBuff, "\r\ninput erro!! get fpga win freeze <winID>;\r\n");
                return WV_SOK;
            }
            WV_U32 r_sum;
            r_sum = FPGA_CONF_GetWinFreezeVal(winID);

            prfBuff += sprintf(prfBuff, "\r\n窗口[%d]是否静帧 R_sum = %d\r\n",winID,r_sum);
                                  
        }
    }else{
        prfBuff += sprintf(prfBuff, "set fpga <cmd>;//cmd like: win\r\n");
    }
    return WV_SOK;
}

/*******************************************************************
WV_S32 FPGA_CONF_Reset();
fpga复位
*******************************************************************/
WV_S32 FPGA_CONF_Reset()
{
    PCA9555_Clr(0x3, 0x2);
    usleep(100000);
    PCA9555_Set(0x3, 0x2);
    usleep(100000);
    WV_U16 reg = 0x4, data = 0;
    WV_S32 i;
    for (i = 0; i < 100; i++)
    {
        HIS_SPI_FpgaRd(reg, &data);
        if (data == 0x3210)
        {
            HIS_SPI_FpgaWd(reg, 0x123);
        }
        if (data == 0x123)
        {
            break;
        }
        usleep(100000);
    }

    return WV_SOK;
}
/*******************************************************************
WV_S32 FPGA_CONF_UpdateFpga(WV_S8 *pFpgaBin);
fpga升级
*******************************************************************/
WV_S32 FPGA_CONF_UpdateFpga(WV_S8 *pFpgaBin)
{
    return FPGA_CONF_UpdateFpga(pFpgaBin);
}
/*******************************************************************
void FPGA_CONF_Init();
fpga初始化
*******************************************************************/
void FPGA_CONF_Init()
{
#if _FPGA_MUTEX_ENA_D

    pthread_mutex_init(&gMutexSetWin,NULL);
#endif
    FPGA_CONF_Reset();
    gpFpgaConfDev = (FPGA_CONF_DEV *)malloc(sizeof(FPGA_CONF_DEV));
    memset(gpFpgaConfDev, 0, sizeof(FPGA_CONF_DEV));

    HIS_SPI_FpgaWd(0x601, 0x0);
    HIS_SPI_FpgaWd(0x602, 0x0);
    HIS_SPI_FpgaWd(0x603, 0x0);
    FPGA_CONF_SetDisAlpha(20);
    //FPGA_CONF_Resolution(0);
    //sdp init
    FPGA_SDP_Init();
    FPGA_UPDATE_Init();
    FPGA_IGMP_Open();
#ifdef FPGA_DEBUG
    int i;
    FPGA_CONF_ETH_T ethCfg;
    for (i = 0; i < 4; i++)
    {
        ethCfg.ipv6Ena = 0;
        sprintf(ethCfg.ipv6, "192.168.1.%d", 11 + i);
        sprintf(ethCfg.subnet_mask, "255.255.255.0");
        sprintf(ethCfg.getway, "192.168.1.1");
        sprintf(ethCfg.mac, "80:9f:fb:88:88:%02x", 1 + i);
        FPGA_CONF_SetETH(&ethCfg, i);
    }
#endif
    WV_CMD_Register("set", "fpga", " set fpga", FPGA_CONF_SetCmd);
    WV_CMD_Register("get", "fpga", "pca9555 get reg", FPGA_CONF_GetCmd);

    return;
}
/*******************************************************************
void FPGA_CONF_DeInit();
fpga初始化
*******************************************************************/
void FPGA_CONF_DeInit()
{
#if _FPGA_MUTEX_ENA_D
    pthread_mutex_destroy(&gMutexSetWin);
#endif
    FPGA_IGMP_Close();
    free(gpFpgaConfDev);
    //sdp init
    FPGA_SDP_DeInit();
    return;
}