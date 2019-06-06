#include "fpga_igmp.h"
#include "fpga_common.h"
#include "wv_thr.h"
#include "his_spi.h"
#include "fpga_conf.h"
#include "sys_ip.h"
#pragma pack(push)
#pragma pack(1)

#define _D_ETH_NUM_ (4)
#define _D_SRC_NUM_ (4)

typedef struct _S_FPGA_IGMP_DATA
{
    WV_U8 desMac[6]; //固定 01 00 5E 00 00 16
    WV_U8 srcMac[6];
    WV_U16 type; //type=0x0800(转换大段模式) 0x0008
    //ip首部
    WV_U8 ipVerion; //0x4
    //WV_U8  ipHeaderLength:4;   //0x6
    WV_U8 ipTOSType;         //0x00
    WV_U16 ipDataLen;        //长度  有源0x002C(转换网络字节序) 即0x2C00//无源 0x2800
    WV_U16 ipId;             //标识  0xb5af(转换网络字节序) 即0xafb5
    WV_U16 ipSign;           //标志 +片位移  0x0
    WV_U8 ipTTL;             //生存时间   0x1
    WV_U8 ipProtocal;        //协议类型   0x2
    WV_U16 ipHeaderChecksum; //首部校验和  计算
    WV_U8 ipSrc[4];          //源ip 本网卡ip地址
    WV_U8 ipDes[4];          //目标ip 固定224.0.0.22
    WV_U8 options[4];        //选项  0x94 0x04 0x00 0x00（计算长度使用，再checksum 内）

    //IGMP报文
    WV_U8 igmpType;               //igmp类型 0x22 bit[0-3]:ver, bit[4-7]:type
    WV_U8 igmpReserved0;          //保留字段 默认为 0x0
    WV_U16 igmpHeaderChecksum;    //igmp校验和
    WV_U16 igmpReserved1;         //保留字段 默认为 0x0
    WV_U16 igmpNumOfGroupRecords; //组记录，本程序固定为 0x0001(转换网络字节序)0x0100;
    WV_U8 igmpRecordType;         //0x03 加入有源的为0x03，加入无源的为0x04，退出组播为0x03
    WV_U8 igmpAuxDataLen;         //0x00
    WV_U16 igmpNumOfSource;       //有源： 始终只有1个(转换网络字节序) 0x0100; 无源：0x0000; 退出设置0x0000
    WV_U8 igmpMulTicastAddr[4];   //需要加入的组播地址
    WV_U8 igmpSourceAddr[4];      //源地址
    WV_U8 igmpAdditionalData[2];  //附加数据  0x0(不再长度内，不在校验内)
} _S_FPGA_IGMP_DATA;

typedef struct FPGA_IGMP_DEV
{
    WV_THR_HNDL_T thrHndl;
    WV_U32 open;
    WV_U32 close;
    WV_U32 secondOfIgmpSend;                              //组播报文加入每隔多长时间执行一次，用户可以设置，总时间 单位秒
    WV_U32 times;                                         //组播报文发送当恰能等待时间 单位秒
    WV_S32 autoSendIgmpEna;                               //自动发送组播使能，默认为字段发送 1
    WV_S32 ena[_D_ETH_NUM_][_D_SRC_NUM_];                 //组播是否使能
    
    _S_FPGA_IGMP_DATA igmpData[_D_ETH_NUM_][_D_SRC_NUM_]; //4个网卡，每个网卡4个报文

} FPGA_IGMP_DEV;

#pragma pack(pop)
static FPGA_IGMP_DEV gFpgaIgmpDev;

static pthread_mutex_t _g_igmp_mutex;

/**************************************************************
 * void FPGA_IGMP_Lock()
 * ***********************************************************/
void FPGA_IGMP_Lock()
{
    pthread_mutex_lock(&_g_igmp_mutex);
}
/**************************************************************
 * void FPGA_IGMP_UnLock()
 * ***********************************************************/
void FPGA_IGMP_UnLock()
{
    pthread_mutex_unlock(&_g_igmp_mutex);
}
/****************************************************************
 * void fpga_igmp_checkSum(WV_U8 *buf,WV_S32 len,WV_U16 *checkSum)
 * 功能：获取校验和
 * 参数说明：buf：需要校验的数据
 *         len：需要校验的数据长度
 *         checkSum：校验和（网络字节序）
 * *************************************************************/
