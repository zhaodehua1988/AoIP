///*****************************************
//  Copyright (C) 2009-2018
//  ITE Tech. Inc. All Rights Reserved
//  Proprietary and Confidential
///*****************************************
//   @file   <iTE6615_EDID_PARSER.c>
//   @author Kuro.Chung@ite.com.tw
//   @date   2018/07/04
//   @fileversion: iTE6615_MCUSRC_0.23
//******************************************/

#include "iTE6615_Global.h"

extern iTE6615_GlobalDATA iTE6615_DATA;

#if _ENABLE_EDID_PARSING_DETAIL_

u32 VDBCount;
u8 ModeSupport[0x80];

typedef struct {
    u16 HActive;
    u16 VActive;
    u16 HTotal;
    u16 VTotal;
    u32 PCLK;
    u16 HFrontPorch;
    u16 HSyncWidth;
    u16 HBackPorch;
    u16 VFrontPorch;
    u16 VSyncWidth;
    u16 VBackPorch;
    u8 ScanMode;
    u8 VPolarity;
    u8 HPolarity;
    u8 PixRpt;
    char *format ;
    u8 VIC;
} MODEINFO ;


_CODE char *strAudFs[] =
{
    "32KHz",
    "44.1KHz",
    "48KHz",
    "88.2KHz",
    "96KHz",
    "176.4KHz",
    "192KHz",
    "reserve"
};

_CODE char *strAudioCode[] =
{
    "Refer to stream header",
    "LPCM",
    "AC3",
    "MPEG-1",
    "MP3",
    "MPEG-2",
    "AAC LC",
    "DTS",
    "ATRAC",
    "DSD",
    "E-AC-3",
    "DTS-HD",
    "MLP",
    "DST",
    "WMA Pro",
    "Audio Extension Type"
};

_CODE char *strVIC[] =
{
    "Unknown VIC",
    "640x480p60",
    "720x480p60 4:3",
    "720x480p60 16:9",
    "1280x720p60",
    "1920x1080i60",
    "1440x480i60 4:3",
    "1440x480i60 16:9",
    "1440x240p60 4:3",
    "1440x240p60 16:9",
    "2880x480i60 4:3",
    "2880x480i60 16:9",
    "2880x240p60 4:3",
    "2880x240p60 16:9",
    "1440x480p60 4:3",
    "1440x480p60 16:9",
    "1920x1080p60",
    "720x576p50 4:3",
    "720x576p50 16:9",
    "1280x720p50",
    "1920x1080i50",
    "1440x576i50 4:3",
    "1440x576i50 16:9",
    "1440x288p50 4:3",
    "1440x288p50 16:9",
    "2880x576i50 4:3",
    "2880x576i50 16:9",
    "2880x288p50 4:3",
    "2880x288p50 16:9",
    "1440x576p50 4:3",
    "1440x576p50 16:9",
    "1920x1080p50",
    "1920x1080p24",
    "1920x1080p25",
    "1920x1080p30",
    "2880x480p60 4:3",
    "2880x480p60 16:9",
    "2880x576p50 4:3",
    "2880x576p50 16:9",
    "1920x1080p50 (HTotal = 2304) 72MHz",
    "1920x1080i100", //40
    "1280x720p100", // 41
    "720x576p100 4:3",   //42
    "720x576p100 16:9",
    "1440x576i100 4:3",    //44
    "1440x576i100 16:9",
    "1920x1080i120", //46
    "1280x720p120",//47
    "720x480p120 4:3", //48
    "720x480p120 16:9",
    "1440x480i120 4:3",  //50
    "1440x480i120 16:9",
    "720x576p100 4:3",   //52
    "720x576p100 16:9",
    "1440x576i100 4:3",    //54
    "1440x576i100 16:9",
    "720x480p120 4:3", //56
    "720x480p120 16:9",
    "1440x480i120 4:3",  //58
    "1440x480i120 16:9",
    "1280x720p24",//60
    "1280x720p25",//61
    "1280x720p30",//62
    "1920x1080p120", //63
    "1920x1080p100", //64
    "1280x720p24 (64:27)",//65
    "1280x720p25 (64:27)",//66
    "1280x720p30 (64:27)",//67
    "1280x720p50 (64:27)",//68
    "1280x720p60 (64:27)",//69
    "1280x720p100 (64:27)",//70
    "1280x720p120 (64:27)",//71

    "1920x1080p24 (64:27)", //72
    "1920x1080p25 (64:27)", //73
    "1920x1080p30 (64:27)", //74
    "1920x1080p50 (64:27)", //75
    "1920x1080p60 (64:27)", //76
    "1920x1080p100 (64:27)", //77
    "1920x1080p120 (64:27)", //78

    "1680x720p24 (64:27)", //79
    "1680x720p25 (64:27)", //80
    "1680x720p30 (64:27)", //81
    "1680x720p50 (64:27)", //82
    "1680x720p60 (64:27)", //83
    "1680x720p100 (64:27)", //84
    "1680x720p120 (64:27)", //85
    //86-92
    "2560x1080p24",
    "2560x1080p25",
    "2560x1080p30",
    "2560x1080p50",
    "2560x1080p60",
    "2560x1080p100",
    "2560x1080p120",

    //93-97
    "3840x2160p24",
    "3840x2160p25",
    "3840x2160p30",
    "3840x2160p50",
    "3840x2160p60",

    //98-102
    "4096x2160p24",
    "4096x2160p25",
    "4096x2160p30",
    "4096x2160p50",
    "4096x2160p60",

    //103-107
    "3840x2160p24 (64:27)",
    "3840x2160p25 (64:27)",
    "3840x2160p30 (64:27)",
    "3840x2160p50 (64:27)",
    "3840x2160p60 (64:27)",

    // Future use.
    "CEA 861F Undefined VIC=108",
};

_CODE char *ES23[] =
{
    "800x600@60Hz",
    "800x600@56Hz",
    "640x480@75Hz",
    "640x480@72Hz",
    "640x480@67Hz",
    "640x480@60Hz",
    "720x400@88Hz",
    "720x400@70Hz"
};

_CODE char *ES24[] =
{
    "1280x1024@75Hz",
    "1024x768@75Hz",
    "1024x768@70Hz",
    "1024x768@60Hz",
    "1024x768@87iHz",
    "832x624@75Hz",
    "800x600@75Hz",
    "800x600@72Hz"
};

