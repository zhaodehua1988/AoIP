#include "wv_conf.h"



/*************************************************************************


WV_S32  WV_CONF_CpyStr(WV_S8 *pSrc,WV_S8 *pDest,WV_S32  maxlen);

*************************************************************************/

WV_S32  WV_CONF_CpyStr(WV_S8 *pSrc,WV_S8 *pDest,WV_S32  maxlen)
{
 WV_S32  len,i,j;
  if(pSrc == NULL  || pSrc == NULL )
   {
       WV_ERROR("input  erro\r\n");
       return WV_EFAIL; 
   }
   
  for(len=0;len < maxlen;len++)
   {
     if(pSrc[len] == '\0')
		{  
		   // len--;
			break;
		}
   }  
 
    if(len == maxlen)
   {
      WV_ERROR("input str len erro %d\r\n",len);
      return WV_EFAIL;
   }
   
  //
   for(i=0;i<len;i++)
   {
     
     if(pSrc[i] != ' ')
				{  
				   break;
				}
   }  
  // 
  for(j=len-1;j>i;j--)
   {
     if(pSrc[j] == ' ' || pSrc[j] == '\r' || pSrc[j] == '\n')
		 {  
		    pSrc[j] = '\0';	 
		 }
		 else
		 {
		    break;
		 }
 
   } 
   
 len = j-i;
 
 // 
     for(j=0;j<=len;j++)
   {
     
     pDest[j] = pSrc[j+i];
   }
   pDest[j] = '\0';
 // 
   
 return WV_SOK; 
}



/*************************************************************************

WV_S32  WV_CONF_Creat(WV_CONF_DEV_E * pConfDev, WV_S8 *pFileName,WV_S8 *pPathName);

*************************************************************************/


WV_S32  WV_CONF_Creat(WV_CONF_DEV_E * pConfDev, WV_S8 *pFileName,WV_S8 *pPathName)
{
  //
  WV_CHECK_RET(  WV_CONF_CpyStr(pFileName,pConfDev->file,WV_CONF_NAME_MAX_LEN));
  WV_CHECK_RET(  WV_CONF_CpyStr(pPathName,pConfDev->path,WV_CONF_NAME_MAX_LEN));
  //
  pConfDev->numItemBuf  =0;   
  pConfDev->pItemBuf = (WV_CONF_ITEM_B *)malloc( sizeof(WV_CONF_ITEM_B) * WV_CONF_ITEM_MAX_NUM +1024);
  
  pConfDev->numItem  =0;  
  pConfDev->pItem    = (WV_CONF_ITEM_E *)malloc( sizeof(WV_CONF_ITEM_E) * WV_CONF_ITEM_MAX_NUM +1024);  
  
  return WV_SOK;  
} 
/*************************************************************************


WV_S32  WV_CONF_Destry(WV_CONF_DEV_E * pConfDev);

*************************************************************************/


WV_S32  WV_CONF_Destry(WV_CONF_DEV_E * pConfDev)
{
   
   
  free(pConfDev->pItemBuf); 
  pConfDev->pItemBuf = NULL; 
  free(pConfDev->pItem);
  pConfDev->pItem = NULL;
  return WV_SOK;   
     
}

/***************************************************************************************************

WV_S32  WV_CONF_Register(WV_CONF_DEV_E *pConfDev,WV_S8 type ,WV_S8 * pName,WV_S8 *pDefault,WV_S8  *pNote);

***************************************************************************************************/

WV_S32  WV_CONF_Register(WV_CONF_DEV_E *pConfDev,WV_S8 type ,WV_S8 * pName,WV_S8 *pDefault,WV_S8  *pNote)
{
  WV_S32 num,i,flag =0;
  WV_CONF_ITEM_E   *pItem;
  WV_CONF_ITEM_B   *pItemBuf;
  num = pConfDev->numItem;
  pConfDev->pItem[num].type = type;
  pItem = &(pConfDev->pItem[num]);
   
  if(type == 0)
    {  
     WV_CHECK_RET(  WV_CONF_CpyStr( pName,pItem->name ,WV_CONF_NAME_MAX_LEN)); 
     pConfDev->numItem++;
     return num;
    }    
  if(type == 1)
   { 
     WV_CHECK_RET( WV_CONF_CpyStr(pName,pItem->name ,WV_CONF_NAME_MAX_LEN)); 
     WV_CHECK_RET( WV_CONF_CpyStr(pDefault,pItem->valueInit ,WV_CONF_VALUE_MAX_LEN)); 
     WV_CHECK_RET( WV_CONF_CpyStr(pNote,pItem->note ,WV_CONF_NOTE_MAX_LEN)); 
    // pConfDev->numItem++; 
     num = pConfDev->numItemBuf; 
     for(i=0;i< num;i++)
     { 
         pItemBuf = &(pConfDev->pItemBuf[i]); 
         if(strcmp(pItemBuf->name, pItem->name) == 0)
         {
            WV_CHECK_RET(WV_CONF_CpyStr( pItemBuf->value,pItem->value ,WV_CONF_VALUE_MAX_LEN));
            flag = 1;  
         }
     } 
     if(flag == 0)
     {
       WV_CHECK_RET(WV_CONF_CpyStr( pItem->valueInit,pItem->value ,WV_CONF_VALUE_MAX_LEN)); 
     }  
      pConfDev->numItem++;
     return num; 
   } 
 return WV_EFAIL;
}

