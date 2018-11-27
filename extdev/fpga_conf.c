#include"fpga_conf.h"
#include"his_spi.h"

/*******************************************************************
 WV_S32 FPGA_CONF_ResetA()
*******************************************************************/
WV_S32 FPGA_CONF_ResetA()
  {
	WV_S32 ret = 0;
	WV_U16 rd28;
	WV_U16 rd30;
	WV_U16 rd31;
	WV_U16 rdb0;
	WV_U16 rdb1;

	WV_S32 i,chl;

	WV_RET_ADD(	HIS_SPI_FpgaWd(0xb3,1),ret);
	WV_RET_ADD(	HIS_SPI_FpgaWd2(0x33,1),ret);
	usleep(12000);
	WV_RET_ADD(	HIS_SPI_FpgaWd(0xb3,0),ret);
	WV_RET_ADD(	HIS_SPI_FpgaWd2(0x33,0),ret);
	for(i=0;i<100;i++){	
		
		WV_RET_ADD(	HIS_SPI_FpgaRd(0xb0,&rdb0),ret);  	
		WV_RET_ADD(	HIS_SPI_FpgaRd(0xb1,&rdb1),ret); 	
		WV_RET_ADD(	HIS_SPI_FpgaRd2(0x30,&rd30),ret);
		WV_RET_ADD(	HIS_SPI_FpgaRd2(0x31,&rd31),ret);
		if(rdb0==1 && rdb1==1 && rd30==1 &&rd31==1){
			WV_printf("***************FPGA Reset OK********************\n");		
			break;	
		}
			usleep(100000);  	
	}		

    return ret;
  }

 /*******************************************************************
 WV_S32 FPGA_CONF_RqMcu();
 *******************************************************************/
 
 WV_S32 FPGA_CONF_RqMcu()
  {
    WV_S32 ret = 0; 
	WV_S32 i;
	WV_U16 data = 1;
     WV_RET_ADD(	HIS_SPI_FpgaWd(0x29,1),ret);
  
	for(i=0;i<1000;i++){

		HIS_SPI_FpgaRd(0x29,&data);

		if(data==0){
			break;	
		}		

		usleep(10000);
	}

    return ret;
  }  
     
 /*******************************************************************
 WV_S32 FPGA_CONF_ClrBuf();
 *******************************************************************/
 WV_S32 FPGA_CONF_ClrBuf()
  {
     WV_S32 ret = 0;
     WV_S32 i; 
     WV_RET_ADD(HIS_SPI_FpgaWd(0xe,1),ret);
     WV_U16 data = 1;
     
     for(i=0;i<100;i++){

	HIS_SPI_FpgaRd(0x0e,&data);

	if(data==0){
		break;	
	}		
	
	usleep(10000);
     }

	
    return ret;
  }
  