_CODE char *strDataBlock[] =
{
    "Reserved",
    "Audio Data Block",
    "Video Data Block",
    "Vendor Specific Data Block",
    "Speaker Allocation Data Blcok",
    "VESA  DTC Data Block",
    "Reserved",
    "Use Extended Tag"
};

_CODE char *str3DFormat[] =
{
    "Frame Packing",
    "Field Alternative",
    "Line alternative",
    "Side-by-Side (Full)",
    "L+Depth",
    "L+depth+graphics+graphics-depth",
    "Top-and-Bottom",
    "reserved for future use",
    "Side-By-Side (Half)",
    "reserved for future use",
    "reserved for future use",
    "reserved for future use",
    "reserved for future use",
    "reserved for future use",
    "reserved for future use",
    "reserved for future use",
};

u32 checksumEDID(u8 EDID[])
{
    u32 i;
    u32 sum;
    sum = 0;
    for( i = 0; i < 128; i++ )
    {
        sum+=EDID[i]; sum &= 0xFF;
    }
    return sum;
}

enum PROJCMD {
    CMD_READ = 0 ,
    CMD_WRITE,
    CMD_AND,
    CMD_OR,
    CMD_SET,
    CMD_LOAD
};

void parseDTD(u8 *dtd)
{
    MODEINFO Mode ;
    u16 HBlank, VBlank;

    //u32 HTotal, VTotal, HActive, VActive, HBlank, VBlank;
    //u32 HFP, HSW, VFP, VSW;

    u32 HSize, VSize;
    unsigned VF;
    //unsigned long PCLK;
    u32 interlace;

    PRINTF_EDID(("::[DTD]\r\n"));

    Mode.PCLK = ((unsigned long) dtd[1])<<8 | (unsigned long)dtd[0];
    Mode.PCLK *= 10000;

    Mode.HActive = dtd[4] & 0xF0; Mode.HActive <<= 4; Mode.HActive |= dtd[2];

    HBlank = dtd[4] & 0xF; HBlank <<= 8; HBlank |= dtd[3];

    Mode.HTotal = Mode.HActive + HBlank;
    Mode.HFrontPorch = dtd[8] | ((dtd[11]&0xC0)<<2);
    Mode.HSyncWidth = dtd[9] | ((dtd[11]&0x30)<<4);
    Mode.VFrontPorch = ((dtd[10]&0xF0)>>4) | ((dtd[11]&0xC)<<2);
    Mode.VSyncWidth = (dtd[10]&0xF) | ((dtd[11]&0x3)<<4);

    Mode.VActive = dtd[7] & 0xF0; Mode.VActive <<= 4; Mode.VActive |= dtd[5];
    VBlank = dtd[7] & 0xF; VBlank <<= 8; VBlank |= dtd[6];
    Mode.VTotal = Mode.VActive + VBlank;

    Mode.HBackPorch = HBlank - Mode.HFrontPorch - Mode.HSyncWidth ;
    Mode.VBackPorch = VBlank - Mode.VFrontPorch - Mode.VSyncWidth ;


    VF = Mode.PCLK;
    if( Mode.HTotal != 0 && Mode.VTotal != 0 )
    {
        VF /= Mode.HTotal; VF += Mode.VTotal / 2;
        VF /= Mode.VTotal;
    }
    else
    {
        VF = 0;
    }
    interlace = dtd[17]&(1<<7)?TRUE:FALSE;
    Mode.ScanMode = interlace?INTR:PROG ;

    HSize = ((int)dtd[14]&0xF0)<<4 | (int)dtd[12];
    VSize = ((int)dtd[14]&0xF)<<8 | (int)dtd[13];
    PRINTF_EDID(("PCLK = %d.%dMHz  Dimension:(%4dmm x %4dmm)\r\n",Mode.PCLK/1000000,(Mode.PCLK/10000)%100,HSize,VSize));
    PRINTF_EDID(("Mode - %d x %d @%uHz (%dx%d)\r\n", Mode.HActive, Mode.VActive, VF, Mode.HTotal, Mode.VTotal ));
    PRINTF_EDID(("{%4d,%4d,%4d,%4d,%4d,%4d,%4d,%4d,%4d,%4d}",
        Mode.HActive, Mode.VActive, Mode.HTotal, Mode.VTotal,
        Mode.HFrontPorch, Mode.HSyncWidth, Mode.HBackPorch, Mode.VFrontPorch, Mode.VSyncWidth, Mode.VBackPorch));
}

void parseDisplayLimitDescriptor(u8 *descriptor)
{
    u32 maxHrate, maxVrate, minHrate, minVrate;
    u32 maxPCLK;

    switch(descriptor[4] & 3)
    {
    case 3:
        maxVrate = 255;
        minVrate = 255;
        break;
    case 2:
        maxVrate = 255;
        minVrate = 0;
        break;
    default:
        maxVrate = 0;
        minVrate = 0;
        break;
    }
    switch(descriptor[4] & 0xc)
    {
    case 0xC:
        maxHrate = 255;
        minHrate = 255;
        break;
    case 0x8:
        maxHrate = 255;
        minHrate = 0;
        break;
    default:
        maxHrate = 0;
        minHrate = 0;
        break;
    }
    PRINTF_EDID(("Display Range Limit:\r\n"));

    minVrate += (int)descriptor[5];
    maxVrate += (int)descriptor[6];
    minHrate += (int)descriptor[7];
    maxHrate += (int)descriptor[8];

    maxPCLK = ((int)descriptor[9] )*10;

    PRINTF_EDID(("HFreq = %dKHz - %dKHz, VFreq = %dHz - %dHz, maximum PCLK = %dMHz\r\n",minHrate,maxHrate, minVrate,maxVrate,maxPCLK));

}

void parseMonitorDescriptor(u8 *descriptor)
{
    if( descriptor[0] == 0 && descriptor[1] == 0 && descriptor[2] == 0 )
    {
        switch(descriptor[3])
        {
        case 0xFD:
            parseDisplayLimitDescriptor(descriptor);
            break;
        default:
            break;
        }
    }
}

