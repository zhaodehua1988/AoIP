///*****************************************
//  Copyright (C) 2009-2017
//  ITE Tech. Inc. All Rights Reserved
//  Proprietary and Confidential
///*****************************************
//   @file   <iTE6805_EDID.h>
//   @author Kuro.Chung@ite.com.tw
//   @date   2017/09/15
//   @fileversion: iTE6805_MCUSRC_1.02
//******************************************/

#ifndef _ITE6805_EDID_H_
#define _ITE6805_EDID_H_

void	iTE6805_EDID_Init();
void 	iTE6805_EDID_RAMInitial();
iTE_u8	iTE6805_EDID_UpdateRAM(iTE_u8 *pEDID,iTE_u8 BlockNUM);
iTE_u8	iTE6805_EDID_Find_Phyaddress(iTE_u8 *pEDID);
void	iTE6805_EDID_ParseVSDB_3Dblock();

#ifdef _HDMI_SWITCH_
void	iTE6805_EDID_Set_Phyaddress();
#endif

#ifdef ENABLE_4K_MODE_4096x2048_DownScale_To_1920x1080p
	#define EDID_SELECT_TABLE	(12) // 2 type 4k EDID
#endif

#endif // _ITE6805_EDID_H_
