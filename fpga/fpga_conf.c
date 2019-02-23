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

#define _FPGA_CONF_FILEPATH_WIN_D "./env/win.ini"
#define _FPGA_CONF_FILEPATH_ETH_D "./env/eth.ini"
#define _FPGA_CONF_FILEPATH_ALPHA_D "./env/volAlpha.ini"

#define _FPGA_FILE_MUTEX_ENA_D 0
//查询fpga警告信息的频率 单位秒
#define _FPGA_GET_WARN_INFO_TIMES_D  (1) 

#define FPGA_DEBUG


#if _FPGA_FILE_MUTEX_ENA_D
pthread_mutex_t gMutexWin;
pthread_mutex_t gMutexEth;
pthread_mutex_t gMutexAlpha;
#endif

FPGA_CONF_DEV *gpFpgaConfDev;

typedef struct _FPGA_CONF_WIN_INFO_T
{
    WV_U16 type; //0:eth 1:sdi 2:hdmi
    WV_U16 ethChl;  //第几路输入,例如第几路网卡，第几路sdi
    WV_U16 ipSel;  //这个只针对网卡，当前窗口属于第几个ip

}_FPGA_CONF_WIN_INFO_T;

static _FPGA_CONF_WIN_INFO_T gWinInfo[FPGA_CONF_WINNUM_D];//这里主要保存由ip地址转换为第几路网卡第几个ip用
/*******************************************************************
WV_S32 fpga_conf_getMacInt(WV_S8 *pSrc,WV_S8* pMac);
*******************************************************************/
WV_S32 fpga_conf_getMacInt(WV_S8 *pSrc,WV_S8* pMac)
{

	WV_S8* pData = pSrc;	
	WV_S32 len;
	WV_S32 i,j,k,data=0;
	WV_S32 des;

	len = strlen(pSrc);

	j = 5;
	k = 1;
	for(i=len-1;i>=0;i--){
		 
		if(SYS_IP_SwitchChar(&pData[i],&des)==0){
		
			data += des*k;
			pMac[j] = data;
			k*=16;
		
		}else{
			k=1;
			data = 0;
		
			j--;
			if(j<0){
				break;
			}
		}
	}

	return WV_SOK;	
}
/*******************************************************************
WV_S32 fpga_conf_getIpInt(WV_S8 *pName,WV_U8* pIp);
*******************************************************************/
WV_S32 fpga_conf_getIpInt(WV_S8 *pSrc,WV_S8* pIp)
{

	WV_S8* pData = pSrc;	
	WV_S32 len;
	WV_S32 i,j,k,data=0;
	WV_S32 des;
	len = strlen(pSrc);
	if(strncmp(pData,".",1) == 0){
		WV_printf("get ip error\r\n");
		return WV_EFAIL;
	}
	j = 3;
	k = 1;
	for(i=len-1;i>=0;i--){
		 
		if(SYS_IP_SwitchChar(&pData[i],&des)==0){
		
			data += des*k;
			pIp[j] = data;
			k*=10;
		
		}else{
			
			k=1;
			data = 0;
		
			j--;
			if(j<0){
				break;
			}
		}
	}

	return WV_SOK;	
}

/****************************************************
 * WV_S32 FPGA_CONF_GetWinFreeze(WV_S32 winID)
 * 查询窗口的视频值
 * 返回值 视频R值的叠加
 * *************************************************/
WV_U32 FPGA_CONF_GetWinFreeze(WV_S32 winID)
{
    if(winID<0 || winID > 15) return 0;
    WV_U16 baseAddr,regAddr,data=0;
    WV_U32 video_r_sum=0;

    baseAddr = 0x500;
    regAddr = 0x5;
   
    baseAddr = ((baseAddr >> 4) | winID) << 4;
    regAddr |=baseAddr;
    

    HIS_SPI_FpgaRd(regAddr,&data);
    video_r_sum |= data << 16;
    HIS_SPI_FpgaRd(regAddr+1,&data);
    video_r_sum |= data;
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
    if(winID<0 || winID > 15) return -1;
    WV_U16 regAddr,data=0;
    WV_S32 videoStreamEffective;//视频流是否有效

    regAddr = 0x18;
    
    HIS_SPI_FpgaRd(regAddr,&data);
    if(((1<<winID) & data) != 0 )
    {
        videoStreamEffective = 1;
    }else{
        videoStreamEffective = 0;
    }

    return videoStreamEffective;
}

/*****************************************************
WV_S32 fpga_conf_DisChangeEna(WV_U16 winena)
*设置视频输出更新（包括窗口坐标更新生效）
*****************************************************/
WV_S32 fpga_conf_DisChangeEna(WV_U16 winena)
{
   
    WV_S32 ret=0;
    ret +=HIS_SPI_FpgaWd(0x604,winena);
    return ret;
}