void parseColorCharateristic(unsigned char *pCh)
{
    pCh = pCh;
    #if 0
    unsigned short Rx,Ry,Gx,Gy,Bx,By,Wx,Wy;

    Rx = (unsigned short)(pCh[2]<<2); Rx |= (unsigned short)((pCh[0]&0xC0)>>6);
    Ry = (unsigned short)(pCh[3]<<2); Ry |= (unsigned short)((pCh[0]&0x30)>>4);
    Gx = (unsigned short)(pCh[4]<<2); Gx |= (unsigned short)((pCh[0]&0xC)>>2);
    Gy = (unsigned short)(pCh[5]<<2); Gy |= (unsigned short)((pCh[0]&0x3));
    Bx = (unsigned short)(pCh[6]<<2); Bx |= (unsigned short)((pCh[1]&0xC0)>>6);
    By = (unsigned short)(pCh[7]<<2); By |= (unsigned short)((pCh[1]&0x30)>>4);
    Wx = (unsigned short)(pCh[8]<<2); Wx |= (unsigned short)((pCh[1]&0xC)>>2);
    Wy = (unsigned short)(pCh[9]<<2); Wy |= (unsigned short)((pCh[1]&0x3));
    #endif
}

void parseHDMI1VSDB(u8 VSDB[])
{
    u16 len, i , next_offset=0;
    u1 check3D_Multi = FALSE;
    len = VSDB[0]&0x1F;

    // hdmi 1 oui
    if (iTE6615_DATA.sink.HDMIVer < 1) {
        iTE6615_DATA.sink.HDMIVer = 1;
        iTE6615_DATA.config.EnHDMI = 1;
    }

    #if _ENABLE_CEC_
    iTE6615_DATA.vars.phy_addrH = VSDB[4];
    iTE6615_DATA.vars.phy_addrL = VSDB[5];
    #endif

    PRINTF_EDID(("Physical address %X:%X:%X:%X\r\n",(VSDB[4]>>4)&0xF,(VSDB[4]>>0)&0xF,(VSDB[5]>>4)&0xF,(VSDB[5]>>0)&0xF));
    if( len >= 6 )
    {
        if( VSDB[6] & 0x5 ) PRINTF_EDID(("Error!! VSDB[6] reserved bit is non-zero!! %02X\r\n",VSDB[6]));
        PRINTF_EDID(("deep color support %s%s%s%s%s%s%s\r\n"
            ,(VSDB[6]&(1<<7))?"AI,": ""
            ,(VSDB[6]&(1<<6))?"DC_48bit,": ""
            ,(VSDB[6]&(1<<5))?"DC_36bit,": ""
            ,(VSDB[6]&(1<<4))?"DC_30bit,": ""
            ,(VSDB[6]&(1<<3))?"DC_Y444,": ""
            ,(VSDB[6]&(1<<1))?"Dual DVI": ""
            ,(VSDB[6]&0xF9)?"": "24 bit only"));

        iTE6615_DATA.sink.dc_48bit = (VSDB[6]>>6)&0x01;
        iTE6615_DATA.sink.dc_36bit = (VSDB[6]>>5)&0x01;
        iTE6615_DATA.sink.dc_30bit = (VSDB[6]>>4)&0x01;
        iTE6615_DATA.sink.dc_y444 = (VSDB[6]>>3)&0x01;
    }
    else
    {
        return;
    }
    if( len >= 7)
    {
        PRINTF_EDID(("Max TMDS Clock=%d MHz\r\n",(int)VSDB[7] * 5 ));
        iTE6615_DATA.sink.VSIF1_MAX_TMDS = VSDB[7];
    }
    else
    {
        return;
    }

    if( len >= 8 )
    {
        if( VSDB[8] & 0x20)
        {
            PRINTF_EDID(("VDSB8: HDMI_Video_present!\r\n"));
            if( len < 13 ){ PRINTF_EDID(("HDMI Video present is fail by length is not enough.\r\n")); }
        }
        if( VSDB[8] & 0x80)
        {
            next_offset = 11;
        }
        else
        {
            next_offset = 9;
        }

        if( VSDB[8] & 0x40)
        {
            next_offset += 2;
        }


    }
    if( len >= next_offset )
    {

        if( VSDB[next_offset] & 0x80 )
        {
            PRINTF_EDID(("VSDB[%d] %02X: 3D presents.\r\n",next_offset, VSDB[next_offset]));
            switch(VSDB[next_offset] & 0xE0)
            {
            case 0x80: PRINTF_EDID(("no 3D_multi present.\r\n"));break;
            case 0xA0:
                PRINTF_EDID(("3D_multi present 01. 3D_Struct_ALL present, 3D_MASK not present.\r\n"));
                check3D_Multi = 1;
                break;
            case 0xC0:
                PRINTF_EDID(("3D_multi present 10. 3D_Struct_ALL present, 3D_MASK present.\r\n"));
                check3D_Multi = 2;
                break;
            default: PRINTF_EDID(("no 3D_multi present.\r\n"));break;
            }
        }
        next_offset++;
    }

    while( len >= next_offset )
    {
        u32 HDMI_VIC_LEN;
        u32 HDMI_3D_LEN;
        u32 VIC_ORDER,_3D_Struct;
        u32 idx3D;

        HDMI_VIC_LEN = (VSDB[next_offset] & 0xE0)>>5;
        HDMI_3D_LEN = VSDB[next_offset] & 0x1F;

        PRINTF_EDID(("HDMI_VIC_LEN = %d , HDMI_3D_LEN = %d\r\n",HDMI_VIC_LEN, HDMI_3D_LEN ));

        next_offset++;

        for( i = 0; i < HDMI_VIC_LEN; i++)
        {
            PRINTF_EDID(("HDMI_VIC_%d = %02X ",i,VSDB[next_offset+i]));

            switch(VSDB[next_offset+i])
            {
            case 0: PRINTF_EDID(("Reserved\r\n")); break;
            case 1: PRINTF_EDID(("3840x2160@30Hz\r\n")); break;
            case 2: PRINTF_EDID(("3840x2160@25Hz\r\n")); break;
            case 3: PRINTF_EDID(("3840x2160@24Hz\r\n")); break;
            case 4: PRINTF_EDID(("4096x2160@24Hz\r\n")); break;
            default: PRINTF_EDID(("Invalid VIC Value\r\n"));
            }
        }
        next_offset += HDMI_VIC_LEN;

        //----------------------------------------- todo: check 3d

        if( check3D_Multi )
        {
            PRINTF_EDID(("3D_Struct_All_15..8 = %02X\r\n3D_Struct_All_7..0 = %02X\r\n",VSDB[next_offset],VSDB[next_offset+1]));
            for( i = 0; i < 8; i++ )
            {
                if( VSDB[next_offset+1] & (1<<i))
                {
                    PRINTF_EDID(("3D format is %s\r\n",str3DFormat[i]));
                }
            }
            for( i = 0; i < 8; i++ )
            {
                if( VSDB[next_offset] & (1<<i))
                {
                    PRINTF_EDID(("3D format is %s\r\n",str3DFormat[i+8]));
                }
            }
            next_offset += 2;
            if( HDMI_3D_LEN >= 2 ) HDMI_3D_LEN -= 2;
        }

        if( check3D_Multi == 2 )
        {
            PRINTF_EDID(("3D_mask_15..8 = %02X\r\n3D_mask_7..0 = %02X\r\n",VSDB[next_offset],VSDB[next_offset+1]));
            for( i = 0; i < 8; i++ )
            {
                if( VSDB[next_offset+1] & (1<<i))
                {
                    PRINTF_EDID(("3D Support mode is %s\r\n",strVIC[ModeSupport[i]]));
                }
            }
            for( i = 0; i < 8; i++ )
            {
                if( VSDB[next_offset] & (1<<i))
                {
                    PRINTF_EDID(("3D Support mode is %s\r\n",strVIC[ModeSupport[i+8]]));
                }
            }
            next_offset += 2;
            if( HDMI_3D_LEN >= 2 ) HDMI_3D_LEN -= 2;
        }
        idx3D = 0;
        for( i = 0; i < HDMI_3D_LEN; idx3D++)
        {
            VIC_ORDER = VSDB[next_offset+i] >>4;
            _3D_Struct = VSDB[next_offset+i]&0xF;
            PRINTF_EDID(("2D_VIC_order_%d = %15s,",idx3D,strVIC[ModeSupport[VIC_ORDER]]));
            PRINTF_EDID(("3D_Struct_%d = %s\r\n",idx3D,str3DFormat[_3D_Struct]));

            if( _3D_Struct > 7 )
            {
                if((VSDB[next_offset+i+1]&0xF0) == 0x10 )
                PRINTF_EDID(("3D_Detail_%d = %d\r\n",idx3D,VSDB[next_offset+i+1]>>4));
                i+=2;
            }
            else
            {
                i++;
            }
        }
        next_offset+=HDMI_3D_LEN;
    }

    //----------------------------------------- todo: check 3d

    for( i = next_offset; i<=len; i++ )
    {
        if( VSDB[i] != 0 ) PRINTF_EDID(("Fail! reserved VSDB[%d] %02X is not zero.\r\n",i,VSDB[i]));
    }

}