/***************************************************************************************************

WV_S32  WV_CONF_SetDefalutAll(WV_CONF_DEV_E *pConfDev);

***************************************************************************************************/
WV_S32  WV_CONF_SetDefalutAll(WV_CONF_DEV_E *pConfDev)
{
    WV_S32 num,i;
    
     num = pConfDev->numItem; 
     
      for(i=0;i< num;i++)
     { 
          if(pConfDev->pItem[i].type == 1)
          {
            WV_CHECK_RET(WV_CONF_CpyStr(pConfDev->pItem[i].valueInit,pConfDev->pItem[i].value,WV_CONF_VALUE_MAX_LEN)); 
          }    
     }    
     
   return WV_SOK;  
}

 
/***************************************************************************************************

WV_S32  WV_CONF_Get(WV_CONF_DEV_E *pConfDev,WV_S8 *pName ,WV_S8 *pValue);

***************************************************************************************************/
WV_S32  WV_CONF_Get(WV_CONF_DEV_E *pConfDev,WV_S8 *pName ,WV_S8 *pValue)
{
   WV_S32 num,i;
  WV_CONF_ITEM_E   *pItem;
  WV_S8  nameBuf[WV_CONF_NAME_MAX_LEN];
   
  num = pConfDev->numItem; 
  
   WV_CHECK_RET(WV_CONF_CpyStr( pName,nameBuf ,WV_CONF_NAME_MAX_LEN));  
  
  
   for(i=0;i< num;i++)
     {
         pItem = &(pConfDev->pItem[i]); 
         
         if(strcmp(pItem->name,nameBuf) == 0)
         {
            WV_CHECK_RET(WV_CONF_CpyStr(pItem->value,pValue,WV_CONF_VALUE_MAX_LEN)); 
           return  i; 
         }         
     }    
  
  
 return WV_EFAIL;
}

 
/***************************************************************************************************

WV_S32  WV_CONF_Set(WV_CONF_DEV_E *pConfDev,WV_S8 *pName ,WV_S8 *pValue);

***************************************************************************************************/
WV_S32  WV_CONF_Set(WV_CONF_DEV_E *pConfDev,WV_S8 *pName ,WV_S8 *pValue)
{
   WV_S32 num,i;
  WV_CONF_ITEM_E   *pItem;
  WV_S8  nameBuf[WV_CONF_NAME_MAX_LEN]; 
  num = pConfDev->numItem; 
  WV_CHECK_RET(WV_CONF_CpyStr( pName,nameBuf,WV_CONF_NAME_MAX_LEN));  
  for(i=0;i< num;i++)
     {
         pItem = &(pConfDev->pItem[i]);  
         if(strcmp(pItem->name,nameBuf) == 0  )
         {   
            if(pItem->type == 1)
            { 
             WV_CHECK_RET(WV_CONF_CpyStr(pValue,pItem->value,WV_CONF_VALUE_MAX_LEN)); 
            }
           return i; 
         }         
     }     
 return WV_EFAIL;
}
 
 
 
 
/***************************************************************************************************

WV_S32  WV_CONF_SetDefalut(WV_CONF_DEV_E *pConfDev,WV_S8 *pName );

***************************************************************************************************/
WV_S32  WV_CONF_SetDefalut(WV_CONF_DEV_E *pConfDev,WV_S8 *pName )
{
    WV_S32 i;
    WV_S8  value[WV_CONF_VALUE_MAX_LEN]; 
    i =   WV_CONF_Get(pConfDev,pName,value);
    
    if(i == WV_EFAIL)
    {
        return WV_EFAIL; 
    }
    if( pConfDev->pItem[i].type == 1)
    {
        WV_CHECK_RET(WV_CONF_CpyStr( pConfDev->pItem[i].valueInit, pConfDev->pItem[i].value,WV_CONF_VALUE_MAX_LEN)); 
    } 
     
   return WV_SOK;  
}