/*******************************************************************
WV_S32 fpga_conf_SetWinIpAndSdp(FPGA_CONF_WIN_T *pWin);
解析窗口ip,sdp并设置
*******************************************************************/
WV_S32 fpga_conf_SetWinIpAndSdp(FPGA_CONF_WIN_T pWin[])
{
    
    FPGA_CONF_SRC_ADDR_T srcAddr[4][4];

    WV_S32 i,j=0,k=0;
    WV_U16 av_sel[16];
    WV_U16 video_type;
    memset(srcAddr,0,sizeof(FPGA_CONF_SRC_ADDR_T)*16);
    
    for(i=0;i<FPGA_CONF_WINNUM_D;i++)
    {
        //WV_U16 av_sel = 0;
        //av_sel[i]|=pWin[i].type << 12;
        if(strlen(pWin[i].type) >4){
            WV_printf("win[%d]=%s video type err,type must be \"net\",\"sdi\",\"hdmi\" \r\n",i,pWin[i].type);
            return WV_EFAIL;
        }
        if(strcmp(pWin[i].type,"net") == 0){
            video_type = 0;
            gWinInfo[i].type = 0;
            av_sel[i]|= 0 << 10;

        }else if(strcmp(pWin[i].type,"sdi") == 0){
            video_type = 1;
            gWinInfo[i].type = 1;
            av_sel[i]|= 1 << 10;
            av_sel[i]|= pWin[i].channel & 0x3;
        }else if(strcmp(pWin[i].type,"hdmi") == 0){
            video_type = 2;
            gWinInfo[i].type = 2;
            av_sel[i]|= 2 << 10;
        }
        
        if(pWin[i].win_ena ==1 && video_type == 0) //窗口使能且输入为网络 // 0 eth; 1 sdi; 2 hdmi
        {
            
            if(pWin[i].channel >3 ) return WV_EFAIL;  //如果网卡sel in >3,则认为是错误
            //video ip
           
            for(j=0;j<4;j++){ //如果这个地址已经存在，切ip和端口号都一样，指定该窗口选择这个ip
                if(srcAddr[pWin[i].channel][j].ena ==1 && \
                     srcAddr[pWin[i].channel][j].port == pWin[i].video_port &&  \
                    strcmp(pWin[i].video_ip,srcAddr[pWin[i].channel][j].ipv6) == 0 && \
                    pWin[i].video_ipv6_ena==srcAddr[pWin[i].channel][j].ipv4OrIpv6)
                {
                    av_sel[i]|= (j & 0xf) << 6; //查询srcAddr列表，如果对应的网卡已经接收这个ip地址，则指定该窗口选择这个ip地址
                    av_sel[i] |= (pWin[i].channel &0x3);
                    
                    //printf("video ip 匹配到合适的值src[%d][%d]\n",pWin[i].channel,j);
                    break;
                }
                

                if(3==j){ //如果对应的网卡没有接收这个ip地址，则在srcAddr列表新增这个ip地址，并且指定该窗口选择这个ip地址
                    for(k=0;k<4;k++){
                        if(srcAddr[pWin[i].channel][k].ena==0)
                        {
                            srcAddr[pWin[i].channel][k].ena=1;
                            memcpy(srcAddr[pWin[i].channel][k].ipv6,pWin[i].video_ip,FPGA_CONF_IPLEN);
                            srcAddr[pWin[i].channel][k].ipv4OrIpv6 = pWin[i].video_ipv6_ena;
                            srcAddr[pWin[i].channel][k].port = pWin[i].video_port;
                            av_sel[i] |= (k & 0xf) << 6; //查询srcAddr列表，如果对应的网卡已经接收这个ip地址，则指定该窗口选择这个ip地址
                            av_sel[i] |= (pWin[i].channel &0x3);                        
                            gWinInfo[i].ethChl = pWin[i].channel;
                            gWinInfo[i].ipSel = k;
                            break;
                        }
                        if(3==k) {
                            WV_printf("video 网卡[%d]接收的ip数量超过4个\r\n",pWin[i].channel);
                            return WV_EFAIL; //该网卡配置的ip地址超出范围，所以
                        }

                    }
                }
            }
            //audio ip
            for(j=0;j<4;j++){
                if(srcAddr[pWin[i].channel][j].ena ==1 \
                    && srcAddr[pWin[i].channel][j].port == pWin[i].audio_port \
                    && strcmp(pWin[i].audio_ip,srcAddr[pWin[i].channel][j].ipv6) == 0 \
                    && pWin[i].video_ipv6_ena==srcAddr[pWin[i].channel][j].ipv4OrIpv6)
                {
                    av_sel[i]|= (j & 0xf) << 2; //查询srcAddr列表，如果对应的网卡已经接收这个ip地址，则指定该窗口选择这个ip地址
                    
                    break;
                }
                if(3==j){ //如果对应的网卡没有接收这个ip地址，则在srcAddr列表新增这个ip地址，并且指定该窗口选择这个ip地址
                    
                    for(k=0;k<4;k++){
                        if(srcAddr[pWin[i].channel][k].ena==0)
                        {
                            srcAddr[pWin[i].channel][k].ena=1;
                            memcpy(srcAddr[pWin[i].channel][k].ipv6,pWin[i].audio_ip,FPGA_CONF_IPLEN);
                            srcAddr[pWin[i].channel][k].ipv4OrIpv6 = pWin[i].audio_ipv6_ena;
                            av_sel[i] |= (k & 0xf) << 2; //查询srcAddr列表，如果对应的网卡已经接收这个ip地址，则指定该窗口选择这个ip地址 
                            srcAddr[pWin[i].channel][k].port = pWin[i].audio_port;                    
                            break;
                        }
                        if(3==k) {
                            WV_printf("网卡[%d]接收的ip数量超过4个\r\n",pWin[i].channel);
                            return WV_EFAIL; //该网卡配置的ip地址超出范围，所以
                        }

                    }
                }
            }



        }

    }

#ifdef FPGA_DEBUG
    for(i=0;i<4;i++){
        for(j=0;j<4;j++){
            if(srcAddr[i][j].ena==1)
            printf("src[%d][%d].ip=%s\n",i,j,srcAddr[i][j].ipv6);
        }
    }

    for(i=0;i<16;i++){
        printf("win[%d] av_sel=0x%04x ",i,av_sel[i]);
        if((i+1)%4==0){
            printf("\r\n");
        }
    }
    printf("\r\n");
#endif

    //视频源ip地址和端口号
    WV_S32 ret = 0;
    WV_U16 baseAddr = 0x0100;
    WV_U16 regAddr = baseAddr;
    for (i = 0; i < FPGA_CONF_ETHNUM_D; i++)
    {
        baseAddr = ((baseAddr >> 8) + i) << 8;
        regAddr = baseAddr;
        //mac
        WV_U16 srcAddrIpv4OrIpv6=0;
        WV_S8 ip[16]={0};
        for(j=0;j<4;j++)
        {
            srcAddrIpv4OrIpv6 = (srcAddr[i][j].ipv4OrIpv6 & 1)<< j;
        }
        ret += HIS_SPI_FpgaWd(regAddr + 0x1f, srcAddrIpv4OrIpv6); //设置接收源ipv4Oripv6 
        //printf("reg[%X]=%X\n",regAddr + 0x1f,sr_FPGA_FILE_MUTEX_ENA_DcAddrIpv4OrIpv6);
        
        for (j = 0; j < 4; j++)
        { 
            if(srcAddr[i][j].ipv4OrIpv6 == 0){ //ipv4
                memset(ip,0,sizeof(ip));
                fpga_conf_getIpInt(srcAddr[i][j].ipv6,ip);
                ret += HIS_SPI_FpgaWd(regAddr + 0x20 + j * 9, (ip[2] << 8) | ip[3]);
                ret += HIS_SPI_FpgaWd(regAddr + 0x21 + j * 9, (ip[0] << 8) | ip[1]);            
            }else{  //ipv6
                //测试阶段暂不处理
            }
            
        }   
    }

    //设置sdp信息


    for(i=0;i<FPGA_CONF_ETHNUM_D;i++){
        for(j=0;j<4;j++){
            if(srcAddr[i][j].ena ==0)continue;
            for(k=0;k<FPGA_CONF_WINNUM_D;k++){
                if(pWin[k].win_ena == 0 ) continue;
                if(pWin[k].channel ==i \
                   &&pWin[k].video_ipv6_ena == srcAddr[i][j].ipv4OrIpv6 \
                   && pWin[k].video_port == srcAddr[i][j].port \
                   && 0 == strcmp(pWin[k].video_ip,srcAddr[i][j].ipv6))
                {
                    //设置视频sdp信息
                   
                    ret=FPGA_SDP_SetInfo(&pWin[k].sdpInfo,(WV_U16)i,(WV_U16)j);
                    if(ret != 0){
                        WV_ERROR("eth[%d]ip[%d],window[%d]::set sdp info error\n",i,j,k);
                    }
                }
                            
            }
        }

    }


    //设置视频源选通选项
     baseAddr = 0x0500;
    //config windows location and video info
    for (i = 0; i < FPGA_CONF_WINNUM_D; i++)
    {
        regAddr = ((baseAddr >> 4) + i) << 4;
        //printf("set spi 0x%04x 0x%04x\n",regAddr+4,av_sel[i]);
         ret += HIS_SPI_FpgaWd(regAddr + 4, av_sel[i]);
    }
    
    if(ret != 0 ){

        return WV_EFAIL;
    }
    return WV_SOK;
}