static void fpga_igmp_checkSum(WV_U8 *buf, WV_S32 len, WV_U16 *checkSum)
{
    //求ipHeaderChecksum，ip首部校验和
    WV_U16 ipData;
    WV_U32 u32ipSum = 0;
    WV_U16 u16ipSum;
    WV_S32 i;

    for (i = 0; i < len / 2; i++)
    {
        ipData = buf[2 * i] << 8 | buf[2 * i + 1];
        u32ipSum += ipData;
    }
    u16ipSum = (u32ipSum >> 16) + (u32ipSum & 0xffff);
    u16ipSum = ~u16ipSum;
    *checkSum = u16ipSum >> 8 | u16ipSum << 8;
}
/************************************************************************
 * WV_S32 fpga_igmp_join(WV_S32 ethID,WV_U8 multicastAddr[],WV_U8 sourceIp[])
 * 加入组播 
 * *********************************************************************/
static WV_S32 fpga_igmp_join(WV_S32 ethID, WV_S32 srcID, WV_U8 multicastAddr[], WV_U8 sourceIp[])
{
    if (ethID >= 4)
    {
        FPGA_printf("igmp join set ethID err!! ethID=%d \n", ethID);
    }

    WV_S32 igmpDataLen = 0;
    WV_S32 ipDataLen = 24;
    WV_U8 srcMac[6] = {0};
    WV_U8 srcIp[4] = {0};

    WV_S32 i, emptyNum = srcID;
    FPGA_CONF_GetEthInt(srcIp, srcMac, ethID);
   
    //生成组播报文
    //80:9f:fb:88:88:01
    //本地网卡mac地址
    for (i = 0; i < 6; i++)
    {
        gFpgaIgmpDev.igmpData[ethID][emptyNum].srcMac[i] = srcMac[i];
    }
    //本地ip地址
    for (i = 0; i < 4; i++)
    {
        gFpgaIgmpDev.igmpData[ethID][emptyNum].ipSrc[i] = srcIp[i];
    }
    //设置加入组播的地址
    gFpgaIgmpDev.igmpData[ethID][emptyNum].igmpMulTicastAddr[0] = multicastAddr[0];
    gFpgaIgmpDev.igmpData[ethID][emptyNum].igmpMulTicastAddr[1] = multicastAddr[1];
    gFpgaIgmpDev.igmpData[ethID][emptyNum].igmpMulTicastAddr[2] = multicastAddr[2];
    gFpgaIgmpDev.igmpData[ethID][emptyNum].igmpMulTicastAddr[3] = multicastAddr[3];

    //设置组播RecordType
    if (sourceIp[0] == 0 && sourceIp[1] == 0 && sourceIp[2] == 0 && sourceIp[3] == 0) //无源
    {
        //无源设置为0
        gFpgaIgmpDev.igmpData[ethID][emptyNum].igmpNumOfSource = 0;
        //加入无源 recrdType设置为4
        gFpgaIgmpDev.igmpData[ethID][emptyNum].igmpRecordType = 0x4;
        //有源设置igmp源地址
        gFpgaIgmpDev.igmpData[ethID][emptyNum].igmpSourceAddr[0] = 0;
        gFpgaIgmpDev.igmpData[ethID][emptyNum].igmpSourceAddr[1] = 0;
        gFpgaIgmpDev.igmpData[ethID][emptyNum].igmpSourceAddr[2] = 0;
        gFpgaIgmpDev.igmpData[ethID][emptyNum].igmpSourceAddr[3] = 0;
        //设置ip数据长度，长度包括ip首部和组播数据
        gFpgaIgmpDev.igmpData[ethID][emptyNum].ipDataLen = 40 << 8;
        igmpDataLen = 16;
    }
    else
    {
        //有源设置为1
        gFpgaIgmpDev.igmpData[ethID][emptyNum].igmpNumOfSource = 0x0100;
        //加入有缘recordType 设置为3
        gFpgaIgmpDev.igmpData[ethID][emptyNum].igmpRecordType = 0x3; //有源
        //有源设置igmp源地址
        gFpgaIgmpDev.igmpData[ethID][emptyNum].igmpSourceAddr[0] = sourceIp[0];
        gFpgaIgmpDev.igmpData[ethID][emptyNum].igmpSourceAddr[1] = sourceIp[1];
        gFpgaIgmpDev.igmpData[ethID][emptyNum].igmpSourceAddr[2] = sourceIp[2];
        gFpgaIgmpDev.igmpData[ethID][emptyNum].igmpSourceAddr[3] = sourceIp[3];
        //设置ip数据长度，长度包括ip首部和组播数据
        gFpgaIgmpDev.igmpData[ethID][emptyNum].ipDataLen = 44 << 8;
        igmpDataLen = 20;
    }

    //get ip checksum
    gFpgaIgmpDev.igmpData[ethID][emptyNum].ipHeaderChecksum = 0;
    WV_U8 *pipHeadData = &gFpgaIgmpDev.igmpData[ethID][emptyNum].ipVerion;
    fpga_igmp_checkSum(pipHeadData, ipDataLen, &gFpgaIgmpDev.igmpData[ethID][emptyNum].ipHeaderChecksum);
    //FPGA_printf("check sum = 0x%04x\n",gFpgaIgmpDev.igmpData[ethID][emptyNum].ipHeaderChecksum);

    //get igmp checksum
    gFpgaIgmpDev.igmpData[ethID][emptyNum].igmpHeaderChecksum = 0;
    WV_U8 *pigmpData = &gFpgaIgmpDev.igmpData[ethID][emptyNum].igmpType;
    fpga_igmp_checkSum(pigmpData, igmpDataLen, &gFpgaIgmpDev.igmpData[ethID][emptyNum].igmpHeaderChecksum);

    //加入组播
    WV_U16 data, regAddr;
    WV_U8 *pIgmpBuf = (WV_U8 *)&gFpgaIgmpDev.igmpData[ethID][emptyNum];
    regAddr = 0x81 | (ethID + 1) << 8;
    for (i = 0; i < sizeof(_S_FPGA_IGMP_DATA); i++)
    {
        if (i == 0)
        {
            data = 0x100;
        }
        else if (i == (sizeof(_S_FPGA_IGMP_DATA) - 1))
        {
            data = 0x200;
        }
        else
        {
            data = 0;
        }
        data |= (srcID << 10);
        data |= pIgmpBuf[i];
        HIS_SPI_FpgaWd(regAddr, data);
        //FPGA_printf("igmp set 0x%04x = 0x%04x\n", regAddr, data);
    }

    gFpgaIgmpDev.ena[ethID][emptyNum] = 1;
    return WV_SOK;
}
/************************************************************
 * WV_S32 fpga_igmp_exit(WV_S32 ethID,WV_U8 srcIp[])
 * *********************************************************/
