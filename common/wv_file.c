#include"wv_file.h"

#define  WV_FILE_BUF_MAXLEN    1024*500

/****************************************************************

WV_S32   WV_FILE_Access(WV_S8 *filename)

****************************************************************/
WV_S32   WV_FILE_Access(WV_S8 *filename)
{  
   WV_S32 ret;
   ret =access(filename, F_OK); 
   if (ret != 0 )
     {
      //WV_ERROR(" %s DOESN'T EXISIT\r\n",filename); 
       return WV_EFAIL;
     }    
   return WV_SOK;
}
/****************************************************************

WV_S32   WV_FILE_OpenDir(WV_S8 *filename)

****************************************************************/
WV_S32   WV_FILE_OpenDir(WV_S8 *filename)
{  
   DIR *pDir;
   pDir =opendir(filename); 
   if (pDir == NULL )
     {
       //WV_ERROR(" %s DOESN'T EXISIT\r\n",filename); 
       return WV_EFAIL;
     }    
   return WV_SOK;
}
/****************************************************************

WV_S32   WV_FILE_GetLen((WV_S8 *filename,WV_U32 *pLen)

****************************************************************/
WV_S32   WV_FILE_GetLen(WV_S8 *filenameAll,WV_S32 * pLen)
{  
   WV_S32 ret;
   struct stat statBuf; 
   ret = stat(filenameAll,&statBuf);
      if(ret != 0)
		  { 
		  	WV_ERROR(" %s GET LEN ERROR !![ %d] \r\n",filenameAll,errno); 
		    return WV_EFAIL; 
		  }  
     *pLen = statBuf.st_size;	 	 
	 if( S_IFREG == (statBuf.st_mode  & S_IFREG) )	      
		{		   
		  return WV_SOK;
		} 
    return WV_EFAIL;	   
}


/****************************************************************

WV_S32   WV_FILE_GetAvail((WV_S8 *filename,WV_S32 *pLenM)

****************************************************************/
WV_S32   WV_FILE_GetAvail(WV_S8 *filenameAll,WV_S32 * pLenM)
{  
     
    struct statfs sfs; 
    WV_S32 ret; 
	ret =  statfs(filenameAll, &sfs);
	if(ret != 0)
	{
		WV_ERROR(" %s get Stat ERROR !! [%d]\r\n",filenameAll,errno); 
	  return  WV_EFAIL;
	}
	*pLenM = (sfs.f_bsize /1024 ) * sfs.f_bavail/1024; 
	
	//printf(" %d %d  %d \r\n",sfs.f_bsize,sfs.f_bavail ,*pLenM);
    return WV_SOK;	   
}
 
 

/****************************************************************

WV_S32   WV_FILE_Remove((WV_S8 *filename,WV_S32 *pLen)

****************************************************************/
WV_S32   WV_FILE_Remove(WV_S8 *filenameAll)
{  
   WV_S32 ret; 
   ret = remove(filenameAll );
	      if(ret != 0)
	      { 
	      	WV_ERROR(" %s remove ERROR !! [%d]\r\n",filenameAll,errno); 
	      return WV_EFAIL; 
	      } 
   return WV_SOK;
}