/*******************************************************************
WV_S32 fpga_conf_GetWinAudioInfo(WV_S32 windowID);
获取某个窗口的音频信息
*******************************************************************/
/*
WV_S32 fpga_conf_GetWinAudioInfo(WV_S32 windowID)
{
    //if(gpFpgaConfDev->win[windowID].type)

    return WV_SOK;
}
*/
/*******************************************************************
WV_S32 fpga_conf_SaveWinCfgToFile();
保存窗口信息
*******************************************************************/
WV_S32 fpga_conf_SaveWinCfgToFile()
{

#if _FPGA_FILE_MUTEX_ENA_D    
    if(pthread_mutex_trylock(&gMutexWin) != 0 ){
        return WV_EFAIL;
    }
#endif
    WV_S32 ret=0;
    FILE *fp;
    fp=fopen(_FPGA_CONF_FILEPATH_WIN_D,"wb");
    if(fp == NULL){
        FPGA_printf("save wincfg err:open file %s err\n",_FPGA_CONF_FILEPATH_WIN_D);
        ret = -1;
    }
    else if(fwrite((WV_S8 *)gpFpgaConfDev->win,1,sizeof(FPGA_CONF_WIN_T)*FPGA_CONF_WINNUM_D,fp) != sizeof(FPGA_CONF_WIN_T)*FPGA_CONF_WINNUM_D)
    {
        FPGA_printf("save wincfg err:fwrite file %s err\n",_FPGA_CONF_FILEPATH_WIN_D);
        fclose(fp);
        ret = -1;
    }else{
        fclose(fp);
    }
    

#if _FPGA_FILE_MUTEX_ENA_D   
    pthread_mutex_unlock(&gMutexWin);
#endif
    return WV_SOK;
}
/*******************************************************************
WV_S32 fpga_conf_ReadWinCfgFromFile();
读取窗口信息
*******************************************************************/
WV_S32 fpga_conf_ReadWinCfgFromFile()
{

#if _FPGA_FILE_MUTEX_ENA_D    
    if(pthread_mutex_trylock(&gMutexWin) != 0 ){
        return WV_EFAIL;
    }
#endif
    WV_S32 ret = 0;
    FILE *fp;
    fp=fopen(_FPGA_CONF_FILEPATH_WIN_D,"rb");
    if(fp == NULL){
        FPGA_printf("read wincfg err:open file %s err\n",_FPGA_CONF_FILEPATH_WIN_D);
        ret = -1;
    }
    else if(fread((WV_S8 *)gpFpgaConfDev->win,1,sizeof(FPGA_CONF_WIN_T)*FPGA_CONF_WINNUM_D,fp) != sizeof(FPGA_CONF_WIN_T)*FPGA_CONF_WINNUM_D)
    {
        FPGA_printf("read wincfg err:fwrite file %s err\n",_FPGA_CONF_FILEPATH_WIN_D);
        fclose(fp);
        ret = -1;;
    }else{
        fclose(fp);
    }

#if _FPGA_FILE_MUTEX_ENA_D   
    pthread_mutex_unlock(&gMutexWin);
#endif
    return WV_SOK;
}