void parseHDMI2VSDB(u8 VSDB[])
{
    u32 len, i;
    len = VSDB[0]&0x1F;

    // hdmi 2.0 oui
    if (iTE6615_DATA.sink.HDMIVer < 2) {
        iTE6615_DATA.sink.HDMIVer = 2;
        iTE6615_DATA.config.EnHDMI = 1;
    }

    PRINTF_EDID(("Version = %d\r\n",VSDB[4]));
    if( len < 7 ) PRINTF_EDID(("Invalid Len %d\r\n", len));

    iTE6615_DATA.sink.VSIF2_MAX_TMDS = VSDB[5];

    if( VSDB[5] == 0 )
    {
        PRINTF_EDID(("Max TMDS Character Rate is lower than 340MHz.\r\n"));
    }
    else
    {
        PRINTF_EDID(("Max TMDS Character Rate = %dMHz.\r\n", (int)VSDB[5] * 5));
    }
    if( VSDB[6] & 0x80){

        PRINTF_EDID(("Support SCDC functionality.\r\n"));
        iTE6615_DATA.sink.scdc_present = 1;

        if(VSDB[6] & 0x40){
            PRINTF_EDID(("Capable of initiating an SCDC Read Request.\r\n"));
            iTE6615_DATA.sink.rr_capable = 1;
        }
        if( VSDB[6] & 0x08){
            PRINTF_EDID(("Support scrambling for TMDS character Rate or below 340Mcsc.\r\n"));
            iTE6615_DATA.sink.lte_340M_csc_scramble = 1;
        }
    }
    else
    {
        if(VSDB[6] & 0x40) PRINTF_EDID((">Invalid VSDB[6] bit 6 as 1 while bit 7 = 0\r\n"));
        if(VSDB[6] & 0x20) PRINTF_EDID((">Invalid VSDB[6] bit 5 as 1 while bit 7 = 0\r\n"));
    }


    if( VSDB[6] & 0x04){
        PRINTF_EDID(("Supports Receiving 3D independent view signaling in the HF-VSIF.\r\n"));
        iTE6615_DATA.sink.independent_view = 1;
    }

    if( VSDB[6] & 0x02){
        PRINTF_EDID(("Supports Receiving 3D Dual view signaling in the HF-VSIF.\r\n"));
        iTE6615_DATA.sink.daul_view2 = 1;
    }

    if( VSDB[6] & 0x01){
        PRINTF_EDID(("Supports Receiving 3D OSD_Disparity indication in the HF-VSIF.\r\n"));
        iTE6615_DATA.sink.osd_3d_disparity = 1;

    }

    if( VSDB[7] & 0x04){
        PRINTF_EDID(("Support 48 bit Deep Color 4:2:0 Pixel Encoding.\r\n"));
        iTE6615_DATA.sink.dc_48bit_420 = 1;
    }

    if( VSDB[7] & 0x02){
        PRINTF_EDID(("Support 36 bit Deep Color 4:2:0 Pixel Encoding.\r\n"));
        iTE6615_DATA.sink.dc_36bit_420 = 1;
    }


    if( VSDB[7] & 0x01){
        PRINTF_EDID(("Support 30 bit Deep Color 4:2:0 Pixel Encoding.\r\n"));
        iTE6615_DATA.sink.dc_30bit_420 = 1;
    }

    if( VSDB[7] & 0xF8 ){PRINTF_EDID(("Invalid bit %02X is not zero\r\n",VSDB[7]&0xF8 ));}

    for( i = 8; i<=len; i++ )
    {
        if( VSDB[i] != 0 ) PRINTF_EDID(("Fail! reserved VSDB[%d] %02X is not zero.\r\n",i,VSDB[i]));
    }

}

void parseVSDB(u8 VSDB[])
{
    long IEEEOUI;

    IEEEOUI = VSDB[3] << 16;
    IEEEOUI |= VSDB[2] << 8;
    IEEEOUI |= VSDB[1];

    if( IEEEOUI == 0x0c03 )
    {
        parseHDMI1VSDB(VSDB);
    }

    if( IEEEOUI == 0xC45DD8 )
    {
        parseHDMI2VSDB(VSDB);
    }
}

