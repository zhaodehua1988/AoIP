#include "hi_common.h"
#include "his_temperature.h"
#define PMC_BASE   0xF8A23000
#define PERI_PMC10  (PMC_BASE + 0x28)
#define PERI_PMC12  (PMC_BASE + 0x30) 

/****************************************************************************

WV_S32 HIS_TEMP_ReadTemp(WV_S16 * pTemp);

****************************************************************************/

WV_S32 HIS_TEMP_ReadTemp(WV_S16 * pTemp)
{
    WV_U32 regData,regAddr;
    WV_U8  i, j;
    WV_U32 average = 0;
    WV_S32  ret;

    regAddr =  PERI_PMC10;
    regData =  0x07200000; 
    WV_ASSERT_RET(HI_SYS_WriteRegister(regAddr,regData));  

    for (j = 0; j < 2; j++)
    {
         regAddr = PERI_PMC12 + 0x4 * j;
         WV_ASSERT_RET( HI_SYS_ReadRegister(regAddr,&regData)); 
         for (i = 0; i < 4; i++)
         {
            average += ((regData >> (8 * i)) & 0xff);
         }
    }
 

   average = (average / 8);
 
    *pTemp = (average * 180) / 255 - 40; 

    return  WV_SOK;
}




/****************************************************************************

WV_S32 HIS_TEMP_GetTEMP(WV_S32 argc, WV_S8 **argv,WV_S8 *prfBuff)

****************************************************************************/
WV_S32 HIS_TEMP_GetTEMP(WV_S32 argc, WV_S8 **argv, WV_S8 * prfBuff)
{
   WV_S16 temp;

   HIS_TEMP_ReadTemp(&temp);
   prfBuff += sprintf(prfBuff,"\r\ncpu temperature  is  %d \r\n",temp);
   return WV_SOK;
}


/***************************************************************************

WV_S32  HIS_TEMP_Init();

***************************************************************************/
WV_S32  HIS_TEMP_Init()
{

WV_CMD_Register("get","temp"," get cpu temperature",HIS_TEMP_GetTEMP); 
return WV_SOK;
}


/***************************************************************************

WV_S32  HIS_TEMP_DeInit();

***************************************************************************/
WV_S32  HIS_TEMP_DeInit()
{
 
return WV_SOK;
}
