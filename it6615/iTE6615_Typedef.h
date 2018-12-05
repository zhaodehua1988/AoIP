///*****************************************
//  Copyright (C) 2009-2018
//  ITE Tech. Inc. All Rights Reserved
//  Proprietary and Confidential
///*****************************************
//   @file   <iTE6615_Typedef.h>
//   @author Kuro.Chung@ite.com.tw
//   @date   2018/07/04
//   @fileversion: iTE6615_MCUSRC_0.23
//******************************************/

///*****************************************
//  Copyright (C) 2009-2014
//  ITE Tech. Inc. All Rights Reserved
//  Proprietary and Confidential
///*****************************************
//   @file   <iTE6615_Typedef.h>
//   @author Jau-Chih.Tseng@ite.com.tw
//   @date   2014/12/30
//   @fileversion: ITE_MHLRX_SAMPLE_V1.16
//******************************************/

#ifndef _ITE6615_TYPEDEF_H_
#define _ITE6615_TYPEDEF_H_

// WIN32 auto define by complier in PC
#ifndef WIN32
//#define _MCU_8051_
#endif


//////////////////////////////////////////////////
// MCU 8051data type
//////////////////////////////////////////////////


#define FALSE   0
#define TRUE    1


#ifndef NULL
    #define NULL ((void *) 0)
#endif


typedef char iTE6615_s8, *piTE6615_s8;
typedef unsigned char u8, *pu8;
typedef short s16, *ps16;
typedef unsigned short u16, *pu16;

#ifdef _MCU_8051_

    typedef unsigned long u32, *pu32;
    typedef long s32, *ps32;

    typedef unsigned char u1;
    #define _CODE code
    #define _BIT       bit
    #define _BDATA     bdata
    #define _DATA      data
    #define _IDATA     idata
    #define _XDATA     xdata
    #define _FAR_       far
    #define _REENTRANT reentrant
#else

    typedef unsigned int  u32, *pu32;
    typedef int s32, *ps32;

    #ifndef bool
    typedef unsigned char bool ;
    #endif
    typedef bool u1;
    #define _CODE
    #define _BIT       BOOL
    #define _BDATA
    #define _DATA
    #define _IDATA
    #define _XDATA
    #define _FAR
    #define _REENTRANT
#endif




#endif // _TYPEDEF_H_