void parseVideoCapabilityBlock(u8 Descriptor[])
{
    //extended tag 0x00
    if( Descriptor[1] != 0x00 ) return ;

    switch(Descriptor[2] & 0x3 )// S_CE
    {
        //case 0: PRINTF_EDID(("CE video format NOT supported over-and-underscan..\r\n")); break ;
        //case 1: PRINTF_EDID(("CE Always Overscanned.\r\n")); break ;
        //case 2: PRINTF_EDID(("CE Always Underscanned.\r\n")); break ;
        //case 3: PRINTF_EDID(("CE supports both over-and-underscan.\r\n"));break ;
    }
    switch(Descriptor[2] & 0xC )// S_IT
    {
        //case 0: PRINTF_EDID(("IT video format NOT supported over-and-underscan..\r\n")); break ;
        //case 4: PRINTF_EDID(("IT Always Overscanned.\r\n")); break ;
        //case 8: PRINTF_EDID(("IT Always Underscanned.\r\n")); break ;
        //case 0xc: PRINTF_EDID(("IT supports both over-and-underscan.\r\n"));break ;
    }
    switch(Descriptor[2] & 0x30 )// S_CE
    {
        //case 0x00: break ;
        //case 0x10: PRINTF_EDID(("Perfer timing Always Overscanned.\r\n")); break ;
        //case 0x20: PRINTF_EDID(("Perfer timing Always Underscanned.\r\n")); break ;
        //case 0x30: PRINTF_EDID(("Perfer timing supports both over-and-underscan.\r\n"));break ;
    }

    if ( (Descriptor[2] & 0x40) ) {
        PRINTF_EDID(("Quantization Range for RGB is Selectable\r\n"));
    }
    if ( (Descriptor[2] & 0x80) ) {
        PRINTF_EDID(("Quantization Range for YCbCr is Selectable\r\n"));
    }

    return ;
}

char *strExtTag05_ColorimetryDataBlock[] ={
    "xvYCC601",
    "xvYCC709",
    "sYCC601",
    "Adobe_YCC601",
    "Adobe_RGB",
    "BT2020_RGB",
    "BT2020_YCC",
    "BT2020_C_YCC",
};

//char *strExtTag05_ColorimetryDataBlock[] ={
//    "xvYCC601 Standard Definition Colorimetry based on IEC 61966-2-4",
//    "xvYCC709 High Definition Colorimetry based on IEC 61966-2-4",
//    "sYCC601 Colorimetry based on IEC 61966-2-1/Amendment 1",
//    "Adobe_YCC601 Colorimetry based on IEC61966-2-5, Annex A",
//    "Adobe_RGB  Colorimetry based on IEC61966-2-5",
//    "BT2020_RGB Colorimetry based on ITU-R BT.2020 RGB",
//    "BT2020_YCC Colorimetry based on ITU-R BT.2020 YCbCr",
//    "BT2020_C_YCC Colorimetry based on ITU-R BT.2020 YcCbcCrc",
//};
//
//char *strExtTag05_Byte4[] ={
//    "MD0: Future metadata profile",
//    "MD1: Future metadata profile",
//    "MD2: Future metadata profile",
//    "MD3: Future metadata profile",
//
//    "DB[4][4] Invalid Bit!!\r\n",
//    "DB[4][5] Invalid Bit!!\r\n",
//    "DB[4][7] Invalid Bit!!\r\n",
//    "DB[4][7] Invalid Bit!!\r\n",
//};

void parseColorDataBlock(u8 Descriptor[])
{
    u32 i ;

    //extended tag 0x05
    if( Descriptor[1] != 0x05 ) return ;

    for( i = 0 ; i < 8 ; i++ )
    {
        if( Descriptor[2] & (1<<i)) PRINTF_EDID(("Support %s\r\n",strExtTag05_ColorimetryDataBlock[i]));
    }
    //for( i = 0 ; i < 8 ; i++ )
    //{
    //    if( Descriptor[3] & (1<<i)) PRINTF_EDID(("Support %s\r\n",strExtTag05_Byte4[i]));
    //}
    return ;
}

void parseVideoFormatPreferenceDataBlock(u8 Descriptor[])
{
    u32 i ,len ;

    //extended tag 0x0D
    if( Descriptor[1] != 0x0D ) return ;

    len = (Descriptor[0] & 0x1f) -1 ;

    PRINTF_EDID(("prefer mode :\r\n"));
    for( i = 0 ;i < len ; i++ )
    {
        if(( Descriptor[2+i] >= 1 )&&( Descriptor[2+i] <= 127 ))
        {
            PRINTF_EDID(("%d-th = %d\r\n",(i+1), Descriptor[2+i]));
        }
        else if(( Descriptor[2+i] >= 129 )&&( Descriptor[2+i] <= 144 ))
        {
            PRINTF_EDID(("%d-th = EDID[%d]\r\n",(i+1), Descriptor[2+i]-129));
        }
    }

    return ;
}

void parseYUV420VideoDataBlock(u8 Descriptor[])
{
    u32 i , len ;

    //extended tag 0x0E
    if( Descriptor[1] != 0x0E ) return ;

    len = (Descriptor[0] & 0x1f) -1 ;

    PRINTF_EDID(("YCbCr4:2:0 only VIC:\r\n"));
    for( i = 0 ; i< len ; i++ )
    {
        PRINTF_EDID(("%d\r\n", Descriptor[2+i]));
    }

    return ;
}

// todo: support mode
void parseYUV420MapDataBlock(u8 Descriptor[])
{
    u32 i , j , k, len ;
    //extended tag 0x0F
    if( Descriptor[1] != 0x0F ) return ;
    len = (Descriptor[0] & 0x1f)-1 ;

    // ModeSupport[VDBCount] = EDID[i+j]&0x7F; VDBCount++;

    if( (len * 8) < VDBCount )
    {
        len *= 8 ;
    }
    else
    {
        len = VDBCount ;
    }

    for(i = 0,k=0 ; i < len ; i+=8 )
    {
        for( j = 0 ;j < 8 ; j++, k++)
        {
            if(Descriptor[2+(i/8)] & (1<<j))
            {
                PRINTF_EDID(("%s support YCbCr4:2:0\r\n",strVIC[ModeSupport[k]]));
            }

        }
    }

    return ;
}

