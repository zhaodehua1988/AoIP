#include "wv_ip.h"
/*************************************************************************************

WV_S32 WV_IP_CheckIp(WV_S8 * pMac)

*************************************************************************************/
WV_S32  WV_IP_CheckMac(WV_S8 * pMac)
{
  WV_S32  len,i;

	if(pMac == NULL)
		{
		return WV_EFAIL;
		}
	len = strlen(pMac);
	if(len != 17)
		{
		return 1;
		}
	for(i =0;i<5;i++)
	{
		if(pMac[i*3+2] != ':')
			{
			return 2;
			}
	}

	for(i =0;i<6;i++)
	{
		if((pMac[i*3] >= '0' &&  pMac[i*3] <= '9') || (pMac[i*3] >= 'a' &&  pMac[i*3] <= 'f') || (pMac[i*3] >= 'A' &&  pMac[i*3] <= 'F'))
			{
			}
			else
			{ 
				return 3;
			}

		if((pMac[i*3+1] >= '0' &&  pMac[i*3+1] <= '9') || (pMac[i*3+1] >= 'a' &&  pMac[i*3+1] <= 'f') || (pMac[i*3+1] >= 'A' &&  pMac[i*3+1] <= 'F'))
			{
			}
			else
			{ 
				return 4;
			}
	}  
  return WV_SOK;
}

/*************************************************************************************

WV_S32 WV_IP_CheckIp(WV_S8 * pIp)

*************************************************************************************/
WV_S32 WV_IP_CheckIp(WV_S8 * pIp)
{
	WV_S32  len,i,start,dot,dotNum;
	WV_U32  data;
	if(pIp == NULL)
	{
		return WV_EFAIL;
	}
	len = strlen(pIp);
	//printf("strlen  = %d \r\n",len);
	if(len< 7 || len > 15)
	{ 
		return 1;
	}

	start= 0;
	dot = 0;
	dotNum = 0;
	for(i=0;i<len;i++)
	{
		if(pIp[i] == '.' )
		{
			dot= i;
			data = 0;
			 if(dot - start <1)
			 {
			   return 2;
			 } 
			 data = 0;
			for(;start< dot;start++)
			{ 
			  if(pIp[start] >'9'  || pIp[start] < '0')
			  {
				return 3;
			  }
			  data = data *10;
			  data += pIp[start] - '0'; 
			 // printf("data = %d ",data); 
			}
			//printf("data = %d  satart = %d dot = %d\r\n",data,start,dot);
		 
			if(data > 255)
			{
			  return 4;
			} 
			start ++;
			dotNum ++;
		 }  
	}
	
	data = 0;
	for(;start< len;start++)
	 { 
			  if(pIp[start] >'9'  || pIp[start] < '0')
			  {
				return 3;
			  }
			  data = data *10;
			  data += pIp[start] - '0'; 
			  //printf("data = %d ",data); 
	 }
	 if(data > 255)
			{
			  return 5;
			} 
	
   //printf("dotNum = %d\r\n",dotNum);
   if(dotNum != 3)
        { 
        return 6;
        }
  return WV_SOK;
}
/*************************************************************************************

WV_S32 WV_IP_Set(WV_S8 * pName ,WV_S8 * pMac ,WV_S8 * pIp ,WV_S8 * pMask ,WV_S8 * pGway)

*************************************************************************************/

WV_S32 WV_IP_Set(WV_S8 * pName ,WV_S8 * pMac ,WV_S8 * pIp ,WV_S8 * pMask ,WV_S8 * pGway)
{


 FILE *fp = NULL;
  
 fp = fopen("/etc/init.d/S82IpConfig", "w");
 if ( fp == NULL ) 
     
     { 
        WV_ERROR("setIpConfig fopen erro\r\n");
        return WV_EFAIL;
      } 
 fprintf(fp, " #!/bin/sh\n");
 fprintf(fp, "ifconfig %s down\n",pName);
 fprintf(fp, "ifconfig %s hw ether %s\n",pName,pMac);
 fprintf(fp, "ifconfig %s up\n",pName);
 fprintf(fp, "ifconfig %s %s netmask %s \n",pName,pIp,pMask);
 fprintf(fp, "route add default gw %s\n",pGway);  
 fprintf(fp, "\n");
 fflush(fp);
 fclose(fp);
 fp = NULL;
 system("sync");
 system("chmod +x /etc/init.d/S82IpConfig");  
  return WV_SOK;
}
 
/*************************************************************************************

WV_S32 WV_IP_Get(WV_S8 * pName ,WV_S8 * pMac ,WV_S8 * pIp)

*************************************************************************************/

WV_S32 WV_IP_Get(WV_S8 * pName ,WV_S8 * pMac ,WV_S8 * pIp )
{
 
 struct ifreq ifr;
 
 WV_S32  fd,ret,j;
 WV_S8 * pChar;
 
 
 fd = socket(AF_INET,SOCK_DGRAM,0);
 
 if(fd < 0)
  {
   WV_ERROR("socket(AF_INET,SOCK_DGRAM,0) erro\r\n");
   close(fd);
   return WV_EFAIL;
  }
   
		sprintf(ifr.ifr_name,"%s" ,pName); 
		ret = ioctl(fd,SIOCGIFFLAGS,&ifr);
		  	if(ret != 0 ) 
		  	{
		  	  WV_ERROR("ioctl(fd,SIOCGIFFLAGS,&ifr) erro\r\n");
				close(fd);
		   	 return WV_EFAIL;
		  	}
		  	 
		ret = ioctl(fd,SIOCGIFHWADDR,(char *)(&ifr));  
		    
		    if(ret != 0 ) 
		  	{
		  	   WV_ERROR("oioctl(fd,SIOCGIFHWADR,(char *)(&ifr_buf[i])) erro\r\n");
				close(fd);
		   	   return WV_EFAIL;
		  	} 
		pChar =  pMac; 	
	    for(j=0;j<6;j++ )
	    {
		 pChar += sprintf(pChar,"%02x:",(WV_S8)ifr.ifr_hwaddr.sa_data[j]); 		 
		}
		pChar--;
		*pChar= 0;  
	   ret = ioctl(fd,SIOCGIFADDR,(char *)(&ifr));  
		    
		    if(ret != 0 ) 
		  	{
		  	    WV_ERROR("ioctl(fd,SIOCGIFADDR,(char *)(&ifr_buf[i])) erro\r\n");
				close(fd);
		   	 return WV_EFAIL;
		  	}  	
      pChar = pIp;
      pChar += sprintf(pChar,"%s",(char  *)inet_ntoa(((struct sockaddr_in *)&(ifr.ifr_addr))->sin_addr)); 
	  *pChar = 0; 
  return WV_SOK;
}
