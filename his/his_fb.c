#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include <assert.h>
#include "hifb.h"
#include "his_fb.h"
#include "hi_go.h"


typedef struct HIS_HIFB_DEV_E 
{ 
  WV_S8   name[20];
  WV_S32  mapped_fd;
  WV_U32  mapped_memlen;
  void  * mapped_mem;
  void  * mapped_phy;
  WV_U32  mapped_bpp;
  WV_U32  mapped_w;
  WV_U32  mapped_h;
  WV_U32  mapped_stride;
  WV_U32  mapped_bufSize;
  HI_U32  screen_x;
  HI_U32  screen_y; 
  HI_U32  screen_w;
  HI_U32  screen_h;
  HI_U32  screen_stride;
  HI_U32  screen_bufSize;
  HI_U32  screen_bufId;
  HI_U8  *screen_mem;
  HI_HANDLE		mapped_surf[4];
  HI_HANDLE		screen_surf;
  HI_U32  	fpga_use;
  HI_U32  	higo_use;
  struct fb_var_screeninfo vinfo;
  struct fb_fix_screeninfo finfo; 
}HIS_HIFB_DEV_E;
 

static  HIS_HIFB_DEV_E gHifbDev[2];
static int colorKeyena; 
static struct fb_var_screeninfo ghifb_st_def_vinfo =
{
    1920,    //visible resolution xres
    1092, // yres
    1920, //virtual resolution xres_virtual
    1092*2, //yres_virtual  
    0, //xoffset
    0, //yoffset
    32, //bits per pixel
    0, //grey levels, 1 means black/white
    {16, 8, 0}, // {16, 8, 0}, //fb_bitfiled red
    { 8, 8, 0},//{ 8, 8, 0}, // green
    { 0, 8, 0}, //{ 0, 8, 0}, //blue
    {24, 8, 0}, // transparency
    0,  //non standard pixel format
    FB_ACTIVATE_FORCE,
    0, //height of picture in mm
    0, //width of picture in mm
    0, //acceleration flags
    -1, //pixclock
    -1, //left margin
    -1, //right margin
    -1, //upper margin
    -1, //lower margin
    -1, //hsync length
    -1, //vsync length
   // -1,   //sync: FB_SYNC
  //   3,   //vmod: FB_VMOD
        //reserved[6]: reserved for future use
};



WV_S32  HIS_FB_PrintVinfo(struct fb_var_screeninfo *vinfo)
{
   WV_printf( "Printing vinfo:\n");
   WV_printf("txres: %d\n", vinfo->xres);
   WV_printf( "tyres: %d\n", vinfo->yres);
   WV_printf( "txres_virtual: %d\n", vinfo->xres_virtual);
   WV_printf( "tyres_virtual: %d\n", vinfo->yres_virtual);
   WV_printf( "txoffset: %d\n", vinfo->xoffset);
   WV_printf( "tyoffset: %d\n", vinfo->yoffset);
   WV_printf( "tbits_per_pixel: %d\n", vinfo->bits_per_pixel);
   WV_printf( "tgrayscale: %d\n", vinfo->grayscale);
   WV_printf( "tnonstd: %d\n", vinfo->nonstd);
   WV_printf( "tactivate: %d\n", vinfo->activate);
   WV_printf( "theight: %d\n", vinfo->height);
   WV_printf( "twidth: %d\n", vinfo->width);
   WV_printf( "taccel_flags: %d\n", vinfo->accel_flags);
   WV_printf( "tpixclock: %d\n", vinfo->pixclock);
   WV_printf( "tleft_margin: %d\n", vinfo->left_margin);
   WV_printf( "tright_margin: %d\n", vinfo->right_margin);
   WV_printf( "tupper_margin: %d\n", vinfo->upper_margin);
   WV_printf( "tlower_margin: %d\n", vinfo->lower_margin);
   WV_printf( "thsync_len: %d\n", vinfo->hsync_len);
   WV_printf( "tvsync_len: %d\n", vinfo->vsync_len);
   WV_printf( "tsync: %d\n", vinfo->sync);
   WV_printf( "tvmode: %d\n", vinfo->vmode);
   WV_printf( "tred: %d/%d\n", vinfo->red.length, vinfo->red.offset);
   WV_printf( "tgreen: %d/%d\n", vinfo->green.length, vinfo->green.offset);
   WV_printf( "tblue: %d/%d\n", vinfo->blue.length, vinfo->blue.offset);
   WV_printf( "talpha: %d/%d\n", vinfo->transp.length, vinfo->transp.offset);
  return WV_SOK;
}


/***************************************************************************

WV_S32  HIS_FB_PrintInfo(struct fb_fix_screeninfo *finfo)

****************************************************************************/
WV_S32  HIS_FB_PrintFinfo(struct fb_fix_screeninfo *finfo)
{
    WV_printf( "Printing finfo:\n");
    WV_printf( "tsmem_start = %p\n", (char *)finfo->smem_start);
    WV_printf( "tsmem_len = %d\n", finfo->smem_len);
    WV_printf( "ttype = %d\n", finfo->type);
    WV_printf( "ttype_aux = %d\n", finfo->type_aux);
    WV_printf( "tvisual = %d\n", finfo->visual);
    WV_printf( "txpanstep = %d\n", finfo->xpanstep);
    WV_printf( "typanstep = %d\n", finfo->ypanstep);
    WV_printf( "tywrapstep = %d\n", finfo->ywrapstep);
    WV_printf( "tline_length = %d\n", finfo->line_length);
    WV_printf( "tmmio_start = %p\n", (char *)finfo->mmio_start);
    WV_printf( "tmmio_len = %d\n", finfo->mmio_len);
    WV_printf( "taccel = %d\n", finfo->accel);
 return WV_SOK;
}
 