void parseCEAExtensionDescriptor(u8 Descriptor[])
{
    u32 len,i;
    u8 tag;

    tag = Descriptor[0] & 0xE0;
    if( tag != 0xE0 ) return ;

    len = Descriptor[0] & 0x1F;
    tag = Descriptor[1];

    PRINTF_EDID(("Extension Tag = %02X, len = %d\r\n",tag,len));
    for( i = 0; i <= len; i++)
    {
        PRINTF_EDID(("%02X ",Descriptor[i]));
    }
    PRINTF_EDID(("\r\n"));

    switch(tag)
    {
            case 0x00: parseVideoCapabilityBlock(Descriptor); break;
            //case 0x01: parseVendorSpecificVideoDataBlock(Descriptor); break;
            //case 0x02: parseVesaDisplayDeviceDataBlock(Descriptor); break;
            //case 0x03: parseVesaTimingDataBlock(Descriptor); break;
            case 0x05: parseColorDataBlock(Descriptor); break;
            case 0x0D: parseVideoFormatPreferenceDataBlock(Descriptor); break;
            case 0x0E: parseYUV420VideoDataBlock(Descriptor); break;
            case 0x0F: parseYUV420MapDataBlock(Descriptor); break;
            //case 0x11: parseVendorSpecificAudioDataBlock(Descriptor); break;
            //case 0x12: parseHDMI2_3DAudioDataBlock(Descriptor); break;
            //case 0x20: parseInfoframeDataBlock(Descriptor); break;
   }
}

void iTE6615_Parser_EDID_block1( u8 *EDID )
{
    u32 i,j,sum;
    u32 DTDoffset ,offset;
    u32 len,tag;

    PRINTF_EDID(("[EDID Extension Block]\r\n"));

    sum =  checksumEDID(EDID);

    if( sum != 0 )
    {
        PRINTF_EDID(("\r\nCorrect Checksum should be %02X\r\n",(EDID[127] - sum)&0xFF));
    }

    if( EDID[0] != 0x02 && EDID[1] != 0x03 )
    {
        PRINTF_EDID(("Not a CEA861B extension. stop parsing.\r\n")); return ;
    }

    DTDoffset = EDID[2];

    if(EDID[3] & (1<<7) ) PRINTF_EDID(("under scan\r\n"));
    if(EDID[3] & (1<<6) ) PRINTF_EDID(("Audio Support\r\n"));
    if(EDID[3] & (1<<5) ) PRINTF_EDID(("YCbCr 4:4:4 support\r\n"));
    if(EDID[3] & (1<<4) ) PRINTF_EDID(("YCbCr 4:2:2 support\r\n"));
    if(EDID[3] & 0xF ) PRINTF_EDID(("Support %d native format\r\n",EDID[3] & 0xF));

    VDBCount = 0;

    for( offset = 4; offset < DTDoffset; )
    {
        len = EDID[offset] & 0x1F;
        tag = EDID[offset] & 0xE0;
        tag >>= 5;

        switch(tag)
        {
        case 1://audio data block.
            PRINTF_EDID(("Audio data block\r\n"));
            i = offset + 1;
            for( j = 0; j < len; j+= 3 )
            {
                u32 k,ac;
                ac = (EDID[i+j]&0x78)>>3;
                PRINTF_EDID(("Audio code = %X(%s)",ac,strAudioCode[ac]));
                PRINTF_EDID((", %d channel%s: ",(EDID[i+j]&7)+1,((EDID[i+j]&7)>0)?"s": ""));
                for( k = 0; k < 7; k++ )
                {
                    if( EDID[i+j+1] & (1<<k)){ PRINTF_EDID((" %s",strAudFs[k]));}
                }
                PRINTF_EDID(("\r\n"));
                if( ac >=2 && ac <= 8 ) PRINTF_EDID(("Maximum bit rate = %d KHz\r\n",EDID[i+j+2]));

            }
            break;

        case 2://video data block.
            i = offset + 1;
            PRINTF_EDID(("Video Mode:\r\n"));
            for( j = 0; j < len; j++ )
            {
                PRINTF_EDID((" VIC %2d ",EDID[i+j]&0x7F));

                //if(EDID[i+j]&0x80) PRINTF_EDID(("(native)"));
                //PRINTF_EDID(("%s\r\n",strVIC[EDID[i+j]&0x7F]));

                ModeSupport[VDBCount] = EDID[i+j]&0x7F; VDBCount++;
            }
            PRINTF_EDID(("\r\n"));
            break;
        case 3://vendor data block.
            parseVSDB(EDID+offset);
            break;
        case 4://speaker allocation
            i = offset + 1;
            break;
        case 5://VESA DTC data block
            i = offset + 1;
            break;
        case 7://Use Extension tag
            i = offset + 1;
            parseCEAExtensionDescriptor(EDID+offset);
            break;
        }
        offset += len + 1;
    }

    for( i = DTDoffset; i < 128; i+=18)
    {
        if( EDID[i] == 0 && EDID[i+1] == 0 )
        {
        }
        else if( i < 127 )
        {
            parseDTD(EDID+i);
        }
    }
}

