#include "hi_common.h"
#include "his_reg.h"
 



/****************************************************************************

WV_S32 HIS_REG_GetReg(WV_S32 argc, WV_S8 **argv,WV_S8 *prfBuff)

****************************************************************************/
WV_S32 HIS_REG_GetReg(WV_S32 argc, WV_S8 **argv, WV_S8 * prfBuff)
{
  WV_U32  regAddr,data,num; 
  WV_S32  ret,i;  
  
  if(argc<1) 
  {
   	prfBuff += sprintf(prfBuff,"get REG <RegAddr> <Num>\r\n");
    return 0;
  }

  
  
  ret =  WV_STR_S2v(argv[0],&regAddr);
  if(ret != WV_SOK)
  {
    prfBuff += sprintf(prfBuff,"<RegAddr> erro!\r\n"); 
  }
  
  num = 0;
   if(argc<3) 
  {
   	  ret =  WV_STR_S2v(argv[1],&num);
	  if(ret != WV_SOK)
	  {
		prfBuff += sprintf(prfBuff," <Num> erro!\r\n"); 
	  }
  }
  
  
  for(i=0;i< num ;i++)
  {

	  ret = HI_SYS_ReadRegister(regAddr,&data); 
	  if(ret != WV_SOK)
		  {
			prfBuff += sprintf(prfBuff,"HI_SYS_ReadRegister(0x%04x) erro!\r\n",regAddr); 
		  }
	  prfBuff += sprintf(prfBuff,"0x%04x 0x%04x \r\n",regAddr,data); 
	regAddr +=4;	  
		  
  }
 	 
 return WV_SOK;
 	
}





/****************************************************************************

WV_S32 HIS_REG_SetReg(WV_S32 argc, WV_S8 **argv,WV_S8 *prfBuff)

****************************************************************************/
WV_S32 HIS_REG_SetReg(WV_S32 argc, WV_S8 **argv, WV_S8 * prfBuff)
{
  WV_U32  regAddr,data; 
  WV_S32  ret;  
  
  if(argc<2) 
  {
   	prfBuff += sprintf(prfBuff,"Set REG <RegAddr> <Data>\r\n");
    return 0;
  }

  
  
	ret =  WV_STR_S2v(argv[0],&regAddr);
	if(ret != WV_SOK)
		{
			prfBuff += sprintf(prfBuff,"<RegAddr> erro!\r\n"); 
		}

	ret =  WV_STR_S2v(argv[1],&data);
	if(ret != WV_SOK)
		{
		prfBuff += sprintf(prfBuff," <Num> erro!\r\n"); 
		}
 
 
	ret = HI_SYS_WriteRegister(regAddr,data); 
	if(ret != WV_SOK)
	{
		prfBuff += sprintf(prfBuff,"HI_SYS_WriteRegister(0x%04x,0x%04x) erro!\r\n",regAddr,data); 
	}
	ret = HI_SYS_ReadRegister(regAddr,&data); 
	if(ret != WV_SOK)
	{
		prfBuff += sprintf(prfBuff,"HI_SYS_ReadRegister(0x%04x) erro!\r\n",regAddr); 
	}   
    prfBuff += sprintf(prfBuff,"0x%04x 0x%04x \r\n",regAddr,data); 
  
 	 
 return WV_SOK; 
}


/*******************************************************************************************************

WV_S32  HIS_REG_Init();

*******************************************************************************************************/

WV_S32  HIS_REG_Init()
{
  WV_CMD_Register("get","reg","sys reg read",HIS_REG_GetReg); 
  WV_CMD_Register("set","reg","sys reg read",HIS_REG_SetReg); 
  return WV_SOK; 
}

/*******************************************************************************************************

WV_S32  HIS_REG_DeInit();

*******************************************************************************************************/

WV_S32  HIS_REG_DeInit()
{
  return WV_SOK;
}