/****************************************************************************

WV_S32 HIS_FB_BufGet(HIS_HIFB_DEV_E * pDev;

****************************************************************************/

WV_S32 HIS_FB_BufGet(HIS_HIFB_DEV_E * pDev)
{ 
  
    if(pDev -> mapped_fd < 0   || pDev -> mapped_mem == NULL)
    {
      WV_ERROR ( "Couldn't get Device %s\n",pDev -> name);
     return WV_EFAIL;
    } 
    
    if (ioctl(pDev -> mapped_fd, FBIOGET_VSCREENINFO, &(pDev ->vinfo)) < 0)
    {
        WV_ERROR ( "Couldn't get %s vscreeninfo\n",pDev -> name);
        return WV_EFAIL;
    } 
  // 
  //   HIS_FB_PrintVinfo(&vinfo); 
    pDev ->screen_mem  = pDev ->mapped_mem; 
    if(pDev ->vinfo.xoffset == 0  &&  pDev ->vinfo.yoffset == 0)
     {
        pDev ->vinfo.xoffset = 0;
        pDev ->vinfo.yoffset = pDev ->screen_h;        
        pDev ->screen_mem    = pDev ->screen_mem + pDev ->screen_bufSize;
        pDev ->screen_bufId  = 1;
        pDev ->screen_surf = pDev ->mapped_surf[1];
      // WV_printf ( "Now buff 1************\r\n");  
      }
    else
    {
        pDev ->vinfo.xoffset = 0;
        pDev ->vinfo.yoffset = 0;        
       //mapped_buf  = mapped_buf + screen_buffsize; 
        pDev ->screen_bufId = 0;
         pDev ->screen_surf = pDev ->mapped_surf[0];
       // WV_printf ( "Now buff 0*******\r\n"); 
    }
    
return WV_SOK;
}

/****************************************************************************

WV_S32 HIS_FB_BufGetForPic(HIS_HIFB_DEV_E * pDev);

****************************************************************************/

WV_S32 HIS_FB_BufGetForPic(HIS_HIFB_DEV_E * pDev)
{ 
  
    if(pDev -> mapped_fd < 0   || pDev -> mapped_mem == NULL)
    {
      WV_ERROR ( "Couldn't get Device %s\n",pDev -> name);
     return WV_EFAIL;
    } 
    
    if (ioctl(pDev -> mapped_fd, FBIOGET_VSCREENINFO, &(pDev ->vinfo)) < 0)
    {
        WV_ERROR ( "Couldn't get %s vscreeninfo\n",pDev -> name);
        return WV_EFAIL;
    } 
  // 
  //   HIS_FB_PrintVinfo(&vinfo); 
    pDev ->screen_mem  = pDev ->mapped_mem; 
    if(pDev ->vinfo.xoffset == 0  &&  pDev ->vinfo.yoffset == 0)
     {
        pDev ->vinfo.xoffset = 0;
        pDev ->vinfo.yoffset = pDev ->screen_h;        
        pDev ->screen_mem    = pDev ->screen_mem + pDev ->screen_bufSize;
        pDev ->screen_bufId  = 1;
        pDev ->screen_surf = pDev ->mapped_surf[3];
      // WV_printf ( "Now buff 1************\r\n");  
      }
    else
    {
        pDev ->vinfo.xoffset = 0;
        pDev ->vinfo.yoffset = 0;        
       //mapped_buf  = mapped_buf + screen_buffsize; 
        pDev ->screen_bufId = 0;
         pDev ->screen_surf = pDev ->mapped_surf[2];
       // WV_printf ( "Now buff 0*******\r\n"); 
    }
    
return WV_SOK;
}
 
/****************************************************************************

WV_S32   HIS_FB_BufDis();

****************************************************************************/

WV_S32 HIS_FB_BufDis(HIS_HIFB_DEV_E * pDev) 
{  
 
   if (ioctl (pDev ->mapped_fd, FBIOGET_VBLANK_HIFB) != 0)
    {
      WV_ERROR("FBIOGET_VBLANK_HIFB failed!\n");
        return  WV_EFAIL;
    }
   
	if (ioctl(pDev ->mapped_fd, FBIOPAN_DISPLAY, &(pDev ->vinfo)) < 0)
	{
		WV_ERROR("pan_display failed!\n");
        return  WV_EFAIL;
	} 
	 
	if (ioctl (pDev ->mapped_fd, FBIOGET_VBLANK_HIFB) != 0)
    {
      WV_ERROR("FBIOGET_VBLANK_HIFB failed!\n");
      return  WV_EFAIL;
    }
   
return WV_SOK;
}

/****************************************************************************

WV_S32   HIS_FB_ClrDis(HIS_HIFB_DEV_E * pDev);

****************************************************************************/