void iTE6615_Parser_EDID_block0( u8 *EDID )
{
    u32 i,sum;

    sum =  checksumEDID(EDID);
    if( sum != 0 ) {
        PRINTF_EDID(("Correct Checksum should be %02X\r\n",(EDID[127]-sum)&0xFF));
    }

    if( EDID[0] != 0x00 ||EDID[7] != 0x00 ||
        EDID[1] != 0xFF ||EDID[2] != 0xFF ||EDID[3] != 0xFF ||
        EDID[4] != 0xFF ||EDID[5] != 0xFF ||EDID[6] != 0xFF )
    {
        PRINTF_EDID(("Header is not 00 FF FF FF  FF FF FF 00\r\n"));
    }

    PRINTF_EDID(("EDID v%d.%d\r\n",EDID[0x12],EDID[0x13]));


    if( EDID[0x14] & 0x80 )
    {
        // digital
        PRINTF_EDID(("Monitor Input is Digital.\r\n"));
        if( EDID[0x13] == 0x04 )
        {
            switch(EDID[0x14]&0x70)
            {
            case 0x00: PRINTF_EDID(("Color Bit Depth is undefined\r\n")); break;
            case 0x10: PRINTF_EDID(("6 Bits per Primary Color\r\n")); break;
            case 0x20: PRINTF_EDID(("8 Bits per Primary Color\r\n")); break;
            case 0x30: PRINTF_EDID(("10 Bits per Primary Color\r\n")); break;
            case 0x40: PRINTF_EDID(("12 Bits per Primary Color\r\n")); break;
            case 0x50: PRINTF_EDID(("14 Bits per Primary Color\r\n")); break;
            case 0x60: PRINTF_EDID(("16 Bits per Primary Color\r\n")); break;
            case 0x70: PRINTF_EDID(("Reserved (Do Not Use)\r\n")); break;
            }
            switch(EDID[0x14]&0xF)
            {
            case 0x0: PRINTF_EDID(("Digital Interface is not defined (see Note 4)\r\n")); break;
            case 0x1: PRINTF_EDID(("DVI is supported\r\n")); break;
            case 0x2: PRINTF_EDID(("HDMI-a is supported\r\n")); break;
            case 0x3: PRINTF_EDID(("HDMI-b is supported\r\n")); break;
            case 0x4: PRINTF_EDID(("MDDI is supported\r\n")); break;
            case 0x5: PRINTF_EDID(("DisplayPort is supported\r\n")); break;
            default: PRINTF_EDID(("All remaining values for Bits 3 to 0 are Reserved: Do Not Use\r\n")); break;
            }
        }
    }
    else
    {
        // analog
        PRINTF_EDID(("Monitor Input is Analog.\r\n"));
    }

    if( EDID[0x15] != 0 && EDID[0x16] != 0 )
    {
        PRINTF_EDID(("Screen Size: %ucm x %ucm.\r\n",(unsigned int)EDID[0x15],(unsigned int)EDID[0x16]));
    }
    else
    {
        if( EDID[0x13] == 4 )
        {
            if( EDID[0x15] != 0 )
            {
                PRINTF_EDID(("Screen Aspect Ration is %u:100\r\n",(unsigned int)EDID[0x15]+99));
            }
            else if( EDID[0x16] != 0 )
            {
                PRINTF_EDID(("Screen Aspect Ration is 100:%u\r\n",(unsigned int)EDID[0x16]+99));
            }
            else
            {
                // both zero.
                PRINTF_EDID(("The screen size or aspect ratio are unknown or undefined."));
            }
        }
        else
        {
                PRINTF_EDID(("The system shall make no assumptions regarding the display size.."));
        }
    }

    if( EDID[0x17] == 0xFF )
    {
        PRINTF_EDID(("Unknow Gamma.\r\n"));
    }
    else
    {
        PRINTF_EDID(("Gamma is 1+(%u/100)\r\n", (unsigned int)EDID[0x17] ));
    }

    if( EDID[0x13] == 4 )
    {
        PRINTF_EDID(("EDID v1.4, Feature support:\r\n"));
        if ( (EDID[0x18]&0x80) ) {
            PRINTF_EDID(("Standby\r\n"));
        }
        if ( (EDID[0x18]&0x40) ) {
            PRINTF_EDID(("Suspend\r\n"));
        }
        if ( (EDID[0x18]&0x20) ) {
            PRINTF_EDID(("Active Off/VeryLow Power\r\n"));
        }

        if(EDID[0x14]&0x80)
        {
            switch(EDID[0x18]&0x18)
            {
            case 0x00: PRINTF_EDID(("Color Encoding: RGB 4:4:4\r\n")); break;
            case 0x08: PRINTF_EDID(("Color Encoding: RGB 4:4:4 & YCrCb 4:4:4\r\n")); break;
            case 0x10: PRINTF_EDID(("Color Encoding: RGB 4:4:4 & YCrCb 4:2:2\r\n")); break;
            case 0x18: PRINTF_EDID(("Color Encoding: RGB 4:4:4 & YCrCb 4:4:4 & YCrCb 4:2:2\r\n")); break;
            }
        }
        else
        {
            switch(EDID[0x18]&0x18)
            {
            case 0x00: PRINTF_EDID(("Monochrome Display.\r\n")); break;
            case 0x08: PRINTF_EDID(("RGB Display.\r\n")); break;
            case 0x10: PRINTF_EDID(("Non-RGB Multicolor Display.\r\n")); break;
            case 0x18: PRINTF_EDID(("Undefined Display.\r\n")); break;
            }
        }

        if( EDID[0x18] & 0x4 ) PRINTF_EDID(("Standard Default Color Space, sRGB\r\n"));
        PRINTF_EDID(("%sSupport Prefer timing.\r\n",( EDID[0x18] & 0x2 ) ?"":"Not "));
        PRINTF_EDID(("Display is %scontinuous frequency.\r\n",(EDID[0x18]&1)?"":"non-"));

    }
    else if( EDID[0x13] == 3 )
    {
        PRINTF_EDID(("EDID v1.3, Feature support\r\n"));
        if ( (EDID[0x18]&0x80) ) {
            PRINTF_EDID(("Standby\r\n"));
        }
        if ( (EDID[0x18]&0x40) ) {
            PRINTF_EDID(("Suspend\r\n"));
        }
        if ( (EDID[0x18]&0x20) ) {
            PRINTF_EDID(("Active Off/VeryLow Power\r\n"));
        }

        switch(EDID[0x18]&0x18)
        {
        case 0x00: PRINTF_EDID(("Monochrome Display.\r\n")); break;
        case 0x08: PRINTF_EDID(("RGB Display.\r\n")); break;
        case 0x10: PRINTF_EDID(("Non-RGB Multicolor Display.\r\n")); break;
        case 0x18: PRINTF_EDID(("Undefined Display.\r\n")); break;
        }

        if( EDID[0x18] & 0x4 ) PRINTF_EDID(("Standard Default Color Space, sRGB\r\n"));
        if( EDID[0x18] & 0x2 ) PRINTF_EDID(("Support Prefer timing.\r\n"));
        if( EDID[0x18] & 0x1 ) PRINTF_EDID(("Default GTF Supported.\r\n"));

    }

    parseColorCharateristic(EDID+0x19);

    for( i = 7; i >= 0; i-- )
    {
        if( EDID[0x23] & (1<<i) ) PRINTF_EDID(("established %s\r\n",ES23[i]));
    }
    for( i = 7; i >= 0; i-- )
    {
        if( EDID[0x24] & (1<<i) ) PRINTF_EDID(("established %s\r\n",ES24[i]));
    }
    if( EDID[0x25] & (1<<7) ) PRINTF_EDID(("established 1152x864@75Hz\r\n"));

    for( i = 0; i < 8; i++ )
    {
        u32 width, height, vf;
        PRINTF_EDID(("Standard timing %d: %02X %02X",i,EDID[0x26+i*2],EDID[0x26+i*2+1]));
        if( EDID[0x26+i*2+1] == 0x01 )
        {
            PRINTF_EDID((" - not determined\r\n"));
        }
        else
        {
            width = EDID[0x26+i*2];
            width += 31; width *= 8;
            switch(EDID[0x26+i*2+1] & 0xc0)
            {
            case 0x00:
                height = width / 16; height *=10;
                break;
            case 0x40:
                height = width / 4; height *=3;
                break;
            case 0x80:
                height = width / 5; height *=4;
                break;
            case 0xC0:
                height = width / 16; height *=9;
                break;
            default:
                height = 0;
                break;
            }
            vf = (EDID[0x26+i*2+1]&0x3F) + 60;
            PRINTF_EDID((" - %dx%d@%dHz\r\n",width,height,vf));
        }
    }

    for( i = 0x36; i < (0x36+18*4); i+=18 )
    {
        if( EDID[i] == 0 && EDID[i+1] == 0 ) {
            parseMonitorDescriptor(EDID+i);
        }
        else {
            parseDTD(EDID+i);
        }
    }

}
#else

