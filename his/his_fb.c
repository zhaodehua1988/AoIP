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
    WV_S8 name[20];
    WV_S32 mapped_fd;
    WV_U32 mapped_memlen;
    void *mapped_mem;
    void *mapped_phy;
    WV_U32 mapped_bpp;
    WV_U32 mapped_w;
    WV_U32 mapped_h;
    WV_U32 mapped_stride;
    WV_U32 mapped_bufSize;
    HI_U32 screen_x;
    HI_U32 screen_y;
    HI_U32 screen_w;
    HI_U32 screen_h;
    HI_U32 screen_stride;
    HI_U32 screen_bufSize;
    HI_U32 screen_bufId;
    HI_U8 *screen_mem;
    HI_HANDLE mapped_surf[2];
    HI_HANDLE screen_surf;
    struct fb_var_screeninfo vinfo;
    struct fb_fix_screeninfo finfo;
} HIS_HIFB_DEV_E;

static HIS_HIFB_DEV_E gHifbDev;
static int colorKeyena=0;
static struct fb_var_screeninfo ghifb_st_def_vinfo =
    {
        HIS_FB_VITURE_SCEEN_W,//960,//1920,       //visible resolution xres
        HIS_FB_VITURE_SCEEN_H,//540, //1080      // yres
        HIS_FB_VITURE_SCEEN_W,//960,//1920,       //1920, //virtual resolution xres_virtual
        HIS_FB_VITURE_SCEEN_H* 2,   //1092*2, //yres_virtual
        0,          //xoffset
        0,          //yoffset
        32,         //bits per pixel
        0,          //grey levels, 1 means black/white
        {16, 8, 0}, // {16, 8, 0}, //fb_bitfiled red
        {8, 8, 0},  //{ 8, 8, 0}, // green
        {0, 8, 0},  //{ 0, 8, 0}, //blue
        {24, 8, 0}, // transparency
        0,          //non standard pixel format
        FB_ACTIVATE_FORCE,
        0,  //height of picture in mm
        0,  //width of picture in mm
        0,  //acceleration flags
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

WV_S32 HIS_FB_PrintVinfo(struct fb_var_screeninfo *vinfo)
{
    WV_printf("Printing vinfo:\n");
    WV_printf("txres: %d\n", vinfo->xres);
    WV_printf("tyres: %d\n", vinfo->yres);
    WV_printf("txres_virtual: %d\n", vinfo->xres_virtual);
    WV_printf("tyres_virtual: %d\n", vinfo->yres_virtual);
    WV_printf("txoffset: %d\n", vinfo->xoffset);
    WV_printf("tyoffset: %d\n", vinfo->yoffset);
    WV_printf("tbits_per_pixel: %d\n", vinfo->bits_per_pixel);
    WV_printf("tgrayscale: %d\n", vinfo->grayscale);
    WV_printf("tnonstd: %d\n", vinfo->nonstd);
    WV_printf("tactivate: %d\n", vinfo->activate);
    WV_printf("theight: %d\n", vinfo->height);
    WV_printf("twidth: %d\n", vinfo->width);
    WV_printf("taccel_flags: %d\n", vinfo->accel_flags);
    WV_printf("tpixclock: %d\n", vinfo->pixclock);
    WV_printf("tleft_margin: %d\n", vinfo->left_margin);
    WV_printf("tright_margin: %d\n", vinfo->right_margin);
    WV_printf("tupper_margin: %d\n", vinfo->upper_margin);
    WV_printf("tlower_margin: %d\n", vinfo->lower_margin);
    WV_printf("thsync_len: %d\n", vinfo->hsync_len);
    WV_printf("tvsync_len: %d\n", vinfo->vsync_len);
    WV_printf("tsync: %d\n", vinfo->sync);
    WV_printf("tvmode: %d\n", vinfo->vmode);
    WV_printf("tred: %d/%d\n", vinfo->red.length, vinfo->red.offset);
    WV_printf("tgreen: %d/%d\n", vinfo->green.length, vinfo->green.offset);
    WV_printf("tblue: %d/%d\n", vinfo->blue.length, vinfo->blue.offset);
    WV_printf("talpha: %d/%d\n", vinfo->transp.length, vinfo->transp.offset);
    return WV_SOK;
}

/***************************************************************************

WV_S32  HIS_FB_PrintInfo(struct fb_fix_screeninfo *finfo)

****************************************************************************/
WV_S32 HIS_FB_PrintFinfo(struct fb_fix_screeninfo *finfo)
{
    WV_printf("Printing finfo:\n");
    WV_printf("tsmem_start = %p\n", (char *)finfo->smem_start);
    WV_printf("tsmem_len = %d\n", finfo->smem_len);
    WV_printf("ttype = %d\n", finfo->type);
    WV_printf("ttype_aux = %d\n", finfo->type_aux);
    WV_printf("tvisual = %d\n", finfo->visual);
    WV_printf("txpanstep = %d\n", finfo->xpanstep);
    WV_printf("typanstep = %d\n", finfo->ypanstep);
    WV_printf("tywrapstep = %d\n", finfo->ywrapstep);
    WV_printf("tline_length = %d\n", finfo->line_length);
    WV_printf("tmmio_start = %p\n", (char *)finfo->mmio_start);
    WV_printf("tmmio_len = %d\n", finfo->mmio_len);
    WV_printf("taccel = %d\n", finfo->accel);
    return WV_SOK;
}

/****************************************************************************

WV_S32 HIS_FB_BufGet(HIS_HIFB_DEV_E * pDev;

****************************************************************************/
WV_S32 HIS_FB_BufGet(HIS_HIFB_DEV_E *pDev)
{

    if (pDev->mapped_fd < 0 || pDev->mapped_mem == NULL)
    {
        WV_ERROR("Couldn't get Device %s\n", pDev->name);
        return WV_EFAIL;
    }

    if (ioctl(pDev->mapped_fd, FBIOGET_VSCREENINFO, &(pDev->vinfo)) < 0)
    {
        WV_ERROR("Couldn't get %s vscreeninfo\n", pDev->name);
        return WV_EFAIL;
    }
    //
    //   HIS_FB_PrintVinfo(&vinfo);
    pDev->screen_mem = pDev->mapped_mem;
    if (pDev->vinfo.xoffset == 0 && pDev->vinfo.yoffset == 0)
    {
        pDev->vinfo.xoffset = 0;
        pDev->vinfo.yoffset = pDev->screen_h;
        pDev->screen_mem = pDev->screen_mem + pDev->screen_bufSize;
        pDev->screen_bufId = 1;
        pDev->screen_surf = pDev->mapped_surf[1];
    }
    else
    {
        pDev->vinfo.xoffset = 0;
        pDev->vinfo.yoffset = 0;
        pDev->screen_bufId = 0;
        pDev->screen_surf = pDev->mapped_surf[0];
    }

    return WV_SOK;
}

/****************************************************************************

WV_S32   HIS_FB_BufDis();

****************************************************************************/

WV_S32 HIS_FB_BufDis(HIS_HIFB_DEV_E *pDev)
{

    if (ioctl(pDev->mapped_fd, FBIOGET_VBLANK_HIFB) != 0)
    {
        WV_ERROR("FBIOGET_VBLANK_HIFB failed!\n");
        return WV_EFAIL;
    }

    if (ioctl(pDev->mapped_fd, FBIOPAN_DISPLAY, &(pDev->vinfo)) < 0)
    {
        WV_ERROR("pan_display failed!\n");
        return WV_EFAIL;
    }

    if (ioctl(pDev->mapped_fd, FBIOGET_VBLANK_HIFB) != 0)
    {
        WV_ERROR("FBIOGET_VBLANK_HIFB failed!\n");
        return WV_EFAIL;
    }

    return WV_SOK;
}

/***************************************
WV_S32  HIS_FB_Init(); 
***************************************/
WV_S32 HIS_FB_Init(HIS_HIFB_DEV_E *pDev)
{
    WV_S32 ret = 0;
    printf("his_fb_init\n");
    pDev->mapped_fd = open(pDev->name, O_RDWR, 0);
    if (pDev->mapped_fd < 0)
    {
        WV_ERROR("Unable to open %s\n", pDev->name);
        return WV_EFAIL;
    }
    if (ioctl(pDev->mapped_fd, FBIOPUT_VSCREENINFO, &(pDev->vinfo)) < 0)
    {
        WV_ERROR("Unable to set variable screeninfo %s!\n", pDev->name);
        ret = WV_EFAIL;
        goto CLOSEFD;
    }

    // Get the fix screen info of hardware
    if (ioctl(pDev->mapped_fd, FBIOGET_FSCREENINFO, &(pDev->finfo)) < 0)
    {
        WV_ERROR("Couldn't get console hardware info %s!\n", pDev->name);
        ret = WV_EFAIL;
        goto CLOSEFD;
    }
    /**********************************************/

    pDev->mapped_memlen = pDev->finfo.smem_len;
    pDev->mapped_bufSize = pDev->vinfo.xres_virtual * pDev->vinfo.yres_virtual * (pDev->vinfo.bits_per_pixel / 8);
    pDev->mapped_stride = pDev->vinfo.xres_virtual * (pDev->vinfo.bits_per_pixel / 8);
    pDev->mapped_w = pDev->vinfo.xres_virtual;
    pDev->mapped_h = pDev->vinfo.yres_virtual;
    pDev->screen_w = pDev->vinfo.xres;
    pDev->screen_h = pDev->vinfo.yres;
    pDev->screen_stride = pDev->vinfo.xres_virtual * (pDev->vinfo.bits_per_pixel / 8);
    pDev->screen_bufSize = pDev->vinfo.xres * pDev->vinfo.yres * (pDev->vinfo.bits_per_pixel / 8);
    pDev->mapped_phy = (void *)pDev->finfo.smem_start;
    //Determine the current screen depth
    if (ioctl(pDev->mapped_fd, FBIOGET_VSCREENINFO, &(pDev->vinfo)) < 0)
    {
        WV_printf("Couldn't get vscreeninfo %s!\n", pDev->name);
        ret = WV_EFAIL;
        goto CLOSEFD;
    }
    pDev->mapped_bpp = pDev->vinfo.bits_per_pixel / 8;
    // WV_printf ( "u32Bpp =  %d\r\n",pDev-> mapped_bpp);
    // HIS_FB_PrintVinfo(&pDev->vinfo);
    if (pDev->mapped_memlen != 0 && pDev->mapped_memlen >= pDev->mapped_bufSize)
    {
        pDev->mapped_mem = mmap(NULL, pDev->mapped_memlen, PROT_READ | PROT_WRITE, MAP_SHARED, pDev->mapped_fd, 0);
        if (pDev->mapped_mem == (char *)-1)
        {
            WV_ERROR("Unable to memory map the video hardware\n");
            pDev->mapped_mem = NULL;
            ret = WV_EFAIL;
            goto CLOSEFD;
        }
        HIFB_POINT_S fbPoint;
        fbPoint.s32XPos = pDev->screen_x;
        fbPoint.s32YPos = pDev->screen_y;
        if (ioctl(pDev->mapped_fd, FBIOPUT_SCREEN_ORIGIN_HIFB, &fbPoint) < 0)
        {
            WV_printf("Couldn't get vscreeninfo %s!\n", pDev->name);
            ret = WV_EFAIL;
            goto CLOSEFD;
        }
        // creat surf for higodec
        HIGO_SURINFO_S SurInfo;
        SurInfo.Width = HIS_FB_VITURE_SCEEN_W;//960;//1920;
        SurInfo.Height = HIS_FB_VITURE_SCEEN_H;//540;//1080; //1084
        SurInfo.PixelFormat = HIGO_PF_8888;
        SurInfo.Pitch[0] = HIS_FB_VITURE_SCEEN_W * 4;
        SurInfo.pVirAddr[0] = pDev->mapped_mem; //+ 1920*4*4;
        SurInfo.pPhyAddr[0] = pDev->mapped_phy; //+ 1920*4*4;
        SurInfo.MemType = HIGO_MEMTYPE_MMZ;
        SurInfo.bPubPalette = HI_TRUE;
        WV_CHECK_RET(HI_GO_CreateSurfaceFromMem(&SurInfo, &(pDev->mapped_surf[0])));
        SurInfo.Width = HIS_FB_VITURE_SCEEN_W;//960;//1920;
        SurInfo.Height = HIS_FB_VITURE_SCEEN_H;//540;//1080; //1084
        SurInfo.PixelFormat = HIGO_PF_8888;
        SurInfo.Pitch[0] = HIS_FB_VITURE_SCEEN_W*4;//960*4;//1920 * 4;
        SurInfo.pVirAddr[0] = pDev->mapped_mem + HIS_FB_VITURE_SCEEN_W * 4 * HIS_FB_VITURE_SCEEN_H;
        SurInfo.pPhyAddr[0] = pDev->mapped_phy + HIS_FB_VITURE_SCEEN_W * 4 * HIS_FB_VITURE_SCEEN_H;
        SurInfo.MemType = HIGO_MEMTYPE_MMZ;
        SurInfo.bPubPalette = HI_TRUE;
        WV_CHECK_RET(HI_GO_CreateSurfaceFromMem(&SurInfo, &(pDev->mapped_surf[1])));
    }
    else
    {
        WV_ERROR("sigle buf eroo\n");
        pDev->mapped_mem = NULL;
        ret = WV_EFAIL;
        goto CLOSEFD;
    }

    return ret;
CLOSEFD:
    close(pDev->mapped_fd);
    pDev->mapped_fd = -1;
    return ret;
}

/***************************************
WV_S32  HIS_FB_DeInit(HIS_HIFB_DEV_E * pDev); 
***************************************/
WV_S32 HIS_FB_DeInit(HIS_HIFB_DEV_E *pDev)
{
    WV_CHECK_RET(HI_GO_FreeSurface(pDev->mapped_surf[0]));
    WV_CHECK_RET(HI_GO_FreeSurface(pDev->mapped_surf[1]));
    munmap(pDev->mapped_mem, pDev->mapped_memlen);
    close(pDev->mapped_fd);
    return WV_SOK;
}
/****************************************************************************

WV_S32  HIS_FB_SurfGet( HI_HANDLE * pScreenSurf ,HI_HANDLE * pScreenSurf); 
 
 ****************************************************************************/

WV_S32 HIS_FB_SurfGet(HI_HANDLE *pScreenSurf)
{
    HIS_HIFB_DEV_E *pDev;
    //WV_S32 i;
   // for(i=0;i<2;i++)

    pDev = &gHifbDev;
    WV_CHECK_RET(HIS_FB_BufGet(pDev));

    *pScreenSurf  = pDev->screen_surf;

    return WV_SOK;
}

/***************************************

WV_S32 HIS_FB_SetColorKey(WV_S32 ena);

***************************************/
WV_S32 HIS_FB_SetColorKey(WV_U32 ena, WV_U32 colorKey)
{
    colorKeyena = ena;
    return 0;
}

/***************************************

WV_S32 HIS_FB_SurfFresh();

***************************************/
WV_S32 HIS_FB_SurfFresh()
{
    HIS_HIFB_DEV_E *pDev;
    pDev = &gHifbDev;
    //HIFB_COLORKEY_S colorKey;
    //
    //WV_U32 w, h;
//    WV_U8 *pW, *pH;
    WV_U8 *pMem;
    pMem = pDev->screen_mem;
//    w = 1920;
//    h = 1080;
//    pW = (WV_U8 *)(&w);
//    pH = (WV_U8 *)(&h);

    if (ioctl(pDev->mapped_fd, FBIOGET_VBLANK_HIFB) != 0)
    {
        WV_ERROR("FBIOGET_VBLANK_HIFB failed!\n");
        colorKeyena = 0;
        return WV_EFAIL;
    }

    if (ioctl(pDev->mapped_fd, FBIOPAN_DISPLAY, &(pDev->vinfo)) < 0)
    {
        WV_ERROR("pan_display failed!\n");
        colorKeyena = 0;
        return WV_EFAIL;
    }

    /*************get color key***********/
/*
    if (colorKeyena == 1)
    {

        //colorKeyena = 0;
        if (ioctl(pDev->mapped_fd, FBIOGET_COLORKEY_HIFB, &colorKey) < 0)
        {
            WV_ERROR("pan_display failed!\n");
            colorKeyena = 0;
            return WV_EFAIL;
        }
 
        colorKey.bKeyEnable = HI_TRUE;
        colorKey.u32Key = 0xff000000;

        if (ioctl(pDev->mapped_fd, FBIOPUT_COLORKEY_HIFB, &colorKey) < 0)
        {
            WV_ERROR("pan_display failed!\n");
            colorKeyena = 0;
            return WV_EFAIL;
        }

        //colorKeyena = 0;
    }
    else if (colorKeyena == 0)
    {
        colorKey.bKeyEnable = HI_FALSE;
        colorKey.u32Key = 0xff000000;
        if (ioctl(pDev->mapped_fd, FBIOPUT_COLORKEY_HIFB, &colorKey) < 0)
        {
            WV_ERROR("pan_display failed!\n");
            colorKeyena = 0;
            return WV_EFAIL;
        }
    }
*/
    if (ioctl(pDev->mapped_fd, FBIOGET_VBLANK_HIFB) != 0)
    {
        WV_ERROR("FBIOGET_VBLANK_HIFB failed!\n");
        colorKeyena = 0;
        return WV_EFAIL;
    }

    colorKeyena = 0;
    return WV_SOK;
}

/***************************************
WV_S32  HIS_FB_Open(); 
***************************************/
WV_S32 HIS_FB_Open()
{
    WV_S32 i = 0;
    HIS_HIFB_DEV_E *pDev;
    pDev = &gHifbDev;
    sprintf(pDev->name, "/dev/fb%d", i);
    pDev->vinfo = ghifb_st_def_vinfo;
    pDev->screen_x = 0;
    pDev->screen_y = 0;
    WV_CHECK_RET(HIS_FB_Init(pDev));

    colorKeyena = 0;
    return WV_SOK;
}

/***************************************
WV_S32  HIS_FB_Close(); 
***************************************/
WV_S32 HIS_FB_Close()
{
    HIS_HIFB_DEV_E *pDev;

    pDev = &gHifbDev;
    WV_CHECK_RET(HIS_FB_DeInit(pDev));
    return WV_SOK;
}