/*******************************************************************
WV_S32 FPGA_CONF_SetWin(FPGA_CONF_WIN_T *pWin);
fpga窗口设置
*******************************************************************/
WV_S32 FPGA_CONF_SetWin(FPGA_CONF_WIN_T winArray[])
{


    WV_S32 i, ret = 0;
    WV_U16 baseAddr = 0x0500;
    WV_U16 regAddr,winEna=0;

    //set src ip
    ret +=fpga_conf_SetWinIpAndSdp(winArray);
    //设置窗口位置信息
    
    for (i = 0; i < FPGA_CONF_WINNUM_D; i++)
    {
        winEna |= winArray[i].win_ena << i;
        regAddr = ((baseAddr >> 4) + i) << 4;
        ret += HIS_SPI_FpgaWd(regAddr, winArray[i].x);
        ret += HIS_SPI_FpgaWd(regAddr + 1, winArray[i].y);
        ret += HIS_SPI_FpgaWd(regAddr + 2, winArray[i].w);
        ret += HIS_SPI_FpgaWd(regAddr + 3, winArray[i].h);
        
    }
    //设置窗口使能
    ret +=fpga_conf_DisChangeEna(winEna);
    
    if(ret == 0){
        FPGA_printf("set windows ok \n");
        memcpy(gpFpgaConfDev->win,winArray,sizeof(FPGA_CONF_WIN_T)*FPGA_CONF_WINNUM_D);
        //fpga_conf_SaveWinCfgToFile();
    }else{
        WV_ERROR("FPGA_CONF_SetWin err\n");
    }
    return ret;
}

/*******************************************************************
WV_S32 FPGA_CONF_GetWin(FPGA_CONF_WIN_T *pWin);
fpga读取窗口信息
*******************************************************************/
WV_S32 FPGA_CONF_GetWin(FPGA_CONF_WIN_T winArray[])
{
    memcpy((WV_S8 *)winArray,(WV_S8 *)gpFpgaConfDev->win,sizeof(FPGA_CONF_WIN_T)*FPGA_CONF_WINNUM_D);
    return WV_SOK;
}


/*******************************************************************
WV_S32 fpga_conf_SaveEthCfgToFile();
保存网卡信息
*******************************************************************/
WV_S32 fpga_conf_SaveEthCfgToFile()
{
#if _FPGA_FILE_MUTEX_ENA_D    
    if(pthread_mutex_trylock(&gMutexEth) != 0 ){
        return WV_EFAIL;
    }
#endif
    WV_S32 ret=0;
    FILE *fp;
    fp=fopen(_FPGA_CONF_FILEPATH_ETH_D,"wb");
    if(fp == NULL){
        FPGA_printf("save ethcfg err:open file %s err\n",_FPGA_CONF_FILEPATH_ETH_D);
        ret = -1;
    
    }else if(fwrite((WV_S8 *)gpFpgaConfDev->eth,1,sizeof(FPGA_CONF_ETH_T)*FPGA_CONF_ETHNUM_D,fp) != sizeof(FPGA_CONF_ETH_T)*FPGA_CONF_ETHNUM_D){
        
        FPGA_printf("save ethcfg err:fwrite file %s err\n",_FPGA_CONF_FILEPATH_ETH_D);
        fclose(fp);
        ret = -1;
    }else{
        fclose(fp);
    }

#if _FPGA_FILE_MUTEX_ENA_D   
    pthread_mutex_unlock(&gMutexEth);
#endif
    return ret;
}
/*******************************************************************
WV_S32 fpga_conf_ReadEthCfgFromFile();
读取网卡信息
*******************************************************************/
WV_S32 fpga_conf_ReadEthCfgFromFile()
{
#if _FPGA_FILE_MUTEX_ENA_D    
    if(pthread_mutex_trylock(&gMutexEth) != 0 ){
        return WV_EFAIL;
    }
#endif   
    WV_S32 ret=0;
    FILE *fp;
    fp=fopen(_FPGA_CONF_FILEPATH_ETH_D,"rb");
    if(fp == NULL){
        FPGA_printf("read ethcfg err:open file %s err\n",_FPGA_CONF_FILEPATH_ETH_D);
        ret = -1;
    
    }else if(fread((WV_S8 *)gpFpgaConfDev->eth,1,sizeof(FPGA_CONF_ETH_T)*FPGA_CONF_ETHNUM_D,fp) != sizeof(FPGA_CONF_ETH_T)*FPGA_CONF_ETHNUM_D){
        FPGA_printf("read ethcfg err:fwrite file %s err\n",_FPGA_CONF_FILEPATH_ETH_D);
        fclose(fp);
        ret = -1;
    }else{
        fclose(fp);
    }

#if _FPGA_FILE_MUTEX_ENA_D   
    pthread_mutex_unlock(&gMutexEth);
#endif
    return WV_SOK;
}

