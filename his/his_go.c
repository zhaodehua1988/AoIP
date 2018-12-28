#include "his_go.h"
#include "hi_common.h"
#define HIS_HIGO_PIXELFORMAT  HIGO_PF_8888

#define HIS_HIGO_LAYER_W 3840
#define HIS_HIGO_LAYER_H 2160
//static int indexgif;

/***************************************************************

WV_S32 HIS_GO_SetColorKey(WV_U32 ena,WV_U32 colorKey);

***************************************************************/
WV_S32 HIS_GO_SetColorKey(WV_U32 ena,WV_U32 colorKey)
{

	HIS_FB_SetColorKey(ena,colorKey);	
	return  WV_SOK;
 
}

/***************************************************************

WV_S32 HIS_GO_Init();

***************************************************************/

WV_S32 HIS_GO_Init()
{
	//indexgif = 0;
   WV_CHECK_RET(HI_GO_Init());
   return  WV_SOK;
 
}


/***************************************************************

WV_S32 HIS_GO_DeInit();

***************************************************************/

WV_S32 HIS_GO_DeInit()
{ 

   WV_CHECK_RET(HI_GO_Deinit());
   return  WV_SOK;
 
}

/***************************************************************

WV_S32 HIS_GO_GetConf(HIS_DEV_HIGO_E * pDev);

***************************************************************/

WV_S32 HIS_GO_GetConf(HIS_DEV_HIGO_E * pDev  ,WV_S32 id)
{
  
  pDev->layerId =  id;
  pDev->bkColor = 0;  
  pDev->layerX  =  0;
  pDev->layerY  =  0; 
  pDev->layerW  =  HIS_HIGO_LAYER_W;
  pDev->layerH  =  HIS_HIGO_LAYER_H;
  
  WV_S32 i; 
  for(i=0;i<HIS_HIGO_GODEC_NUM;i++)
  {
	   pDev->imgDev[i].imgStamp = 0;
	   pDev->imgDev[i].ena =  0;
	   pDev->imgDev[i].imgIndex =  0;
	   pDev->imgDev[i].imgIndexCur =  0;

	   pDev->imgDev[i].colorKeyEna = 0;
	   pDev->imgDev[i].imgClosed = 1;
	   
	   pDev->imgDev[i].disX = 0;
	   pDev->imgDev[i].disY = 0;
	   pDev->imgDev[i].disW = 800;
	   pDev->imgDev[i].disH = 600;
	   pDev->imgDev[i].bkDisX = 0;
	   pDev->imgDev[i].bkDisY = 0;
	   pDev->imgDev[i].bkDisW = 800;
	   pDev->imgDev[i].bkDisH = 600; 
	}

 return WV_SOK;
}

/***************************************************************

WV_S32 HIS_GO_Open(HIS_DEV_HIGO_E * pDev);

***************************************************************/

WV_S32 HIS_GO_Open(HIS_DEV_HIGO_E * pDev)
{ 
     
     // great surf for all img 
    WV_CHECK_RET( HI_GO_CreateSurface(pDev->layerW,pDev->layerH,HIS_HIGO_PIXELFORMAT,&(pDev->layerSurfHndl)) ); 
    WV_CHECK_RET( HI_GO_FillRect(pDev->layerSurfHndl, NULL, pDev->bkColor, HIGO_COMPOPT_NONE) );

  return  WV_SOK;
 
}



/***************************************************************

WV_S32 HIS_GO_Close(HIS_DEV_HIGO_E * pDev);

***************************************************************/

WV_S32 HIS_GO_Close(HIS_DEV_HIGO_E * pDev)
{
  WV_S32 i;
  WV_CHECK_RET( HI_GO_FreeSurface(pDev->layerSurfHndl));  
 return  WV_SOK;
 
}

/***************************************************************

WV_S32 HIS_GO_SetPos(HIS_DEV_HIGO_E * pDev);

***************************************************************/