/***************************************************************************************************

WV_S32  WV_CONF_Save(WV_CONF_DEV_E *pConfDev);

***************************************************************************************************/

WV_S32  WV_CONF_Save(WV_CONF_DEV_E *pConfDev)
{
  WV_S32  ret;
  WV_S32 num,i,len;
  WV_S8 nameAndPath[WV_CONF_NAME_MAX_LEN*2]; 
  
  WV_S8 lineBuf[WV_CONF_ITEM_MAX_LEN];
  
  WV_S8 *pTemp;
   WV_CONF_ITEM_E   *pItem;
  
  
  FILE *fp = NULL;
  ret = WV_FILE_Access(pConfDev->path);
  if(ret == WV_EFAIL)
  {
    mkdir(pConfDev->path,0755);
  }
  
 sprintf(nameAndPath,"%s%s",pConfDev->path,pConfDev->file);
   
  fp = fopen(nameAndPath, "w");
  if ( fp == NULL ) 
   {
  	 WV_ERROR("save() fopen erro\r\n");
  	 return WV_EFAIL; 
    }
    
   num = pConfDev->numItem;  
   
   pItem = pConfDev->pItem; 
 
  for(i=0;i< num;i++)
     {
      memset(lineBuf, 0x20, WV_CONF_ITEM_MAX_LEN);  
      //
      len = strlen(pItem[i].name);
      pTemp = lineBuf;
      memcpy(pTemp,pItem[i].name,len);
      pTemp += WV_CONF_NAME_MAX_LEN -1;
      if(pItem[i].type == 1)
		{ 
		  *(pTemp) = '=';
		  len = strlen(pItem[i].value);
		  pTemp  += 1;
		  memcpy(pTemp,pItem[i].value,len);  
		  pTemp += len; 
		} 
	    *(pTemp)      = '\n';
	    *(pTemp + 1)  = '\0';
       fprintf(fp, "%s",lineBuf);  
     } 
     fclose(fp); 
      system("sync"); 
  return WV_SOK; 
}



 
/***************************************************************************************************

WV_S32  WV_CONF_Load(WV_CONF_DEV_E *pConfDev);

***************************************************************************************************/

WV_S32  WV_CONF_Load(WV_CONF_DEV_E *pConfDev)
{
  WV_S32  ret;
  WV_S32 num,len,i;
  WV_S8 nameAndPath[WV_CONF_NAME_MAX_LEN*2];   
 
  WV_S8 lineBuf[WV_CONF_ITEM_MAX_LEN*2]; 
  WV_S8  *pTemp,type;
  WV_CONF_ITEM_B   *pItemBuf; 
  
  FILE *fp = NULL;
  ret = WV_FILE_Access(pConfDev->path);
  if(ret == WV_EFAIL)
  {
   return WV_EFAIL;
  }
  
 sprintf(nameAndPath,"%s%s",pConfDev->path,pConfDev->file);
   
  fp = fopen(nameAndPath, "r");
  if ( fp == NULL ) 
   {
  	 WV_ERROR("load() fopen erro\r\n");
  	 return WV_EFAIL; 
   } 
   num = 0;  
   pItemBuf = pConfDev->pItemBuf; 
   pConfDev->numItemBuf =0;
   while(fgets(lineBuf, WV_CONF_ITEM_MAX_LEN, fp) != NULL)
     {
		   					   
		   len = strlen(lineBuf);  
		   for(i=0;i<len;i++)
		   {
		      if(lineBuf[i] == '=')
		       {
		         lineBuf[i] = '\0';
		          break;
		       }
		   } 
		   type = 1;
		   lineBuf[i] = '\0';
		   if(i == len)
		    {
		     lineBuf[WV_CONF_NAME_MAX_LEN -1] = '\0';
		     type = 0;
		    }		 
		  
		  
		 
		 // 
		  memset(pItemBuf[num].name, 0x00,WV_CONF_NAME_MAX_LEN);   
		  WV_CHECK_RET(WV_CONF_CpyStr( lineBuf ,pItemBuf[num].name,WV_CONF_NAME_MAX_LEN));
		  
		  //  proc value 
		   memset(pItemBuf[num].value, 0x00,WV_CONF_VALUE_MAX_LEN);
	 	  if(  type == 1)
 	       { 
 	         pTemp =  lineBuf + i +1; 
 	         WV_CHECK_RET(WV_CONF_CpyStr( pTemp ,pItemBuf[num].value,WV_CONF_VALUE_MAX_LEN)); 
 	         
 	       }  
	      
	     num++;
	 	 pConfDev->numItemBuf = num;  
     } 
     fclose(fp);   
  return WV_SOK; 
}



