#ifndef  _FPGA_CONF_CONF_H_
#define  _FPGA_CONF_CONF_H_
#include "wv_common.h"

typedef struct FPGA_WIN_INFO_S 
{
 
WV_U16             		x;
WV_U16            		y; 
WV_U16            		w;
WV_U16             		h; 
WV_U8             		outId;
WV_U8             		videoId; 
 
} FPGA_WIN_INFO_S;


typedef struct FPGA_SPLIT_INFO_S 
{
	WV_U16             		startX0;
	WV_U16            		endX0;
	WV_U16             		startY0;
	WV_U16            		endY0; 

	WV_U16             		startX1;
	WV_U16            		endX1;
	WV_U16             		startY1;
	WV_U16            		endY1; 

	WV_U16             		startX2;
	WV_U16            		endX2;
	WV_U16             		startY2;
	WV_U16            		endY2; 

	WV_U16             		startX3;
	WV_U16            		endX3; 
	WV_U16             		startY3;
	WV_U16            		endY3; 

	WV_U16             		startX4;
	WV_U16            		endX4; 
	WV_U16             		startY4;
	WV_U16            		endY4; 

	WV_U16             		startX5;
	WV_U16            		endX5; 
	WV_U16             		startY5;
	WV_U16            		endY5;

	WV_U16             		startX6;
	WV_U16            		endX6;  
	WV_U16             		startY6;
	WV_U16            		endY6; 

	WV_U16             		startX7;
	WV_U16            		endX7; 
	WV_U16             		startY7;
	WV_U16            		endY7; 

} FPGA_SPLIT_INFO_S ;

typedef struct FPGA_RESOLUTION_INFO_S 
{
 WV_U16 frameRate;
 WV_U16 totalPix;
 WV_U16 totalLine;
 WV_U16 hsyncWidth;
 WV_U16 vsyncWidth;
 WV_U16 actHstart;
 WV_U16 actHsize;
 WV_U16 actVstart;
 WV_U16 actVsize;
} FPGA_RESOLUTION_INFO_S ; 

WV_U16 FPGA_CONF_GerVer();
WV_S32 FPGA_CONF_ResetA();
WV_S32 FPGA_CONF_RqMcu();
WV_S32 FPGA_CONF_ClrBuf();



WV_S32 FPGA_CONF_SetWin(WV_U16  num, FPGA_WIN_INFO_S * pInfo);
WV_S32 FPGA_CONF_SetResolution();
WV_S32 FPGA_CONF_SetOutput(WV_U16  ena);
WV_U16 FPGA_CONF_GetOutput();
WV_S32 FPGA_CONF_GetVersion();
WV_S32 FPGA_CONF_GetOutChl_Num();
WV_S32 FPGA_CONF_GetVersion_ID(); 
WV_U16 FPGA_CONF_SetGlobalOutput(WV_U16 ena);
#endif