WV_S32 HIS_GO_SetPos(HIS_DEV_HIGO_E * pDev, WV_S32 x,WV_S32 y)
{
 
   pDev->layerX  = x;
   pDev->layerY  = y; 
   return  WV_SOK;
}

/***************************************************************

WV_S32 HIS_GO_SetSize(HIS_DEV_HIGO_E * pDev);

***************************************************************/

WV_S32 HIS_GO_SetSize(HIS_DEV_HIGO_E * pDev, WV_S32 w,WV_S32 h)
{
 
    pDev->layerW = w;
    pDev->layerH = h;  
   return  WV_SOK;
 
}


/***************************************************************

WV_S32 HIS_GODEC_Open(HIS_DEV_GODEC_E * pDev);

***************************************************************/
WV_S32 HIS_GODEC_Open(HIS_DEV_GODEC_E * pDev,WV_S8 *pFileName,HI_HANDLE  dstSurfHndl)
{

	//printf("<-----------------HIS_GODEC_Open --------------->\n");
    WV_S32 ret;

	while(pDev->imgClosed != 1)
	{
		usleep(30000);
	}
    
    sprintf(pDev->decFileNameCur,"%s",pFileName);
	//WV_printf("gif path[%s]\n",pDev->decFileNameCur);
    ret = WV_FILE_Access(pDev->decFileNameCur);
	if(ret == WV_EFAIL)
	{
		return WV_EFAIL;
	}
	
	pDev->dstSurfHndl = dstSurfHndl;  
    pDev  -> imgStamp =0;
    pDev -> imgIndex =0;
    pDev -> imgIndexCur=-1;
    pDev -> ena =1;

	return WV_SOK;
} 

/***************************************************************

WV_S32 HIS_GODEC_Close(HIS_DEV_GODEC_E * pDev);

***************************************************************/
WV_S32 HIS_GODEC_Close(HIS_DEV_GODEC_E * pDev )
{


    pDev -> ena =0; 
    return WV_SOK;
} 

