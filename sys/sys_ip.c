#include "sys_ip.h"
#include "sys_env.h"
typedef struct SYS_IP_CONF_E
{
	WV_S8 name[20];
	WV_S8 mac[20];
	WV_S8 ip[20];
	WV_S8 mask[20];
	WV_S8 gw[20];
} SYS_IP_CONF_E;

static SYS_IP_CONF_E gSysIpConf;

/********************************************************

WV_S32 SYS_IP_SwitchChar(WV_S8 *pStr,WV_S32 *pValue)

*********************************************************/
WV_S32 SYS_IP_SwitchChar(WV_S8 *pStr, WV_S32 *pValue)
{
	if (strncmp(pStr, "0", 1) == 0)
	{
		pValue[0] = 0;
		return WV_SOK;
	}
	else if (strncmp(pStr, "1", 1) == 0)
	{
		pValue[0] = 1;
		return WV_SOK;
	}
	else if (strncmp(pStr, "2", 1) == 0)
	{
		pValue[0] = 2;
		return WV_SOK;
	}
	else if (strncmp(pStr, "3", 1) == 0)
	{
		pValue[0] = 3;
		return WV_SOK;
	}
	else if (strncmp(pStr, "4", 1) == 0)
	{
		pValue[0] = 4;
		return WV_SOK;
	}
	else if (strncmp(pStr, "5", 1) == 0)
	{
		pValue[0] = 5;
		return WV_SOK;
	}
	else if (strncmp(pStr, "6", 1) == 0)
	{
		pValue[0] = 6;
		return WV_SOK;
	}
	else if (strncmp(pStr, "7", 1) == 0)
	{
		pValue[0] = 7;
		return WV_SOK;
	}
	else if (strncmp(pStr, "8", 1) == 0)
	{
		pValue[0] = 8;
		return WV_SOK;
	}
	else if (strncmp(pStr, "9", 1) == 0)
	{
		pValue[0] = 9;
		return WV_SOK;
	}
	else if ((strncmp(pStr, "a", 1) == 0) || (strncmp(pStr, "A", 1) == 0))
	{
		pValue[0] = 10;
		return WV_SOK;
	}
	else if ((strncmp(pStr, "b", 1) == 0) || (strncmp(pStr, "B", 1) == 0))
	{
		pValue[0] = 11;
		return WV_SOK;
	}
	else if ((strncmp(pStr, "c", 1) == 0) || (strncmp(pStr, "C", 1) == 0))
	{
		pValue[0] = 12;
		return WV_SOK;
	}
	else if ((strncmp(pStr, "d", 1) == 0) || (strncmp(pStr, "D", 1) == 0))
	{
		pValue[0] = 13;
		return WV_SOK;
	}
	else if ((strncmp(pStr, "e", 1) == 0) || (strncmp(pStr, "E", 1) == 0))
	{
		pValue[0] = 14;
		return WV_SOK;
	}
	else if ((strncmp(pStr, "f", 1) == 0) || (strncmp(pStr, "F", 1) == 0))
	{
		pValue[0] = 15;
		return WV_SOK;
	}
	else
	{
		return WV_EFAIL;
	}
}

