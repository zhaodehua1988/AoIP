#include "fpga_conf.h"
#include "fpga_common.h"
#include "his_spi.h"
#include "sys_env.h"
#include "sys_ip.h"
#include "fpga_conf_json.h"
#include "fpga_sdp.h"

#define FPGA_CONF_FILEPATH_WIN_D "./env/win.ini"
#define FPGA_CONF_FILEPATH_ETH_D "./env/eth.ini"
#define FPGA_CONF_FILEPATH_ALPHA_D "./env/volAlpha.ini"

#define FPGA_FILE_MUTEX_ENA_D 0

#define FPGA_DEBUG
FPGA_CONF_DEV *gpFpgaConfDev;

#if FPGA_FILE_MUTEX_ENA_D
pthread_mutex_t gMutexWin;
pthread_mutex_t gMutexEth;
pthread_mutex_t gMutexAlpha;
#endif

/*******************************************************************
WV_S32 FPGA_CONF_getMacInt(WV_U8* pMac);
*******************************************************************/
WV_S32 FPGA_CONF_getMacInt(WV_S8 *pSrc,WV_S8* pMac)
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
			//#define FPGA_DEBUG
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
WV_S32 FPGA_CONF_getIpInt(WV_S8 *pName,WV_U8* pIp);
*******************************************************************/
WV_S32 FPGA_CONF_getIpInt(WV_S8 *pSrc,WV_S8* pIp)
{

	WV_S8* pData = pSrc;	
	WV_S32 len;
	WV_S32 i,j,k,data=0;
	WV_S32 des;
	len = strlen(pSrc);
	if(strncmp(pData,".",1) == 0){
		printf("get ip error\r\n");
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

/*****************************************************
WV_S32 FPGA_CONF_DisChangeEna(WV_U16 winena)
*设置视频输出更新（包括窗口坐标更新生效）
*****************************************************/
WV_S32 FPGA_CONF_DisChangeEna(WV_U16 winena)
{
   
    WV_U16 temp=0;
    WV_S32 ret=0;
    ret +=HIS_SPI_FpgaWd(0x604,winena);
    ret +=HIS_SPI_FpgaRd(0x600,&temp);
    temp |= 1<<12;
    ret+=HIS_SPI_FpgaWd(0x600,temp);
    return ret;
}
/*******************************************************************
WV_S32 FPGA_CONF_SetWinIpAndSdp(FPGA_CONF_WIN_T *pWin);
解析窗口ip,sdp并设置
*******************************************************************/
WV_S32 FPGA_CONF_SetWinIpAndSdp(FPGA_CONF_WIN_T *pWin)
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
            printf("video type err,type must be \"net\",\"sdi\",\"hdmi\" \r\n");
            return WV_EFAIL;
        }
        if(strcmp(pWin[i].type,"net") == 0){
            video_type = 0;
            av_sel[i]|= 0 << 10;
        }else if(strcmp(pWin[i].type,"sdi") == 0){
            video_type = 1;
            av_sel[i]|= 1 << 10;
            av_sel[i]|= pWin[i].channel & 0x3;
        }else if(strcmp(pWin[i].type,"hdmi") == 0){
            video_type = 2;
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
                            break;
                        }
                        if(3==k) {
                            printf("video 网卡[%d]接收的ip数量超过4个\r\n",pWin[i].channel);
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
                            printf("网卡[%d]接收的ip数量超过4个\r\n",pWin[i].channel);
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
        printf("win[%d] av_sel=0x%04X",i,av_sel[i]);
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
        //printf("reg[%X]=%X\n",regAddr + 0x1f,srFPGA_FILE_MUTEX_ENA_DcAddrIpv4OrIpv6);
        
        for (j = 0; j < 4; j++)
        { 
            if(srcAddr[i][j].ipv4OrIpv6 == 0){ //ipv4
                memset(ip,0,sizeof(ip));
                FPGA_CONF_getIpInt(srcAddr[i][j].ipv6,ip);
                //printf("ip[%d][%d]=[%d.%d.%d.%d]\n",i,j,ip[0],ip[1],ip[2],ip[3]);
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
                if(pWin[k].video_ipv6_ena == srcAddr[i][j].ipv4OrIpv6 \
                   && pWin[k].video_port == srcAddr[i][j].port \
                   && 0 == strcmp(pWin[k].video_ip,srcAddr[i][j].ipv6))
                {
                    //设置视频sdp信息
                    FPGA_SDP_SetInfo(&pWin[k].sdpInfo,0,(WV_U16)i,(WV_U16)j);
                }
                if(pWin[k].audio_ipv6_ena == srcAddr[i][j].ipv4OrIpv6 \
                   && pWin[k].audio_port == srcAddr[i][j].port \
                   && 0 == strcmp(pWin[k].audio_ip,srcAddr[i][j].ipv6))
                {
                    //设置音频sdp信息
                    FPGA_SDP_SetInfo(&pWin[k].sdpInfo,1,(WV_U16)i,(WV_U16)j);
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
         ret += HIS_SPI_FpgaWd(regAddr + 4, av_sel[i]);
    }
    
    if(ret != 0 ){

        return WV_EFAIL;
    }
    return WV_SOK;
}

/*******************************************************************
WV_S32 FPGA_CONF_SaveWinCfgToFile();
保存窗口信息
*******************************************************************/
WV_S32 FPGA_CONF_SaveWinCfgToFile()
{

#if FPGA_FILE_MUTEX_ENA_D    
    if(pthread_mutex_trylock(&gMutexWin) != 0 ){
        return WV_EFAIL;
    }
#endif
    WV_S32 ret=0;
    FILE *fp;
    fp=fopen(FPGA_CONF_FILEPATH_WIN_D,"wb");
    if(fp == NULL){
        FPGA_printf("save wincfg err:open file %s err\n",FPGA_CONF_FILEPATH_WIN_D);
        ret = -1;
    }
    else if(fwrite((WV_S8 *)gpFpgaConfDev->win,1,sizeof(FPGA_CONF_WIN_T)*FPGA_CONF_WINNUM_D,fp) != sizeof(FPGA_CONF_WIN_T)*FPGA_CONF_WINNUM_D)
    {
        FPGA_printf("save wincfg err:fwrite file %s err\n",FPGA_CONF_FILEPATH_WIN_D);
        fclose(fp);
        ret = -1;
    }else{
        fclose(fp);
    }
    

#if FPGA_FILE_MUTEX_ENA_D   
    pthread_mutex_unlock(&gMutexWin);
#endif
    return WV_SOK;
}
/*******************************************************************
WV_S32 FPGA_CONF_ReadWinCfgFromFile();
读取窗口信息
*******************************************************************/
WV_S32 FPGA_CONF_ReadWinCfgFromFile()
{

#if FPGA_FILE_MUTEX_ENA_D    
    if(pthread_mutex_trylock(&gMutexWin) != 0 ){
        return WV_EFAIL;
    }
#endif
    WV_S32 ret = 0;
    FILE *fp;
    fp=fopen(FPGA_CONF_FILEPATH_WIN_D,"rb");
    if(fp == NULL){
        FPGA_printf("read wincfg err:open file %s err\n",FPGA_CONF_FILEPATH_WIN_D);
        ret = -1;
    }
    else if(fread((WV_S8 *)gpFpgaConfDev->win,1,sizeof(FPGA_CONF_WIN_T)*FPGA_CONF_WINNUM_D,fp) != sizeof(FPGA_CONF_WIN_T)*FPGA_CONF_WINNUM_D)
    {
        FPGA_printf("read wincfg err:fwrite file %s err\n",FPGA_CONF_FILEPATH_WIN_D);
        fclose(fp);
        ret = -1;;
    }else{
        fclose(fp);
    }

#if FPGA_FILE_MUTEX_ENA_D   
    pthread_mutex_unlock(&gMutexWin);
#endif
    return WV_SOK;
}


/*******************************************************************
WV_S32 FPGA_CONF_SetWin(FPGA_CONF_WIN_T *pWin);
fpga窗口设置
*******************************************************************/
WV_S32 FPGA_CONF_SetWin(FPGA_CONF_WIN_T *pWin)
{


    WV_S32 i, ret = 0;
    WV_U16 baseAddr = 0x0500;
    WV_U16 regAddr,winEna=0;
    //set src ip
    ret +=FPGA_CONF_SetWinIpAndSdp(pWin);
    //设置窗口位置信息
    for (i = 0; i < FPGA_CONF_WINNUM_D; i++)
    {
        winEna |= pWin[i].win_ena << i;
        regAddr = ((baseAddr >> 4) + i) << 4;
        ret += HIS_SPI_FpgaWd(regAddr, pWin[i].x);
        ret += HIS_SPI_FpgaWd(regAddr + 1, pWin[i].y);
        ret += HIS_SPI_FpgaWd(regAddr + 2, pWin[i].w);
        ret += HIS_SPI_FpgaWd(regAddr + 3, pWin[i].h);
        
    }
    //设置窗口使能
    ret +=FPGA_CONF_DisChangeEna(winEna);
   
    if(ret == 0){
        FPGA_printf("set windows ok \n");
        memcpy(gpFpgaConfDev->win,pWin,sizeof(FPGA_CONF_WIN_T)*FPGA_CONF_WINNUM_D);
        FPGA_CONF_SaveWinCfgToFile();
    }else{
        WV_ERROR("FPGA_CONF_SetWin err\n");
    }
   
    return ret;
}

/*******************************************************************
WV_S32 FPGA_CONF_GetWin(FPGA_CONF_WIN_T *pWin);
fpga读取窗口信息
*******************************************************************/
WV_S32 FPGA_CONF_GetWin(FPGA_CONF_WIN_T *pWin)
{
    memcpy((WV_S8 *)pWin,(WV_S8 *)gpFpgaConfDev->win,sizeof(FPGA_CONF_WIN_T)*FPGA_CONF_WINNUM_D);
    return WV_SOK;
}


/*******************************************************************
WV_S32 FPGA_CONF_SaveEthCfgToFile();
保存网卡信息
*******************************************************************/
WV_S32 FPGA_CONF_SaveEthCfgToFile()
{
#if FPGA_FILE_MUTEX_ENA_D    
    if(pthread_mutex_trylock(&gMutexEth) != 0 ){
        return WV_EFAIL;
    }
#endif
    WV_S32 ret=0;
    FILE *fp;
    fp=fopen(FPGA_CONF_FILEPATH_ETH_D,"wb");
    if(fp == NULL){
        FPGA_printf("save ethcfg err:open file %s err\n",FPGA_CONF_FILEPATH_ETH_D);
        ret = -1;
    
    }else if(fwrite((WV_S8 *)gpFpgaConfDev->eth,1,sizeof(FPGA_CONF_ETH_T)*FPGA_CONF_ETHNUM_D,fp) != sizeof(FPGA_CONF_ETH_T)*FPGA_CONF_ETHNUM_D){
        
        FPGA_printf("save ethcfg err:fwrite file %s err\n",FPGA_CONF_FILEPATH_ETH_D);
        fclose(fp);
        ret = -1;
    }else{
        fclose(fp);
    }

#if FPGA_FILE_MUTEX_ENA_D   
    pthread_mutex_unlock(&gMutexEth);
#endif
    return ret;
}
/*******************************************************************
WV_S32 FPGA_CONF_ReadEthCfgFromFile();
读取网卡信息
*******************************************************************/
WV_S32 FPGA_CONF_ReadEthCfgFromFile()
{
#if FPGA_FILE_MUTEX_ENA_D    
    if(pthread_mutex_trylock(&gMutexEth) != 0 ){
        return WV_EFAIL;
    }
#endif   
    WV_S32 ret=0;
    FILE *fp;
    fp=fopen(FPGA_CONF_FILEPATH_ETH_D,"rb");
    if(fp == NULL){
        FPGA_printf("read ethcfg err:open file %s err\n",FPGA_CONF_FILEPATH_ETH_D);
        ret = -1;
    
    }else if(fread((WV_S8 *)gpFpgaConfDev->eth,1,sizeof(FPGA_CONF_ETH_T)*FPGA_CONF_ETHNUM_D,fp) != sizeof(FPGA_CONF_ETH_T)*FPGA_CONF_ETHNUM_D){
        FPGA_printf("read ethcfg err:fwrite file %s err\n",FPGA_CONF_FILEPATH_ETH_D);
        fclose(fp);
        ret = -1;
    }else{
        fclose(fp);
    }

#if FPGA_FILE_MUTEX_ENA_D   
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
    ret +=FPGA_CONF_getIpInt(pEth->ipv6,ipInt);
    ret +=FPGA_CONF_getMacInt(pEth->mac,mac);
    ret +=FPGA_CONF_getIpInt(pEth->subnet_mask,netMask);
    ret +=FPGA_CONF_getIpInt(pEth->getway,getWay);
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

    ret += HIS_SPI_FpgaWd(regAddr + 0xd,getWay[2]<<8 | getWay[3]);
    ret += HIS_SPI_FpgaWd(regAddr + 0xe,getWay[0]<<8 | getWay[1]);    
    if(ret != 0 ){
        WV_ERROR("set eth[%d] err\n",ethID);
    }else{
        memcpy((WV_S8 *)&gpFpgaConfDev->eth[ethID],(WV_S8 *)pEth,sizeof(FPGA_CONF_ETH_T));
        FPGA_CONF_SaveEthCfgToFile();
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

/**************************************************************
* WV_S32 FPGA_CONF_SetTransparency(WV_S32 alpha);
*设置音量显示透明度
*************************************************************/
WV_S32 FPGA_CONF_SetTransparency(WV_S32 alpha)
{

#if FPGA_FILE_MUTEX_ENA_D    
    if(pthread_mutex_trylock(&gMutexAlpha) != 0 ){
        return WV_EFAIL;
    }
#endif 
    WV_S32 ret = 0;
    if(alpha >= 128 || alpha < 0){
        FPGA_printf("save alpha err!alpha=%d, [0< alpha <=128)\n",alpha);
        ret = -1;       
    }else{

        FILE *fp;
        fp=fopen(FPGA_CONF_FILEPATH_ALPHA_D,"wb");
        if(fp == NULL){
            FPGA_printf("fopen file [%s]err\n",FPGA_CONF_FILEPATH_ALPHA_D);
            ret = -1;
        }
        else if(fwrite((WV_S8*)&alpha,1,4,fp) != sizeof(alpha))
        {
            FPGA_printf("save alpha to file err\n");
            fclose(fp);
            ret= -1;
        }else{
            fclose(fp);
        }


    }

#if FPGA_FILE_MUTEX_ENA_D   
    pthread_mutex_unlock(&gMutexAlpha);
#endif
    return WV_EFAIL;

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
        prfBuff += sprintf(prfBuff, "set fpga <cmd>;//cmd like: wincfg/win/eth/dis\r\n");
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
    }
    return 0;
}

/*******************************************************************
void FPGA_CONF_Init();
fpga初始化
*******************************************************************/
void FPGA_CONF_Init()
{
#if FPGA_FILE_MUTEX_ENA_D    

    if (pthread_mutex_init(&gMutexWin, NULL) != 0 \
        || pthread_mutex_init(&gMutexEth, NULL) != 0 \
        || pthread_mutex_init(&gMutexAlpha, NULL) != 0)
    {
            // 互斥锁初始化失败
            WV_ERROR("mutex init err\n");
            return ;
    }
#endif
    gpFpgaConfDev = (FPGA_CONF_DEV *)malloc(sizeof(FPGA_CONF_DEV));
    memset(gpFpgaConfDev,0,sizeof(FPGA_CONF_DEV));

    WV_S32 i;
    HIS_SPI_FpgaWd(0x15,0xff00);
    //HIS_SPI_FpgaWd(0x12,0x189);
    HIS_SPI_FpgaWd(0x600,0x800);
    HIS_SPI_FpgaWd(0x601,0x0);
    HIS_SPI_FpgaWd(0x602,0x0);
    HIS_SPI_FpgaWd(0x603,0x0);
    HIS_SPI_FpgaWd(0x605,100);
    //HIS_SPI_FpgaWd(0x15,0xff00);
   
    FPGA_CONF_GetVersion(gpFpgaConfDev->fpgaVer);
    //test set eth
    
    if(FPGA_CONF_ReadEthCfgFromFile() == WV_SOK)
    {
        for(i=0;i<FPGA_CONF_ETHNUM_D;i++){
            FPGA_CONF_SetETH(&gpFpgaConfDev->eth[i],i);
        }
    }

    if(FPGA_CONF_ReadWinCfgFromFile() == WV_SOK)
    {
        FPGA_CONF_SetWin(gpFpgaConfDev->win);
    }
    
    WV_CMD_Register("set", "fpga", " set fpga", FPGA_CONF_SetCmd);

    //WV_CMD_Register("get", "fpga", "pca9555 get reg", FPGA_CONF_GetCmd);

    return ;
}
void FPGA_CONF_DeInit()
{
#if FPGA_FILE_MUTEX_ENA_D 
    pthread_mutex_destroy(&gMutexEth);
    pthread_mutex_destroy(&gMutexWin);
    pthread_mutex_destroy(&gMutexAlpha);
#endif
    free(gpFpgaConfDev);
    return ;
}
#if 0

/****************************************************************************

WV_S32 FPGA_CONF_SetCmd(WV_S32 argc, WV_S8 **argv,WV_S8 *prfBuff)

****************************************************************************/
WV_S32 FPGA_CONF_SetCmd(WV_S32 argc, WV_S8 **argv, WV_S8 *prfBuff)
{
    WV_U32  data,id;
    WV_S32 ret=0;
    if (argc < 1)
    {

        prfBuff += sprintf(prfBuff, "set fpga <cmd>;//cmd like: win/eth\r\n");
        return 0;
    }
    //设置寄存器
    if (strcmp(argv[0], "win") == 0 )
    {
        if(argc < 2 ){
            prfBuff += sprintf(prfBuff, "set fpga win def //(set win default)\r\n");
            prfBuff += sprintf(prfBuff, "set fpga win <id> <x> <y> <w> <h> <videoInType> <videoID> <video_ip><audio_ip>\r\n");

        }
        
        if (strcmp(argv[1], "def") == 0 )
        {  
            FPGA_CONF_SetConfDefault(&gfpgaConfDev);
            FPGA_CONF_SetWin(gfpgaConfDev.win);

            return WV_SOK;
        }else if(argc < 6)
        {
            prfBuff += sprintf(prfBuff, "set fpga win <id> <x> <y> <w> <h> <videoInType> <videoID> <video_ip><audio_ip>\r\n");
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
        gfpgaConfDev.win[id].x = data;
        //get win y
        ret = WV_STR_S2v(argv[3], &data);
		if (ret != WV_SOK)
		{
			prfBuff += sprintf(prfBuff, "input erro!\r\n");
			return WV_SOK;
		}
        gfpgaConfDev.win[id].y = data;
        //get win w
        ret = WV_STR_S2v(argv[4], &data);
		if (ret != WV_SOK)
		{
			prfBuff += sprintf(prfBuff, "input erro!\r\n");
			return WV_SOK;
		}
        gfpgaConfDev.win[id].w = data;
        //get win h
        ret = WV_STR_S2v(argv[5], &data);
		if (ret != WV_SOK)
		{
			prfBuff += sprintf(prfBuff, "input erro!\r\n");
			return WV_SOK;
		}
        gfpgaConfDev.win[id].h = data;
        if(argc >= 8 ){

            //get video type
            ret = WV_STR_S2v(argv[6], &data);
            if (ret != WV_SOK)
            {
                prfBuff += sprintf(prfBuff, "input erro!\r\n");
                return WV_SOK;
            }     
            gfpgaConfDev.win[id].video_type = data;
             //get video id
            ret = WV_STR_S2v(argv[7], &data);
            if (ret != WV_SOK)
            {
                prfBuff += sprintf(prfBuff, "input erro!\r\n");
                return WV_SOK;
            }
            gfpgaConfDev.win[id].sel_in = data;     

        }
        if(argc >= 9){
             //get video id
            ret = WV_STR_S2v(argv[8], &data);
            if (ret != WV_SOK)
            {
                prfBuff += sprintf(prfBuff, "input erro!\r\n");
                return WV_SOK;
            }
            gfpgaConfDev.win[id].video_ip = data;     
        }
        if(argc == 10){
             //get video id
            ret = WV_STR_S2v(argv[9], &data);
            if (ret != WV_SOK)
            {
                prfBuff += sprintf(prfBuff, "input erro!\r\n");
                return WV_SOK;
            }
            gfpgaConfDev.win[id].audio_ip = data;     
        }
        if(argc > 10 ){
            prfBuff += sprintf(prfBuff, "input erro!\r\n");
        }

        WV_U16 baseAddr = 0x0500;
        WV_U16 regAddr=0,avSel=0;

        regAddr = ((baseAddr >> 4) + id) << 4;
        ret += HIS_SPI_FpgaWd(regAddr, gfpgaConfDev.win[id].x);
        ret += HIS_SPI_FpgaWd(regAddr + 1, gfpgaConfDev.win[id].y);
        ret += HIS_SPI_FpgaWd(regAddr + 2, gfpgaConfDev.win[id].w);
        ret += HIS_SPI_FpgaWd(regAddr + 3, gfpgaConfDev.win[id].h);

        avSel |= (gfpgaConfDev.win[id].video_type & 0x3) << 10;
        avSel |= (gfpgaConfDev.win[id].video_ip & 0x3) << 6;
        avSel |= (gfpgaConfDev.win[id].audio_ip & 0x3) << 2;
        avSel |= gfpgaConfDev.win[id].sel_in & 0x3;
        ret += HIS_SPI_FpgaWd(regAddr + 4, avSel);       
        FPGA_CONF_DisChangeEna();
        FPGA_CONF_SaveConfToFile();
    }else if(strcmp(argv[0], "winena") == 0){
        if(argc != 2  ){
            prfBuff += sprintf(prfBuff, "input erro!cmd like:set fpga winena <val>\r\n");
            return WV_SOK;
        }
        ret = WV_STR_S2v(argv[1], &data);
        if (ret != WV_SOK)
        {
            prfBuff += sprintf(prfBuff, "input erro!\r\n");
            return WV_SOK;
        }
        gfpgaConfDev.display.valid = data;
        FPGA_CONF_DisConf(&gfpgaConfDev.display);
        FPGA_CONF_SaveConfToFile();
    }else if(strcmp(argv[0], "eth") == 0){
        if(argc <2 ){
            prfBuff += sprintf(prfBuff, "cmd like:set fpga eth <cmd> ;//cmd like:def/local/src/des/\r\n");
            return WV_SOK;
        }
        if(strcmp(argv[1], "def") == 0){//set to default
           FPGA_CONF_NetConf(gfpgaConfDev.eth);
           prfBuff += sprintf(prfBuff, "set net config to default \r\n");
        }else if(strcmp(argv[1], "local") == 0){//set local eth ip and mac
            if(argc <6){
                prfBuff += sprintf(prfBuff, "cmd like:set fpga eth local <eth_id> <str_ip><str_mask><str_getway><str_mac>\r\n");

                return WV_SOK;              
            }
            //get eth num
            ret = WV_STR_S2v(argv[2], &data);
            if (ret != WV_SOK || data > 3)
            {
                prfBuff += sprintf(prfBuff, "input erro!\r\n");
                return WV_SOK;
            }
            WV_U8 ip[10]={0};
            WV_U8 mask[10] = {0};
            WV_U8 getway[10]={0};
            WV_U8 mac[10]={0};
            //get ip
            FPGA_CONF_getIpInt(argv[3],ip);
  	        prfBuff += sprintf(prfBuff,"eth[%d]ip %d.%d.%d.%d \n",data,ip[0],ip[1],ip[2],ip[3]);  
            //get  netmask
            FPGA_CONF_getIpInt(argv[4],mask);
  	        prfBuff += sprintf(prfBuff,"eth[%d]mask %d.%d.%d.%d \n",data,mask[0],mask[1],mask[2],mask[3]);  
            //get getway
            FPGA_CONF_getIpInt(argv[5],getway);
  	        prfBuff += sprintf(prfBuff,"eth[%d]getway %d.%d.%d.%d \n",data,getway[0],getway[1],getway[2],getway[3]);            
            if(argc == 7){
                //get mac
                FPGA_CONF_getMacInt(argv[6],mac);
	            prfBuff += sprintf(prfBuff,"eth[%d]mac  %02x:%02x:%02x:%02x:%02x:%02x \n",data,mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
            }
            WV_S32 i=0,j;
            j=3;
            for(i=0;i<4;i++){
                gfpgaConfDev.eth[data].localAddr.ipv6[i]=ip[j-i];
                gfpgaConfDev.eth[data].localAddr.subnet_mask[i]=mask[j-i];
                gfpgaConfDev.eth[data].localAddr.getway[i]=getway[j-i];        
            }
            j=5;
            for(i=0;i<6;i++){
                gfpgaConfDev.eth[data].localAddr.mac[i]=mac[j-i]; 
            } 
            FPGA_CONF_NetConf(gfpgaConfDev.eth);
            FPGA_CONF_SaveConfToFile();
            //prfBuff += sprintf(prfBuff, "set net config to default \r\n");
        }else if(strcmp(argv[1], "src") == 0){//set local eth ip and mac
            if(argc < 6){
                prfBuff += sprintf(prfBuff, "cmd like:set fpga eth src <eth_id> <src_id><src_ip><src_port>\r\n");
                return WV_SOK;   
            }
            //get eth id
            WV_S32 i,j;
            WV_U32 ethID=0,srcID=0,port;
            WV_U8 ip[10];
            ret = WV_STR_S2v(argv[2], &ethID);
            if (ret != WV_SOK || ethID > 3)
            {
                prfBuff += sprintf(prfBuff, "input erro!\r\n");
                return WV_SOK;
            }
            ret = WV_STR_S2v(argv[3], &srcID);
            if (ret != WV_SOK || srcID > 3)
            {
                prfBuff += sprintf(prfBuff, "input erro!\r\n");
                return WV_SOK;
            }
            //get srcport
            ret = WV_STR_S2v(argv[5], &port);
            if (ret != WV_SOK )
            {
                prfBuff += sprintf(prfBuff, "input erro!\r\n");
                return WV_SOK;
            }
            gfpgaConfDev.eth[ethID].srcAddr[srcID].port = port;
            //get src ip
            FPGA_CONF_getIpInt(argv[4],ip);
            prfBuff += sprintf(prfBuff,"eth[%d][%d] ip %d.%d.%d.%d ,port[%d]\n",ethID,srcID,ip[0],ip[1],ip[2],ip[3],port);  
            j=3;
            for(i=0;i<4;i++){
                gfpgaConfDev.eth[ethID].srcAddr[srcID].ipv6[i] = ip[j-i];
            }

            FPGA_CONF_NetConf(gfpgaConfDev.eth);
            FPGA_CONF_SaveConfToFile();
        }else if(strcmp(argv[1], "des") == 0){
            WV_U8 ip[10]={0},mac[10] = {0};
            WV_U32 ethID,desPort,srcPort,sdiID,videoPt,audioPt;
            WV_S32 i,j;
            if(argc < 10){
                prfBuff += sprintf(prfBuff, "cmd like:set fpga eth des <eth_id><sdi_id><src_port><des_ip><des_port><des_mac><videoPt><audioPt>\r\n");
                return WV_SOK;                 
            }
            ret = WV_STR_S2v(argv[2], &ethID);
            if (ret != WV_SOK || ethID > 3)
            {
                prfBuff += sprintf(prfBuff, "input erro!\r\n");
                return WV_SOK;
            }
            ret = WV_STR_S2v(argv[3], &sdiID);
            if (ret != WV_SOK || sdiID > 3)
            {
                prfBuff += sprintf(prfBuff, "input erro!\r\n");
                return WV_SOK;
            }
            ret = WV_STR_S2v(argv[4], &srcPort);
            if (ret != WV_SOK)
            {
                prfBuff += sprintf(prfBuff, "input erro!\r\n");
                return WV_SOK;
            }
            ret = WV_STR_S2v(argv[6], &desPort);
            if (ret != WV_SOK )
            {
                prfBuff += sprintf(prfBuff, "input erro!\r\n");
                return WV_SOK;
            }
            ret = WV_STR_S2v(argv[8], &videoPt);
            if (ret != WV_SOK )
            {
                prfBuff += sprintf(prfBuff, "input erro!\r\n");
                return WV_SOK;
            }
            ret = WV_STR_S2v(argv[9],&audioPt);
            if (ret != WV_SOK )
            {
                prfBuff += sprintf(prfBuff, "input erro!\r\n");
                return WV_SOK;
            }
            FPGA_CONF_getIpInt(argv[5],ip);
            FPGA_CONF_getMacInt(argv[7],mac);
            prfBuff += sprintf(prfBuff,"eth[%d],sdi[%d],srcPort[%d],desIP[%d.%d.%d.%d],desPort[%d],desMac[%02x:%02x:%02x:%02x:%02x:%02x]videoPt[%d],audioPt[%d]\n",\
                        ethID,sdiID,srcPort,ip[0],ip[1],ip[2],ip[3],desPort,mac[0],mac[1],mac[2],mac[3],mac[4],mac[5],videoPt,audioPt);

            gfpgaConfDev.eth[ethID].desAddr.desPort = desPort;
            j=3;

            for(i=0;i<4;i++){
                gfpgaConfDev.eth[ethID].desAddr.ipv6[i]=ip[j-i];
            }
            j=5;
            for(i=0;i<5;i++){
                gfpgaConfDev.eth[ethID].desAddr.mac[i]=mac[j-i];
            }
            gfpgaConfDev.eth[ethID].desAddr.srcPort = srcPort;
            gfpgaConfDev.eth[ethID].desAddr.txSel = sdiID & 0x3;
            gfpgaConfDev.eth[ethID].desAddr.txCtrl = ((videoPt & 0x7f) << 7) | (audioPt & 0x7f) | 0x4000;
            
            FPGA_CONF_NetConf(gfpgaConfDev.eth);
            FPGA_CONF_SaveConfToFile();
        }
    }else if(strcmp(argv[0], "def") == 0){

        FPGA_CONF_NetConf(gfpgaConfDev.eth);
        FPGA_CONF_SetWin(gfpgaConfDev.win);
        //FPGA_CONF_SHOW_INFO(&gfpgaConfDev.showInfo);
        gfpgaConfDev.display.valid = 0xffff;
        FPGA_CONF_DisConf(&gfpgaConfDev.display);
        FPGA_CONF_SaveConfToFile();
        //HIS_SPI_FpgaWd(0x604, gfpgaConfDev.win[id].h);
        prfBuff += sprintf(prfBuff, "set net config to default \r\n");
        
    }  
    return WV_SOK;
}
/****************************************************************************

WV_S32 FPGA_CONF_GetCmd(WV_S32 argc, WV_S8 **argv,WV_S8 *prfBuff)

****************************************************************************/
WV_S32 FPGA_CONF_GetCmd(WV_S32 argc, WV_S8 **argv, WV_S8 *prfBuff)
{
        if (argc < 1)
    {

        prfBuff += sprintf(prfBuff, "set fpga <cmd>;//cmd like: win/eth\r\n");
        return 0;
    }
    //设置寄存器
    if (strcmp(argv[0], "win") == 0)
    {

        WV_S32  i,ret = 0;
        WV_U16 baseAddr = 0x0500;
        WV_U16 regAddr, data = 0;
        FPGA_CONF_WIN win;

        //config windows location and video info
        for (i = 0; i < FPGA_CONF_WINNUM_D; i++)
        {
            regAddr = ((baseAddr >> 4) + i) << 4;
            ret += HIS_SPI_FpgaRd(regAddr, &win.x);
            ret += HIS_SPI_FpgaRd(regAddr + 1, &win.y);
            ret += HIS_SPI_FpgaRd(regAddr + 2, &win.w);
            ret += HIS_SPI_FpgaRd(regAddr + 3, &win.h);
            /*
            avSel |= (pWin[i].video_type & 0x3) << 10;
            avSel |= (pWin[i].video_ip & 0x3) << 6;
            avSel |= (pWin[i].audio_ip & 0x3) << 2;
            avSel |= pWin[i].sel_in & 0x3;
            ret += HIS_SPI_FpgaWd(regAddr + 4, avSel);*/
            
            prfBuff += sprintf(prfBuff,"win[%d]X=%d  ",i,win.x);
            prfBuff += sprintf(prfBuff,"Y=%d  ",win.y);
            prfBuff += sprintf(prfBuff,"W=%d  ",win.w);
            prfBuff += sprintf(prfBuff,"H=%d\n",win.h);
        }
        HIS_SPI_FpgaRd(0x0604,&data);
        prfBuff += sprintf(prfBuff,"windows enable 0x0604= 0x%04x\n",data);
        
    }else if(strcmp(argv[0], "eth") == 0){

    
        WV_S32 i,j;
        for(i=0;i<FPGA_CONF_ETHNUM_D;i++){
            //gfpgaConfDev.eth[i].localAddr
            prfBuff += sprintf(prfBuff,"\n-------------------------------------------------\n");
            prfBuff += sprintf(prfBuff,"ETH[%d] localAddr[ip:%d.%d.%d.%d netmask:%d.%d.%d.%d getway:%d.%d.%d.%d  mac:%02x:%02x:%02x:%02x:%02x:%02x]\n",\
                i,\
                gfpgaConfDev.eth[i].localAddr.ipv6[3],\
                gfpgaConfDev.eth[i].localAddr.ipv6[2],\
                gfpgaConfDev.eth[i].localAddr.ipv6[1],\
                gfpgaConfDev.eth[i].localAddr.ipv6[0],\
                gfpgaConfDev.eth[i].localAddr.subnet_mask[3],\
                gfpgaConfDev.eth[i].localAddr.subnet_mask[2],\
                gfpgaConfDev.eth[i].localAddr.subnet_mask[1],\
                gfpgaConfDev.eth[i].localAddr.subnet_mask[0],\
                gfpgaConfDev.eth[i].localAddr.getway[3],\
                gfpgaConfDev.eth[i].localAddr.getway[2],\
                gfpgaConfDev.eth[i].localAddr.getway[1],\
                gfpgaConfDev.eth[i].localAddr.getway[0],\
                gfpgaConfDev.eth[i].localAddr.mac[5],\
                gfpgaConfDev.eth[i].localAddr.mac[4],\
                gfpgaConfDev.eth[i].localAddr.mac[3],\
                gfpgaConfDev.eth[i].localAddr.mac[2],\
                gfpgaConfDev.eth[i].localAddr.mac[1],\
                gfpgaConfDev.eth[i].localAddr.mac[0]);

            prfBuff += sprintf(prfBuff,"ETH[%d]TX,sdi[%d],srcPort[%d],desIP[%d.%d.%d.%d],desPort[%d],desMac[%02x:%02x:%02x:%02x:%02x:%02x]videoPt[%d],audioPt[%d]\n",\
                i,\
                gfpgaConfDev.eth[i].desAddr.txSel & 0x3,\
                gfpgaConfDev.eth[i].desAddr.srcPort,\
                gfpgaConfDev.eth[i].desAddr.ipv6[3],\
                gfpgaConfDev.eth[i].desAddr.ipv6[2],\
                gfpgaConfDev.eth[i].desAddr.ipv6[1],\
                gfpgaConfDev.eth[i].desAddr.ipv6[0],\
                gfpgaConfDev.eth[i].desAddr.desPort,\
                gfpgaConfDev.eth[i].desAddr.mac[5],\
                gfpgaConfDev.eth[i].desAddr.mac[4],\
                gfpgaConfDev.eth[i].desAddr.mac[3],\
                gfpgaConfDev.eth[i].desAddr.mac[2],\
                gfpgaConfDev.eth[i].desAddr.mac[1],\
                gfpgaConfDev.eth[i].desAddr.mac[0],\
                (gfpgaConfDev.eth[i].desAddr.txCtrl>>7) & 0x7f,\
                gfpgaConfDev.eth[i].desAddr.txCtrl & 0x7f );
                
            for(j=0;j<4;j++){
                prfBuff += sprintf(prfBuff,"ETH[%d]inputSrc[%d] :[ip:%d.%d.%d.%d  port=%d \n",\
                i,j,\
                gfpgaConfDev.eth[i].srcAddr[j].ipv6[3],\
                gfpgaConfDev.eth[i].srcAddr[j].ipv6[2],\
                gfpgaConfDev.eth[i].srcAddr[j].ipv6[1],\
                gfpgaConfDev.eth[i].srcAddr[j].ipv6[0],\
                gfpgaConfDev.eth[i].srcAddr[j].port);
            }
            
        }
    }
    return WV_SOK;
}
WV_S32 FPGA_CONF_SetWinTotal(FPGA_CONF_WIN_TOTAL_T *winTotal)
{

}


/*****************************************************
* WV_S32 FPGA_CONF_Init(FPGA_CONF_DEV *pConf);
*初始化FPGA设置
*****************************************************/
void FPGA_CONF_Init()
{
    WV_S32 ret=0; 
    FPGA_VER ver;
    FPGA_CONF_GetVersion(&ver);
    ret=FPGA_CONF_ReadConfFromFile();

    FPGA_CONF_NetConf(gfpgaConfDev.eth);
    FPGA_CONF_SetWin(gfpgaConfDev.win);
    //FPGA_CONF_SHOW_INFO(&gfpgaConfDev.showInfo);
    FPGA_CONF_DisConf(&gfpgaConfDev.display);
    FPGA_CONF_PrintEth();
    //FPGA_CONF_SaveConfToFile();
    WV_CMD_Register("set", "fpga", "pca9555 set reg", FPGA_CONF_SetCmd);
    WV_CMD_Register("get", "fpga", "pca9555 get reg", FPGA_CONF_GetCmd);
}

/*****************************************************
* WV_S32 FPGA_CONF_Init(FPGA_CONF_DEV *pConf);
*初始化FPGA设置
*****************************************************/
void FPGA_CONF_DeInit()
{

}
#endif