/*******************************************************************
WV_S32 FPGA_CONF_SetETH(FPGA_CONF_ETH_T *pEth,WV_S32 ethID);
配置fpga网卡
pEth:网卡信息
ethID:网卡id[0~3]
*******************************************************************/
WV_S32 FPGA_CONF_SetETH(FPGA_CONF_ETH_T *pEth,WV_S32 ethID)
{

    WV_U16 baseAddr;
    WV_U16 regAddr;
    WV_S32 ret=0;
    baseAddr = 0x100;
    regAddr = ((baseAddr >> 8) + ethID) << 8;
    WV_S8 ipInt[16]={0};
    WV_S8 mac[6]={0};
    WV_S8 netMask[4]={0};
    WV_S8 getWay[4]={0};
    ret +=fpga_conf_getIpInt(pEth->ipv6,ipInt);
    ret +=fpga_conf_getMacInt(pEth->mac,mac);
    ret +=fpga_conf_getIpInt(pEth->subnet_mask,netMask);
    ret +=fpga_conf_getIpInt(pEth->getway,getWay);
#ifdef FPGA_DEBUG
    if(ret == 0){
        printf("\n--------------eth[%d]------------------\n",ethID);
        printf("ip:%d.%d.%d.%d\n",ipInt[0],ipInt[1],ipInt[2],ipInt[3]); //192.168.1.100
        printf("netmask:%d.%d.%d.%d\n",netMask[0],netMask[1],netMask[2],netMask[3]); //255.255.255.0
        printf("getway:%d.%d.%d.%d\n",getWay[0],getWay[1],getWay[2],getWay[3]); //192.168.1.1
        printf("mac:%02x:%02x:%02x:%02x:%02x:%02x\n",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);//80-9f-fb-88-88-00
        printf("----------------------------------------\n");    
    }
#endif
    //mac 
    ret += HIS_SPI_FpgaWd(regAddr + 0x2,mac[4]<<8 | mac[5]);
    ret += HIS_SPI_FpgaWd(regAddr + 0x3,mac[2]<<8 | mac[3]);
    ret += HIS_SPI_FpgaWd(regAddr + 0x4,mac[0]<<8 | mac[1]);
    //ip
    if(pEth->ipv6Ena == 0){ //ipv4
        ret += HIS_SPI_FpgaWd(regAddr + 0x5,ipInt[2]<<8 | ipInt[3]); 
        ret += HIS_SPI_FpgaWd(regAddr + 0x6,ipInt[0]<<8 | ipInt[1]); 
    }else{

    }
    //net_mask
    ret += HIS_SPI_FpgaWd(regAddr + 0xd,netMask[2]<<8 | netMask[3]);
    ret += HIS_SPI_FpgaWd(regAddr + 0xe,netMask[0]<<8 | netMask[1]);
    //getway

    ret += HIS_SPI_FpgaWd(regAddr + 0xf,getWay[2]<<8 | getWay[3]);
    ret += HIS_SPI_FpgaWd(regAddr + 0x10,getWay[0]<<8 | getWay[1]);    
    if(ret != 0 ){
        WV_ERROR("set eth[%d] err\n",ethID);
    }else{
        memcpy((WV_S8 *)&gpFpgaConfDev->eth[ethID],(WV_S8 *)pEth,sizeof(FPGA_CONF_ETH_T));
        //fpga_conf_SaveEthCfgToFile();
    }

    return ret;

}
/*******************************************************************
WV_S32 FPGA_CONF_GetETH(FPGA_CONF_ETH_T *pEth,WV_S32 ethID);
配置fpga网卡
pEth:网卡信息
ethID:网卡id[0~3]
*******************************************************************/
WV_S32 FPGA_CONF_GetETH(FPGA_CONF_ETH_T *pEth,WV_S32 ethID)
{
    memcpy((WV_S8 *)pEth,(WV_S8 *)&gpFpgaConfDev->eth[ethID],sizeof(FPGA_CONF_ETH_T));
    return WV_SOK;
}


