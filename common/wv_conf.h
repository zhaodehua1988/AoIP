#ifndef _WV_COMMON_CONFIG_H_
#define _WV_COMMON_CONFIG_H_
#include"wv_common.h"

#define WV_CONF_NAME_MAX_LEN     	20
#define WV_CONF_VALUE_MAX_LEN     	20
#define WV_CONF_NOTE_MAX_LEN     	40				
#define WV_CONF_ITEM_MAX_LEN     	WV_CONF_NAME_MAX_LEN + WV_CONF_VALUE_MAX_LEN + WV_CONF_NOTE_MAX_LEN +1

#define WV_CONF_ITEM_MAX_NUM     	1024*3*5

typedef struct  WV_CONF_ITEM_B
{ 
	WV_S8 	name[WV_CONF_NAME_MAX_LEN];
	WV_S8 	value[WV_CONF_VALUE_MAX_LEN]; 
} WV_CONF_ITEM_B;


typedef struct  WV_CONF_ITEM_E
{ 
	WV_S8 	type;
	WV_S8 	name[WV_CONF_NAME_MAX_LEN];
	WV_S8 	value[WV_CONF_VALUE_MAX_LEN ];
	WV_S8 	valueInit[WV_CONF_VALUE_MAX_LEN];
	WV_S8 	note[WV_CONF_NOTE_MAX_LEN];
	
	 
} WV_CONF_ITEM_E;



typedef struct WV_CONF_DEV_E{
	WV_S8  file[WV_CONF_NAME_MAX_LEN];		//save file name 
	WV_S8  path[WV_CONF_NAME_MAX_LEN];		//save file path  
	WV_CONF_ITEM_E   *pItem;                //conf map 
	WV_S32  numItem;                       //conf number 
	WV_CONF_ITEM_B   *pItemBuf;			  //conf buf load form file
	WV_S32  numItemBuf; 
}WV_CONF_DEV_E;


WV_S32  WV_CONF_Creat(WV_CONF_DEV_E * pConfDev, WV_S8 *pFileName,WV_S8 *pPathName);
WV_S32  WV_CONF_Destry(WV_CONF_DEV_E * pConfDev);
WV_S32  WV_CONF_Register(WV_CONF_DEV_E *pConfDev,WV_S8 type ,WV_S8 * pName,WV_S8 *pDefault,WV_S8  *pNote);
WV_S32  WV_CONF_Del(WV_CONF_DEV_E *pConfDev,WV_S8 *pName);
WV_S32  WV_CONF_Get(WV_CONF_DEV_E *pConfDev,WV_S8 *pName ,WV_S8 *pValue);
WV_S32  WV_CONF_Set(WV_CONF_DEV_E *pConfDev,WV_S8 * pName,WV_S8 *pValue);
WV_S32  WV_CONF_Save(WV_CONF_DEV_E *pConfDev);
WV_S32  WV_CONF_Load(WV_CONF_DEV_E *pConfDev);
WV_S32  WV_CONF_SetDefalutAll(WV_CONF_DEV_E *pConfDev);
WV_S32  WV_CONF_SetDefalut(WV_CONF_DEV_E *pConfDev,WV_S8 * pName);

WV_S32  WV_CONF_CpyStr(WV_S8 *pSrc,WV_S8 *pDest,WV_S32  maxlen); 
#endif
