#include "fpga_conf.h"
#include "fpga_common.h"
#include "his_spi.h"
#include "sys_env.h"
#include "sys_ip.h"
#include "fpga_conf_json.h"
#include "fpga_sdp.h"
#define FPGA_CONF_FILE_PATH_D "./env/fpga.cfg"

#define FPGA_DEBUG
FPGA_CONF_DEV *gpFpgaConfDev;

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
*设置视频输出g更新（包括窗口坐标更新生效）
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
                    printf("video ip 匹配到合适的值src[%d][%d]\n",pWin[i].channel,j);
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
#if 1
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
        printf("reg[%X]=%X\n",regAddr + 0x1f,srcAddrIpv4OrIpv6);
        
        for (j = 0; j < 4; j++)
        { 
            if(srcAddr[i][j].ipv4OrIpv6 == 0){ //ipv4
                memset(ip,0,sizeof(ip));
                FPGA_CONF_getIpInt(srcAddr[i][j].ipv6,ip);
                printf("ip[%d][%d]=[%d.%d.%d.%d]\n",i,j,ip[0],ip[1],ip[2],ip[3]);
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
#endif
    return WV_SOK;
}

/*******************************************************************
WV_S32 FPGA_CONF_SetWin(FPGA_CONF_WIN_T *pWin);
fpga初始化
*******************************************************************/
WV_S32 FPGA_CONF_SetWin(FPGA_CONF_WIN_T *pWin)
{
    { 

        FPGA_CONF_SetWinIpAndSdp(pWin);
        WV_S32 i, ret = 0;
        WV_U16 baseAddr = 0x0500;
        WV_U16 regAddr,winEna=0;
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
        FPGA_CONF_DisChangeEna(winEna);
        FPGA_printf("set windows ok \n");
    }
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
    }

    return ret;

}
/*****************************************************
* WV_S32 FPGA_CONF_GetVersion(FPGA_VER *pVer);
*查询fpga版本信息
*****************************************************/
WV_S32 FPGA_CONF_GetVersion(FPGA_VER *pVer)
{

    WV_U16 temp;
    HIS_SPI_FpgaRd(0, &pVer->version);
    HIS_SPI_FpgaRd(0x1, &pVer->type);
    HIS_SPI_FpgaRd(0x2, &pVer->year);

    HIS_SPI_FpgaRd(0x3, &temp);

    pVer->month = (temp & 0xf00) >> 8;
    pVer->day = temp & 0xff >> 8;

    FPGA_printf("ver=0x%04x,type=0x%04x,date:%d-%d-%d\r\n", pVer->version, pVer->type, pVer->year, pVer->month, pVer->day);



    return WV_SOK;
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
        prfBuff += sprintf(prfBuff, "set fpga <cmd>;//cmd like: win/eth/dis\r\n");
        return 0;
    }
    if (strcmp(argv[0], "win") == 0 )
    {
        if(argc < 2 ){
            //prfBuff += sprintf(prfBuff, "set fpga win def //(set win default)\r\n");
            prfBuff += sprintf(prfBuff, "set fpga win <id> <x> <y> <w> <h>\r\n");

        }        
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
        WV_U16 baseAddr = 0x0500;
        WV_U16 regAddr=0;

        regAddr = ((baseAddr >> 4) + id) << 4;
        ret += HIS_SPI_FpgaWd(regAddr, x);
        ret += HIS_SPI_FpgaWd(regAddr + 1, y);
        ret += HIS_SPI_FpgaWd(regAddr + 2, w);
        ret += HIS_SPI_FpgaWd(regAddr + 3, h);    

        WV_U16 winEna;
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
    gpFpgaConfDev = (FPGA_CONF_DEV *)malloc(sizeof(FPGA_CONF_DEV));

    FPGA_VER ver;
    HIS_SPI_FpgaWd(0x600,0x0);
    FPGA_CONF_GetVersion(&ver);
    //test set eth
    int i;
    FPGA_CONF_ETH_T eth[FPGA_CONF_ETHNUM_D];
    memset(eth,0,sizeof(FPGA_CONF_ETH_T)*4);
    if(FPGA_CONF_EthAnalysisJson("./eth.json",eth) == 0)
    {
        for(i=0;i<FPGA_CONF_ETHNUM_D;i++){
            FPGA_CONF_SetETH(eth,i);
        }
    }
    //test win.json
    FPGA_CONF_WIN_T win[16];
    memset(win,0,sizeof(FPGA_CONF_WIN_T)*16);

    if(FPGA_CONF_WinAnalysisJson("./win.json",win) == 0)
    {
        FPGA_CONF_SetWin(win);
    }

    //set dis

    WV_CMD_Register("set", "fpga", "pca9555 set reg", FPGA_CONF_SetCmd);
    //WV_CMD_Register("get", "fpga", "pca9555 get reg", FPGA_CONF_GetCmd);

    return ;
}
void FPGA_CONF_DeInit()
{

}
#if 0
// 窗口信息
FPGA_CONF_DEV gfpgaConfDev;