/*******************************************************************
 WV_S32 FPGA_CONF_SetSplit();
*******************************************************************/

 WV_S32 FPGA_CONF_SetSplit()
  {

	WV_S32 ret;
	FPGA_SPLIT_INFO_S info,*pInfo;
	pInfo = &info;
	pInfo->startX0 = 0;
	pInfo->endX0 = 959;
	pInfo->startY0 = 0;
	pInfo->endY0   = 1079;

	pInfo->startX1 = 960;
	pInfo->endX1 = 1919;
	pInfo->startY1 = 0;
	pInfo->endY1   = 1079;

	pInfo->startX2 = 0;
	pInfo->endX2 = 959;
	pInfo->startY2 = 1079;
	pInfo->endY2   = 2159;

	pInfo->startX3 =960;
	pInfo->endX3 = 1919;
	pInfo->startY3 = 0;
	pInfo->endY3   = 1079;

	pInfo->startX4 = 0;
	pInfo->endX4 = 959;
	pInfo->startY4 = 0;
	pInfo->endY4   = 1079;

	pInfo->startX5 = 0;
	pInfo->endX5 = 959;
	pInfo->startY5 = 0;
	pInfo->endY5   = 1079;

	pInfo->startX6 = 0;
	pInfo->endX6 = 959;
	pInfo->startY6 = 0;
	pInfo->endY6   = 1079;

	pInfo->startX7 = 0;
	pInfo->endX7 = 959;
	pInfo->startY7 = 0;
	pInfo->endY7   = 1079;



				//config chl 1
	WV_RET_ADD(	HIS_SPI_FpgaWd(0x12,pInfo->startX0),ret);//  
	WV_RET_ADD(	HIS_SPI_FpgaWd(0x13,pInfo->endX0),ret);//
	WV_RET_ADD(	HIS_SPI_FpgaWd(0x40,pInfo->startY0),ret);//  
	WV_RET_ADD(	HIS_SPI_FpgaWd(0x41,pInfo->endY0),ret);// 
	//config chl 2
	WV_RET_ADD(	HIS_SPI_FpgaWd(0x14,pInfo->startX1),ret);//  
	WV_RET_ADD(	HIS_SPI_FpgaWd(0x15,pInfo->endX1),ret);//
	WV_RET_ADD(	HIS_SPI_FpgaWd(0x42,pInfo->startY1),ret);//  
	WV_RET_ADD(	HIS_SPI_FpgaWd(0x43,pInfo->endY1),ret);// 
	//config chl 3
	WV_RET_ADD(	HIS_SPI_FpgaWd(0x16,pInfo->startX2),ret);//  
	WV_RET_ADD(	HIS_SPI_FpgaWd(0x17,pInfo->endX2),ret);//
	WV_RET_ADD(	HIS_SPI_FpgaWd(0x44,pInfo->startY2),ret);//  
	WV_RET_ADD(	HIS_SPI_FpgaWd(0x45,pInfo->endY2),ret);// 
	//config chl 4
	WV_RET_ADD(	HIS_SPI_FpgaWd(0x18,pInfo->startX3),ret);//  
	WV_RET_ADD(	HIS_SPI_FpgaWd(0x19,pInfo->endX3),ret);//  
	WV_RET_ADD(	HIS_SPI_FpgaWd(0x46,pInfo->startY3),ret);//  
	WV_RET_ADD(	HIS_SPI_FpgaWd(0x47,pInfo->endY3),ret);// 
	//config chl 5	
	WV_RET_ADD(	HIS_SPI_FpgaWd(0x1a,pInfo->startX4),ret);//  
	WV_RET_ADD(	HIS_SPI_FpgaWd(0x1b,pInfo->endX4),ret);//  
	WV_RET_ADD(	HIS_SPI_FpgaWd(0x48,pInfo->startY4),ret);//  
	WV_RET_ADD(	HIS_SPI_FpgaWd(0x49,pInfo->endY4),ret);// 
	//config chl 6
	WV_RET_ADD(	HIS_SPI_FpgaWd(0x1c,pInfo->startX5),ret);//  
	WV_RET_ADD(	HIS_SPI_FpgaWd(0x1d,pInfo->endX5),ret);//  
	WV_RET_ADD(	HIS_SPI_FpgaWd(0x4a,pInfo->startY5),ret);//  
	WV_RET_ADD(	HIS_SPI_FpgaWd(0x4b,pInfo->endY5),ret);// 		
	//start split
	WV_RET_ADD(	HIS_SPI_FpgaWd(0x0c,0x3f),ret);//
	WV_RET_ADD(	HIS_SPI_FpgaWd(0x25,0x0),ret);//		

    return WV_SOK;
}  