/**************************************************************
* WV_S32 FPGA_CONF_GetVersion(WV_S8 *pFpgaVer);
*查询版本信息
*************************************************************/
WV_S32 FPGA_CONF_GetVersion(WV_S8 *pFpgaVer)
{

    WV_U16 temp;
    WV_U16 year;
    WV_U16 month;
    WV_U16 day;
    WV_U16 ver;
    
    HIS_SPI_FpgaRd(0, &ver);
    //HIS_SPI_FpgaRd(0x1, &pVer->type);
    HIS_SPI_FpgaRd(0x2, &year);

    HIS_SPI_FpgaRd(0x3, &temp);

    month = (temp & 0xf00) >> 8;
    day = temp & 0xff >> 8;
    sprintf(pFpgaVer,"%d-%d-%d:%d",(WV_S32)year,(WV_S32)month,(WV_S32)day,(WV_S32)ver);
    FPGA_printf("fpga version=%s\r\n", pFpgaVer);

    //sprintf(pMainVer,"%s:%s",SYS_ENV_VERSION_NO,SYS_ENV_DAYE);

    return WV_SOK;
}

/******************************************************************************
 * WV_S32 FPGA_CONF_SetResolution(WV_S32 resolution)
 * ***************************************************************************/
WV_S32 FPGA_CONF_Resolution(WV_S32 resolution)
{
    //resolution
    WV_U16 data=0;
    WV_U16 winEna=0;
    HIS_SPI_FpgaRd(0x600,&data);
    HIS_SPI_FpgaRd(0x604,&winEna);
    WV_printf("resolution = %d \n",resolution);
    switch(resolution)
    {
        case 0://3840*2160 p60
            data &= 0xff;
            data |= 0xc00; //60HZ
            //HIS_SPI_FpgaWd(0x12,0);
            //HIS_SPI_FpgaWd(0x604,0);
            HIS_SPI_FpgaWd(0x15,0xcccc);            
            HIS_SPI_FpgaWd(0x600,data);
            HIS_SPI_FpgaWd(0x15,0xcccc);
            HIS_SPI_FpgaWd(0x12,0x20b);
            //HIS_SPI_FpgaWd(0x604,winEna);
            break;
        case 1: //3840*2160 p50
            data = data & 0xff;
            data = data | 0xc00; //60HZ
            //HIS_SPI_FpgaWd(0x12,0);
            //HIS_SPI_FpgaWd(0x604,0);
            HIS_SPI_FpgaWd(0x600,data);
            HIS_SPI_FpgaWd(0x15,0xf0f0);
            HIS_SPI_FpgaWd(0x12,0x20b);
            //HIS_SPI_FpgaWd(0x604,winEna);
            break;
        case 2://3840*2160 p30
            data =data & 0xff;
            data =data | 0x800;
            //HIS_SPI_FpgaWd(0x12,0);
            //HIS_SPI_FpgaWd(0x604,0);
            HIS_SPI_FpgaWd(0x600,data); 
            HIS_SPI_FpgaWd(0x15,0xf0f0);   
            HIS_SPI_FpgaWd(0x12,0x0b);    
            //HIS_SPI_FpgaWd(0x604,winEna);
            break;
        case 3: //1920*1080 p60 VSEA
            data = data & 0xff;
            data = data | 0x900; //60HZ
            //data = data & 0xfdff; //1080p
            //HIS_SPI_FpgaWd(0x12,0);
            //HIS_SPI_FpgaWd(0x604,0);
            HIS_SPI_FpgaWd(0x600,data);    
            HIS_SPI_FpgaWd(0x15,0xf0f0);
            HIS_SPI_FpgaWd(0x12,0x0b);
            //HIS_SPI_FpgaWd(0x604,winEna);
            break;
        case 4: //1920*1080 i60
            //HIS_SPI_FpgaWd(0x12,0);
            //HIS_SPI_FpgaWd(0x604,0);
            data = data | 0x900; //60HZ
            data = data & 0xfdff; //1080p
            HIS_SPI_FpgaWd(0x600,data); 
            HIS_SPI_FpgaWd(0x15,0xf0f0);  
            HIS_SPI_FpgaWd(0x12,0x0b); 
            //HIS_SPI_FpgaWd(0x604,winEna);
            break;
        case 5: //1920*1080 i50
            data = data | 0x900; //60HZ
            data = data & 0xfdff; //1080p
            //HIS_SPI_FpgaWd(0x12,0);
            //HIS_SPI_FpgaWd(0x604,0);
            HIS_SPI_FpgaWd(0x600,data); 
            HIS_SPI_FpgaWd(0x15,0xf0f0);  
            HIS_SPI_FpgaWd(0x12,0x0b); 
            //HIS_SPI_FpgaWd(0x604,winEna);
            break;
        default:
            break;
    }
    return HIS_DIS_SetCustomTiming((WV_U32) resolution);
}
/*****************************************************************************
 * WV_S32 FPGA_CONF_SetOutPutDisColorDepth(WV_S32 colorDepth)
 * //设置输出颜色位宽
 * **************************************************************************/
