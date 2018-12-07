#include "ti1297_reg.h"
#include "ti1297_IO.h"
#define TI1297_DEV_ADDRESS_1  0x5a
#define TI1297_DEV_ADDRESS_2  0x60
#define TI1297_DEV_ADDRESS_3  0x72
#define TI1297_DEV_ADDRESS_4  0x78

typedef struct
{
    WV_U8 ucAddr;
    WV_U8 andmask;
    WV_U8 ucValue;
} TI1297_INI_REG;

TI1297_INI_REG  TI1297_INIT_TABLE[] = {

	{0xFE ,0xFF ,0x32 }, //set SDI output power down
	{0xFF ,0xFF ,0x00 }, //set to CTLE-CDR Registers
	{0x00 ,0xFF ,0x00 }, //Reset CTLE/CDR Registers
    {0x01 ,0xFF ,0x03 },
    {0x24 ,0xFF ,0x02 }, //default 0x00 
    {0x31 ,0xFF ,0x20 }, //default 0x00 
    {0xFF ,0xFF, 0x01 }, //set to Config IO Registers
    {0x00 ,0x08, 0x08 },
    {0x60 ,0xFF, 0xC1 },
    {0xFE ,0xFF, 0x00 }, //set SDI ouput power up
    {0xAA ,0xFF, 0x00 }, //0xAA: end of table
};


WV_S32  ti1297_SetReg( WV_U8 devAddr,WV_U8  offset, WV_U8  mask, WV_U8  ucdata )
{
    WV_U8  temp;
    i2c_1297_read_byte(devAddr,offset,&temp);
    temp = (temp&((~mask)&0xFF))+(mask&ucdata);
    i2c_1297_write_byte(devAddr,offset,temp);
    return  WV_SOK;
}
/******************************************************
 * WV_S32 ti1297_reg_init(TI1297_INI_REG *pTable)
 * ***************************************************/
void ti1297_Config(TI1297_INI_REG *tdata)
{

    printf("1297 table init start..\n");
    WV_U8 cnt = 0;
    while(tdata[cnt].ucAddr != 0xAA)
    {
        ti1297_SetReg(TI1297_DEV_ADDRESS_1,tdata[cnt].ucAddr,tdata[cnt].andmask,tdata[cnt].ucValue);
        cnt++;
    }
    printf("1297 table init end..\n");
}


/******************************************************
 * WV_S32 ti1297_reg_init(TI1297_INI_REG *pTable)
 * ***************************************************/
WV_S32 ti1297_reg_init()
{
    ti1297_Config(TI1297_INIT_TABLE);
    return WV_SOK;
}