/*******************************************************************
WV_S32 SYS_IP_getIpInt(WV_S8 *pName,WV_U8* pIp);
*******************************************************************/
/*WV_S32 SYS_IP_getIpInt(WV_S8 *pName, WV_U8 *pIp)
{

	WV_S8 pSrc[20];
	SYS_ENV_Get(pName, pSrc);
	WV_S8 *pData = pSrc;
	WV_S32 len;
	WV_S32 i, j, k, data = 0;
	WV_S32 des;
	len = strlen(pSrc);
	if (strncmp(pData, ".", 1) == 0)
	{
		printf("get ip error\r\n");
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
}*/
/*******************************************************************
WV_S32 SYS_IP_getIpInt(WV_S8 *pName,WV_U8* pIp);
*******************************************************************/
WV_S32 SYS_IP_getIpInt(WV_S8 *pName,WV_U8* pIp)
{

	WV_S8 pSrc[20];
	SYS_ENV_Get(pName,pSrc);
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
			printf("%d = %d\n",j,pIp[j]);
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
/*******************************************************************
WV_S32 SYS_IP_getMacInt(WV_U8* pMac);
*******************************************************************/
WV_S32 SYS_IP_getMacInt(WV_U8 *pMac)
{

	WV_S8 pSrc[20];
	memcpy(pSrc, gSysIpConf.mac, 20);
	WV_S8 *pData = pSrc;
	WV_S32 len;
	WV_S32 i, j, k, data = 0;
	WV_S32 des;

	len = strlen(pSrc);
	/*
	if(strncmp(pData,":",1) == 0){
		printf("get ip error\r\n");
		return WV_EFAIL;
	}
   */
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

	//WV_printf("mac  %02x:%02x:%02x:%02x:%02x:%02x \n",pMac[0],pMac[1],pMac[2],pMac[3],pMac[4],pMac[5],pMac[6]);

	return WV_SOK;
}

/*******************************************************************
 WV_S32 SYS_IP_RegisterConf();
*******************************************************************/
WV_S32 SYS_IP_RegisterConf()
{
	SYS_ENV_Register(0, "IPconfigAll", NULL, NULL);
	SYS_ENV_Register(1, "IP_Name", "eth0", "eth name");
	SYS_ENV_Register(1, "IP_Mac", "00:00:01:37:98:00", "eth mac");
	SYS_ENV_Register(1, "IP_Ip", "192.168.15.98", "eth ip");
	SYS_ENV_Register(1, "IP_Mask", "255.255.255.0", "eth mask");
	SYS_ENV_Register(1, "IP_Gatway", "192.168.15.1", "eth gate way");
	return WV_SOK;
}

/*******************************************************************
 WV_S32 SYS_IP_GetConf();
*******************************************************************/

WV_S32 SYS_IP_GetConf()
{

	SYS_ENV_Get("IP_Name", gSysIpConf.name);
	SYS_ENV_Get("IP_Mac", gSysIpConf.mac);
	SYS_ENV_Get("IP_Ip", gSysIpConf.ip);
	SYS_ENV_Get("IP_Mask", gSysIpConf.mask);
	SYS_ENV_Get("IP_Gatway", gSysIpConf.gw);
	return WV_SOK;
}

/*******************************************************************
 WV_S32 SYS_IP_SaveConf();
*******************************************************************/
WV_S32 SYS_IP_SaveConf()
{

	SYS_ENV_Set("IP_Name", gSysIpConf.name);
	SYS_ENV_Set("IP_Mac", gSysIpConf.mac);
	SYS_ENV_Set("IP_Ip", gSysIpConf.ip);
	SYS_ENV_Set("IP_Mask", gSysIpConf.mask);
	SYS_ENV_Set("IP_Gatway", gSysIpConf.gw);
	WV_IP_Set(gSysIpConf.name, gSysIpConf.mac, gSysIpConf.ip, gSysIpConf.mask, gSysIpConf.gw);
	SYS_ENV_Save();
	//sleep(2);
	//WV_CMD_GetReset();

	WV_S8 cmd[128] = {0};
	sprintf(cmd, "ifconfig %s down\n", gSysIpConf.name);
	system(cmd);
	sprintf(cmd, "ifconfig %s hw ether %s\n", gSysIpConf.name, gSysIpConf.mac);
	system(cmd);
	sprintf(cmd, "ifconfig %s up\n", gSysIpConf.name);
	system(cmd);
	sprintf(cmd, "ifconfig %s %s netmask %s", gSysIpConf.name, gSysIpConf.ip, gSysIpConf.mask);
	system(cmd);
	sprintf(cmd, "route add default gw %s", gSysIpConf.gw);
	system(cmd);

	return WV_SOK;
}
/*******************************************************************
 WV_S32 SYS_IP_SaveUpdate(WV_S8 *mac);
*******************************************************************/
WV_S32 SYS_IP_SaveUpdate(WV_S8 *mac)
{

	SYS_ENV_Set("IP_Name", gSysIpConf.name);
	SYS_ENV_Set("IP_Mac", gSysIpConf.mac);
	SYS_ENV_Set("IP_Ip", gSysIpConf.ip);
	SYS_ENV_Set("IP_Mask", gSysIpConf.mask);
	SYS_ENV_Set("IP_Gatway", gSysIpConf.gw);
	WV_IP_Set(gSysIpConf.name, gSysIpConf.mac, gSysIpConf.ip, gSysIpConf.mask, gSysIpConf.gw);
	SYS_ENV_Save();
	return WV_SOK;
}

/*******************************************************************
 WV_S32 SYS_IP_CheckIP();
*******************************************************************/
WV_S32 SYS_IP_CheckIP()
{
	WV_S8 mac[30], ip[30];
	WV_IP_Get(gSysIpConf.name, mac, ip);

	if (strcmp(mac, gSysIpConf.mac) != 0)
	{
		return WV_EFAIL;
	}
	if (strcmp(ip, gSysIpConf.ip) != 0)
	{
		return WV_EFAIL;
	}
	printf("IP config: name [eth0] mac[%s] ip[%s]\r\n", mac, ip);
	return WV_SOK;
}

/*******************************************************************
 WV_S32 SYS_IP_SetMac();
*******************************************************************/

WV_S32 SYS_IP_SetMac(WV_S8 *pMac)
{
	if (strcmp(pMac, gSysIpConf.mac) != 0)
	{
		WV_CONF_CpyStr(pMac, gSysIpConf.mac, 20);
		return WV_EFAIL;
	}
	return WV_SOK;
}

/*******************************************************************
 WV_S32 SYS_GetMac();
*******************************************************************/
WV_S32 SYS_IP_GetMac(WV_S8 *pMac)
{

	WV_CONF_CpyStr(gSysIpConf.mac, pMac, 20);
	return WV_SOK;
}

/*******************************************************************
 WV_S32 SYS_IP_SetIp();
*******************************************************************/

WV_S32 SYS_IP_SetIp(WV_S8 *pIp)
{
	if (strcmp(pIp, gSysIpConf.ip) != 0)
	{
		WV_CONF_CpyStr(pIp, gSysIpConf.ip, 20);
		return WV_EFAIL;
	}
	return WV_SOK;
}
/*******************************************************************
 WV_S32 SYS_GetIP();
*******************************************************************/

WV_S32 SYS_IP_GetIp(WV_S8 *pIp)
{

	WV_CONF_CpyStr(gSysIpConf.ip, pIp, 20);
	return WV_SOK;
}
/*******************************************************************
 WV_S32 SYS_IP_SetMask();
*******************************************************************/

WV_S32 SYS_IP_SetMask(WV_S8 *pMsk)
{
	if (strcmp(pMsk, gSysIpConf.mask) != 0)
	{
		WV_CONF_CpyStr(pMsk, gSysIpConf.mask, 20);
		return WV_EFAIL;
	}
	return WV_SOK;
}

/*******************************************************************
 WV_S32 SYS_GetMask();
*******************************************************************/
WV_S32 SYS_IP_GetMask(WV_S8 *pMask)
{

	WV_CONF_CpyStr(gSysIpConf.mask, pMask, 20);
	return WV_SOK;
}
/*******************************************************************
 WV_S32 SYS_IP_SetGw();
*******************************************************************/

WV_S32 SYS_IP_SetGw(WV_S8 *pGw)
{
	if (strcmp(pGw, gSysIpConf.gw) != 0)
	{
		WV_CONF_CpyStr(pGw, gSysIpConf.gw, 20);
		return WV_EFAIL;
	}
	return WV_SOK;
}
/*******************************************************************
 WV_S32 SYS_GetGw();
*******************************************************************/

WV_S32 SYS_IP_GetGw(WV_S8 *pGw)
{
	WV_CONF_CpyStr(gSysIpConf.gw, pGw, 20);
	return WV_SOK;
}
/**********************************************************************
 * WV_S32 SYS_IP_SetEthIpConf(WV_U8 ip[], WV_U8 mask[], WV_U8 gw[])
 * 设置ip，网关等
 * ********************************************************************/
WV_S32 SYS_IP_SetEthIpConf(WV_U8 ip[], WV_U8 mask[], WV_U8 gw[])
{
	WV_S8 pIp[32]={0};
	WV_S8 pMask[32]={0};
	WV_S8 pGetWay[32]={0};
	
	sprintf(pIp,"%d.%d.%d.%d",ip[0],ip[1],ip[2],ip[3]);
	sprintf(pMask,"%d.%d.%d.%d",mask[0],mask[1],mask[2],mask[3]);
	sprintf(pGetWay,"%d.%d.%d.%d",gw[0],gw[1],gw[2],gw[3]);

	SYS_IP_SetIp(pIp);
	SYS_IP_SetMask(pMask);
	SYS_IP_SetGw(pGetWay);
	SYS_IP_SaveConf();

	return WV_SOK;
}
/**********************************************************************
 * WV_S32 SYS_IP_SetEthMacConf(WV_U8 mac[])
 * 设置mac地址，因为mac地址不经常改变，所以mac地址单独设置，跟ip网关掩码分开
 * ********************************************************************/
WV_S32 SYS_IP_SetEthMacConf(WV_U8 mac[])
{
	WV_S8 pMac[32]={0};
	sprintf(pMac,"%02x:%02x:%02x:%02x:%02x:%02x",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
	SYS_IP_SetMac(pMac);
	SYS_IP_SaveConf();

	return WV_SOK;
}
/**********************************************************************
 * WV_S32 SYS_IP_GetEthConf(WV_U8 ip[], WV_U8 mask[], WV_U8 gw[],WV_U8 mac[])
 * 查询ip地址
 * ********************************************************************/
WV_S32 SYS_IP_GetEthConf(WV_U8 ip[], WV_U8 mask[], WV_U8 gw[],WV_U8 mac[])
{
	SYS_IP_getIpInt("IP_Ip",ip);
	SYS_IP_getIpInt("IP_Mask",mask);
	SYS_IP_getIpInt("IP_Gatway",gw);
	SYS_IP_getMacInt(mac);

	WV_printf("ip    %d.%d.%d.%d \n",ip[0],ip[1],ip[2],ip[3]);
	WV_printf("mask  %d.%d.%d.%d\n",mask[0],mask[1],mask[2],mask[3]);
	WV_printf("gw    %d.%d.%d.%d\n",gw[0],gw[1],gw[2],gw[3]);
	WV_printf("mask  %02x:%02x:%02x:%02x:%02x:%02x\n",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
	

	return WV_SOK;
}
/****************************************************************************
WV_S32 SYS_IP_CMDGet(WV_S32 argc, WV_S8 **argv,WV_S8 *prfBuff)
****************************************************************************/
WV_S32 SYS_IP_CMDGet(WV_S32 argc, WV_S8 **argv, WV_S8 *prfBuff)
{
	WV_S8 mac[30], ip[30];
	WV_IP_Get(gSysIpConf.name, mac, ip);
	prfBuff += sprintf(prfBuff, "ip config:\r\n");
	prfBuff += sprintf(prfBuff, "eth name :  %s\r\n", gSysIpConf.name);
	prfBuff += sprintf(prfBuff, "mac addr :  %s[%s]\r\n", gSysIpConf.mac, mac);
	prfBuff += sprintf(prfBuff, "ip  addr :  %s[%s]\r\n", gSysIpConf.ip, ip);
	prfBuff += sprintf(prfBuff, "ip  mask :  %s\r\n", gSysIpConf.mask);
	prfBuff += sprintf(prfBuff, "gate way :  %s\r\n", gSysIpConf.gw);

	// WV_U8 u8Ip[4];
	// WV_U8 u8Mask[4];
	// WV_U8 u8Gw[4]={0};
	// WV_U8 u8Mac[6];
	
	// SYS_IP_GetEthConf(u8Ip,u8Mask,u8Gw,u8Mac);

	return WV_SOK;
}

/****************************************************************************

WV_S32 SYS_IP_CMDSet(WV_S32 argc, WV_S8 **argv,WV_S8 *prfBuff)

****************************************************************************/
WV_S32 SYS_IP_CMDSet(WV_S32 argc, WV_S8 **argv, WV_S8 *prfBuff)
{
	WV_S8 mac[30], ip[30];
	WV_S32 ret;
	WV_IP_Get(gSysIpConf.name, mac, ip);
	if (argc < 1)
	{
		prfBuff += sprintf(prfBuff, "set ip  <cmd >  cdm: mac/ip/mask/gw/save\r\n");
		return WV_SOK;
	}

	if (strcmp(argv[0], "ip") == 0)
	{
		if (argc < 2)
		{
			prfBuff += sprintf(prfBuff, "set ip  ip ipAddr  --set ip ip 192.168.15.98\r\n");
			return WV_SOK;
		}

		ret = WV_IP_CheckIp(argv[1]);

		if (ret != WV_SOK)
		{
			prfBuff += sprintf(prfBuff, "ipAddr erro %d: %s \r\n", ret, argv[1]);
			return WV_SOK;
		}
		SYS_IP_SetIp(argv[1]);
		prfBuff += sprintf(prfBuff, "ipAddr = %s \r\n", gSysIpConf.ip);
		return WV_SOK;
	}
	if (strcmp(argv[0], "mac") == 0)
	{
		if (argc < 2)
		{
			prfBuff += sprintf(prfBuff, "set ip  mac macAddr  --set ip mac 00:00:00:01:02:03\r\n");
			return WV_SOK;
		}

		ret = WV_IP_CheckMac(argv[1]);
		if (ret != WV_SOK)
		{
			prfBuff += sprintf(prfBuff, "macAddr erro %d: %s \r\n", ret, argv[1]);
			return WV_SOK;
		}
		SYS_IP_SetMac(argv[1]);
		prfBuff += sprintf(prfBuff, "macAddr = %s \r\n", gSysIpConf.mac);
		return WV_SOK;
	}

	if (strcmp(argv[0], "mask") == 0)
	{
		if (argc < 2)
		{
			prfBuff += sprintf(prfBuff, "set ip  mask maskAddr  --set ip mask 255.255.255.0\r\n");
			return WV_SOK;
		}
		SYS_IP_SetMask(argv[1]);
		prfBuff += sprintf(prfBuff, "maskAddr = %s \r\n", gSysIpConf.mask);
		return WV_SOK;
	}
	if (strcmp(argv[0], "gw") == 0)
	{
		if (argc < 2)
		{
			prfBuff += sprintf(prfBuff, "set ip  gw gateway  --set ip gw 192.168.15.1\r\n");
			return WV_SOK;
		}
		SYS_IP_SetGw(argv[1]);
		prfBuff += sprintf(prfBuff, "gate way = %s \r\n", gSysIpConf.gw);
		return WV_SOK;
	}
	if (strcmp(argv[0], "save") == 0)
	{
		prfBuff += sprintf(prfBuff, "save ifconfig,resarting!!!!! \r\n");
		SYS_IP_SaveConf();
		return WV_SOK;
	}
	prfBuff += sprintf(prfBuff, "no cmd!!!!!: %s \r\n", argv[0]);
	return WV_SOK;
}
/*******************************************************************
 WV_S32 SYS_IP_Init();
*******************************************************************/
WV_S32 SYS_IP_Init()
{

	WV_S32 ret;
	SYS_IP_RegisterConf();
	SYS_IP_GetConf();
	ret = SYS_IP_CheckIP();
	WV_CMD_Register("get", "ip", "print ip info", SYS_IP_CMDGet);
	WV_CMD_Register("set", "ip", "set ip config", SYS_IP_CMDSet);
	if (ret != WV_SOK)
	{

		WV_IP_Set(gSysIpConf.name, gSysIpConf.mac, gSysIpConf.ip, gSysIpConf.mask, gSysIpConf.gw);
		WV_CMD_GetReset();
		return WV_EFAIL;
	}
	return WV_SOK;
}
/*******************************************************************
 WV_S32 SYS_IP_DeInit();
*******************************************************************/

WV_S32 SYS_IP_DeInit()
{

	return WV_SOK;
}