WV_S32   HIS_FB_ClrDis(HIS_HIFB_DEV_E * pDev)
{  
  HI_U8 *pMem;
  WV_CHECK_RET(HIS_FB_BufGet(pDev));//    
// proc fb 
   memset(pDev->screen_mem, 0x00, pDev-> screen_bufSize);// 
//set cmd for video 
    pMem = pDev -> screen_mem;
   *(HI_U32*)(pMem)      = 0xFF000000 ;
   *(HI_U32*)(pMem + 4)  = 0xFF000000 ;
   *(HI_U32*)(pMem + 8)  = 0xFFA5A5A5;
   *(HI_U32*)(pMem + 12) = 0xFF000780;//col 3840/2=1920
   *(HI_U32*)(pMem + 16) = 0xFF000438;//row   1080      
   *(HI_U32*)(pMem + 20) = 0xFFFFFFFF;  
   WV_CHECK_RET(HIS_FB_BufDis(pDev));//  
   return WV_SOK; 
 
}


/***************************************
WV_S32  HIS_FB_Init(); 
***************************************/
WV_S32  HIS_FB_Init(HIS_HIFB_DEV_E * pDev)
{
   WV_S32 ret = 0;
        printf("his_fb_init\n");
    pDev -> mapped_fd = open( pDev -> name, O_RDWR, 0); 
    if (pDev -> mapped_fd < 0)
    { 
        WV_ERROR( "Unable to open %s\n",pDev -> name);  
        return WV_EFAIL;
    } 
    if (ioctl(pDev -> mapped_fd, FBIOPUT_VSCREENINFO, &(pDev-> vinfo)) < 0)
    {
        WV_ERROR( "Unable to set variable screeninfo %s!\n",pDev -> name); 
        ret = WV_EFAIL;
        goto CLOSEFD;
    }
    
    // Get the fix screen info of hardware  
     if (ioctl(pDev ->mapped_fd, FBIOGET_FSCREENINFO, &(pDev-> finfo)) < 0)
     {
        WV_ERROR( "Couldn't get console hardware info %s!\n",pDev -> name); 
        ret = WV_EFAIL;
        goto CLOSEFD;
     }
	/**********************************************/

     pDev-> mapped_memlen = pDev-> finfo.smem_len;
     pDev-> mapped_bufSize = pDev-> vinfo.xres_virtual*pDev-> vinfo.yres_virtual*(pDev-> vinfo.bits_per_pixel/8);
     pDev-> mapped_stride  = pDev-> vinfo.xres_virtual*(pDev-> vinfo.bits_per_pixel/8);
     pDev-> mapped_w   = pDev-> vinfo.xres_virtual;
     pDev->  mapped_h   = pDev-> vinfo.yres_virtual; 
     pDev-> screen_w   = pDev-> vinfo.xres;
     pDev-> screen_h   = pDev-> vinfo.yres;
     pDev-> screen_stride  = pDev-> vinfo.xres_virtual*(pDev-> vinfo.bits_per_pixel/8);
     pDev-> screen_bufSize = pDev-> vinfo.xres * pDev-> vinfo.yres * (pDev-> vinfo.bits_per_pixel/8);  
     pDev-> mapped_phy  = (void *)pDev-> finfo.smem_start;
    //Determine the current screen depth  
    if (ioctl(pDev-> mapped_fd, FBIOGET_VSCREENINFO, &(pDev-> vinfo)) < 0)
    {
        WV_printf ( "Couldn't get vscreeninfo %s!\n",pDev -> name); 
        ret = WV_EFAIL;
        goto CLOSEFD;
    }
     pDev-> mapped_bpp = pDev-> vinfo.bits_per_pixel/8;
    // WV_printf ( "u32Bpp =  %d\r\n",pDev-> mapped_bpp);
    // HIS_FB_PrintVinfo(&pDev->vinfo);
      if (pDev-> mapped_memlen != 0 && pDev-> mapped_memlen >= pDev-> mapped_bufSize)
    { 
		pDev-> mapped_mem = mmap(NULL, pDev-> mapped_memlen,PROT_READ | PROT_WRITE, MAP_SHARED, pDev-> mapped_fd, 0);
		if (pDev-> mapped_mem == (char *)-1)
		{
			WV_ERROR ( "Unable to memory map the video hardware\n");
			pDev-> mapped_mem = NULL;
			ret = WV_EFAIL;
			goto CLOSEFD;
		}
		HIFB_POINT_S  fbPoint;
		fbPoint.s32XPos = pDev->screen_x;
		fbPoint.s32YPos = pDev->screen_y;
		 if (ioctl(pDev-> mapped_fd, FBIOPUT_SCREEN_ORIGIN_HIFB, &fbPoint) < 0)
			{
				WV_printf ( "Couldn't get vscreeninfo %s!\n",pDev -> name); 
				ret = WV_EFAIL;
				goto CLOSEFD;
			}
		// creat surf for higodec
		HIGO_SURINFO_S   SurInfo;

		//SurInfo.Width  = 1920;
		SurInfo.Width  = 1920;
		SurInfo.Height = 1084;//1084
		SurInfo.PixelFormat =  HIGO_PF_8888;
		SurInfo.Pitch[0] = 1920*4;
		SurInfo.pVirAddr[0] =    pDev->mapped_mem + 1920*4*4;
		SurInfo.pPhyAddr[0] =    pDev->mapped_phy + 1920*4*4;
		//SurInfo.pVirAddr[0] =    pDev->mapped_mem + 1920*4*2;
		//SurInfo.pPhyAddr[0] =    pDev->mapped_phy + 1920*4*2;
		SurInfo.MemType  = HIGO_MEMTYPE_MMZ;
		SurInfo.bPubPalette = HI_TRUE; 
		WV_CHECK_RET( HI_GO_CreateSurfaceFromMem ( &SurInfo,&(pDev-> mapped_surf[0])));
		//SurInfo.Width = 1920;
		SurInfo.Width  = 1920;
		SurInfo.Height = 1084;//1084
		SurInfo.PixelFormat =  HIGO_PF_8888;
		SurInfo.Pitch[0] = 1920*4;
		SurInfo.pVirAddr[0] =    pDev->mapped_mem  + 1920*4*1096;
		SurInfo.pPhyAddr[0] =    pDev->mapped_phy  + 1920*4*1096;
		//SurInfo.pVirAddr[0] =    pDev->mapped_mem  + 1920*4*1094;
		//SurInfo.pPhyAddr[0] =    pDev->mapped_phy  + 1920*4*1094;
		SurInfo.MemType  = HIGO_MEMTYPE_MMZ;
		SurInfo.bPubPalette = HI_TRUE;
		WV_CHECK_RET( HI_GO_CreateSurfaceFromMem ( &SurInfo,&(pDev-> mapped_surf[1])));  
		//HIGO_SURINFO_S   SurInfo;

		//SurInfo.Width  = 1920;
		SurInfo.Width  = 1920;
		SurInfo.Height = 1082;//1084
		SurInfo.PixelFormat =  HIGO_PF_8888;
		SurInfo.Pitch[0] = 1920*4;
		//SurInfo.pVirAddr[0] =    pDev->mapped_mem + 1920*4*4;
		//SurInfo.pPhyAddr[0] =    pDev->mapped_phy + 1920*4*4;
		SurInfo.pVirAddr[0] =    pDev->mapped_mem + 1920*4*2;
		SurInfo.pPhyAddr[0] =    pDev->mapped_phy + 1920*4*2;
		SurInfo.MemType  = HIGO_MEMTYPE_MMZ;
		SurInfo.bPubPalette = HI_TRUE; 
		WV_CHECK_RET( HI_GO_CreateSurfaceFromMem ( &SurInfo,&(pDev-> mapped_surf[2])));
		//SurInfo.Width = 1920;
		SurInfo.Width  = 1920;
		SurInfo.Height = 1082;//1084
		SurInfo.PixelFormat =  HIGO_PF_8888;
		SurInfo.Pitch[0] = 1920*4;
		//SurInfo.pVirAddr[0] =    pDev->mapped_mem  + 1920*4*1096;
		//SurInfo.pPhyAddr[0] =    pDev->mapped_phy  + 1920*4*1096;
		SurInfo.pVirAddr[0] =    pDev->mapped_mem  + 1920*4*1094;
		SurInfo.pPhyAddr[0] =    pDev->mapped_phy  + 1920*4*1094;
		SurInfo.MemType  = HIGO_MEMTYPE_MMZ;
		SurInfo.bPubPalette = HI_TRUE;
		WV_CHECK_RET( HI_GO_CreateSurfaceFromMem ( &SurInfo,&(pDev-> mapped_surf[3])));  

        
    }
    else
    {
            WV_ERROR ( "sigle buf eroo\n");
            pDev-> mapped_mem = NULL;
            ret = WV_EFAIL;
            goto CLOSEFD;
    }
   
    return ret;  
CLOSEFD: 
    close(pDev-> mapped_fd); 
    pDev-> mapped_fd = -1;
    return ret; 
}