/*******************************************************************
 WV_S32 FPGA_CONF_SetResolution();
*******************************************************************/
 
 WV_S32 FPGA_CONF_SetResolution()
  {
		WV_S32 ret = 0;  
		WV_S32 chl;
		WV_U16 ena;
		float fqM;
		WV_U32 fqL;
		WV_U32 fqH; 

		FPGA_RESOLUTION_INFO_S info,*pInfo;

		info.frameRate = 60;
		info.totalPix = 2200;
		info.totalLine = 1125;
		info.hsyncWidth = 44;
		info.vsyncWidth = 5;
		info.actHstart = 192;
		info.actHsize = 1920;
		info.actVstart = 41;
		info.actVsize = 1080;

		pInfo = &info;
	
		fqM =  pInfo->totalPix * pInfo->totalLine * pInfo->frameRate;  
		fqM =   fqM/1000000;
		fqH =(WV_U32 )fqM;
		fqM=(fqM -fqH);
		fqL = fqM*1024*64;
		fqL &= 0XFFFF; 


		WV_RET_ADD(	HIS_SPI_FpgaWd2(0xf,0),ret);// 				
		WV_RET_ADD(	HIS_SPI_FpgaWd2(0x12,fqH),ret);// 	
		WV_RET_ADD(	HIS_SPI_FpgaWd2(0x11,fqL),ret);// 	
		WV_RET_ADD(	HIS_SPI_FpgaWd2(0x16,pInfo->totalPix),ret);// 
		WV_RET_ADD(	HIS_SPI_FpgaWd2(0x17,pInfo->totalLine),ret);// 
		WV_RET_ADD(	HIS_SPI_FpgaWd2(0x18,pInfo->hsyncWidth),ret);// 
		WV_RET_ADD(	HIS_SPI_FpgaWd2(0x19,pInfo->vsyncWidth),ret);// 
		WV_RET_ADD(	HIS_SPI_FpgaWd2(0x1a,pInfo->actHstart),ret);// 
		WV_RET_ADD(	HIS_SPI_FpgaWd2(0x1b,pInfo->actHsize),ret);// 
		WV_RET_ADD(	HIS_SPI_FpgaWd2(0x1c,pInfo->actVstart),ret);// 
		WV_RET_ADD(	HIS_SPI_FpgaWd2(0x1d,pInfo->actVsize),ret);//  
		WV_RET_ADD(	HIS_SPI_FpgaWd2(0x13,1),ret);//
		WV_RET_ADD(	HIS_SPI_FpgaWd2(0x13,0),ret);//	
		//set fpga 1

		WV_RET_ADD(	HIS_SPI_FpgaWd(0x8f,0),ret);// 
		WV_RET_ADD(	HIS_SPI_FpgaWd(0x92,fqH),ret);// 
		WV_RET_ADD(	HIS_SPI_FpgaWd(0x91,fqL),ret);// 	
		WV_RET_ADD(	HIS_SPI_FpgaWd(0x96,pInfo->totalPix),ret);// 
		WV_RET_ADD(	HIS_SPI_FpgaWd(0x97,pInfo->totalLine),ret);// 
		WV_RET_ADD(	HIS_SPI_FpgaWd(0x98,pInfo->hsyncWidth),ret);// 
		WV_RET_ADD(	HIS_SPI_FpgaWd(0x99,pInfo->vsyncWidth),ret);// 
		WV_RET_ADD(	HIS_SPI_FpgaWd(0x9a,pInfo->actHstart),ret);// 
		WV_RET_ADD(	HIS_SPI_FpgaWd(0x9b,pInfo->actHsize),ret);// 
		WV_RET_ADD(	HIS_SPI_FpgaWd(0x9c,pInfo->actVstart),ret);// 
		WV_RET_ADD(	HIS_SPI_FpgaWd(0x9d,pInfo->actVsize),ret);//  
		WV_RET_ADD(	HIS_SPI_FpgaWd(0x93,1),ret);// 
		WV_RET_ADD(	HIS_SPI_FpgaWd(0x93,0),ret);// 

		//set output	
		//HIS_SPI_FpgaWd(0x8f,0x33);
		//HIS_SPI_FpgaWd2(0xf,0xff);
		
    return WV_SOK;
  }  
 

/*******************************************************************
 WV_S32 FPGA_Init();
*******************************************************************/
WV_S32 FPGA_Init()
{
	FPGA_CONF_ResetA();
    //设置分辨率
    FPGA_CONF_SetResolution();
     //关闭融合带
    HIS_SPI_FpgaWd(0x26,0x3);
     //配置分割参数
    FPGA_CONF_SetSplit();
    
    HIS_FB_ClrFpga();
    FPGA_CONF_ClrBuf();
    HIS_SPI_FpgaWd(0x9,0);// stop win number
	HIS_SPI_FpgaWd(0xb,0);
	HIS_SPI_FpgaWd(0x8f,0x33);
	HIS_SPI_FpgaWd2(0xf,0xff);
    return WV_SOK;
}

WV_S32 FPGA_DeInit()
{
	return WV_SOK;
}