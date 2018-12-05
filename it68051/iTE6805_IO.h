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

#ifndef _ITE6805_IO_h_
#define _ITE6805_IO_h_

#include  "iTE6805_typedef.h"
iTE_u1 i2c_6805_write_byte(iTE_u8 address, iTE_u8 offset, iTE_u8 byteno, iTE_u8 *p_data, iTE_u8 device);
iTE_u1 i2c_6805_read_byte(iTE_u8 address, iTE_u8 offset, iTE_u8 byteno, iTE_u8 *p_data, iTE_u8 device);

#endif