static WV_S32 fpga_igmp_exit(WV_S32 ethID, WV_S32 srcID, WV_U8 multicastAddr[])
{

    if (ethID >= 4)
    {
        FPGA_printf("igmp exit set ethID err!! ethID=%d \n", ethID);
    }

    WV_S32 igmpDataLen = 0;
    WV_S32 ipDataLen = 24;
    WV_U8 srcMac[6] = {0};
    WV_U8 srcIp[4] = {0};

    WV_S32 i, multicastAddrIsExist = srcID;
    FPGA_CONF_GetEthInt(srcIp, srcMac, ethID);

    //生成组播报文
    //80:9f:fb:88:88:01
    //本地网卡mac地址
    for (i = 0; i < 6; i++)
    {
        gFpgaIgmpDev.igmpData[ethID][multicastAddrIsExist].srcMac[i] = srcMac[i];
    }
    //本地ip地址
    for (i = 0; i < 4; i++)
    {
        gFpgaIgmpDev.igmpData[ethID][multicastAddrIsExist].ipSrc[i] = srcIp[i];
    }
    //设置退出的组播的地址
    gFpgaIgmpDev.igmpData[ethID][multicastAddrIsExist].igmpMulTicastAddr[0] = multicastAddr[0];
    gFpgaIgmpDev.igmpData[ethID][multicastAddrIsExist].igmpMulTicastAddr[1] = multicastAddr[1];
    gFpgaIgmpDev.igmpData[ethID][multicastAddrIsExist].igmpMulTicastAddr[2] = multicastAddr[2];
    gFpgaIgmpDev.igmpData[ethID][multicastAddrIsExist].igmpMulTicastAddr[3] = multicastAddr[3];

    //设置组播RecordType

    //无源设置为0
    gFpgaIgmpDev.igmpData[ethID][multicastAddrIsExist].igmpNumOfSource = 0;
    //加入无源 recrdType设置为4
    gFpgaIgmpDev.igmpData[ethID][multicastAddrIsExist].igmpRecordType = 0x3;
    //设置ip数据长度，长度包括ip首部和组播数据
    gFpgaIgmpDev.igmpData[ethID][multicastAddrIsExist].ipDataLen = 40 << 8;

    gFpgaIgmpDev.igmpData[ethID][multicastAddrIsExist].igmpSourceAddr[0] = 0;
    gFpgaIgmpDev.igmpData[ethID][multicastAddrIsExist].igmpSourceAddr[1] = 0;
    gFpgaIgmpDev.igmpData[ethID][multicastAddrIsExist].igmpSourceAddr[2] = 0;
    gFpgaIgmpDev.igmpData[ethID][multicastAddrIsExist].igmpSourceAddr[3] = 0;

    //get ip checksum
    gFpgaIgmpDev.igmpData[ethID][multicastAddrIsExist].ipHeaderChecksum = 0;
    WV_U8 *pipHeadData = &gFpgaIgmpDev.igmpData[ethID][multicastAddrIsExist].ipVerion;
    fpga_igmp_checkSum(pipHeadData, ipDataLen, &gFpgaIgmpDev.igmpData[ethID][multicastAddrIsExist].ipHeaderChecksum);
    //FPGA_printf("check sum = 0x%04x\n",gFpgaIgmpDev.igmpData[ethID][emptyNum].ipHeaderChecksum);

    //get igmp checksum
    gFpgaIgmpDev.igmpData[ethID][multicastAddrIsExist].igmpHeaderChecksum = 0;
    WV_U8 *pigmpData = &gFpgaIgmpDev.igmpData[ethID][multicastAddrIsExist].igmpType;
    igmpDataLen = 16; //离开组播因为没有设置离开哪个源，所以直接没有源地址
    fpga_igmp_checkSum(pigmpData, igmpDataLen, &gFpgaIgmpDev.igmpData[ethID][multicastAddrIsExist].igmpHeaderChecksum);

    //退出组播
    WV_U16 data, regAddr;
    WV_U8 *pIgmpBuf = (WV_U8 *)&gFpgaIgmpDev.igmpData[ethID][multicastAddrIsExist];
    regAddr = 0x81 | (ethID + 1) << 8;
    for (i = 0; i < sizeof(_S_FPGA_IGMP_DATA); i++)
    {
        if (i == 0)
        {
            data = 0x100;
        }
        else if (i == (sizeof(_S_FPGA_IGMP_DATA) - 1))
        {
            data = 0x200;
        }
        else
        {
            data = 0;
        }
        data |= (srcID << 10);
        data |= pIgmpBuf[i];
        HIS_SPI_FpgaWd(regAddr, data);

    }
    gFpgaIgmpDev.ena[ethID][multicastAddrIsExist] = 0;
    return WV_SOK;
}
/****************************************************
*发送组播协议
*void fpga_igmp_sendIGMPData(WV_S32 ethID,WV_S32 srcID);
****************************************************/
static void fpga_igmp_sendIGMPData(WV_S32 ethID, WV_S32 srcID)
{

    if (ethID > 3 || ethID < 0 || srcID > 3 || srcID < 0)
    {
        WV_ERROR("fpga send igmp data err!! ethID=[%d],srcID=[%d]\n", ethID, srcID);
        return;
    }
    WV_U16 regAddr, data;
    regAddr = ((ethID + 1) << 8) | 0x82;
    do
    {
        HIS_SPI_FpgaRd(regAddr, &data);
        //WV_printf("his spi rd %X = %X \n");
        usleep(10000);
        //sleep(2);
    } while ((data & (1 << (srcID + 4))) == 0);
    data = 1 << srcID;

    HIS_SPI_FpgaWd(regAddr, data);
    FPGA_printf("**send igmp*****reg=0x%X ,data=0x%X\n", regAddr, data);
}
/****************************************************
*void fpga_igmp_Send()
*发送当前所有的组播协议
****************************************************/
static void fpga_igmp_Send()
{

    pthread_mutex_lock(&_g_igmp_mutex);
    WV_S32 i, j,k;
    WV_U16 reg, data = 0;

    for (i = 0; i < _D_ETH_NUM_; i++)
    {
        data = 0;
        reg = 0x82 | ((i + 1) << 8);
        for(k=0;k<100;k++){
            HIS_SPI_FpgaRd(reg, &data);
            if((data & 0xf0 ) == 0xf0) break;
            usleep(10000);
        }
        if(k==100) continue;
        data = 0;
        for (j = 0; j < _D_SRC_NUM_; j++)
        {
            if (gFpgaIgmpDev.ena[i][j] == 0)
                continue;

            data |= (1 << j);
        }
        if (data != 0)
        {
            HIS_SPI_FpgaWd(reg, data);
        }
    }
    pthread_mutex_unlock(&_g_igmp_mutex);
}
/****************************************************
*组播协议自动发送开关
*void fpga_igmp_autoSendData(WV_S32 ena)
****************************************************/
void fpga_igmp_autoSendData(WV_S32 ena)
{
    if(ena == 0)
    {
        gFpgaIgmpDev.autoSendIgmpEna = 0;
    }else{
        gFpgaIgmpDev.autoSendIgmpEna = 1;
    }
}
/****************************************************
*组播协议发送时间间隔
*void FPGA_IGMP_SetSecondOfIgmpSend(WV_S32 sec)
****************************************************/
void FPGA_IGMP_SetSecondOfIgmpSend(WV_U32 sec)
{
    gFpgaIgmpDev.secondOfIgmpSend = sec;
    //FPGA_printf("set second of igmp data send ,sec=%d\n", sec);
}
/****************************************************
*组播协议加入
*void FPGA_IGMP_join(WV_S32 ethID, WV_U8 multicastAddr[], WV_U8 sourceIp[])
****************************************************/
void FPGA_IGMP_join(WV_S32 ethID, WV_S32 srcID, WV_U8 multicastAddr[], WV_U8 sourceIp[])
{
    if(multicastAddr[0] < 224 || multicastAddr[0] > 239) return;
    pthread_mutex_lock(&_g_igmp_mutex);
    fpga_igmp_join(ethID, srcID, multicastAddr, sourceIp);
    fpga_igmp_sendIGMPData(ethID, srcID);
    pthread_mutex_unlock(&_g_igmp_mutex);
}
/****************************************************
*组播协议退出
*void FPGA_IGMP_exit(WV_S32 ethID,  WV_S32 srcID, WV_U8 multicastAddr[])
****************************************************/
void FPGA_IGMP_exit(WV_S32 ethID, WV_S32 srcID, WV_U8 multicastAddr[])
{
    if(multicastAddr[0] < 224 || multicastAddr[0] > 239) return;
    pthread_mutex_lock(&_g_igmp_mutex);
    fpga_igmp_exit(ethID, srcID, multicastAddr);
    fpga_igmp_sendIGMPData(ethID, srcID);
    pthread_mutex_unlock(&_g_igmp_mutex);
}

