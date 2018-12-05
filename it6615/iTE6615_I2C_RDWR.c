///*****************************************
//  Copyright (C) 2009-2018
//  ITE Tech. Inc. All Rights Reserved
//  Proprietary and Confidential
///*****************************************
//   @file   <iTE6615_I2C_RDWR.c>
//   @author Kuro.Chung@ite.com.tw
//   @date   2018/07/04
//   @fileversion: iTE6615_MCUSRC_0.23
//******************************************/
#include "iTE6615_Global.h"
#include "iTE6615_IO.h"
extern iTE6615_GlobalDATA iTE6615_DATA;

// IO.h for MCU , include for auto define those two function , otherwise redefine it for WIN32
#ifdef WIN32

#endif

#define HDMI_DEV      0

// RX

u8 lvrxrd( u8 RegAddr)
{
    u8 FLAG;
    u8 mDataIn;

    FLAG = i2c_6615_read_byte(ADDR_LVDSRX, RegAddr, 1, &mDataIn, HDMI_DEV);

    if(FLAG==0)
    {
        REG_PRINTF(("=====HDMI Read ERROR Read Reg0x%X=\r\n",(int) RegAddr));
    }
    return mDataIn;
}

u8 lvrxbrd( u8 RegAddr, u8 RegBytes, u8 *buffer)
{
    u8 FLAG;

    FLAG = i2c_6615_read_byte(ADDR_LVDSRX, RegAddr, RegBytes, buffer, HDMI_DEV);

    if(FLAG==0)
    {
        REG_PRINTF(("HDMI Read ERROR !!!"));
        REG_PRINTF(("=====  Read Reg0x%X=\r\n",(int) RegAddr));
    }
    return FLAG;
}

u8 lvrxwr( u8 RegAddr,u8 DataIn)
{

    u8 flag;
    flag= i2c_6615_write_byte(ADDR_LVDSRX, RegAddr, 1, &DataIn, HDMI_DEV);
    if(iTE6615_DATA.DumpREG == TRUE)
    {
        REG_PRINTF(("i2c_single_write(0x90, 0x%02X, 1, 0x%02X);\r\n",(int)RegAddr,(int)DataIn));
    }

    if(flag==0)
    {
        REG_PRINTF(("=====HDMI I2C ERROR Write Reg0x%X=%X =====\r\n",(int)RegAddr,(int)DataIn));
    }
    return !flag;
}

u8  lvrxset( u8  offset, u8  mask, u8  ucdata )
{
    u8  temp;
    temp = lvrxrd(offset);
    temp = (temp&((~mask)&0xFF))+(mask&ucdata);
    return lvrxwr(offset, temp);
}

void lvrxbwr( u8 offset, u8 byteno, u8 *rddata )
{
    u8 flag, i;
    if( byteno>0 )
    {
        if(iTE6615_DATA.DumpREG == TRUE)
        {
            REG_PRINTF(("KK====  Write byteno=%X =====\r\n",(int) byteno));
            for(i = 0 ; i < byteno ; i++)
            {
                REG_PRINTF(("KK====  Write Reg0x1 %X=%X =====\r\n",(int)offset+i,(int)rddata[i]));
            }
        }
        flag= i2c_6615_write_byte(ADDR_LVDSRX, offset, byteno, rddata, HDMI_DEV);
    }

    if(flag==0)
    {
        REG_PRINTF(("=====IT6805 I2C ERROR Write Reg0x%X=%X =====\r\n",(int)offset,(int)rddata));
    }
}


// TX
u8 hdmitxrd( u8 RegAddr)
{
    u8 FLAG;
    u8 mDataIn;

    FLAG = i2c_6615_read_byte(ADDR_HDMITX, RegAddr, 1, &mDataIn, HDMI_DEV);

    if(FLAG==0)
    {
        REG_PRINTF(("=====HDMI Read ERROR Read Reg0x%X=\r\n",(int) RegAddr));
    }
    return mDataIn;
}

