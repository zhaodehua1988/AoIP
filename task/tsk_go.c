#include "his_go.h"
#include "his_dis.h"
#include "tsk_go.h"
#include <sys/time.h>
#include "hi_go_gdev.h"

//usb and mobile 图片和视频位置信息保存路径

typedef struct TSK_GO_DEV_E
{
	WV_THR_HNDL_T thrHndl;
	WV_U32 open;
	WV_U32 close;
	HIS_DEV_HIGO_E goDev;
	WV_U32 colorKeyEna;
} TSK_GO_DEV_E;
TSK_GO_DEV_E gGoDev;

/****************************************************************************

WV_S32 TSK_GO_GetCmd(WV_S32 argc, WV_S8 **argv,WV_S8 *prfBuff)

****************************************************************************/
WV_S32 TSK_GO_GetCmd(WV_S32 argc, WV_S8 **argv, WV_S8 *prfBuff)
{
	return WV_SOK;
}

/****************************************************************************

WV_S32 TSK_GO_GetCmd(WV_S32 argc, WV_S8 **argv,WV_S8 *prfBuff)

****************************************************************************/
WV_S32 TSK_GO_SetCmd(WV_S32 argc, WV_S8 **argv, WV_S8 *prfBuff)
{
	WV_U32 color;
	WV_S32 ret;

	HI_RECT Rect;
	Rect.x = 0;
	Rect.y = 0;
	Rect.w = HIS_FB_VITURE_SCEEN_W;//1920;
	Rect.h = HIS_FB_VITURE_SCEEN_H;

	if (argc < 1)
	{

		prfBuff += sprintf(prfBuff, "set higo <cmd> //cmd like:color/clear/pic\r\n");
		return 0;
	}
	if (strcmp(argv[0], "color") == 0)
	{
		if (argc < 2)
		{
			prfBuff += sprintf(prfBuff, "set higo color <val>//val like:0xffff0000\r\n");
		}

		ret = WV_STR_S2v(argv[1], &color);
		if (ret != WV_SOK)
		{

			prfBuff += sprintf(prfBuff, "input erro!\r\n");
		}
		if(argc >=6){
			ret = WV_STR_S2v(argv[2], (WV_U32*)&Rect.x);
			if (ret != WV_SOK)
			{

				prfBuff += sprintf(prfBuff, "input erro!\r\n");
			}	
			ret = WV_STR_S2v(argv[3], (WV_U32*)&Rect.y);
			if (ret != WV_SOK)
			{

				prfBuff += sprintf(prfBuff, "input erro!\r\n");
			}
			ret = WV_STR_S2v(argv[4], (WV_U32*)&Rect.w);
			if (ret != WV_SOK)
			{

				prfBuff += sprintf(prfBuff, "input erro!\r\n");
			}
			ret = WV_STR_S2v(argv[5],(WV_U32*) &Rect.h);
			if (ret != WV_SOK)
			{

				prfBuff += sprintf(prfBuff, "input erro!\r\n");
			}					
		}
		color = color | 0xff000000;
		HI_GO_FillRect(gGoDev.goDev.layerSurfHndl, &Rect, color, HIGO_COMPOPT_NONE);
		HIS_GO_RefreshLayer(&gGoDev.goDev);
	}else 	if (strcmp(argv[0], "clear") == 0)
	{
		HI_GO_FillRect(gGoDev.goDev.layerSurfHndl, &Rect, 0, HIGO_COMPOPT_NONE);
		HIS_GO_RefreshLayer(&gGoDev.goDev);
	}else if (strcmp(argv[0], "pic") == 0)
	{
		
		if(argc < 2 )
		{
			prfBuff += sprintf(prfBuff, "err!cmd like-> set higo pic <filename>\r\n");
		}
		WV_S8 name[100]= {0};
		sprintf(name,"%s",argv[1]);
		TSK_GO_DecClose(0);
		usleep(100000);
		TSK_GO_DecOpen(0,name,0,0,3840,2160);

	}

	return WV_SOK;

}