/****************************************************
*重新加入组播，这个一般在设置完万兆网卡以后进行重新加入组播
*void FPGA_IGMP_Reset()
****************************************************/
void FPGA_IGMP_Reset()
{
    WV_S32 i, j;
    for (i = 0; i < _D_ETH_NUM_; i++)
    {
        for (j = 0; j < _D_SRC_NUM_; j++)
        {
            if (gFpgaIgmpDev.ena[i][j] == 0)
                continue;
            //FPGA_IGMP_exit(i, j, gFpgaIgmpDev.igmpData[i][j].igmpMulTicastAddr);
            FPGA_IGMP_join(i, j, gFpgaIgmpDev.igmpData[i][j].igmpMulTicastAddr, gFpgaIgmpDev.igmpData[i][j].igmpSourceAddr);
        }
    }
}

/*****************************************************
 * void *fpga_igmp_proc(void *prm)
 * 组播定时发送
 * ***************************************************/
void *fpga_igmp_proc(void *prm)
{

    FPGA_IGMP_DEV *pDev = (FPGA_IGMP_DEV *)prm;
    pDev->open = 1;
    pDev->close = 0;

    while (pDev->open == 1)
    {
        if(pDev->autoSendIgmpEna == 1){
            fpga_igmp_Send();
        }

        for(pDev->times=0;pDev->times < pDev->secondOfIgmpSend;pDev->times++){
            sleep(1);
        }

    }
    pDev->open = 0;
    pDev->close = 1;
    return NULL;
}

