///*****************************************
//  Copyright (C) 2009-2017
//  ITE Tech. Inc. All Rights Reserved
//  Proprietary and Confidential
///*****************************************
//   @file   <iTE6805_I2C_RDWR.c>
//   @author Kuro.Chung@ite.com.tw
//   @date   2017/09/15
//   @fileversion: iTE6805_MCUSRC_1.02
//******************************************/
#include "iTE6805_Global.h"

extern _iTE6805_DATA iTE6805_DATA;

#define HDMI_DEV  	0

iTE_u1 edid_ram_wr(iTE_u8 offset,iTE_u8 byteno,iTE_u8 *p_data )
{
	iTE_u16 i = 0;
	iTE_u8 flag;
	if(iTE6805_DATA.DumpREG == TRUE)
	{
		for(i = 0 ; i < byteno ; i++)
		{
			REG_PRINTF(("i2c_single_write(0xA8, 0x%02X, 1, 0x%02X); \n",(int)offset+i,(int)p_data[i]));
		}
	}

    flag = i2c_6805_write_byte(ADDR_EDID, offset, byteno, p_data, HDMI_DEV);
    return !flag;
}



iTE_u8  lvdsrxset( iTE_u8  offset, iTE_u8  mask, iTE_u8  ucdata )
{
    iTE_u8 temp, FLAG;
	FLAG = i2c_6805_read_byte(ADDR_LVDS, offset, 1, &temp, HDMI_DEV);
    if(FLAG==0)
    {
	    REG_PRINTF(("===== LVDS Read ERROR Read Reg0x%X=\n",(int) offset));
    }
    //temp = hdmirxrd(offset);
    temp = (temp&((~mask)&0xFF))+(mask&ucdata);
    return hdmirxwr(offset, temp);
}

iTE_u8 lvdsrxwr( iTE_u8 RegAddr,iTE_u8 DataIn)
{

    iTE_u8 flag;
    flag= i2c_6805_write_byte(ADDR_LVDS, RegAddr, 1, &DataIn, HDMI_DEV);
	if(iTE6805_DATA.DumpREG == TRUE)
	{
		REG_PRINTF(("KK====  Write Reg0x%X=%X =====\n",(int)RegAddr,(int)DataIn));
	}

    if(flag==0)
    {
	    REG_PRINTF(("===== HDMI I2C ERROR Write Reg0x%X=%X Error=====\n",(int)RegAddr,(int)DataIn));
    }
    return !flag;
}

iTE_u8 hdmirxrd( iTE_u8 RegAddr)
{
    iTE_u8 FLAG;
    iTE_u8 mDataIn;

    FLAG = i2c_6805_read_byte(ADDR_HDMI, RegAddr, 1, &mDataIn, HDMI_DEV);

    if(FLAG==0)
    {
	    REG_PRINTF(("=====HDMI Read ERROR Read Reg0x%X=\n",(int) RegAddr));
    }
    return mDataIn;
}

iTE_u8 hdmirxbrd( iTE_u8 RegAddr, iTE_u8 RegBytes, iTE_u8 *buffer)
{
    iTE_u8 FLAG;

    FLAG = i2c_6805_read_byte(ADDR_HDMI, RegAddr, RegBytes, buffer, HDMI_DEV);

    if(FLAG==0)
    {
	    REG_PRINTF(("HDMI Read ERROR !!!"));
	    REG_PRINTF(("=====  Read Reg0x%X=\n",(int) RegAddr));
    }
    return FLAG;
}

iTE_u8 hdmirxwr( iTE_u8 RegAddr,iTE_u8 DataIn)
{

    iTE_u8 flag;
    flag= i2c_6805_write_byte(ADDR_HDMI, RegAddr, 1, &DataIn, HDMI_DEV);
	if(iTE6805_DATA.DumpREG == TRUE)
	{
		REG_PRINTF(("i2c_single_write(0x90, 0x%02X, 1, 0x%02X);\n",(int)RegAddr,(int)DataIn));
	}

    if(flag==0)
    {
	    REG_PRINTF(("=====HDMI I2C ERROR Write Reg0x%X=%X =====\n",(int)RegAddr,(int)DataIn));
    }
    return !flag;
}

iTE_u8  hdmirxset( iTE_u8  offset, iTE_u8  mask, iTE_u8  ucdata )
{
    iTE_u8  temp;
    temp = hdmirxrd(offset);
    temp = (temp&((~mask)&0xFF))+(mask&ucdata);
    return hdmirxwr(offset, temp);
}

void hdmirxbwr( iTE_u8 offset, iTE_u8 byteno, iTE_u8 *rddata )
{
    iTE_u8 flag, i;
    if( byteno>0 )
	{
		if(iTE6805_DATA.DumpREG == TRUE)
		{
			REG_PRINTF(("KK====  Write byteno=%X =====\n",(int) byteno));
			for(i = 0 ; i < byteno ; i++)
			{
				REG_PRINTF(("KK====  Write Reg0x1 %X=%X =====\n",(int)offset+i,(int)rddata[i]));
			}
		}
		flag= i2c_6805_write_byte(ADDR_HDMI, offset, byteno, rddata, HDMI_DEV);
	}

    if(flag==0)
    {
	    REG_PRINTF(("=====IT6805 I2C ERROR Write Reg0x%X=%X =====\n",(int)offset,(int)rddata));
    }
}

