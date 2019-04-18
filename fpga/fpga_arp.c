#include "fpga_arp.h"
#include "fpga_common.h"
#include "fpga_conf.h"
#include "his_spi.h"

typedef struct FPGA_CONF_ARP
{
    WV_U8 desMac[6];       //ff:ff:ff:ff:ff:ff
    WV_U8 srcMac[6];       //源mac地址
    WV_U8 framType[2];     //0x0806 (arp)
    WV_U8 hardwareType[2]; //0x0001
    WV_U8 protocolType[2]; //0x0800 (ipv4)
    WV_U8 hardwareSize;    //0x6
    WV_U8 protocalSize;    //0x4
    WV_U8 op[2];           //0x0001 操作类型字段
    WV_U8 senderMac[6];    //发送端mac地址（同源mac地址）
    WV_U8 senderIp[4];     //发送端ip地址
    WV_U8 TargetMac[6];    //00:00:00:00:00:00 广播时全为0
    WV_U8 TargetIp[4];     //目的地址，需要用户输入
} _S_FPGA_CONF_ARP;

_S_FPGA_CONF_ARP *gpFpgaArpData;

/**************************************************************
 * void FPGA_ARP_Request(WV_U16 ethID,WV_S8 *desIp)
 * ************************************************************/
void FPGA_ARP_Request(WV_U16 ethID, WV_S8 *desIp)
{
    WV_U8 targetIp[_D_FPGA_COMMON_IPLEN + 1] = {0};
    WV_S32 i;
    FPGA_COMMON_getIpInt(desIp, targetIp);
    for (i = 0; i < _D_FPGA_COMMON_IPLEN; i++)
    {
        gpFpgaArpData->TargetIp[i] = targetIp[i];
    }
    WV_U8 sendIp[_D_FPGA_COMMON_IPLEN] = {0};
    WV_U8 sendMac[_D_FPGA_COMMON_MACLEN] = {0};
    FPGA_CONF_GetEthInt(sendIp, sendMac, ethID);
    for (i = 0; i < _D_FPGA_COMMON_MACLEN; i++)
    {
        gpFpgaArpData->senderMac[i] = sendMac[i];
        gpFpgaArpData->srcMac[i] = sendMac[i];
    }
    for (i = 0; i < _D_FPGA_COMMON_IPLEN; i++)
    {
        gpFpgaArpData->senderIp[i] = sendIp[i];
    }

    WV_U16 data, regAddr;
    WV_U8 *pArpData = (WV_U8 *)gpFpgaArpData;
    regAddr = 0x81 | (ethID + 1) << 8;
    for (i = 0; i < sizeof(_S_FPGA_CONF_ARP); i++)
    {
        if (i == 0)
        {
            data = 0x100;
        }
        else if (i == (sizeof(_S_FPGA_CONF_ARP) - 1))
        {
            data = 0x200;
        }
        else
        {
            data = 0;
        }

        data |= pArpData[i];
        //HIS_SPI_FpgaWd(regAddr, data);
        FPGA_printf("arp request set 0x%04x = 0x%04x\n", regAddr, data);
    }

    regAddr = ((ethID + 1) << 8) | 0x82;
    do
    {
        HIS_SPI_FpgaRd(regAddr, &data);
        usleep(10000);
    } while ((data & (1 << 4)) == 0);
    data = 1;

    //HIS_SPI_FpgaWd(regAddr, data);
    FPGA_printf("arp request set 0x%04x = 0x%04x\n", regAddr, data);
}

/***************************************************************
 * void FPGA_ARP_Init()
 * ************************************************************/
void FPGA_ARP_Init()
{

    gpFpgaArpData = (_S_FPGA_CONF_ARP *)malloc(sizeof(_S_FPGA_CONF_ARP));
    memset(gpFpgaArpData, 0, sizeof(_S_FPGA_CONF_ARP));
    WV_S32 i;
    for (i = 0; i < 6; i++)
    {
        gpFpgaArpData->desMac[i] = 0xff;
    }
    gpFpgaArpData->framType[0] = 0x08;
    gpFpgaArpData->framType[1] = 0x06;
    gpFpgaArpData->hardwareType[0] = 0;
    gpFpgaArpData->hardwareType[1] = 0x1;
    gpFpgaArpData->protocolType[0] = 0x08;
    gpFpgaArpData->protocolType[1] = 0;
    gpFpgaArpData->hardwareSize = 0x6;
    gpFpgaArpData->protocalSize = 0x4;
    gpFpgaArpData->op[0] = 0;
    gpFpgaArpData->op[1] = 1;
    for (i = 0; i < 6; i++)
    {
        gpFpgaArpData->TargetMac[i] = 0;
    }

    //WV_CMD_Register("set", "fpga", " set fpga", FPGA_CONF_SetCmd);
}
/***************************************************************
 * void FPGA_ARP_DeInit()
 * ************************************************************/
void FPGA_ARP_DeInit()
{
    free(gpFpgaArpData);
}