/****************************************************************************

WV_S32 FPGA_CONF_SetCmd(WV_S32 argc, WV_S8 **argv,WV_S8 *prfBuff)

****************************************************************************/
WV_S32 FPGA_IGMP_SetCmd(WV_S32 argc, WV_S8 **argv, WV_S8 *prfBuff)
{
    WV_U32 id, srcId;
    WV_S32 ret = 0;
    if (argc < 1)
    {
        prfBuff += sprintf(prfBuff, "set igmp <cmd>;//cmd like: join/exit/send/sec/auto\r\n");
        return 0;
    }

    if (strcmp(argv[0], "join") == 0)
    {

        if (argc < 5)
        {

            prfBuff += sprintf(prfBuff, "set igmp join <ethID> <srcID> <muticastAddr> <srcAddr>\r\n");
            return WV_SOK;
        }
        ret = WV_STR_S2v(argv[1], &id);
        ret = WV_STR_S2v(argv[2], &srcId);
        WV_U8 muticastAddr[4] = {0};
        WV_U8 srcAddr[4] = {0};
        FPGA_COMMON_getIpInt(argv[3], muticastAddr);
        FPGA_COMMON_getIpInt(argv[4], srcAddr);
        prfBuff += sprintf(prfBuff, "igmp join eth[%d],muticastAddr=%d.%d.%d.%d,srcAddr=%d.%d.%d.%d\r\n", id,
                           muticastAddr[0], muticastAddr[1], muticastAddr[2], muticastAddr[3],
                           srcAddr[0], srcAddr[1], srcAddr[2], srcAddr[3]);
        FPGA_IGMP_join(id, srcId, muticastAddr, srcAddr);
        return WV_SOK;
    }
    else if (strcmp(argv[0], "exit") == 0)
    {
        if (argc < 3)
        {

            prfBuff += sprintf(prfBuff, "set igmp exit <ethID><srcID> <muticastAddr>\r\n");
            return WV_SOK;
        }

        ret = WV_STR_S2v(argv[1], &id);
        ret = WV_STR_S2v(argv[2], &srcId);

        WV_U8 muticastAddr[4] = {0};
        FPGA_COMMON_getIpInt(argv[3], muticastAddr);
        FPGA_IGMP_exit(id, srcId, muticastAddr);
        prfBuff += sprintf(prfBuff, "igmp exit eth[%d][%d],muticastAddr=%d.%d.%d.%d\r\n", id, srcId, muticastAddr[0], muticastAddr[1], muticastAddr[2], muticastAddr[3]);
    }
    else if (strcmp(argv[0], "send") == 0)
    {
        fpga_igmp_Send();
    }
    else if (strcmp(argv[0], "sec") == 0)
    {
        if (argc < 2)
        {

            prfBuff += sprintf(prfBuff, "set igmp sec <time sec>\r\n");
            return WV_SOK;
        }
        WV_U32 sec;
        ret = WV_STR_S2v(argv[1], &sec);
        FPGA_IGMP_SetSecondOfIgmpSend(sec);
    }
    else if (strcmp(argv[0], "auto") == 0)
    {
        if (argc < 2)
        {

            prfBuff += sprintf(prfBuff, "set igmp auto <0 or 1>\r\n");
            return WV_SOK;
        }
        WV_U32 ena;
        ret = WV_STR_S2v(argv[1], &ena);
        fpga_igmp_autoSendData((WV_S32 )ena);
    }
    else
    {
        prfBuff += sprintf(prfBuff, "err!there is no cmd like:set igmp %s\r\n", argv[0]);
    }

    return WV_SOK;
}