/*******************************************************************
WV_S32 FPGA_CONF_getIpInt(WV_S8 *pName,WV_U8* pIp);
*******************************************************************/
WV_S32 FPGA_CONF_getIpInt(WV_S8 *pSrc,WV_U8* pIp)
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
* WV_S32 FPGA_CONF_GetVersion(FPGA_VER *pVer);
*查询fpga版本信息
*****************************************************/
WV_S32 FPGA_CONF_GetVersion(FPGA_VER *pVer)
{

    WV_U16 temp;
    HIS_SPI_FpgaRd(0, &pVer->version);
    HIS_SPI_FpgaRd(0x1, &pVer->type);
    HIS_SPI_FpgaRd(0x2, &pVer->year);

    HIS_SPI_FpgaRd(0x3, &temp);

    pVer->month = (temp & 0xf00) >> 8;
    pVer->day = temp & 0xff >> 8;

    FPGA_printf("ver=0x%04x,type=0x%04x,date:%d-%d-%d\r\n", pVer->version, pVer->type, pVer->year, pVer->month, pVer->day);
    return WV_SOK;
}

/*****************************************************
* WV_S32 FPGA_CONF_SetConfDefault();
*fpga 恢复默认设置
*****************************************************/
WV_S32 FPGA_CONF_SetConfDefault()
{
    WV_S32 i, j;
    for (i = 0; i < FPGA_CONF_WINNUM_D; i++)
    { 

        //设置显示11路ETH输入，4路SDI输入，1路HDMI输入
        if (i < 11)
        {
            gfpgaConfDev.win[i].audio_ip = i % 3;
            gfpgaConfDev.win[i].video_ip = i % 3;
            gfpgaConfDev.win[i].video_type = FPGA_CONF_WIN_VIDEO_ETH;
            gfpgaConfDev.win[i].sel_in = i / 3; //eth0～eth3分别三路输出,eth4两路输出
        }
        else if (i >= 11 && i < 15)
        {
            gfpgaConfDev.win[i].video_type = FPGA_CONF_WIN_VIDEO_SDI;
            gfpgaConfDev.win[i].sel_in = (i + 1) % 4; //分别选择sdi的0 ,1 ,2 ,3
        }
        else
        {
            gfpgaConfDev.win[i].video_type = FPGA_CONF_WIN_VIDEO_HDMI;
            gfpgaConfDev.win[i].sel_in = 0;
        }

        gfpgaConfDev.win[i].x = (i % 4) * 960;
        gfpgaConfDev.win[i].y = (i / 4) * 540;
        gfpgaConfDev.win[i].w = 960;
        gfpgaConfDev.win[i].h = 540;
    }    
    for (i = 0; i < FPGA_CONF_ETHNUM_D; i++)
    {
        //80-9f-fb-88-88-00;
        //255.255.0.0
        //192.168.1.1
        //配置本地四路网卡ip
        gfpgaConfDev.eth[i].localAddr.ipv6[3] = 192;
        gfpgaConfDev.eth[i].localAddr.ipv6[2] = 168;
        gfpgaConfDev.eth[i].localAddr.ipv6[1] = 1;
        gfpgaConfDev.eth[i].localAddr.ipv6[0] = 10 + i; //10/11/12/13

        gfpgaConfDev.eth[i].localAddr.mac[5] = 0x80;
        gfpgaConfDev.eth[i].localAddr.mac[4] = 0x9f;
        gfpgaConfDev.eth[i].localAddr.mac[3] = 0xfb;
        gfpgaConfDev.eth[i].localAddr.mac[2] = 0x88;
        gfpgaConfDev.eth[i].localAddr.mac[1] = 0x88;
        gfpgaConfDev.eth[i].localAddr.mac[0] = 0x00 + i;

        gfpgaConfDev.eth[i].localAddr.subnet_mask[3] = 255;
        gfpgaConfDev.eth[i].localAddr.subnet_mask[2] = 255;
        gfpgaConfDev.eth[i].localAddr.subnet_mask[1] = 0;
        gfpgaConfDev.eth[i].localAddr.subnet_mask[0] = 0;

        gfpgaConfDev.eth[i].localAddr.getway[3] = 192;
        gfpgaConfDev.eth[i].localAddr.getway[3] = 168;
        gfpgaConfDev.eth[i].localAddr.getway[3] = 1;
        gfpgaConfDev.eth[i].localAddr.getway[3] = 1;
        //////////////////////////////////////
        //配置数据源过滤选择IP
        for (j = 0; j < 4; j++)
        {
            gfpgaConfDev.eth[i].srcAddr[j].ipv6[3] = 244;
            gfpgaConfDev.eth[i].srcAddr[j].ipv6[2] = 0;
            gfpgaConfDev.eth[i].srcAddr[j].ipv6[1] = 0;
            gfpgaConfDev.eth[i].srcAddr[j].ipv6[0] = 1 + j;
            gfpgaConfDev.eth[i].srcAddr[j].port = 9875 + j;
        }
        ///////////////////////////////////////
        //配置发送目标地址
        gfpgaConfDev.eth[i].desAddr.ipv6[3] = 192;
        gfpgaConfDev.eth[i].desAddr.ipv6[2] = 168;
        gfpgaConfDev.eth[i].desAddr.ipv6[1] = 1;
        gfpgaConfDev.eth[i].desAddr.ipv6[0] = 10 + i;
        gfpgaConfDev.eth[i].desAddr.desPort = 9875 + i;
        gfpgaConfDev.eth[i].desAddr.srcPort = 8875 + i;
        gfpgaConfDev.eth[i].desAddr.txCtrl = 0;
        gfpgaConfDev.eth[i].desAddr.txSel = 0;
    }
        gfpgaConfDev.display.videoCtrl |= 0x1400;
        gfpgaConfDev.display.valid = 0xffff;
        gfpgaConfDev.display.coloKey_R = 0;
        gfpgaConfDev.display.coloKey_G = 0;
        gfpgaConfDev.display.coloKey_B = 0;
        gfpgaConfDev.display.alpha = 102;
        
        //FPGA_CONF_PrintWin();

    for(i=0;i<FPGA_CONF_ETHNUM_D;i++){
        gfpgaConfDev.eth[i].localAddr.ipv6[3] = 192;
        gfpgaConfDev.eth[i].localAddr.ipv6[2] = 168;
        gfpgaConfDev.eth[i].localAddr.ipv6[1] = 2;
        gfpgaConfDev.eth[i].localAddr.ipv6[0] = 44;

        gfpgaConfDev.eth[i].localAddr.mac[5] = 0x90;
        gfpgaConfDev.eth[i].localAddr.mac[4] = 0xe2;
        gfpgaConfDev.eth[i].localAddr.mac[3] = 0xba;
        gfpgaConfDev.eth[i].localAddr.mac[2] = 0x89;
        gfpgaConfDev.eth[i].localAddr.mac[1] = 0x20;
        gfpgaConfDev.eth[i].localAddr.mac[0] = 0x70;

        gfpgaConfDev.eth[i].srcAddr[0].ipv6[3] = 192;
        gfpgaConfDev.eth[i].srcAddr[0].ipv6[2] = 168;
        gfpgaConfDev.eth[i].srcAddr[0].ipv6[1] = 2;
        gfpgaConfDev.eth[i].srcAddr[0].ipv6[0] = 22;
        gfpgaConfDev.eth[i].srcAddr[0].port = 5200;
    
    }

    return WV_SOK;
        
}

