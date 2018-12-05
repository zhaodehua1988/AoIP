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
#include "iTE6805_IO.h"
//#include "his_iic.h"
#include "PCA9548A.h"
iTE_u1 i2c_6805_write_byte(iTE_u8 address, iTE_u8 offset, iTE_u8 byteno, iTE_u8 *p_data, iTE_u8 device)
{

    int i;
    int flag;
    iTE_u1 ret = 1; //true
    for(i=0;i<byteno;i++){
        
        //flag = HIS_IIC_Write(2, address,offset+i, p_data[i]);
        flag = PCA9548_IIC_Write(PCA9548A_IIC_SWID_68051_HDMI_IN,address,offset+i, p_data[i]);
        if(flag != 0) { 
            ret =0 ; //false
            break;
        }   
    }
    return ret;
}

iTE_u1 i2c_6805_read_byte(iTE_u8 address, iTE_u8 offset, iTE_u8 byteno, iTE_u8 *p_data, iTE_u8 device)
{
    int i;
    int flag;
    iTE_u1 ret = 1;//true
    for(i=0;i<byteno;i++){
        //flag = HIS_IIC_Read(2, address,offset+i, &p_data[i]);
        flag = PCA9548_IIC_Read(PCA9548A_IIC_SWID_68051_HDMI_IN,address,offset+i, &p_data[i]);
        if(flag != 0 ){
            ret = 0;//false
            break; 
        }    
    }
    return ret;
}

