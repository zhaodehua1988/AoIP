///*****************************************
//  Copyright (C) 2009-2018
//  ITE Tech. Inc. All Rights Reserved
//  Proprietary and Confidential
///*****************************************
//   @file   <iTE6615_EDID_PARSER.h>
//   @author Kuro.Chung@ite.com.tw
//   @date   2018/07/04
//   @fileversion: iTE6615_MCUSRC_0.23
//******************************************/

#ifndef _iTE6615_EDID_PARSER_H_
#define _iTE6615_EDID_PARSER_H_

#if _ENABLE_EDID_PARSING_DETAIL_
void parseHDMI1VSDB(u8 VSDB[]);
void parseHDMI2VSDB(u8 VSDB[]);
void parseCEAExtensionDescriptor(u8 Descriptor[]);
void parseVideoCapabilityBlock(u8 Descriptor[]);
void parseVendorSpecificVideoDataBlock(u8 Descriptor[]);
void parseVesaDisplayDeviceDataBlock(u8 Descriptor[]);
void parseVesaTimingDataBlock(u8 Descriptor[]);
void parseColorDataBlock(u8 Descriptor[]);
void parseVideoFormatPreferenceDataBlock(u8 Descriptor[]);
void parseYUV420VideoDataBlock(u8 Descriptor[]);
void parseYUV420MapDataBlock(u8 Descriptor[]);
#endif
void iTE6615_Parser_EDID_block0( u8 *EDID );
void iTE6615_Parser_EDID_block1( u8 *EDID );

#endif // _ITE_EDID_H_
