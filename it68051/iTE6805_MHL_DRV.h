///*****************************************
//  Copyright (C) 2009-2017
//  ITE Tech. Inc. All Rights Reserved
//  Proprietary and Confidential
///*****************************************
//   @file   <iTE6805_MHL_DRV.h>
//   @author Kuro.Chung@ite.com.tw
//   @date   2017/09/15
//   @fileversion: iTE6805_MCUSRC_1.02
//******************************************/



// MSC Function
iTE_u8		iTE6805_MHL_MSC_Get_Response_State();
void		iTE6805_MHL_MSC_Decoder();
iTE_u8		iTE6805_MHL_MSC_Fire(MSC_PKT_ID MSC_PKT);
iTE_u8		iTE6805_MHL_MSC_Fire_With_Retry(MSC_PKT_ID MSC_PKT);
void		iTE6805_MHL_MSC_DevCap_Parse(iTE_u8 ucOffset, iTE_u8 ucData);
iTE_u8		iTE6805_MHL_MSC_WriteBurstDataFill(iTE_u8 Offset, iTE_u8 ucByteNo, iTE_u8 *pucData);
void		iTE6805_MHL_3D_REQ_fsm(MHL3D_STATE *e3DReqState);

void		iTE6805_MHL_Set_RAP_Content(iTE_u8 RCP_CONTENT);