/***************************************************************

WV_S32 HIS_GODEC_render(HIS_DEV_HIDEC_E * pDev);

***************************************************************/
WV_S32 HIS_GODEC_Render(HIS_DEV_GODEC_E * pDev)
{
    HIGO_DEC_ATTR_S SrcDesc; 
    HI_RECT Rect, DstRect, LastRect,BKRect;
    HI_U32  BKColor,index; 
    HIGO_DEC_IMGINFO_S 		*pImgInfo,*pLastImgInfo;
    HI_S32   disposal ,lastDisposal; 
    HIGO_BLTOPT_S BltOpt = {0};
    HIGO_BLTOPT_S BltOptKey = {0};
   // HI_HANDLE ImgSurface;
    HI_U32  curStamp;
     HI_S32 ret;
	//printf("pDev->ena = %d \n",pDev->ena);  

     // judge  ena
     if(pDev-> ena  == 0)
      {
			if(pDev-> imgStamp != 0)
			{
				pDev-> imgStamp = 0;
				LastRect.x = pDev->disX;
				LastRect.y = pDev->disY;
				LastRect.w = pDev->disW;
				LastRect.h = pDev->disH;

				if(pDev-> decHndl != 0 )
				{
					WV_CHECK_RET( HI_GO_DestroyDecoder(pDev-> decHndl));
					pDev-> decHndl = 0;
				}


				if(pDev-> bkSurfHndl != 0)  
				{  
					WV_CHECK_RET( HI_GO_FreeSurface(pDev->bkSurfHndl)); 
					pDev->bkSurfHndl = 0; 
				}
				//printf("***HI_GO_FillRect(pDev -> dstSurfHndl, &LastRect, 0X0, HIGO_COMPOPT_NONE)x[%d]y[%d]w[%d]h[%d] \n",LastRect.x,LastRect.y,LastRect.w,LastRect.h);
				WV_CHECK_RET( HI_GO_FillRect(pDev -> dstSurfHndl, &LastRect, 0X0, HIGO_COMPOPT_NONE) );  
				return 1; 			
			}
        
			if(pDev	-> decHndl != 0 )
			 {
			   WV_CHECK_RET( HI_GO_DestroyDecoder(pDev-> decHndl));
			   pDev-> decHndl = 0;
			 }
		
		
			if(pDev-> bkSurfHndl != 0)  
			{  
				WV_CHECK_RET( HI_GO_FreeSurface(pDev->bkSurfHndl)); 
				pDev->bkSurfHndl = 0; 
			 }
			 pDev->imgClosed =1; 	
			return 0;
      }
	  else if(pDev->ena == 1)
	  {

			SrcDesc.SrcType = HIGO_DEC_SRCTYPE_FILE;
			SrcDesc.SrcInfo.pFileName =  pDev->decFileNameCur;
			sprintf(pDev-> decFileName,"%s",pDev->decFileNameCur);
			if(pDev->decHndl== 0)
			{		
				WV_CHECK_RET( HI_GO_CreateDecoder(&SrcDesc, &(pDev->decHndl)) );
				WV_CHECK_RET( HI_GO_DecCommInfo(pDev->decHndl, &(pDev->decInfo)));		
			}			
			pDev->imgClosed = 0;
	
	  }
    
     //judge stamp  
      HI_SYS_GetTimeStampMs  ( &curStamp); 
      if(pDev-> imgStamp == 0)
      {
         pDev-> imgStamp =  curStamp;
      }
      
    
      if(curStamp < pDev-> imgStamp )
      {
        return WV_SOK;
      }
      
  
    /***************  creat dec handl*****************/   
    //get curent  imgage INFo 
    pImgInfo 		= &(pDev->imgInfoA );
    WV_CHECK_RET( HI_GO_DecImgInfo(pDev->decHndl,pDev->imgIndex, pImgInfo));
   // pos change 
    if(pDev->disX != pDev->bkDisX || pDev->disY != pDev->bkDisY ||  pDev->disW != pDev->bkDisW || pDev->disH != pDev->bkDisH)
    {
     // WV_printf("%s pos change from [%d:%d:%d:%d] to [%d:%d:%d:%d]\r\n",\
       pDev-> decFileName,pDev->disX ,pDev->disY ,pDev->disW ,pDev->disH ,pDev->bkDisX ,pDev->bkDisY,pDev->bkDisW ,pDev->bkDisH  );
		LastRect.x = pDev->disX;
		LastRect.y = pDev->disY;
		LastRect.w = pDev->disW;
		LastRect.h = pDev->disH;
		
		pDev->disX=pDev->bkDisX;
		pDev->disY=pDev->bkDisY;
	    pDev->disW=pDev->bkDisW;
	    pDev->disH=pDev->bkDisH;
	
		WV_CHECK_RET( HI_GO_FillRect(pDev -> dstSurfHndl, &LastRect, 0X0, HIGO_COMPOPT_NONE) );  

	  // return WV_SOK; 
    }
    
    DstRect.x = pDev->disX;
    DstRect.y = pDev->disY;
    DstRect.w = pDev->disW;
    DstRect.h = pDev->disH;  
    
     
    if(pDev-> bkSurfHndl != 0)  
    {  
    	WV_CHECK_RET( HI_GO_FreeSurface(pDev->bkSurfHndl)); 
    	pDev->bkSurfHndl = 0; 
     } 	
     
    //fill the new surface
	if(	pDev->imgIndexCur ==  pDev->imgIndex)
	{
		return 0;
	}
    
	WV_CHECK_RET( HI_GO_DecImgData(pDev->decHndl,pDev->imgIndex, NULL, &(pDev->bkSurfHndl)));    // the imgatt must be null
	pDev->imgIndexCur = pDev->imgIndex;
    //printf("pDev->imgIndex [%d]\n",pDev->imgIndex );
    BltOpt.EnableScale = HI_TRUE;
  	// BltOpt.EnablePixelAlpha = HI_TRUE;
    //BltOpt.EnableGlobalAlpha = HI_TRUE;
    //BltOpt.PixelAlphaComp = HIGO_COMPOPT_SRC;
    //printf("higo_render x=%d,y=%d,w=%d,h=%d\n",DstRect.x,DstRect.y,DstRect.w,DstRect.h);
    WV_ASSERT_RET( HI_GO_Blit(pDev -> bkSurfHndl, HI_NULL, pDev -> dstSurfHndl, &DstRect, &BltOpt ));  
   
    if(pImgInfo->DelayTime == 0)
     {
		pDev -> imgDelay = 30; 
     }
     else
     {
		pDev -> imgDelay = pImgInfo->DelayTime * 10;
     }
   
   pDev -> imgStamp = curStamp + pDev ->imgDelay;  
   WV_CHECK_RET(HI_GO_FreeSurface(pDev->bkSurfHndl)); 
   pDev->bkSurfHndl = 0; 
  
   pDev->imgIndex ++; 
   if(pDev->imgIndex >= pDev->decInfo.Count)
   {
		//printf("pDev->imgIndex [%d]\n",pDev->imgIndex );
		pDev->imgIndex = 0;
		
   }
  // printf("render: %s  [%d]\r\n",pDev-> decFileNameCur,curStamp);

   return  1;
 
}


