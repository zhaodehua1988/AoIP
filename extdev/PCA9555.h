#ifndef __PCA9555_H__
#define __PCA9555_H__

#include "wv_common.h"
#include "his_iic.h"
#include "PCA9548A.h"

#define PCA9555_IN_PORT0_REG              0x00
#define PCA9555_OUT_PORT0_REG             0x02
#define PCA9555_POLINV_PORT0_REG          0x04
#define PCA9555_CFG_PORT0_REG             0x06

#define PCA9555_IN_PORT1_REG              0x01
#define PCA9555_OUT_PORT1_REG             0x03
#define PCA9555_POLINV_PORT1_REG          0x05
#define PCA9555_CFG_PORT1_REG             0x07

#define PCA9555_INPUT      	           0xFF
#define PCA9555_OUTPUT		           0x00
#define PCA9555_POLINVEN	               0xFF
#define PCA9555_POLINVDIS	           0x00

#define PCA9555_PIN_P00                  0x01 //
#define PCA9555_PIN_P01                  0x02 //
#define PCA9555_PIN_P02                  0x04 //
#define PCA9555_PIN_P03                  0x08 //
#define PCA9555_PIN_P04                  0x10 //
#define PCA9555_PIN_P05                  0x20 //
#define PCA9555_PIN_P06                  0x40 //
#define PCA9555_PIN_P07                  0x80 //

#define PCA9555_PIN_P10                  0x01 //
#define PCA9555_PIN_P11                  0x02 //
#define PCA9555_PIN_P12                  0x04 //
#define PCA9555_PIN_P13                  0x08 //
#define PCA9555_PIN_P14                  0x10 //
#define PCA9555_PIN_P15                  0x20 //
#define PCA9555_PIN_P16                  0x40 //
#define PCA9555_PIN_P17                  0x80 //

WV_S32 PCA9555_Init();
WV_S32 PCA9555_DeInit();
WV_S32 PCA9555_WriteRegData( unsigned char ucReg, unsigned char ucData);
WV_S32 PCA9555_ReadRegData( unsigned char ucReg, unsigned char *pUcData);
WV_S32 PCA9555_Set( unsigned char ucReg, unsigned char ucPin);
WV_S32 PCA9555_Clr(unsigned char ucReg, unsigned char ucPin);
WV_S32 PCA9555_Turn( unsigned char ucReg, unsigned char ucPin);

#endif