/***************************************
WV_S32  HIS_FB_DeInit(HIS_HIFB_DEV_E * pDev); 
***************************************/
WV_S32  HIS_FB_DeInit(HIS_HIFB_DEV_E * pDev)
{  
    WV_CHECK_RET( HI_GO_FreeSurface(pDev-> mapped_surf[0]));
    WV_CHECK_RET( HI_GO_FreeSurface(pDev-> mapped_surf[1]));
    munmap(pDev -> mapped_mem, pDev -> mapped_memlen); 
    close(pDev -> mapped_fd);  
    return WV_SOK;
}


/****************************************************************************

WV_S32   HIS_FB_SetFpga();

****************************************************************************/
WV_S32  HIS_FB_SetFpga(WV_U8 * pCmdData ,WV_U8 cmdType, WV_U8 cmdChnl,WV_U32 cmdW ,WV_U32 cmdH)
{ 
   HI_U32 i,j,k=0,bpp;
   HI_U8 *pMem,chennl; 
   HIS_HIFB_DEV_E * pDev;
   pDev = &(gHifbDev[0]);
   chennl = cmdChnl; 
   pDev -> fpga_use =1; 
   while( pDev -> higo_use == 1)
   { 
     usleep(30000);
   }
   
   
   WV_CHECK_RET(HIS_FB_BufGet(pDev));//  
   bpp = pDev -> mapped_bpp;  
// proc fb
   memset(pDev -> screen_mem, 0xff, pDev -> screen_bufSize);// 
  //
   HI_U32  u32Color;
  
  for (i =0; i < cmdH; i++)
	{
        pMem = 	pDev -> screen_mem + (i + 4)* pDev -> screen_stride;
        for (j= 0;j < cmdW;j++)
	  {	 
	   
				 u32Color   = 0xff000000; 
				// u32Color  |=  (pCmdData[k] & 0xff); 
				 u32Color  |=  (pCmdData[k] & 0xff)<<16;
				 k++;
				 u32Color  |=  (pCmdData[k] & 0xff)<<8;
				 k++;
				// u32Color  |=  (pCmdData[k] & 0xff)<<16;
				 u32Color  |=  (pCmdData[k] & 0xff); 
				 k++;
		         *(HI_U32*)(pMem + j * bpp) =  u32Color;   
		}
	}
	//set cmd for video 
	WV_U32  w,h;
	WV_U8   *pW,*pH; 
	
    pMem = pDev -> screen_mem;
    w   = cmdW/2;
    h   = cmdH;
    pW  = (WV_U8 *)(&w);
    pH  = (WV_U8 *)(&h);
     
   *(HI_U32*)(pMem)      = 0xFF000000 | (cmdType & 0xFF) |((cmdType & 0xFF)<<8 )| ((cmdType & 0xFF)<< 16);
   *(HI_U32*)(pMem + 4)  = 0xFF000000 | (chennl & 0xFF) |((chennl & 0xFF)<<8 )| ((chennl & 0xFF)<< 16);
   *(HI_U32*)(pMem + 8)  = 0xFFA5A5A5; 
   *(HI_U32*)(pMem + 12) = 0xFF000000 | (pW[2] & 0xFF) |((pW[1] & 0xFF)<<8 )| ((pW[0] & 0xFF)<< 16);
   *(HI_U32*)(pMem + 16) = 0xFF000000 | (pH[2] & 0xFF) |((pH[1] & 0xFF)<<8 )| ((pH[0] & 0xFF)<< 16);
   *(HI_U32*)(pMem + 20) = 0xFFFFFFFF;   
   //
   WV_CHECK_RET(HIS_FB_BufDis(pDev));//  
   return WV_SOK; 
}