/****************************************************************************
 * //初始化igmp
 * void fpga_igmp_init()
****************************************************************************/
void fpga_igmp_init()
{
    //初始化线程锁
    pthread_mutex_init(&_g_igmp_mutex, NULL);
    memset(&gFpgaIgmpDev, 0, sizeof(FPGA_IGMP_DEV));

    //secondOfIgmpSend
    gFpgaIgmpDev.secondOfIgmpSend = 1;
    gFpgaIgmpDev.times = 0;
    gFpgaIgmpDev.autoSendIgmpEna = 1;
    WV_S32 i, j;
    for (i = 0; i < 4; i++)
    {
        for (j = 0; j < 4; j++)
        {

            gFpgaIgmpDev.igmpData[i][j].desMac[0] = 0x01;
            gFpgaIgmpDev.igmpData[i][j].desMac[1] = 0x00;
            gFpgaIgmpDev.igmpData[i][j].desMac[2] = 0x5E;
            gFpgaIgmpDev.igmpData[i][j].desMac[3] = 0x00;
            gFpgaIgmpDev.igmpData[i][j].desMac[4] = 0x00;
            gFpgaIgmpDev.igmpData[i][j].desMac[5] = 0x16;
            gFpgaIgmpDev.igmpData[i][j].type = 0x0008;

            gFpgaIgmpDev.igmpData[i][j].ipVerion = 0x46;
            gFpgaIgmpDev.igmpData[i][j].ipTOSType = 0x00;
            gFpgaIgmpDev.igmpData[i][j].ipId = 0xAFB5;
            gFpgaIgmpDev.igmpData[i][j].ipSign = 0;
            gFpgaIgmpDev.igmpData[i][j].ipTTL = 0x01;
            gFpgaIgmpDev.igmpData[i][j].ipProtocal = 0x2;
            gFpgaIgmpDev.igmpData[i][j].ipDes[0] = 0xe0;
            gFpgaIgmpDev.igmpData[i][j].ipDes[1] = 0x00;
            gFpgaIgmpDev.igmpData[i][j].ipDes[2] = 0x00;
            gFpgaIgmpDev.igmpData[i][j].ipDes[3] = 0x16;
            gFpgaIgmpDev.igmpData[i][j].options[0] = 0x94;
            gFpgaIgmpDev.igmpData[i][j].options[1] = 0x04;
            gFpgaIgmpDev.igmpData[i][j].options[2] = 0x00;
            gFpgaIgmpDev.igmpData[i][j].options[3] = 0x00;

            gFpgaIgmpDev.igmpData[i][j].igmpType = 0x22;
            gFpgaIgmpDev.igmpData[i][j].igmpReserved0 = 0x00;
            gFpgaIgmpDev.igmpData[i][j].igmpReserved1 = 0x00;
            gFpgaIgmpDev.igmpData[i][j].igmpNumOfGroupRecords = 0x0100;
            gFpgaIgmpDev.igmpData[i][j].igmpAuxDataLen = 0x00;
            gFpgaIgmpDev.igmpData[i][j].igmpAdditionalData[0] = 0x00;
            gFpgaIgmpDev.igmpData[i][j].igmpAdditionalData[1] = 0x00;
        }
    }
}
/****************************************************************************
 * //去初始化igmp
 * void FPGA_IGMP_DeInit()
****************************************************************************/
void fpga_igmp_deInit()
{
    pthread_mutex_destroy(&_g_igmp_mutex);
}
/************************************************************
 * void FPGA_IGMP_Open()
 * *********************************************************/
void FPGA_IGMP_Open()
{
    fpga_igmp_init();

    WV_THR_Create(&gFpgaIgmpDev.thrHndl, fpga_igmp_proc, WV_THR_PRI_DEFAULT, WV_THR_STACK_SIZE_DEFAULT, &gFpgaIgmpDev);
    WV_CMD_Register("set", "igmp", " set igmp", FPGA_IGMP_SetCmd);
    FPGA_printf("\nfpga igmp init end \n");
}
/************************************************************
 * void FPGA_IGMP_Close()
 * *********************************************************/
void FPGA_IGMP_Close()
{

    if (gFpgaIgmpDev.open == 1)
    {
        gFpgaIgmpDev.open = 0;
        while (gFpgaIgmpDev.close == 1)
            ;
        WV_THR_Destroy(&gFpgaIgmpDev.thrHndl);
    }
    fpga_igmp_deInit();
    FPGA_printf("\nfpga igmp deinit ok..\n");
}