/******************************************************************************

void *TSK_GO_Proc(Void * prm);
TSK_GO_OPEN的线程处理函数
******************************************************************************/
void *TSK_GO_Proc(void *prm)
{
	TSK_GO_DEV_E *pDev;
	HIS_DEV_HIGO_E *pGoDev;
	WV_S32 fresh, ret, i;
	pDev = (TSK_GO_DEV_E *)prm;

	pGoDev = &(pDev->goDev);
	pDev->open = 1;
	pDev->close = 0;

	while (pDev->open == 1)
	{
		
		fresh = 0;
		
		for (i = 0; i < TSK_HIGO_ANI_MAX_NUM; i++)
		{

			ret = HIS_GODEC_Render(&(pGoDev->imgDev[i]));
			if (ret == 1)
			{
				fresh = 1;
			}
		}
		if (fresh == 1)
		{
			//HIS_GO_SetColorKey(gGoDev.colorKeyEna, 0xff000000);
			HIS_GO_RefreshLayer(pGoDev);
			usleep(30000);
		}
		else
		{
			usleep(300000);
		}
	}
	pDev->open = 0;
	pDev->close = 1;
	return NULL;
}
/***************************************************************

WV_S32 TSK_GO_DecOpen(WV_U32 aniId,WV_S8 *pFileName, WV_U32 x, WV_U32 y,WV_U32 w,WV_U32 h);

***************************************************************/
WV_S32 TSK_GO_DecOpen(WV_U32 aniId, WV_S8 *pFileName, WV_U32 x, WV_U32 y, WV_U32 w, WV_U32 h)
{

	WV_S32 ret = 0;
	HIS_DEV_GODEC_E *pDev;
	//WV_S8 pFileName[20];

	if (aniId >= TSK_HIGO_ANI_MAX_NUM)
	{
		return WV_EFAIL;
	}

	if (aniId >= TSK_HIGO_FILE_MAX_NUM)
	{
		return WV_EFAIL;
	}
	pDev = &(gGoDev.goDev.imgDev[aniId]);
	pDev->bkDisX = x / 2;
	pDev->bkDisY = y / 2;
	pDev->bkDisW = w / 2;
	pDev->bkDisH = h / 2;
	printf("tsk_go_decOpen x=%d,y=%d,w=%d,h=%d\n", x / 2, y / 2, w / 2, h / 2);
	ret = HIS_GODEC_Open(pDev, pFileName, gGoDev.goDev.layerSurfHndl);
	return ret;
}
/***************************************************************

WV_S32 TSK_GO_DecClose( WV_U32 id);

***************************************************************/
WV_S32 TSK_GO_DecClose(WV_U32 id)
{

	//printf("go dec close %d \n",id);
	HIS_DEV_GODEC_E *pDev;
	if (id >= HIS_HIGO_GODEC_NUM)
	{
		return WV_EFAIL;
	}
	pDev = &(gGoDev.goDev.imgDev[id]);
	WV_CHECK_RET(HIS_GODEC_Close(pDev));
	return WV_SOK;
}
//#####################################################################################

/***************************************************************

WV_S32 TSK_GO_Open();

***************************************************************/

WV_S32 TSK_GO_Open()
{

	//注册一些和go相关的串口命令
	gGoDev.colorKeyEna = 0;
	WV_CHECK_RET(HIS_GO_GetConf(&gGoDev.goDev, 0)); //获取配置
	WV_CHECK_RET(HIS_GO_Open(&gGoDev.goDev));		//打开HIS_GO

	//gRecvPicFlag=2;
	WV_CHECK_RET(WV_THR_Create(&gGoDev.thrHndl, TSK_GO_Proc, WV_THR_PRI_DEFAULT, WV_THR_STACK_SIZE_DEFAULT, &gGoDev));
	//TSK_GO_DecOpen(0,"./gif/0.gif",0,0,200,200);

	WV_CMD_Register("get", "higo", "get higo ", TSK_GO_GetCmd);
	WV_CMD_Register("set", "higo", "set higo cmd ", TSK_GO_SetCmd);
	return WV_SOK;
}

/***************************************************************

WV_S32 TSK_GO_Close();

***************************************************************/

WV_S32 TSK_GO_Close()
{
	if (gGoDev.open == 1)
	{
		gGoDev.open = 0;
		while (gGoDev.close == 1)
			;
		WV_CHECK_RET(WV_THR_Destroy(&gGoDev.thrHndl));
	}

	WV_CHECK_RET(HIS_GO_Close(&(gGoDev.goDev)));
	return WV_SOK;
}