/****************************************************************

WV_S32    WV_FILE_Cpy(WV_S8 *filenameScr,WV_S8 *filenameDst)

****************************************************************/
WV_S32   WV_FILE_Cpy(WV_S8 *filenameScr,WV_S8 *filenameDst)
{  

	WV_S32 ret;
	WV_S32 readlen,writeLen,fileLen,rLen,wLen,lenAvailM;
	FILE    *fpScr = NULL ,*fpDst = NULL; 
	WV_S8  buf[WV_FILE_BUF_MAXLEN]; 
	ret = WV_FILE_Access(filenameScr);
	if(ret != 0)
		{ 
		WV_ERROR("  %s not exist!! \r\n",filenameScr); 
		return WV_EFAIL;
		}
	fileLen = 0; 
	ret = WV_FILE_GetLen(filenameScr,&fileLen);
	if(ret != WV_SOK)
		{  
		WV_ERROR("  %s not _IFREG!! \r\n",filenameScr);  
		return WV_EFAIL;
		}
	if(fileLen <= 0)
		{
		WV_ERROR("  file %s len is erro [%d]!!\r\n",filenameScr,fileLen); 
		return WV_EFAIL;  
		}	

	ret = WV_FILE_GetAvail(filenameScr,&lenAvailM);		
		if(ret != 0)
		{  
		return WV_EFAIL;
		} 
	if(fileLen/1000000 >= lenAvailM)
	{
		WV_ERROR(" there is not enough disk space  %d[%d] \r\n",fileLen/1000000,lenAvailM); 
		return WV_EFAIL; 
	}
	// 
	fpScr = fopen(filenameScr, "rb");
	if(fpScr == NULL)
		{ 
		WV_ERROR(" open %s rb ERROR!! [%d]\r\n",filenameScr,errno); 
		return WV_EFAIL; 
		} 

	//printf("%s open\r\n",filenameScr);	

	ret = WV_FILE_Access(filenameDst);
	if(ret == 0)
		{  
		WV_FILE_Remove(filenameDst);
		}       

	fpDst = fopen(filenameDst, "wb");
	if(fpDst == NULL)
		{ 
		WV_ERROR(" open  %s wb ERROR!! [%d]\r\n",filenameDst,errno); 
		fclose(fpScr);
		return WV_EFAIL; 
		} 

	readlen =0;
	writeLen = 0;
	while(writeLen < fileLen)

	{     
		rLen = fread(buf, sizeof(WV_S8),WV_FILE_BUF_MAXLEN ,fpScr);
		if(rLen == 0)
			{
			WV_ERROR(" fread  %s  ERROR!! [%d]\r\n",filenameScr,errno);  
			break;  
			}
		readlen +=rLen;
		wLen = fwrite(buf, sizeof(WV_S8),rLen,fpDst);
		if(wLen != rLen)
			{
			WV_ERROR(" fwrite  %s  ERROR!! [%d]\r\n",filenameDst,errno);  
			break;  
			} 
		
		writeLen += wLen;   
	}
	fclose(fpDst);
	fclose(fpScr);
	sync();
	//
	struct stat statBuf; 
	ret = stat(filenameScr,&statBuf);
	if(ret != 0)
		{ 
		WV_ERROR(" %s GET mode !![ %d] \r\n",filenameScr,errno); 
		return WV_EFAIL; 
		}   
	ret = chmod(filenameDst,statBuf.st_mode);
	if(ret != 0)
		{ 
		WV_ERROR(" %s SET mode  !![ %d] \r\n",filenameDst,errno); 
		return WV_EFAIL; 
		}
	//printf("<<<<%s read %d[%d]  %s write %d>>>\r\n",filenameScr,readlen,fileLen,filenameDst,writeLen); 
	if(writeLen == fileLen)  return WV_SOK;
	return WV_EFAIL;
}