/***************************************************************

WV_S32 HIS_GO_RefreshLayer(HIS_DEV_HIGO_E * pDev);

***************************************************************/
WV_S32 HIS_GO_RefreshLayer(HIS_DEV_HIGO_E * pDev)
{
 HI_HANDLE screenSurf[2];
 WV_S32 ret;
 HIGO_BLTOPT_S BltOpt = {0};

   ret = HIS_FB_SurfGet(screenSurf);
   if(ret == WV_EFAIL)
   {
     return WV_EFAIL;
   }
    HI_RECT Rect;

    Rect.x =0;
    Rect.y = 0;
    Rect.w = HIS_FB_VITURE_SCEEN_W;//1920;
    Rect.h = HIS_FB_VITURE_SCEEN_H;//1080;
    WV_CHECK_RET( HI_GO_Blit(pDev->layerSurfHndl,&Rect,screenSurf[0], NULL, &BltOpt)); 
	
    WV_CHECK_RET( HIS_FB_SurfFresh());
    
   return  WV_SOK;
}

/***************************************************************

WV_S32 HIS_GO_PrintLayer(HIS_DEV_HIGO_E * pDev);

***************************************************************/

WV_S32 HIS_GO_PrintLayer(HIS_DEV_HIGO_E * pDev, WV_S8 * pBuf)
{ 
 WV_S32 len;
 
  len = 0;
  len +=sprintf(pBuf+len ,"layerId      = %d \r\n", pDev -> layerId); 
  len +=sprintf(pBuf+len ,"surfHndl     = %d \r\n", pDev -> layerSurfHndl);
  len +=sprintf(pBuf+len ,"layerX       = %d \r\n", pDev -> layerX);
  len +=sprintf(pBuf+len ,"layerY       = %d \r\n", pDev -> layerY);
  len +=sprintf(pBuf+len ,"layerW       = %d \r\n", pDev -> layerW);
  len +=sprintf(pBuf+len ,"layerH       = %d \r\n", pDev -> layerH); 
  len +=sprintf(pBuf+len ,"bkColor      = 0x%08x \r\n", pDev -> bkColor);
  
  return len;
 }

