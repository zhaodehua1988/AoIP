///*****************************************
//  Copyright (C) 2009-2018
//  ITE Tech. Inc. All Rights Reserved
//  Proprietary and Confidential
///*****************************************
//   @file   <iTE6615_IO.c>
//   @author Kuro.Chung@ite.com.tw
//   @date   2018/07/04
//   @fileversion: iTE6615_MCUSRC_0.23
//******************************************/
#include "iTE6615_IO.h"
#include "PCA9548A.h"
u1 i2c_6615_write_byte(u8 address, u8 offset, u8 byteno, u8 *p_data, u8 device)
{

    int i;
    int flag;
    u1 ret = 1;
    for(i=0;i<byteno;i++){
        
        //flag = HIS_IIC_Write(2, address,offset+i, p_data[i]);
        flag = PCA9548_IIC_Write(PCA9548A_IIC_SWID_6615_HDMI_OUT,address,offset+i, p_data[i]);
        if(flag != 0) { 
            ret =0 ;
            break;
        }   
    }
    return ret;
}

u1 i2c_6615_read_byte(u8 address, u8 offset, u8 byteno, u8 *p_data, u8 device)
{
    int i;
    int flag;
    u1 ret = 1;
    for(i=0;i<byteno;i++){
        //flag = HIS_IIC_Read(2, address,offset+i, &p_data[i]);
        flag = PCA9548_IIC_Read(PCA9548A_IIC_SWID_6615_HDMI_OUT,address,offset+i, &p_data[i]);
        if(flag != 0 ){
            ret = 0;
            break; 
        }    
    }
    return ret;
}

