#ifndef _TSK_HIS_GO_H_H__
#define _TSK_HIS_GO_H_H__
#include "wv_common.h"
#include "his_go.h"

#define TSK_HIGO_ANI_MAX_NUM   		HIS_HIGO_GODEC_NUM 
#define TSK_HIGO_FILE_MAX_NUM       16 


WV_S32 TSK_GO_Open();
WV_S32 TSK_GO_Close();

WV_S32 TSK_GO_DecOpen(WV_U32 aniId,WV_S8 *pFileName, WV_U32 x, WV_U32 y,WV_U32 w,WV_U32 h);
WV_S32 TSK_GO_DecClose( WV_U32 id);
#endif