void iTE6615_Parser_EDID_block0( u8 *EDID )
{
    iTE6615_print_buffer( EDID, 128 );
}

void iTE6615_Parser_EDID_block1( u8 *EDID )
{
    u8 *tmp;
    u8 offset;
    u8 offset_max;
    u8 length;

    iTE6615_print_buffer( EDID, 128 );

    offset = 0;
    tmp = &EDID[offset];

    if (tmp[0] == 0x02 && tmp[1] == 0x03) {
        int cea_type;

        if (tmp[3] & 0x80) {
            PRINTF_EDID(("underscan\r\n"));
        }
        if (tmp[3] & 0x40) {
            PRINTF_EDID(("Audio support\r\n"));
        }
        if (tmp[3] & 0x20) {
            PRINTF_EDID(("YCbCr 444\r\n"));
        }
        if (tmp[3] & 0x10) {
            PRINTF_EDID(("YCbCr 422\r\n"));
        }

        PRINTF_EDID(("%d native DTD\r\n", (int)(tmp[3] & 0xF)));

        // start to parse cea tag
        offset = 4;
        length = (tmp[4] & 0x1F) + 2;
        offset_max = tmp[2];

        for (;;) {

            tmp = &EDID[offset];

            cea_type = (tmp[0] & 0xE0) >> 5;

            PRINTF_EDID(("CEA Type=0x%02X, len=%d\r\n", (int)cea_type, (int)(length - 1)));

            switch (cea_type) {
            case 0x02:
                {
                    int i;
                    for( i=1 ; i<length-1 ; i++ )
                    {
                        PRINTF_EDID(("SVD[%d]=%d\r\n", (int)i, (int)tmp[i]));
                    }
                }
                break;
            case 0x03:
                if (tmp[1] == 0xD8 && tmp[2] == 0x5D && tmp[3] == 0xC4) {
                    // hdmi 2.x VSIF OUI
                    if (iTE6615_DATA.sink.HDMIVer < 2) {
                        iTE6615_DATA.sink.HDMIVer = 2;
                        iTE6615_DATA.config.EnHDMI = 1;
                    }

                    iTE6615_DATA.sink.max_TMDS_clk = tmp[5];

                    if (tmp[6] & 0x80) {
                        iTE6615_DATA.sink.scdc_present = 1;
                    }
                    if (tmp[6] & 0x40) {
                        iTE6615_DATA.sink.rr_capable = 1;
                    }
                    if (tmp[6] & 0x08) {
                        iTE6615_DATA.sink.lte_340M_csc_scramble = 1;
                    }
                    if (tmp[6] & 0x04) {
                        iTE6615_DATA.sink.independent_view = 1;
                    }
                    if (tmp[6] & 0x02) {
                        iTE6615_DATA.sink.daul_view2 = 1;
                    }
                    if (tmp[6] & 0x01) {
                        iTE6615_DATA.sink.osd_3d_disparity = 1;
                    }
                    if (tmp[7] & 0x04) {
                        iTE6615_DATA.sink.dc_48bit_420 = 1;
                    }
                    if (tmp[7] & 0x02) {
                        iTE6615_DATA.sink.dc_36bit_420 = 1;
                    }
                    if (tmp[7] & 0x01) {
                        iTE6615_DATA.sink.dc_30bit_420 = 1;
                    }

                    PRINTF_EDID(("HDMI 2.0 HF_VSDB\r\n"));

                } else if (tmp[1] == 0x03 && tmp[2] == 0x0C && tmp[3] == 0x00) {
                    // hdmi 1 oui
                    if (iTE6615_DATA.sink.HDMIVer < 1) {
                        iTE6615_DATA.sink.HDMIVer = 1;
                        iTE6615_DATA.config.EnHDMI = 1;
                    }
                    iTE6615_DATA.sink.max_TMDS_clk = tmp[7];

                    if (tmp[6] & 0x40) {
                        iTE6615_DATA.sink.dc_48bit = 1;
                    }
                    if (tmp[6] & 0x20) {
                        iTE6615_DATA.sink.dc_36bit = 1;
                    }
                    if (tmp[6] & 0x10) {
                        iTE6615_DATA.sink.dc_30bit = 1;
                    }
                    if (tmp[6] & 0x08) {
                        iTE6615_DATA.sink.dc_y444 = 1;
                    }

                    PRINTF_EDID(("HDMI 1.0 VSDB\r\n"));

                }
                break;

            case 0x07:
                switch (tmp[1]) {
                case 0x0E: // 420 VDB
                    {
                        //int ilen = (tmp[0]&0x1F)-1;

                        iTE6615_DATA.sink.yuv420_svd_or_cmdb = 1;
                    }
                    break;
                case 0x0F: // 420 CMDB
                    {
                        //int ilen = tmp[0]&0x1F;

                        iTE6615_DATA.sink.yuv420_svd_or_cmdb = 1;
                    }

                    break;
                }
                break;

            default:
                break;
            } // switch( cea_type )

            offset += (length - 1);
            length = (tmp[length - 1] & 0x1F) + 2;

            if (offset >= offset_max) {
                break;
            }
        } // for(;;)
    } // if ( tmp[0] == 0x02 && tmp[1] == 0x03 )

}

#endif

