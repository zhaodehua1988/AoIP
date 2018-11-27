///*****************************************
//  Copyright (C) 2009-2017
//  ITE Tech. Inc. All Rights Reserved
//  Proprietary and Confidential
///*****************************************
//   @file   <typedef.h>
//   @author Kuro.Chung@ite.com.tw
//   @date   2017/09/15
//   @fileversion: iTE6805_MCUSRC_1.02
//******************************************/

///*****************************************
//  Copyright (C) 2009-2014
//  ITE Tech. Inc. All Rights Reserved
//  Proprietary and Confidential
///*****************************************
//   @file   <typedef.h>
//   @author Jau-Chih.Tseng@ite.com.tw
//   @date   2014/12/30
//   @fileversion: ITE_MHLRX_SAMPLE_V1.16
//******************************************/

#ifndef _TYPEDEF_H_
#define _TYPEDEF_H_

//////////////////////////////////////////////////
// MCU 8051data type
//////////////////////////////////////////////////
/* typedef bit BOOL;
//#define const code
#define _IDATA idata
#define _XDATA xdata
typedef	code	unsigned char	cBYTE;

typedef char CHAR, *PCHAR;
typedef unsigned char uchar, *puchar;
typedef unsigned char UCHAR, *PUCHAR;
typedef unsigned char byte, *pbyte;
typedef unsigned char BYTE, *PBYTE;

typedef short SHORT, *PSHORT;
typedef unsigned short ushort, *pushort;
typedef unsigned short USHORT, *PUSHORT;
typedef unsigned short word, *pword;
typedef unsigned short WORD, *PWORD;

typedef long LONG, *PLONG;
typedef unsigned long ulong, *pulong;
typedef unsigned long ULONG, *PULONG;
typedef unsigned long dword, *pdword;
typedef unsigned long DWORD, *PDWORD;

typedef unsigned int UINT, uint, *PUINT, *puint; */

#define _HPDMOS_
#ifdef _HPDMOS_
    #define PORT1_HPD_ON	0
    #define PORT1_HPD_OFF	1
#else
    #define PORT1_HPD_ON	1
    #define PORT1_HPD_OFF	0
#endif


#define FALSE 		0
#define TRUE 		1
#define SUCCESS 	0
#define FAIL 		-1
#define ON 			1
#define OFF        	0
#define HIGH       	1
#define LOW        	0

#ifndef NULL
	#define NULL ((void *) 0)
#endif

//#define _MCU_8051_
typedef unsigned short iTE_u1;
/*
#ifdef _MCU_8051_
	//typedef bit iTE_u1 ;
	#define _CODE code
    #define _BIT       bit
    #define _BDATA     bdata
    #define _DATA      data
    #define _IDATA     idata
    #define _XDATA     xdata
    #define _FAR_       far
    #define _REENTRANT reentrant
#else
	#ifdef WIN32
		typedef iTE_u16 iTE_u1 ;
		#define _CODE
	#else
		typedef iTE_u16 iTE_u1 ;
	//	#define _CODE
		#define _CODE __attribute__ ((section ("._OEM_BU1_RODATA ")))
	#endif
    #define _BIT       BOOL
    #define _BDATA
    #define _DATA
    #define _IDATA
    #define _XDATA
    #define _FAR
    #define _REENTRANT
#endif
*/
#define _CODE
#define _BIT       
#define _BDATA
#define _DATA
#define _IDATA
#define _XDATA
#define _FAR
#define _REENTRANT
typedef char iTE_s8, *piTE_s8;
typedef unsigned char iTE_u8, *piTE_u8;
typedef short iTE_s16, *piTE_s16;
typedef unsigned short iTE_u16, *piTE_u16;

#ifndef _MCU_8051_
typedef unsigned long  iTE_u32, *piTE_u32;
typedef long iTE_s32, *piTE_s32;
#else
typedef unsigned long iTE_u32, *piTE_u32;
typedef long iTE_s32, *piTE_s32;
#endif



#endif // _TYPEDEF_H_