void FPGA_CONF_SetOutPutDisColorDepth(WV_S32 colorDepth)
{
    //设置colorDepth
    WV_U16 data=0;
    HIS_SPI_FpgaRd(0x600,&data);
    colorDepth &= 0x1;
    data |= colorDepth <<9;
    HIS_SPI_FpgaRd(0x600,&data);
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
//设置叠加显示透明度，取值范围[0~128],0代表透明，128代表不透明
****************************************************************************/
void FPGA_CONF_SetDisAlpha(WV_S32 alpha)
{
    if(alpha>=0 && alpha <= 128){
        HIS_SPI_FpgaWd(0x605,(WV_U16)alpha);
    }else if(alpha > 128){
        HIS_SPI_FpgaWd(0x605,128);
    }

}
/****************************************************************************

void FPGA_CONF_SetOutPutAudioSel(WV_S32 winID,WV_S32 audioChl) 
//设置输出声音来自那个窗口,第几个声道
****************************************************************************/
void FPGA_CONF_SetOutPutAudioSel(WV_S32 winID,WV_S32 audioChl) 
{
    if(winID > 15 || audioChl < 0 ) return; //audioSel取值范围[0~15]
    if(audioChl > 8 || audioChl < 0 ) return; //audioChl取值范围[0~16]
    if(gpFpgaConfDev->win[winID].win_ena == 0) return; //如果窗口未使能，则返回错误

    
    //WV_U16 data;
    if(strcmp(gpFpgaConfDev->win[winID].type,"eth") == 0){
        //HIS_SPI_FpgaRd(0x600,&data);
        //data |= (audioChl & 0xf);
            
    }else if(strcmp(gpFpgaConfDev->win[winID].type,"sdi") == 0){


    }if(strcmp(gpFpgaConfDev->win[winID].type,"hdmi") == 0)
    {

    }
}
/****************************************************************************

void FPGA_CONF_SetOutPutVolume(WV_S32 volume)
//设置输出音量的大小 【0～15】
****************************************************************************/
void FPGA_CONF_SetOutPutVolume(WV_S32 volume)
{
    
}

/****************************************************************************

WV_S32 FPGA_CONF_GetSdpInfo(WV_S32 winID,FPGA_SDP_Info *pSdpInfoOut);
//查询窗口sdp信息
****************************************************************************/
WV_S32 FPGA_CONF_GetSdpInfo(WV_S32 winID,FPGA_SDP_Info *pSdpInfoOut)
{
    WV_S32 ret=-1;
    if(strcmp(gpFpgaConfDev->win[winID].type,"eth") == 0){
        
        ret=FPGA_SDP_ReadFromFpga(gWinInfo[winID].ethChl,gWinInfo[winID].ipSel,pSdpInfoOut);
    }
    return ret;
}
/****************************************************************************

WV_S32 FPGA_CONF_GetSdpInfo(WV_S32 winID,FPGA_SDP_Info *pSdpInfoOut);
//设置窗口sdp信息
****************************************************************************/
WV_S32 FPGA_CONF_SetSdpInfo(WV_S32 winID,FPGA_SDP_Info *pSdpInfoIn)
{
    WV_S32 ret=-1;
    if(strcmp(gpFpgaConfDev->win[winID].type,"eth") == 0){
        
        ret=FPGA_SDP_SetInfo(pSdpInfoIn,gWinInfo[winID].ethChl,gWinInfo[winID].ipSel);
    }
    return ret;
}

/****************************************************************************

WV_S32 FPGA_CONF_SetCmd(WV_S32 argc, WV_S8 **argv,WV_S8 *prfBuff)

****************************************************************************/
WV_S32 FPGA_CONF_SetCmd(WV_S32 argc, WV_S8 **argv, WV_S8 *prfBuff)
{
    WV_U32  data,id;
    WV_U16 x=0,y=0,w=0,h=0;
    WV_S32 ret=0;
    if (argc < 1)
    {
        prfBuff += sprintf(prfBuff, "set fpga <cmd>;//cmd like: wincfg/win/dis/alpha/reset\r\n");
        return 0;
    }

    if(strcmp(argv[0], "wincfg") == 0 ){

        if(argc < 2 ){
            
            prfBuff += sprintf(prfBuff, "set fpga wincfg <win.json>\r\n");

        }
        if(strstr(argv[1],"win") == NULL || strstr(argv[1],"json") == NULL)
        {
            prfBuff += sprintf(prfBuff, "err!cmd like: set fpga wincfg <win.json>\r\n");
            return 0;
        }
        FPGA_CONF_WIN_T win[16];
        memset(win,0,sizeof(FPGA_CONF_WIN_T)*16);

        if(FPGA_CONF_WinAnalysisJson(argv[1],win) == 0)
        {
            FPGA_CONF_SetWin(win);
        }
        return 0;

    }else if (strcmp(argv[0], "win") == 0 ) {

        if(argc < 6)
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
        if(id >= 16 ){
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
        
        prfBuff += sprintf(prfBuff, "set win[%d],x=%d,y=%d,w=%d,h=%d\n",id,x,y,w,h);
        WV_U16 baseAddr = 0x0500;
        WV_U16 regAddr=0;

        regAddr = ((baseAddr >> 4) + id) << 4;

        prfBuff += sprintf(prfBuff, "ret 0x%04x=0x%04x[%d]\r\n",regAddr,x,x);
        prfBuff += sprintf(prfBuff, "ret 0x%04x=0x%04x[%d]\r\n",regAddr+1,y,y);
        prfBuff += sprintf(prfBuff, "ret 0x%04x=0x%04x[%d]\r\n",regAddr+2,w,w);
        prfBuff += sprintf(prfBuff, "ret 0x%04x=0x%04x[%d]\r\n",regAddr+3,h,h);
        
        ret += HIS_SPI_FpgaWd(regAddr, x);
        ret += HIS_SPI_FpgaWd(regAddr + 1, y);
        ret += HIS_SPI_FpgaWd(regAddr + 2, w);
        ret += HIS_SPI_FpgaWd(regAddr + 3, h);    

        WV_U16 winEna=0;
        HIS_SPI_FpgaRd(0x604,&winEna);
        winEna |= 1 << id;
        HIS_SPI_FpgaWd(0x604,winEna);
    }else if (strcmp(argv[0], "dis") == 0 ) {

        if(argc < 2 ){
            
            prfBuff += sprintf(prfBuff, "set fpga dis <num>//num(0)4k60 1<4k50>  2<4k30> 3<1080p60> 4<1080i60> 5<1080i50> 6<1080i30> 7<1080P30>\r\n");
            return WV_SOK;
        }
        ret = WV_STR_S2v(argv[1], &data);
		if (ret != WV_SOK)
		{
			prfBuff += sprintf(prfBuff, "input erro!\r\n");
			return WV_SOK;
		}
        prfBuff += sprintf(prfBuff, "set fpga dis =%d\r\n",data);
        //FPGA_CONF_SetResolution(data);
        FPGA_CONF_Resolution(data);
    }else if (strcmp(argv[0], "alpha") == 0 ){

        if(argc < 2 ){
            
            prfBuff += sprintf(prfBuff, "set fpga alpha <val>//val取值范围0～128\r\n");
            return WV_SOK;
        }
        ret = WV_STR_S2v(argv[1], &data);
		if (ret != WV_SOK)
		{
			prfBuff += sprintf(prfBuff, "input erro!\r\n");
			return WV_SOK;
		}
        FPGA_CONF_SetDisAlpha((WV_S32 )data);
    } if(strcmp(argv[0], "reset") == 0 ){
        PCA9555_Clr(0x3,0x2);
        usleep(500000);
        PCA9555_Set(0x3,0x2);
        
    }
    return 0;
}

/****************************************************************************

WV_S32 FPGA_CONF_GetCmd(WV_S32 argc, WV_S8 **argv,WV_S8 *prfBuff)

****************************************************************************/
WV_S32 FPGA_CONF_GetCmd(WV_S32 argc, WV_S8 **argv, WV_S8 *prfBuff)
{
    if (argc < 1)
    {

        prfBuff += sprintf(prfBuff, "set fpga <cmd>;//cmd like: win\r\n");
        return 0;
    }
    //设置寄存器
    if (strcmp(argv[0], "win") == 0)
    {
        WV_U16 winEna;

        HIS_SPI_FpgaRd(0x604,&winEna);
        WV_S32 i;
        for(i=0;i<FPGA_CONF_WINNUM_D;i++){
            if( ((1<<i) & winEna) != 0){
                if(FPGA_CONF_GetWinStream(i) == 1)
                {
                    prfBuff += sprintf(prfBuff, "win[%d] video stream is ok\r\n",i);
                    //if(FPGA_CONF_GetWinFreeze(i) == 1){
                        prfBuff += sprintf(prfBuff, "win[%d] video stream r sum=0x%X\r\n",i,FPGA_CONF_GetWinFreeze(i));
                    //}
                }else{
                    prfBuff += sprintf(prfBuff, "win[%d] video stream is error\r\n",i);
                }
            }
        }

        
    }
    return WV_SOK;
}

/*******************************************************************
WV_S32 FPGA_CONF_Reset();
fpga复位
*******************************************************************/
WV_S32 FPGA_CONF_Reset()
{
    PCA9555_Clr(0x3,0x2);
    usleep(100000);
    PCA9555_Set(0x3,0x2);
    usleep(100000);
    return WV_SOK;
}
/*******************************************************************
void FPGA_CONF_Init();
fpga初始化
*******************************************************************/
void FPGA_CONF_Init()
{
#if _FPGA_FILE_MUTEX_ENA_D    

    if (pthread_mutex_init(&gMutexWin, NULL) != 0 \
        || pthread_mutex_init(&gMutexEth, NULL) != 0 \
        || pthread_mutex_init(&gMutexAlpha, NULL) != 0)
    {
            // 互斥锁初始化失败
            WV_ERROR("mutex init err\n");
            return ;
    }
#endif
    FPGA_CONF_Reset();
    gpFpgaConfDev = (FPGA_CONF_DEV *)malloc(sizeof(FPGA_CONF_DEV));
    memset(gpFpgaConfDev,0,sizeof(FPGA_CONF_DEV));
    
    //memset(&gWarnInfo,0,sizeof(gWarnInfo));

    HIS_SPI_FpgaWd(0x601,0x0);
    HIS_SPI_FpgaWd(0x602,0x0);
    HIS_SPI_FpgaWd(0x603,0x0);
    //sdp init
    FPGA_SDP_Init();
    WV_CMD_Register("set", "fpga", " set fpga", FPGA_CONF_SetCmd);
    WV_CMD_Register("get", "fpga", "pca9555 get reg", FPGA_CONF_GetCmd);
    
    return ;
}
void FPGA_CONF_DeInit()
{
#if _FPGA_FILE_MUTEX_ENA_D 
    pthread_mutex_destroy(&gMutexEth);
    pthread_mutex_destroy(&gMutexWin);
    pthread_mutex_destroy(&gMutexAlpha);
#endif
    free(gpFpgaConfDev);
    //sdp init
    FPGA_SDP_DeInit();
    return ;
}