/****************************************************************************

WV_S32   HIS_FB_ClrFpga();

****************************************************************************/
WV_S32   HIS_FB_ClrFpga()
{  
   HI_U8 *pMem;
   HIS_HIFB_DEV_E * pDev;
   pDev = &(gHifbDev[0]);
   WV_CHECK_RET(HIS_FB_BufGet(pDev));//    
// proc fb 
   memset(pDev -> screen_mem, 0x00, pDev -> screen_bufSize);// 
//set cmd for video  
 
   	WV_U32  w,h;
	WV_U8   *pW,*pH;  
    pMem = pDev -> screen_mem;
    w= 3840/2;
    h = 1092;
	//h=2160;    
	pW = (WV_U8 *)(&w);
    pH = (WV_U8 *)(&h);
     
   *(HI_U32*)(pMem)      = 0xFF000000 ;
   *(HI_U32*)(pMem + 4)  = 0xFF000000 ;
   *(HI_U32*)(pMem + 8)  = 0xFFA5A5A5; 
   *(HI_U32*)(pMem + 12) = 0xFF000000 | (pW[2] & 0xFF) |((pW[1] & 0xFF)<<8 )| ((pW[0] & 0xFF)<< 16);
   *(HI_U32*)(pMem + 16) = 0xFF000000 | (pH[2] & 0xFF) |((pH[1] & 0xFF)<<8 )| ((pH[0] & 0xFF)<< 16);
   *(HI_U32*)(pMem + 20) = 0xFFFFFFFF;  
    WV_CHECK_RET(HIS_FB_BufDis(pDev));// 
    usleep(60000);
    pDev -> fpga_use =0;;  
   return WV_SOK; 
 
}

/****************************************************************************

WV_S32   HIS_FB_clrFillRect();

****************************************************************************/
WV_S32   HIS_FB_clrFillRect()
{  
   HI_U8 *pMem;
   HIS_HIFB_DEV_E * pDev;
   pDev = &(gHifbDev[0]);
   WV_CHECK_RET(HIS_FB_BufGet(pDev));//    
// proc fb 
   //memset(pDev -> screen_mem, 0x00, pDev -> screen_bufSize);// 
//set cmd for video  
 
   	WV_U32  w,h;
	WV_U8   *pW,*pH;  
    pMem = pDev -> screen_mem;
    w= 3840/2;
    h = 1084;
	//h = 2160;
    pW = (WV_U8 *)(&w);
    pH = (WV_U8 *)(&h);
     
   *(HI_U32*)(pMem)      = 0xFF000000;
   *(HI_U32*)(pMem + 4)  = 0xFF000000;
   *(HI_U32*)(pMem + 8)  = 0xFFA5A5A5; 
   *(HI_U32*)(pMem + 12) = 0xFF000000 | (pW[2] & 0xFF) |((pW[1] & 0xFF)<<8 )| ((pW[0] & 0xFF)<< 16);
   *(HI_U32*)(pMem + 16) = 0xFF000000 | (pH[2] & 0xFF) |((pH[1] & 0xFF)<<8 )| ((pH[0] & 0xFF)<< 16);
   *(HI_U32*)(pMem + 20) = 0xFFFFFFFF;  
    WV_CHECK_RET(HIS_FB_BufDis(pDev));// 
    usleep(60000);
    pDev -> fpga_use =0; 
   return WV_SOK; 
 
}

/****************************************************************************

WV_S32   HIS_FB_clrDisPlayBmp();

****************************************************************************/
WV_S32 HIS_FB_clrDisPlayBmp()
{  
    HIS_HIFB_DEV_E * pDev;
    pDev = &(gHifbDev[0]);

    memset(pDev -> screen_mem, 0x00, pDev -> screen_bufSize);
    WV_CHECK_RET(HIS_FB_BufDis(pDev));
    usleep(60000);
    pDev -> fpga_use =0;  
   return WV_SOK; 
 
}