/*****************************************************
* WV_S32 FPGA_CONF_ReadConfFromFile();
*从配置文件读取fpga配置信息，如果没有配置文件，则使用默认配置
*****************************************************/
WV_S32 FPGA_CONF_ReadConfFromFile()
{
    FILE *fp;
    fp = fopen(FPGA_CONF_FILE_PATH_D, "rb");
    if (fp == NULL)
    {
        FPGA_printf("fpga config file not exit!set conf default\n");
        FPGA_CONF_SetConfDefault();
        return WV_EFAIL;
    }
    else
    {

        if (fread(&gfpgaConfDev, 1, sizeof(FPGA_CONF_DEV), fp) != sizeof(FPGA_CONF_DEV))
        {
            FPGA_printf("read fpga config file err!set conf default\n");
            FPGA_CONF_SetConfDefault();
        }
        fclose(fp);
        return WV_EFAIL;
    }
    printf("read conf from file\n");WV_U16 baseAddr = 0x0500;
    WV_U16 regAddr, avSel = 0;
    //config windows location and video info
    for (i = 0; i < FPGA_CONF_WINNUM_D; i++)
    {
        regAddr = ((baseAddr >> 4) + i) << 4;
        ret += HIS_SPI_FpgaWd(regAddr, pWin[i].x);
        ret += HIS_SPI_FpgaWd(regAddr + 1, pWin[i].y);
        ret += HIS_SPI_FpgaWd(regAddr + 2, pWin[i].w);
        ret += HIS_SPI_FpgaWd(regAddr + 3, pWin[i].h);

        avSel |= (pWin[i].video_type & 0x3) << 10;
        avSel |= (pWin[i].video_ip & 0x3) << 6;
        avSel |= (pWin[i].audio_ip & 0x3) << 2;
        avSel |= pWin[i].sel_in & 0x3;
        ret += HIS_SPI_FpgaWd(regAddr + 4, avSel);
    }
    return WV_SOK;
}


