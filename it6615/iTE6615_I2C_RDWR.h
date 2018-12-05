///*****************************************
//  Copyright (C) 2009-2018
//  ITE Tech. Inc. All Rights Reserved
//  Proprietary and Confidential
///*****************************************
//   @file   <iTE6615_I2C_RDWR.h>
//   @author Kuro.Chung@ite.com.tw
//   @date   2018/07/04
//   @fileversion: iTE6615_MCUSRC_0.23
//******************************************/

typedef struct
{
    u8 ucAddr;
    u8 andmask;
    u8 ucValue;
} iTE6615_INI_REG;


  u8    lvrxrd( u8 RegAddr);
  u8    lvrxwr( u8 RegAddr,u8 DataIn);
  u8    lvrxbrd( u8 RegAddr, u8 RegBytes, u8 *buffer);
  u8    lvrxset( u8  offset, u8  mask, u8  ucdata );
void    lvrxbwr( u8 offset, u8 byteno, u8 *rddata );
void    lvrx_write_init(iTE6615_INI_REG _CODE *tdata) ;

  u8    hdmitxrd( u8 RegAddr);
  u8    hdmitxwr( u8 RegAddr,u8 DataIn);
  u8    hdmitxbrd( u8 RegAddr, u8 RegBytes, u8 *buffer);
  u8    hdmitxset( u8  offset, u8  mask, u8  ucdata );
void    hdmitxbwr( u8 offset, u8 byteno, u8 *rddata );
void    hdimtx_write_init(iTE6615_INI_REG _CODE *tdata) ;


  u8    cec6615rd(u8 RegAddr);
  u8    cec6615wr(u8 offset, u8 buffer);
  u8    cec6615brd( u8 offset, u8 byteno, u8 *rddata );
void    cec6615bwr( u8 offset, u8 byteno, u8 *rddata );
  u8    cec6615set( u8 offset, u8 mask, u8 ucdata );