/****************************************************************************

WV_S32  HIS_FB_SurfGet( HI_HANDLE * pScreenSurf ,HI_HANDLE * pScreenSurf); 
 
 ****************************************************************************/
 
 WV_S32  HIS_FB_SurfGet( HI_HANDLE * pScreenSurf)
 { 
  HIS_HIFB_DEV_E * pDev;
  WV_S32 i;
  for(i = 0;i< 2; i++)
  {  
    pDev = &(gHifbDev[i]);
    
    pDev -> higo_use  = 0;
   if(pDev -> fpga_use == 1)
    {
      return WV_EFAIL;
    }
    pDev -> higo_use  = 1;
    WV_CHECK_RET(HIS_FB_BufGet(pDev));
    
   *( pScreenSurf + i) = pDev -> screen_surf;
    
  }
   
   return WV_SOK; 
 }
 
/****************************************************************************

WV_S32  HIS_FB_SurfGetForPic( HI_HANDLE * pScreenSurf ,HI_HANDLE * pScreenSurf); 
 
 ****************************************************************************/
 
 WV_S32  HIS_FB_SurfGetForPic( HI_HANDLE * pScreenSurf)
 { 
  HIS_HIFB_DEV_E * pDev;
  WV_S32 i;
  for(i = 0;i< 2; i++)
  {  
    pDev = &(gHifbDev[i]);
    
    pDev -> higo_use  = 0;
   if(pDev -> fpga_use == 1)
    {
      return WV_EFAIL;
    }
    pDev -> higo_use  = 1;
    WV_CHECK_RET(HIS_FB_BufGetForPic(pDev));
    
   *( pScreenSurf + i) = pDev -> screen_surf;
    
  }
   
   return WV_SOK; 
 }
/***************************************

WV_S32 HIS_FB_SetColorKey(WV_S32 ena);

***************************************/
WV_S32 HIS_FB_SetColorKey(WV_U32 ena,WV_U32 colorKey)
{
	colorKeyena = ena;
	return 0;
}
 
/***************************************

WV_S32 HIS_FB_SurfFresh();

***************************************/
WV_S32 HIS_FB_SurfFresh()
{
    HIS_HIFB_DEV_E * pDev;
    pDev = &(gHifbDev[0]);
    HIFB_COLORKEY_S colorKey;
    //
    WV_U32  w,h;
	WV_U8   *pW,*pH; 
	WV_U8   *pMem; 
    pMem = pDev -> screen_mem;
    //w= 3840/2;
   // h= 1920;
	w = 1920;
	h = 1080;	
	//h = 2160;
    pW = (WV_U8 *)(&w);
    pH = (WV_U8 *)(&h);  
    memset(pMem, 0xff, w*4);
/*
   *(HI_U32*)(pMem)      = 0xFF000000  ;
   *(HI_U32*)(pMem + 4)  = 0xFF000000  ;
   *(HI_U32*)(pMem + 8)  = 0xFFA5A5A5;
   *(HI_U32*)(pMem + 12) = 0xFF000000 | (cmdW/2);
   *(HI_U32*)(pMem + 12) = 0xFF000000 | (pW[2] & 0xFF) |((pW[1] & 0xFF)<<8 )| ((pW[0] & 0xFF)<< 16);
   *(HI_U32*)(pMem + 16) = 0xFF000000 | (pH[2] & 0xFF) |((pH[1] & 0xFF)<<8 )| ((pH[0] & 0xFF)<< 16);
   *(HI_U32*)(pMem + 20) = 0xFFFFFFFF; 
    //
*/    
   if (ioctl (pDev ->mapped_fd, FBIOGET_VBLANK_HIFB) != 0)
    {
		WV_ERROR("FBIOGET_VBLANK_HIFB failed!\n");
		pDev -> higo_use  = 0;
		colorKeyena = 0; 
		return  WV_EFAIL;
    }
   
	if (ioctl(pDev ->mapped_fd, FBIOPAN_DISPLAY, &(pDev ->vinfo)) < 0)
	{
		WV_ERROR("pan_display failed!\n");
		pDev -> higo_use  = 0; 
		colorKeyena = 0;
		return  WV_EFAIL;
	} 
	 
	/*************get color key***********/

	if(colorKeyena == 1){

		//colorKeyena = 0;
		if (ioctl(pDev ->mapped_fd, FBIOGET_COLORKEY_HIFB, &colorKey) < 0)
		{
			WV_ERROR("pan_display failed!\n");
			pDev -> higo_use  = 0; 
			colorKeyena = 0;
			return  WV_EFAIL;
		} 
		//printf("color KEY ena[%d],val[%08x]\n",colorKey.bKeyEnable,colorKey.u32Key);

		colorKey.bKeyEnable = HI_TRUE;
		colorKey.u32Key =  0xff000000;

		if (ioctl(pDev ->mapped_fd, FBIOPUT_COLORKEY_HIFB, &colorKey) < 0)
		{
			WV_ERROR("pan_display failed!\n");
			pDev -> higo_use  = 0; 
			colorKeyena = 0;
			return  WV_EFAIL;
		}
	/*
		//set alpha val
		HIFB_ALPHA_S alpha;

		alpha.bAlphaEnable = HI_TRUE;
		alpha.bAlphaChannel = HI_TRUE;
		alpha.u8Alpha0 = 0;
		alpha.u8Alpha1 = 0;
		alpha.u8GlobalAlpha = 255;

		if (ioctl(pDev ->mapped_fd, FBIOPUT_ALPHA_HIFB, &alpha) < 0)
		{
			WV_ERROR("pan_display failed!\n");
			pDev -> higo_use  = 0; 
			colorKeyena = 0;
			return  WV_EFAIL;
		}
   */

		//colorKeyena = 0;
	}else if(colorKeyena== 0){
		colorKey.bKeyEnable = HI_FALSE;
		colorKey.u32Key =  0xff000000;
		if (ioctl(pDev ->mapped_fd, FBIOPUT_COLORKEY_HIFB, &colorKey) < 0)
		{
			WV_ERROR("pan_display failed!\n");
			pDev -> higo_use  = 0; 
			colorKeyena = 0;
			return  WV_EFAIL;
		}
	}

//################################################
	pDev -> higo_use  = 1;  
	pDev = &(gHifbDev[1]); 
	if(colorKeyena == 1){//set colorKey

		//colorKeyena = 0;
		if (ioctl(pDev ->mapped_fd, FBIOGET_COLORKEY_HIFB, &colorKey) < 0)
		{
			WV_ERROR("pan_display failed!\n");
			pDev -> higo_use  = 0; 
			colorKeyena = 0;
			return  WV_EFAIL;
		} 
		//printf("2color KEY ena[%d],val[%08x]\n",colorKey.bKeyEnable,colorKey.u32Key);
		colorKey.bKeyEnable = HI_TRUE;
		colorKey.u32Key =  0xff000000;
		if (ioctl(pDev ->mapped_fd, FBIOPUT_COLORKEY_HIFB, &colorKey) < 0)
		{
			WV_ERROR("pan_display failed!\n");
			pDev -> higo_use  = 0; 
			colorKeyena = 0;
			return  WV_EFAIL;
		}
		//colorKeyena = 0;
	}else if(colorKeyena== 0){
		colorKey.bKeyEnable = HI_FALSE;
		colorKey.u32Key =  0xff000000;
		if (ioctl(pDev ->mapped_fd, FBIOPUT_COLORKEY_HIFB, &colorKey) < 0)
		{
			WV_ERROR("pan_display failed!\n");
			pDev -> higo_use  = 0;
			colorKeyena = 0; 
			return  WV_EFAIL;
		}
	}
	pDev -> higo_use  = 1;  
	pDev = &(gHifbDev[1]); 
	if (ioctl(pDev ->mapped_fd, FBIOPAN_DISPLAY, &(pDev ->vinfo)) < 0)
	{
		WV_ERROR("pan_display failed!\n");
		pDev -> higo_use  = 0; 
		colorKeyena = 0;
		return  WV_EFAIL;
	} 
 	pDev = &(gHifbDev[0]); 
	if (ioctl (pDev ->mapped_fd, FBIOGET_VBLANK_HIFB) != 0)
   {
        WV_ERROR("FBIOGET_VBLANK_HIFB failed!\n");
		pDev -> higo_use  = 0; 
		colorKeyena = 0;
      	return  WV_EFAIL;
   }

	colorKeyena = 0;
   pDev -> higo_use  = 0;  
   return WV_SOK; 
}