/*****************************************************
* WV_S32 FPGA_CONF_SaveConfToFile();
*保存到配置文件
*****************************************************/
WV_S32 FPGA_CONF_SaveConfToFile()
{
    FILE *fp;
    fp = fopen(FPGA_CONF_FILE_PATH_D, "wb+");
    if (fp == NULL)
    {
        FPGA_printf("fpga config file save err \n");
        return WV_EFAIL;
    }
    fwrite(&gfpgaConfDev, 1, sizeof(FPGA_CONF_DEV), fp);

    fclose(fp);
    printf("save conf to file\n");
    return WV_SOK;
}
/*****************************************************
* void FPGA_CONF_GetCurrentConf(FPGA_CONF_DEV *pConf);
*获取当前配置
*****************************************************/
void FPGA_CONF_GetCurrentConf(FPGA_CONF_DEV *pConf)
{
    memcpy((void *)pConf, (const void *)&gfpgaConfDev, sizeof(FPGA_CONF_DEV));
}

/*****************************************************
* WV_S32 FPGA_CONF_PrintWin();
*打印窗口位置信息
*****************************************************/
WV_S32 FPGA_CONF_PrintWin()
{
    printf("\n");
    WV_S32 i=0;
  /*
    for(i=0;i<FPGA_CONF_WINNUM_D;i++){
        //FPGA_printf("win[%d]videoType=%d,videoSel=%d,videoIP=%d,audioIP=%d\n",i,gfpgaConfDev.win[i].video_type,gfpgaConfDev.win[i].sel_in,gfpgaConfDev.win[i].video_ip,gfpgaConfDev.win[i].audio_ip);
        FPGA_printf("win[%d]X=%d  ",i,gfpgaConfDev.win[i].x);
        FPGA_printf("Y=%d  ",gfpgaConfDev.win[i].y);
        FPGA_printf("W=%d  ",gfpgaConfDev.win[i].w);
        FPGA_printf("H=%d\n",gfpgaConfDev.win[i].h);
    }*/
    //
    WV_S32  ret = 0;
    WV_U16 baseAddr = /*****************************************************
WV_S32 FPGA_CONF_DisChangeEna();
*设置视频输出g更新（包括窗口坐标更新生效）
*****************************************************/
WV_S32 FPGA_CONF_DisChangeEna()
{
    gfpgaConfDev.display.videoCtrl |= 0x1000;
    HIS_SPI_FpgaWd(0x600,gfpgaConfDev.display.videoCtrl);
    return WV_SOK;
}
    WV_U16 regAddr, da/*****************************************************
WV_S32 FPGA_CONF_DisChangeEna();
*设置视频输出g更新（包括窗口坐标更新生效）
*****************************************************/
WV_S32 FPGA_CONF_DisChangeEna()
{
    gfpgaConfDev.display.videoCtrl |= 0x1000;
    HIS_SPI_FpgaWd(0x600,gfpgaConfDev.display.videoCtrl);
    return WV_SOK;
}
    FPGA_CONF_WIN win;/*****************************************************
WV_S32 FPGA_CONF_DisChangeEna();
*设置视频输出g更新（包括窗口坐标更新生效）
*****************************************************/
WV_S32 FPGA_CONF_DisChangeEna()
{
    gfpgaConfDev.display.videoCtrl |= 0x1000;
    HIS_SPI_FpgaWd(0x600,gfpgaConfDev.display.videoCtrl);
    return WV_SOK;
}

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
        
        FPGA_printf("win[%d]X=%d  ",i,win.x);
        FPGA_printf("Y=%d  ",win.y);videoCtrl
        FPGA_printf("W=%d  ",win.w);
        FPGA_printf("H=%d\n",win.h);
    }
    HIS_SPI_FpgaRd(0x0604,&data);
    FPGA_printf("windows enable 0x0604= 0x%04x\n",data);

    return WV_SOK;    
}
/*****************************************************
* WV_S32 FPGA_CONF_PrintEth();
*打印窗口信息
*****************************************************/
WV_S32 FPGA_CONF_PrintEth()
{
    WV_S32 i,j;
    for(i=0;i<FPGA_CONF_ETHNUM_D;i++){
        //gfpgaConfDev.eth[i].localAddr
        FPGA_printf("\n-------------------------------------------------\n");
        FPGA_printf("ETH[%d] localAddr[ip:%d.%d.%d.%d netmask:%d.%d.%d.%d getway:%d.%d.%d.%d  mac:%02x:%02x:%02x:%02x:%02x:%02x]\n",\
            i,\
            gfpgaConfDev.eth[i].localAddr.ipv6[3],\
            gfpgaConfDev.eth[i].localAddr.ipv6[2],\
            gfpgaConfDev.eth[i].localAddr.ipv6[1],\
            gfpgaConfDev.eth[i].localAddr.ipv6[0],\
            gfpgaConfDev.eth[i].localAddr.subnet_mask[3],\
            gfpgaConfDev.eth[i].localAddr.subnet_mask[2],\
            gfpgaConfDev.eth[i].localAddr.subnet_mask[1],\videoCtrl
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

        FPGA_printf("ETH[%d]TX,sdi[%d],srcPort[%d],desIP[%d.%d.%d.%d],desPort[%d],desMac[%02x:%02x:%02x:%02x:%02x:%02x]videoPt[%d],audioPt[%d]\n",\
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
            FPGA_printf("ETH[%d]inputSrc[%d] :[ip:%d.%d.%d.%d  port=%d \n",\
            i,j,\
            gfpgaConfDev.eth[i].srcAddr[j].ipv6[3],\
            gfpgaConfDev.eth[i].srcAddr[j].ipv6[2],\
            gfpgaConfDev.eth[i].srcAddr[j].ipv6[1],\
            gfpgaConfDev.eth[i].srcAddr[j].ipv6[0],\
            gfpgaConfDev.eth[i].srcAddr[j].port);
        }
        
    }

    return WV_SOK;
}
/*****************************************************
*WV_S32 FPGA_CONF_SetWin(FPGA_CONF_WIN *pWin);
*设置视频开窗信息
*****************************************************/
WV_S32 FPGA_CONF_SetWin(FPGA_CONF_WIN *pWin)
{
    WV_S32 i, ret = 0;
    WV_U16 baseAddr = 0x0500;
    WV_U16 regAddr, avSel = 0;
    //config windows location and video info
    for (i = 0; i < FPGA_CONF_WINNUM_D; i++)
    {
        regAddr = ((baseAddr >> 4) + i) << 4;
        ret += HIS_SPI_FpgaWd(regAddr, pWin[i].x);
        ret += HIS_SPI_FpgaWd(regAddr + 1, pWin[i].y);baseAddr = ((baseAddr >> 8) + i) << 8;
        regAddr = baseAddr;
        //mac
        ret += HIS_SPI_FpgaWd(regAddr + 2, (pNet[i].localAddr.mac[1] << 8) | pNet[i].localAddr.mac[0]);
        ret += HIS_SPI_FpgaWd(regAddr + 3, (pNet[i].localAddr.mac[3] << 8) | pNet[i].localAddr.mac[2]);
        ret += HIS_SPI_FpgaWd(regAddr + 4, (pNet[i].localAddr.mac[5] << 8) | pNet[i].localAddr.mac[4]);
        //ipv6
        ret += HIS_SPI_FpgaWd(regAddr + 5, (pNet[i].localAddr.ipv6[1] << 8) | pNet[i].localAddr.ipv6[0]);
        ret += HIS_SPI_FpgaWd(regAddr + 6, (pNet[i].localAddr.ipv6[3] << 8) | pNet[i].localAddr.ipv6[2]);
        ret += HIS_SPI_FpgaWd(regAddr + 7, (pNet[i].localAddr.ipv6[5] << 8) | pNet[i].localAddr.ipv6[3]);
        ret += HIS_SPI_FpgaWd(regAddr + 8, (pNet[i].localAddr.ipv6[7] << 8) | pNet[i].localAddr.ipv6[6]);
        ret += HIS_SPI_FpgaWd(regAddr + 9, (pNet[i].localAddr.ipv6[9] << 8) | pNet[i].localAddr.ipv6[8]);
        ret += HIS_SPI_FpgaWd(regAddr + 0xa, (pNet[i].localAddr.ipv6[11] << 8) | pNet[i].localAddr.ipv6[10]);
        ret += HIS_SPI_FpgaWd(regAddr + 0xb, (pNet[i].localAddr.ipv6[13] << 8) | pNet[i].localAddr.ipv6[12]);
        ret += HIS_SPI_FpgaWd(regAddr + 0xc, (pNet[i].localAddr.ipv6[15] << 8) | pNet[i].localAddr.ipv6[14]);
        //subnet mask
        ret += HIS_SPI_FpgaWd(regAddr + 0xd, (pNet[i].localAddr.subnet_mask[1] << 8) | pNet[i].localAddr.subnet_mask[0]);
        ret += HIS_SPI_FpgaWd(regAddr + 0xe, (pNet[i].localAddr.subnet_mask[3] << 8) | pNet[i].localAddr.subnet_mask[2]);
        //gateway
        ret += HIS_SPI_FpgaWd(regAddr + 0xf, (pNet[i].localAddr.getway[1] << 8) | pNet[i].localAddr.getway[0]);
        ret += HIS_SPI_FpgaWd(regAddr + 0x10, (pNet[i].localAddr.getway[3] << 8) | pNet[i].localAddr.getway[2]);
        ret += HIS_SPI_FpgaWd(regAddr + 2, pWin[i].w);
        ret += HIS_SPI_FpgaWd(regAddr + 3, pWin[i].h);

        avSel |= (pWin[i].video_type & 0x3) << 10;
        avSel |= (pWin[i].video_ip & 0x3) << 6;
        avSel |= (pWin[i].audio_ip & 0x3) << 2;
        avSel |= pWin[i].sel_in & 0x3;
        ret += HIS_SPI_FpgaWd(regAddr + 4, avSel);
    }
    if (ret != 0)
    {
        FPGA_printf("set win location err \n");
        return WV_EFAIL;
    }
    FPGA_printf("set windows ok \n");
    FPGA_CONF_PrintWin();
    FPGA_CONF_SaveConfToFile();
    return WV_SOK;
}

/*****************************************************
WV_S32 FPGA_CONF_DisChangeEna();
*设置视频输出g更新（包括窗口坐标更新生效）
*****************************************************/
WV_S32 FPGA_CONF_DisChangeEna()
{
    gfpgaConfDev.display.videoCtrl |= 0x1000;
    HIS_SPI_FpgaWd(0x600,gfpgaConfDev.display.videoCtrl);
    return WV_SOK;
}
/*****************************************************
*WV_S32 FPGA_CONF_DisConf(FPGA_CONF_DIS *pDis);
*设置视频输出
*****************************************************/
WV_S32 FPGA_CONF_DisConf(FPGA_CONF_DIS *pDis)
{
    WV_S32 ret = 0;
    //enable display
    WV_U16 baseAddr = 0x0600;
    WV_U16 regAddr = baseAddr;

    ret += HIS_SPI_FpgaWd(regAddr + 1, pDis->coloKey_G);
    ret += HIS_SPI_FpgaWd(regAddr + 2, pDis->coloKey_G);
    ret += HIS_SPI_FpgaWd(regAddr + 3, pDis->coloKey_R);
    ret += HIS_SPI_FpgaWd(regAddr + 4, pDis->valid);
    ret += HIS_SPI_FpgaWd(regAddr + 5, pDis->alpha);
    ret += HIS_SPI_FpgaWd(regAddr, pDis->videoCtrl);
    if (ret != 0)
    {
        FPGA_printf("set display err \n");
        return WV_EFAIL;
    }
    FPGA_printf("set display ok");
    return WV_SOK;
}
/*****************************************************
*WV_S32 FPGA_CONF_NetConf(FPGA_CONF_DIS *pDis);
*设置网络
*****************************************************/
WV_S32 FPGA_CONF_NetConf(FPGA_CONF_ETH *pNet)
{
    WV_S32 i, j, ret = 0;
    WV_U16 baseAddr = 0x0100;
    WV_U16 regAddr = baseAddr;
    for (i = 0; i < FPGA_CONF_ETHNUM_D; i++)
    {
        //printf("\n eth conf-------------------------------------------------------------------------------\n");
        baseAddr = ((baseAddr >> 8) + i) << 8;
        regAddr = baseAddr;
        //mac
        ret += HIS_SPI_FpgaWd(regAddr + 2, (pNet[i].localAddr.mac[1] << 8) | pNet[i].localAddr.mac[0]);
        ret += HIS_SPI_FpgaWd(regAddr + 3, (pNet[i].localAddr.mac[3] << 8) | pNet[i].localAddr.mac[2]);
        ret += HIS_SPI_FpgaWd(regAddr + 4, (pNet[i].localAddr.mac[5] << 8) | pNet[i].localAddr.mac[4]);
        //ipv6
        ret += HIS_SPI_FpgaWd(regAddr + 5, (pNet[i].localAddr.ipv6[1] << 8) | pNet[i].localAddr.ipv6[0]);
        ret += HIS_SPI_FpgaWd(regAddr + 6, (pNet[i].localAddr.ipv6[3] << 8) | pNet[i].localAddr.ipv6[2]);
        ret += HIS_SPI_FpgaWd(regAddr + 7, (pNet[i].localAddr.ipv6[5] << 8) | pNet[i].localAddr.ipv6[3]);
        ret += HIS_SPI_FpgaWd(regAddr + 8, (pNet[i].localAddr.ipv6[7] << 8) | pNet[i].localAddr.ipv6[6]);
        ret += HIS_SPI_FpgaWd(regAddr + 9, (pNet[i].localAddr.ipv6[9] << 8) | pNet[i].localAddr.ipv6[8]);
        ret += HIS_SPI_FpgaWd(regAddr + 0xa, (pNet[i].localAddr.ipv6[11] << 8) | pNet[i].localAddr.ipv6[10]);
        ret += HIS_SPI_FpgaWd(regAddr + 0xb, (pNet[i].localAddr.ipv6[13] << 8) | pNet[i].localAddr.ipv6[12]);
        ret += HIS_SPI_FpgaWd(regAddr + 0xc, (pNet[i].localAddr.ipv6[15] << 8) | pNet[i].localAddr.ipv6[14]);
        //subnet mask
        ret += HIS_SPI_FpgaWd(regAddr + 0xd, (pNet[i].localAddr.subnet_mask[1] << 8) | pNet[i].localAddr.subnet_mask[0]);
        ret += HIS_SPI_FpgaWd(regAddr + 0xe, (pNet[i].localAddr.subnet_mask[3] << 8) | pNet[i].localAddr.subnet_mask[2]);
        //gateway
        ret += HIS_SPI_FpgaWd(regAddr + 0xf, (pNet[i].localAddr.getway[1] << 8) | pNet[i].localAddr.getway[0]);
        ret += HIS_SPI_FpgaWd(regAddr + 0x10, (pNet[i].localAddr.getway[3] << 8) | pNet[i].localAddr.getway[2]);
        ////////////////////////////////////////////////////////////////////////////////////
        //源数据过滤ip地址设置
        ret += HIS_SPI_FpgaWd(regAddr + 0x1f, pNet[i].ipv4Oripv6);
        for (j = 0; j < 4; j++)
        {

            ret += HIS_SPI_FpgaWd(regAddr + 0x20 + j * 9, (pNet[i].srcAddr[j].ipv6[1] << 8) | pNet[i].srcAddr[j].ipv6[0]);
            ret += HIS_SPI_FpgaWd(regAddr + 0x21 + j * 9, (pNet[i].srcAddr[j].ipv6[3] << 8) | pNet[i].srcAddr[j].ipv6[2]);
            ret += HIS_SPI_FpgaWd(regAddr + 0x22 + j * 9, (pNet[i].srcAddr[j].ipv6[5] << 8) | pNet[i].srcAddr[j].ipv6[3]);
            ret += HIS_SPI_FpgaWd(regAddr + 0x23 + j * 9, (pNet[i].srcAddr[j].ipv6[7] << 8) | pNet[i].srcAddr[j].ipv6[6]);
            ret += HIS_SPI_FpgaWd(regAddr + 0x24 + j * 9, (pNet[i].srcAddr[j].ipv6[9] << 8) | pNet[i].srcAddr[j].ipv6[8]);
            ret += HIS_SPI_FpgaWd(regAddr + 0x25 + j * 9, (pNet[i].srcAddr[j].ipv6[11] << 8) | pNet[i].srcAddr[j].ipv6[10]);
            ret += HIS_SPI_FpgaWd(regAddr + 0x26 + j * 9, (pNet[i].srcAddr[j].ipv6[13] << 8) | pNet[i].srcAddr[j].ipv6[12]);
            ret += HIS_SPI_FpgaWd(regAddr + 0x27 + j * 9, (pNet[i].srcAddr[j].ipv6[15] << 8) | pNet[i].srcAddr[j].ipv6[14]);
            ret += HIS_SPI_FpgaWd(regAddr + 0x28 + j * 9, pNet[i].srcAddr[j].port);
        }

        //ETH发送设置
        ret += HIS_SPI_FpgaWd(regAddr + 0x5a, pNet[i].desAddr.txCtrl);
        ret += HIS_SPI_FpgaWd(regAddr + 0x5b, (pNet[i].desAddr.mac[1] << 8) | pNet[i].desAddr.mac[0]);
        ret += HIS_SPI_FpgaWd(regAddr + 0x5c, (pNet[i].desAddr.mac[3] << 8) | pNet[i].desAddr.mac[2]);
        ret += HIS_SPI_FpgaWd(regAddr + 0x5d, (pNet[i].desAddr.mac[5] << 8) | pNet[i].desAddr.mac[4]);
        ret += HIS_SPI_FpgaWd(regAddr + 0x5e, (pNet[i].desAddr.ipv6[1] << 8) | pNet[i].desAddr.ipv6[0]);
        ret += HIS_SPI_FpgaWd(regAddr + 0x5f, (pNet[i].desAddr.ipv6[3] << 8) | pNet[i].desAddr.ipv6[2]);

        ret += HIS_SPI_FpgaWd(regAddr + 0x60, pNet[i].desAddr.desPort);
        ret += HIS_SPI_FpgaWd(regAddr + 0x61, pNet[i].desAddr.srcPort);
        ret += HIS_SPI_FpgaWd(regAddr + 0x62, pNet[i].desAddr.txSel);

    }


    return ret;
}

/*****************************************************
* void FPGA_CONF_SetAllConf(FPGA_CONF_DEV *pConf);
*设置所有配置
*****************************************************/
/*
void FPGA_CONF_SetAllConf(FPGA_CONF_DEV *pConf)
{
    //保存到配置文件
    memcpy((void *)&gfpgaConfDev, (const void *)&pConf, sizeof(FPGA_CONF_DEV));
    FPGA_CONF_SaveConfToFile();
    //配饰fpga
}*/
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