u8 hdmitxbrd( u8 RegAddr, u8 RegBytes, u8 *buffer)
{
    u8 FLAG;

    FLAG = i2c_6615_read_byte(ADDR_HDMITX, RegAddr, RegBytes, buffer, HDMI_DEV);

    if(FLAG==0)
    {
        REG_PRINTF(("HDMI Read ERROR !!!"));
        REG_PRINTF(("=====  Read Reg0x%X=\r\n",(int) RegAddr));
    }
    return FLAG;
}

u8 hdmitxwr( u8 RegAddr,u8 DataIn)
{

    u8 flag;
    flag= i2c_6615_write_byte(ADDR_HDMITX, RegAddr, 1, &DataIn, HDMI_DEV);
    if(iTE6615_DATA.DumpREG == TRUE)
    {
        REG_PRINTF(("pccmd w %02X %02X 98;\r\n", RegAddr, DataIn));  //wmh
    }

    if(flag==0)
    {
        REG_PRINTF(("=====HDMI I2C ERROR Write Reg0x%X=%X =====\r\n",(int)RegAddr,(int)DataIn));
    }
    return !flag;
}

u8  hdmitxset( u8  offset, u8  mask, u8  ucdata )
{
    u8  temp;
    temp = hdmitxrd(offset);
    temp = (temp&((~mask)&0xFF))+(mask&ucdata);
    return hdmitxwr(offset, temp);
}

void hdmitxbwr( u8 offset, u8 byteno, u8 *rddata )
{
    u8 flag, i;
    if( byteno>0 )
    {
        if(iTE6615_DATA.DumpREG == TRUE)
        {
            REG_PRINTF(("KK====  Write byteno=%X =====\r\n",(int) byteno));
            for(i = 0 ; i < byteno ; i++)
            {
                REG_PRINTF(("KK====  Write Reg0x1 %X=%X =====\r\n",(int)offset+i,(int)rddata[i]));
            }
        }
        flag= i2c_6615_write_byte(ADDR_HDMITX, offset, byteno, rddata, HDMI_DEV);
    }

    if(flag==0)
    {
        REG_PRINTF(("=====IT6805 I2C ERROR Write Reg0x%X=%X =====\r\n",(int)offset,(int)rddata));
    }
}



u8 cec6615rd(u8 RegAddr)
{
    u8  p_data;
    u1    FLAG;

    FLAG = i2c_6615_read_byte(ADDR6615_CEC, RegAddr, 1, &p_data, iTE6615CECGPIOid);

    if (FLAG == 0)
    {
        CEC_DEBUG_PRINTF(("iTE6805_CEC I2C ERROR !!!"));
        CEC_DEBUG_PRINTF(("=====  Read Reg0x%X=\r\n",(int) RegAddr));
    }
    return p_data;
}

u8 cec6615brd( u8 offset, u8 byteno, u8 *rddata )
{
    u8 FLAG;

    FLAG = i2c_6615_read_byte(ADDR6615_CEC, offset, byteno, rddata, HDMI_DEV);

    if(FLAG==0)
    {
        REG_PRINTF(("iTE6805_CEC I2C ERROR !!!"));
        REG_PRINTF(("=====  Read Reg0x%X=\r\n",(int) offset));
    }
    return FLAG;
}


u8 cec6615wr(u8 offset, u8 buffer)
{
    u8  flag;
    flag = i2c_6615_write_byte(ADDR6615_CEC, offset, 1, &buffer, iTE6615CECGPIOid);

    return !flag;
}

void cec6615bwr( u8 offset, u8 byteno, u8 *rddata )
{
    if( byteno>0 )
    i2c_6615_write_byte(ADDR6615_CEC, offset, byteno, rddata, HDMI_DEV);
}

u8 cec6615set( u8 offset, u8 mask, u8 ucdata )
{
    u8 temp;
    temp = cec6615rd(offset);
    temp = (temp&((~mask)&0xFF))+(mask&ucdata);
    return cec6615wr(offset, temp);
}