void hdimrx_write_init(iTE6805_INI_REG _CODE *tdata)
{
    iTE_u16 cnt = 0;
    while(tdata[cnt].ucAddr != 0xFF)
    {
        hdmirxset(tdata[cnt].ucAddr,tdata[cnt].andmask,tdata[cnt].ucValue);
        cnt++;
    }
}

#ifdef _ENABLE_IT6805_MHL_I2C_

void mhlrx_write_init(iTE6805_INI_REG _CODE *tdata)
{
    iTE_u16 cnt = 0;
    while(tdata[cnt].ucAddr != 0xFF)
    {
        mhlrxset(tdata[cnt].ucAddr,tdata[cnt].andmask,tdata[cnt].ucValue);
        cnt++;
    }
}

iTE_u8 mhlrxrd( iTE_u8 offset )
{
    iTE_u8	mDataIn;
    iTE_u8	FLAG;
    FLAG=i2c_6805_read_byte(ADDR_MHL, offset, 1, &mDataIn, HDMI_DEV);
    if(FLAG==0)
    {
		REG_PRINTF(("=====MHL I2C ERROR read Reg0x%X=%X =====\n",(int)offset,(int)mDataIn));
    }
    return mDataIn;
}

iTE_u8 mhlrxwr( iTE_u8 offset, iTE_u8 ucdata )
{
    iTE_u8  flag;
    flag= i2c_6805_write_byte(ADDR_MHL, offset, 1, &ucdata, HDMI_DEV);
	if(iTE6805_DATA.DumpREG == TRUE)
	{
		REG_PRINTF(("KK==== MHLWrite Reg0x%X=%X =====\n",(int)offset,(int)ucdata));
	}

    if(flag==0)
    {
		REG_PRINTF(("=====MHL I2C ERROR Write Reg0x%X=%X =====\n",(int)offset,(int)ucdata));
    }
    return !flag;
}

iTE_u8 mhlrxset( iTE_u8 offset, iTE_u8 mask, iTE_u8 ucdata )
{
    iTE_u8 temp;
    temp = mhlrxrd(offset);
    temp = (temp&((~mask)&0xFF))+(mask&ucdata);
    return mhlrxwr(offset, temp);
}

void mhlrxbrd( iTE_u8 offset, iTE_u8 byteno, iTE_u8 *rddata )
{
    if( byteno>0 )
    i2c_6805_read_byte(ADDR_MHL, offset, byteno, rddata, HDMI_DEV);
}

void mhlrxbwr( iTE_u8 offset, iTE_u8 byteno, iTE_u8 *rddata )
{
	iTE_u8 i;
    if( byteno>0 )
	{
		if(iTE6805_DATA.DumpREG == TRUE)
		{
			REG_PRINTF(("KK== MHL Write byteno=%X =====\n",(int) byteno));
			for(i = 0 ; i < byteno ; i++)
			{
				REG_PRINTF(("KK== MHL Write Reg0x %X=%X =====\n",(int)offset+i,(int)rddata[i]));
			}
		}
		i2c_6805_write_byte(ADDR_MHL, offset, byteno, rddata, HDMI_DEV);
	}

}
#endif



#ifdef _ENABLE_IT6805_CEC_I2C_
iTE_u8 cecrd(iTE_u8 RegAddr)
{
	iTE_u8  p_data;
	iTE_u1	FLAG;

	FLAG = i2c_6805_read_byte(ADDR_CEC, RegAddr, 1, &p_data, iTE6805CECGPIOid);

	if (FLAG == 0)
	{
		CEC_DEBUG_PRINTF(("iTE6805_CEC I2C ERROR !!!"));
		CEC_DEBUG_PRINTF(("=====  Read Reg0x%X=\n",(int) RegAddr));
	}
	return p_data;
}

iTE_u8 cecbrd( iTE_u8 offset, iTE_u8 byteno, iTE_u8 *rddata )
{
    iTE_u8 FLAG;

    FLAG = i2c_6805_read_byte(ADDR_CEC, offset, byteno, rddata, HDMI_DEV);

    if(FLAG==0)
    {
	    REG_PRINTF(("iTE6805_CEC I2C ERROR !!!"));
	    REG_PRINTF(("=====  Read Reg0x%X=\n",(int) offset));
    }
	return FLAG;
}


iTE_u1 cecwr(iTE_u8 offset, iTE_u8 buffer)
{
	iTE_u1  flag;

	flag = i2c_6805_write_byte(ADDR_CEC, offset, 1, &buffer, iTE6805CECGPIOid);

	return !flag;
}

void cecbwr( iTE_u8 offset, iTE_u8 byteno, iTE_u8 *rddata )
{
    if( byteno>0 )
    i2c_6805_write_byte(ADDR_CEC, offset, byteno, rddata, HDMI_DEV);
}

iTE_u8 cecset( iTE_u8 offset, iTE_u8 mask, iTE_u8 ucdata )
{
    iTE_u8 temp;
    temp = cecrd(offset);
    temp = (temp&((~mask)&0xFF))+(mask&ucdata);
    return cecwr(offset, temp);
}
#endif
