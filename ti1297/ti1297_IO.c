

#include "PCA9548A.h"
#include "ti1297_IO.h"
/********************************************************************
 * WV_S32 i2c_1297_write_byte(WV_U8 devAddr, WV_U8 regAddr,  WV_U8 data)
 * *****************************************************************/
WV_S32 i2c_1297_write_byte(WV_U8 devAddr, WV_U8 regAddr,  WV_U8 data)
{
    return  PCA9548_IIC_Write(PCA9548A_IIC_SWID_1297,devAddr,regAddr,data);
}
/********************************************************************
 * WV_S32 i2c_1297_read_byte(WV_U8 devAddr, WV_U8 regAddr,  WV_U8 data)
 * *****************************************************************/
WV_S32 i2c_1297_read_byte(WV_U8 devAddr, WV_U8 regAddr,  WV_U8 *data)
{
    return  PCA9548_IIC_Read(PCA9548A_IIC_SWID_1297,devAddr,regAddr,data);
}