/***************************************************************

WV_S32 HIS_GO_Open(HIS_DEV_GODEC_E * pDev);

***************************************************************/
WV_S32 HIS_GO_PrintDec(HIS_DEV_GODEC_E * pDev, WV_S8 * pBuf)
{ 
  WV_S32 len,i;
  HIGO_DEC_IMGINFO_S 		*pImgInfo;
  len = 0;
   
	  len +=sprintf(pBuf+len ,"DEC INFO:\r\n");
	  len +=sprintf(pBuf+len ,"ena              = %d\r\n",pDev ->ena);
	  len +=sprintf(pBuf+len ,"fielName         = %s\r\n",pDev ->decFileName);
	  len +=sprintf(pBuf+len ,"decHndl          = %d\r\n",pDev ->decHndl);
	  len +=sprintf(pBuf+len ,"dstSurfHndl      = %d\r\n",pDev ->dstSurfHndl);
	  len +=sprintf(pBuf+len ,"bkSurfHndl       = %d\r\n",pDev ->bkSurfHndl);
	 
	  len +=sprintf(pBuf+len ,"decNum           = %d\r\n",pDev ->decInfo.Count); 
	  len +=sprintf(pBuf+len ,"decType          = %d\r\n",pDev ->decInfo.ImgType);
	  len +=sprintf(pBuf+len ,"decWidth         = %d\r\n",pDev ->decInfo.ScrWidth);
	  len +=sprintf(pBuf+len ,"decHeight        = %d\r\n",pDev ->decInfo.ScrHeight);
	  len +=sprintf(pBuf+len ,"decBGColorEna    = %d\r\n",pDev ->decInfo.IsHaveBGColor);
	  len +=sprintf(pBuf+len ,"decBGColor       = 0x%08x\r\n",pDev ->decInfo.BGColor);
	  if((pDev ->imgIndex & 1) == 1)
	  {
	  	pImgInfo = &(pDev ->imgInfoA);
	  }
	  else
	  {
	   pImgInfo = &(pDev ->imgInfoB);
	  }
	  	  
	  len +=sprintf(pBuf+len ,"imgIndex         = %d\r\n",pDev ->imgIndex);
	  len +=sprintf(pBuf+len ,"imgDelay         = %d\r\n",pDev ->imgDelay);
	  len +=sprintf(pBuf+len ,"imgStamp         = %d\r\n",pDev ->imgStamp);
	  len +=sprintf(pBuf+len ,"OffSetX          = %d\r\n",pImgInfo ->OffSetX);
	  len +=sprintf(pBuf+len ,"OffSetY          = %d\r\n",pImgInfo ->OffSetY);
	  len +=sprintf(pBuf+len ,"Width            = %d\r\n",pImgInfo ->Width);
	  len +=sprintf(pBuf+len ,"Height           = %d\r\n",pImgInfo ->Height);
	  len +=sprintf(pBuf+len ,"Alpha            = %d\r\n",pImgInfo ->Alpha);
	  len +=sprintf(pBuf+len ,"IsHaveKey        = %d\r\n",pImgInfo ->IsHaveKey);
	  len +=sprintf(pBuf+len ,"Key              = %d\r\n",pImgInfo ->Key);
	  len +=sprintf(pBuf+len ,"Format           = %d\r\n",pImgInfo ->Format);
	  len +=sprintf(pBuf+len ,"DelayTime        = %d\r\n",pImgInfo ->DelayTime);
	  len +=sprintf(pBuf+len ,"Disposal         = %d\r\n",pImgInfo ->DisposalMethod); 
	  
	  HIGO_DEC_IMGINFO_S 		imgInfo;
	  for(i=0;i< pDev ->decInfo.Count;i++)
	  {
	  
	   WV_CHECK_RET( HI_GO_DecImgInfo(pDev->decHndl,i, &imgInfo));  
	  len +=sprintf(pBuf+len ,"[%2d] [%3d.%3d.%3d.%3d][%3d][%3d:%3d][%3d][%3d][%3d]\r\n",\
	                           i,imgInfo.OffSetX,imgInfo.OffSetY,imgInfo.Width,imgInfo.Height,imgInfo.Alpha,imgInfo.IsHaveKey,imgInfo.Key,\
	                           imgInfo.Format,imgInfo.DelayTime,imgInfo.DisposalMethod);    
	  }
 
  return len;

}