/***************************************

WV_S32 HIS_FB_SetAlpha(WV_U8 alphaKey);

***************************************/
WV_S32 HIS_FB_SetAlpha(WV_U8 alphaKey)
{
    colorKeyena = 1;
    HIS_HIFB_DEV_E * pDev;
    pDev = &(gHifbDev[0]);
    HIFB_COLORKEY_S colorKey;

    WV_U32  w,h;
	WV_U8   *pW,*pH; 
	WV_U8   *pMem; 
    pMem = pDev -> screen_mem;
	w = 1920;
	h = 1080;	
    pW = (WV_U8 *)(&w);
    pH = (WV_U8 *)(&h);  
    memset(pMem, 0xff, w*4);
  
   if (ioctl (pDev ->mapped_fd, FBIOGET_VBLANK_HIFB) != 0)
    {
		WV_ERROR("FBIOGET_VBLANK_HIFB failed!\n");
		pDev -> higo_use  = 0;
		colorKeyena = 0; 
		return  WV_EFAIL;
    }
   
	if (ioctl(pDev ->mapped_fd, FBIOPAN_DISPLAY, &(pDev ->vinfo)) < 0)
	{
		WV_ERROR("pan_display failed!\n");
		pDev -> higo_use  = 0; 
		colorKeyena = 0;
		return  WV_EFAIL;
	} 
	 
	/*************get color key***********/

	if(colorKeyena == 1){

		//colorKeyena = 0;
		if (ioctl(pDev ->mapped_fd, FBIOGET_COLORKEY_HIFB, &colorKey) < 0)
		{
			WV_ERROR("pan_display failed!\n");
			pDev -> higo_use  = 0; 
			colorKeyena = 0;
			return  WV_EFAIL;
		} 
		//printf("color KEY ena[%d],val[%08x]\n",colorKey.bKeyEnable,colorKey.u32Key);

		colorKey.bKeyEnable = HI_TRUE;
		colorKey.u32Key =  0xff000000;

		if (ioctl(pDev ->mapped_fd, FBIOPUT_COLORKEY_HIFB, &colorKey) < 0)
		{
			WV_ERROR("pan_display failed!\n");
			pDev -> higo_use  = 0; 
			colorKeyena = 0;
			return  WV_EFAIL;
		}
		//set alpha val
		HIFB_ALPHA_S alpha;

//typedef struct
//{
 //   HI_BOOL bAlphaEnable;   /**<  alpha enable flag *//**<CNcomment:alpha使能标识*/
 //   HI_BOOL bAlphaChannel;  /**<  alpha channel enable flag *//**<CNcomment:alpha通道使能*/
 //   HI_U8 u8Alpha0;         /**<  alpha0 value, used in ARGB1555 *//**CNcomment:alpha0取值，ARGB1555格式时使用*/
  //  HI_U8 u8Alpha1;         /**<  alpha1 value, used in ARGB1555 *//**CNcomment:alpha1取值，ARGB1555格式时使用*/
  //  HI_U8 u8GlobalAlpha;    /**<  global alpha value *//**<CNcomment:全局alpha取值*/
  //  HI_U8 u8Reserved;
//}HIFB_ALPHA_S;

		alpha.bAlphaEnable = HI_TRUE;
		alpha.bAlphaChannel = HI_TRUE;
		alpha.u8Alpha0 = 0;
		alpha.u8Alpha1 = 0;
		alpha.u8GlobalAlpha = alphaKey;

		if (ioctl(pDev ->mapped_fd, FBIOPUT_ALPHA_HIFB, &alpha) < 0)
		{
			WV_ERROR("pan_display failed!\n");
			pDev -> higo_use  = 0; 
			colorKeyena = 0;
			return  WV_EFAIL;
		}


		//colorKeyena = 0;
	}else if(colorKeyena== 0){
		colorKey.bKeyEnable = HI_FALSE;
		colorKey.u32Key =  0xff000000;
		if (ioctl(pDev ->mapped_fd, FBIOPUT_COLORKEY_HIFB, &colorKey) < 0)
		{
			WV_ERROR("pan_display failed!\n");
			pDev -> higo_use  = 0; 
			colorKeyena = 0;
			return  WV_EFAIL;
		}
	}

//################################################
	pDev -> higo_use  = 1;  
	pDev = &(gHifbDev[1]); 
	if(colorKeyena == 1){//set colorKey

		//colorKeyena = 0;
		if (ioctl(pDev ->mapped_fd, FBIOGET_COLORKEY_HIFB, &colorKey) < 0)
		{
			WV_ERROR("pan_display failed!\n");
			pDev -> higo_use  = 0; 
			colorKeyena = 0;
			return  WV_EFAIL;
		} 
		//printf("2color KEY ena[%d],val[%08x]\n",colorKey.bKeyEnable,colorKey.u32Key);
		colorKey.bKeyEnable = HI_TRUE;
		colorKey.u32Key =  0xff000000;
		if (ioctl(pDev ->mapped_fd, FBIOPUT_COLORKEY_HIFB, &colorKey) < 0)
		{
			WV_ERROR("pan_display failed!\n");
			pDev -> higo_use  = 0; 
			colorKeyena = 0;
			return  WV_EFAIL;
		}
		//colorKeyena = 0;
	}else if(colorKeyena== 0){
		colorKey.bKeyEnable = HI_FALSE;
		colorKey.u32Key =  0xff000000;
		if (ioctl(pDev ->mapped_fd, FBIOPUT_COLORKEY_HIFB, &colorKey) < 0)
		{
			WV_ERROR("pan_display failed!\n");
			pDev -> higo_use  = 0;
			colorKeyena = 0; 
			return  WV_EFAIL;
		}
	}
	pDev -> higo_use  = 1;  
	pDev = &(gHifbDev[1]); 
	if (ioctl(pDev ->mapped_fd, FBIOPAN_DISPLAY, &(pDev ->vinfo)) < 0)
	{
		WV_ERROR("pan_display failed!\n");
		pDev -> higo_use  = 0; 
		colorKeyena = 0;
		return  WV_EFAIL;
	} 
 	pDev = &(gHifbDev[0]); 
	if (ioctl (pDev ->mapped_fd, FBIOGET_VBLANK_HIFB) != 0)
   {
        WV_ERROR("FBIOGET_VBLANK_HIFB failed!\n");
		pDev -> higo_use  = 0; 
		colorKeyena = 0;
      	return  WV_EFAIL;
   }

	colorKeyena = 0;
   pDev -> higo_use  = 0;  
   return WV_SOK; 
}

/***************************************
WV_S32  HIS_FB_Open(); 
***************************************/
WV_S32  HIS_FB_Open()
{  
	WV_S32 i;
	HIS_HIFB_DEV_E * pDev;
	for(i =0 ; i<2;i++)
	{
		pDev = &(gHifbDev[i]);
		sprintf(pDev -> name , "/dev/fb%d",i);
		pDev -> vinfo  = ghifb_st_def_vinfo;
		pDev -> screen_x = i* 1920; 
		pDev -> screen_y = 0; 
		//pDev -> vinfo.xoffset = i* 1920; 
		WV_CHECK_RET( HIS_FB_Init(pDev)); 
	} 

	colorKeyena = 0; 
    return WV_SOK;
} 
 

/***************************************
WV_S32  HIS_FB_Close(); 
***************************************/
WV_S32  HIS_FB_Close()
{  
 WV_S32 i;
 HIS_HIFB_DEV_E * pDev;
 for(i =0 ; i<2;i++)
 {   
    pDev = &(gHifbDev[i]);  
    WV_CHECK_RET( HIS_FB_DeInit(pDev)); 
 }  
    return WV_SOK;
} 
  