/****************************************************************

WV_S32   WV_FILE_CpyWithOutHead(WV_U32 headLen,WV_S8 *filenameScr,WV_S8 *filenameDst)

****************************************************************/
WV_S32   WV_FILE_CpyWithOutHead(WV_U32 headLen,WV_S8 *filenameScr,WV_S8 *filenameDst)
{  
	WV_S32 firstData=0;
	WV_S32 ret;
	WV_S32 readlen,writeLen,fileLen,rLen,wLen,lenAvailM;
	FILE    *fpScr = NULL ,*fpDst = NULL; 
	WV_S8  buf[WV_FILE_BUF_MAXLEN]; 
	ret = WV_FILE_Access(filenameScr);
	if(ret != 0)
		{ 
		WV_ERROR("  %s not exist!! \r\n",filenameScr); 
		return WV_EFAIL;
		}
	fileLen = 0; 
	ret = WV_FILE_GetLen(filenameScr,&fileLen);
	if(ret != WV_SOK)
		{  
		WV_ERROR("  %s not _IFREG!! \r\n",filenameScr);  
		return WV_EFAIL;
		}
	if(fileLen <= 0)
		{
		WV_ERROR("  file %s len is erro [%d]!!\r\n",filenameScr,fileLen); 
		return WV_EFAIL;  
		}	

	ret = WV_FILE_GetAvail(filenameScr,&lenAvailM);		
		if(ret != 0)
		{  
		return WV_EFAIL;
		} 
	if(fileLen/1000000 >= lenAvailM-256)//保留256兆的空间以备其他地方使用
	{
		WV_ERROR(" there is not enough disk space  %d[%d] \r\n",fileLen/1000000,lenAvailM); 
		return WV_EFAIL; 
	}

	if(fileLen < headLen )
	{
		WV_ERROR(" headLen > fileLen error \n"); 
		return WV_EFAIL; 	
	}
	// 
	fpScr = fopen(filenameScr, "rb");
	if(fpScr == NULL)
		{ 
		WV_ERROR(" open %s rb ERROR!! [%d]\r\n",filenameScr,errno); 
		return WV_EFAIL; 
		} 

	//printf("%s open\r\n",filenameScr);	

	ret = WV_FILE_Access(filenameDst);
	if(ret == 0)
		{  
		WV_FILE_Remove(filenameDst);
		}       

	fpDst = fopen(filenameDst, "wb");
	if(fpDst == NULL)
		{ 
		WV_ERROR(" open  %s wb ERROR!! [%d]\r\n",filenameDst,errno); 
		fclose(fpScr);
		return WV_EFAIL; 
		} 

	readlen =0;
	writeLen = 0;

	while(writeLen < (fileLen-headLen))

	{     
		rLen = fread(buf, sizeof(WV_S8),WV_FILE_BUF_MAXLEN ,fpScr);
		if(rLen == 0)
			{
			WV_ERROR(" fread  %s  ERROR!! [%d]\r\n",filenameScr,errno);  
			break;  
			}
		
		readlen +=rLen;
		if(firstData == 0 )
		{
			wLen = fwrite(&buf[headLen], sizeof(WV_S8),rLen-headLen,fpDst);
			if(wLen != (rLen - headLen))
				{
				WV_ERROR(" fwrite  %s  ERROR!! [%d]\r\n",filenameDst,errno);  
				break;  
				} 
		
			writeLen += wLen;
		firstData = 1;
		}else{
			wLen = fwrite(buf, sizeof(WV_S8),rLen,fpDst);
			if(wLen != rLen)
				{
				WV_ERROR(" fwrite  %s  ERROR!! [%d]\r\n",filenameDst,errno);  
				break;  
				} 
		
			writeLen += wLen;
		}   
	}
	fclose(fpDst);
	fclose(fpScr);
	sync();
	//
	struct stat statBuf; 
	ret = stat(filenameScr,&statBuf);
	if(ret != 0)
		{ 
		WV_ERROR(" %s GET mode !![ %d] \r\n",filenameScr,errno); 
		return WV_EFAIL; 
		}   
	ret = chmod(filenameDst,statBuf.st_mode);
	if(ret != 0)
		{ 
		WV_ERROR(" %s SET mode  !![ %d] \r\n",filenameDst,errno); 
		return WV_EFAIL; 
		}
	//printf("<<<<%s read %d[%d]  %s write %d>>>\r\n",filenameScr,readlen,fileLen,filenameDst,writeLen); 
	if(writeLen == (fileLen-headLen))  return WV_SOK;
	return WV_EFAIL;
}
/*
打印版本信息
*/
WV_S32 WV_FILE_PRINTF_VERSION()
{
    char filename[] = "./env/conf64.ini";
        FILE *fp;
        int whichLine = 2;
        int CurrentIndex = 0;
        char StrLine[128];
        if((fp = fopen(filename,"r")) == NULL)
        {
            printf("open error!\n");
            return -1;
        }
        while(!feof(fp))
        {
            if(CurrentIndex == whichLine)
            {
                fgets(StrLine,128,fp);
                printf("%s\n",StrLine);
                return 0;
            }
            fgets(StrLine,128,fp);
            CurrentIndex ++;
        }
        fclose(fp);
        return 0;
}
