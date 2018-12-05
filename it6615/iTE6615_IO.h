///*****************************************
//  Copyright (C) 2009-2018
//  ITE Tech. Inc. All Rights Reserved
//  Proprietary and Confidential
///*****************************************
//   @file   <iTE6615_IO.h>
//   @author Kuro.Chung@ite.com.tw
//   @date   2018/07/04
//   @fileversion: iTE6615_MCUSRC_0.23
//******************************************/

#ifndef _ITE6615_IO_h_
#define _ITE6615_IO_h_

#include  "iTE6615_Typedef.h"
u1 i2c_6615_write_byte(u8 address, u8 offset, u8 byteno, u8 *p_data, u8 device);
u1 i2c_6615_read_byte(u8 address, u8 offset, u8 byteno, u8 *p_data, u8 device);

#endif
