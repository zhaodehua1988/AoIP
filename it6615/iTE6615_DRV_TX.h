///*****************************************
//  Copyright (C) 2009-2018
//  ITE Tech. Inc. All Rights Reserved
//  Proprietary and Confidential
///*****************************************
//   @file   <iTE6615_DRV_TX.h>
//   @author Kuro.Chung@ite.com.tw
//   @date   2018/07/04
//   @fileversion: iTE6615_MCUSRC_0.23
//******************************************/
#ifndef _iTE6615_DRV_TX_H_
#define _iTE6615_DRV_TX_H_


// INIT functions
void    iTE6615_INIT_Chip(void);


// Check Block
// RX
  u8    iTE6615_CheckRX_SCDT(void);
// TX
  u8    iTE6615_CheckTX_RXSENSE(void);
  u8    iTE6615_CheckTX_HPD(void);
  u8    iTE6615_Check_RCLK_Valid( u32 rclk );

// Check config
  u8    iTE6615_CheckConfig_Audio(void);
  u8    iTE6615_CheckConfig_Video(void);

// GET
  u8    iTE6615_Get_EDID_Block(u8 block, u8 *edid_buffer);


// Output setting function
void    iTE6615_SetTX_AVMute(u8 mute);
void    iTE6615_SetTX_Power(u8 POWER_STATE);
void    iTE6615_SetTX_AudioOption( void );
void    iTE6615_SetTX_AudioFormat( void );
void    iTE6615_SetTX_CSCMetrix(const u8 *csc_matrix );
void    iTE6615_SetTX_Timestamp( u8 active );
void    iTE6615_SetTX_AFE();
void    iTE6615_SetTX_RCLK();
// Reset
void    iTE6615_SetTX_Reset(void);

// GET
void    iTE6615_Get_TXCLK(void);
void    iTE6615_Get_VideoInfo(void);

// Show
void    iTE6615_Show_VideoInfo(void);
void    iTE6615_Show_AudioCTS(void);

// RCLK
void    iTE6615_RCLK_Load( void );
void    iTE6615_RCLK_Set( void );

// SCDC
  u8    iTE6615_SCDC_Read(u8 offset, u8 bytenum);
  u8    iTE6615_SCDC_Write(u8 offset, u8 scdc_data);
  u8    iTE6615_SCDC_Set_RR(u8 rr_enable);
void    iTE6615_SCDC_RR_Cancel(void);
// DDC
  u8    iTE6615_DDC_Wait(void);
  u8    iTE6615_DDC_Read(u8 block, u8 offset, u16 length, u8 *edid_buffer);
void    iTE6615_DDC_FIFOClear(void);
// HDCP
  u8    iTE6615_HDCP_Read(u8 offset, u16 bytenum);
void    iTE6615_HDCP1_Enable( void );
void    iTE6615_HDCP2_Enable( void );
void    iTE6615_HDCP1_Show_Status( void );
void    iTE6615_HDCP2_Show_Status( void );
void    iTE6615_HDCP_Show_RI( void );
void    iTE6615_HDCP_Show_PJ( void );
  u8    iTE6615_HDCP_Check_Revocation(u8 *RxID);

void    iTE6615_HDCP2Repeater_StreamManage(void);
void    iTE6615_HDCP2Repeater_ContinueSKESendEKS(void);
  u8    iTE6615_HDCPRepeater_CheckSupport( void );
// HDCP Repeater
#if _HDCP_Enable_RepeaterMode_
void    iTE6615_HDCPRepeater_SHATransfrom( u32 *h, u32 *w );
void    iTE6615_HDCPRepeater_SHASimple( u8 *p, u32 msgcnt , u8 *output);
#endif

// other
void    iTE6615_print_buffer( void *buffer, int length );
void    chgtxbank( u8 bank );



#if iTE6264
void iTE6264_Get_SSCPCLK( void );
#endif



#endif


