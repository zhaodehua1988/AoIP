#ifndef _TI1297_IO_H_H
#define _TI1297_IO_H_H
#include "wv_common.h"

WV_S32 i2c_1297_write_byte(WV_U8 devAddr, WV_U8 regAddr,  WV_U8 data);

WV_S32 i2c_1297_read_byte(WV_U8 devAddr, WV_U8 regAddr,  WV_U8 data);

#endif