#include "hi_unf_gpio.h" 
#include "his_gpio.h" 

#define  HIS_PINMUX_BASEADDR    0XF8A21000
#define  HIS_PINMUX_IIC2SCL     0X07C
#define  HIS_PINMUX_IIC2SDA     0X080

#define  HIS_PINMUX_IIS0BCLK    0X154
#define  HIS_PINMUX_IIS0WS      0X158
#define  HIS_PINMUX_IIS0DO      0X15C
#define  HIS_PINMUX_IIS0MCLK    0X160
#define  HIS_PINMUX_IIS0DI      0X164 

#define  HIS_PINMUX_SPISCLK      0X13C
#define  HIS_PINMUX_SPISDO       0X140
#define  HIS_PINMUX_SPISDI       0X144
#define  HIS_PINMUX_SPICS        0X148
 
#define HIS_GPIO_USB_ENA	 	6*8+7
#define HIS_GPIO_USB_CUR	 	6*8+6
 
/*************************************************************************************
WV_S32  HIS_IO_PinInit();
*************************************************************************************/
WV_S32  HIS_IO_PinInit()
{
 WV_U32  regAdr;
 WV_U32  regVal;
	//conf IIC2
	 
	 // mode[2..0]-- 0 : PWM  4:IIC2SCL  6:GPIO_2_6 
	//SR[8] --0:no SR  1:SR ctrl
	//DSx[10..9] 0: 4mA  1:3mA :2:1mA  3:1mA  
	//PD[12]  --0: no PD  1:enable PD
	
	regAdr = HIS_PINMUX_BASEADDR + HIS_PINMUX_IIC2SCL;
	regVal = 0x304;
	WV_ASSERT_RET( HI_SYS_WriteRegister(regAdr,regVal));  
    //WV_printf("set reg 0x%08x[0x%08x]\r\n",regAdr,regVal);

	// mode[2..0]-- 0 : PWM  4:IIC2SCL  6:GPIO_2_6 
	//SR[8] --0:no SR  1:SR ctrl
	//DSx[10..9] 0: 4mA  1:3mA :2:1mA  3:1mA  
	//PD[12]  --0: no PD  1:enable PD
    regAdr = HIS_PINMUX_BASEADDR + HIS_PINMUX_IIC2SDA;
	regVal = 0x304; 
	WV_ASSERT_RET( HI_SYS_WriteRegister(regAdr,regVal));
	//WV_printf("set reg 0x%08x[0x%08x]\r\n",regAdr,regVal);
	
//conf IIS  GPIO_6_3  -- GPIO_6_7  conf for GPIO  
    // mode[0]-- 0 : GPIO_6_3  1:I2S0_BCLK
	//SR[8] --0:no SR  1:SR ctrl
	//DSx[10..9] 0: 4mA  1:3mA :2:1mA  3:1mA  
	//PD[12]  --0: no PD  1:enable PD
	regAdr = HIS_PINMUX_BASEADDR + HIS_PINMUX_IIS0BCLK ;
	regVal = 0x50;  
	WV_ASSERT_RET(HI_SYS_WriteRegister(regAdr,regVal));
   // WV_printf("set reg 0x%08x[0x%08x]\r\n",regAdr,regVal);
	 
    // mode[0]-- 0 : GPIO_6_4  1:I2S0_WS
	//SR[8] --0:no SR  1:SR ctrl
	//DSx[10..9] 0: 4mA  1:3mA :2:1mA  3:1mA  
	//PD[12]  --0: no PD  1:enable PD
	regAdr = HIS_PINMUX_BASEADDR + HIS_PINMUX_IIS0WS;
	regVal = 0x70; // no PD : 
	WV_ASSERT_RET(HI_SYS_WriteRegister(regAdr,regVal));
	//WV_printf("set reg 0x%08x[0x%08x]\r\n",regAdr,regVal);
	
 
	 // mode[0]-- 0 : GPIO_6_5  1:I2S0_DO
	//SR[8] --0:no SR  1:SR ctrl
	//DSx[10..9] 0: 4mA  1:3mA :2:1mA  3:1mA  
	//PD[12]  --0: no PD  1:enable PD
	regAdr = HIS_PINMUX_BASEADDR + HIS_PINMUX_IIS0DO;
	regVal = 0x70;  
	WV_ASSERT_RET(HI_SYS_WriteRegister(regAdr,regVal));
	//WV_printf("set reg 0x%08x[0x%08x]\r\n",regAdr,regVal);
	
	//GPIO_6_6 
    // mode[0]-- 0 : GPIO_6_6  1:I2S0_MCLK
	//SR[8] --0:no SR  1:SR ctrl
	//DSx[10..9] 0: 4mA  1:3mA :2:1mA  3:1mA  
	//PD[12]  --0: no PD  1:enable PD
	regAdr = HIS_PINMUX_BASEADDR + HIS_PINMUX_IIS0MCLK;
	regVal = 0x30; 
	WV_ASSERT_RET(HI_SYS_WriteRegister(regAdr,regVal));
	//WV_printf("set reg 0x%08x[0x%08x]\r\n",regAdr,regVal);
	
 
	 // mode[0]-- 0 : GPIO_6_7  1:I2S0_DI
	//SR[8] --0:no SR  1:SR ctrl
	//DSx[10..9] 0: 4mA  1:3mA :2:1mA  3:1mA  
	//PD[12]  --0: no PD  1:enable PD
	regAdr = HIS_PINMUX_BASEADDR + HIS_PINMUX_IIS0DI;
    regVal = 0x70; 
	WV_ASSERT_RET(HI_SYS_WriteRegister(regAdr,regVal));
   // WV_printf("set reg 0x%08x[0x%08x]\r\n",regAdr,regVal);
 
//spi 
 
    //1:SPI_SCLK
     // mode[1..0]--0:GPIO_0_3  1:SPI_SCLK  2:UART2_CTSN
	//SR[8] --0:no SR  1:SR ctrl
	//DSx[10..9] 0: 4mA  1:3mA :2:1mA  3:1mA  
	//PD[12]  --0: no PD  1:enable PD
	regAdr = HIS_PINMUX_BASEADDR + HIS_PINMUX_SPISCLK;
    regVal = 0x51; 
	WV_ASSERT_RET(HI_SYS_WriteRegister(regAdr,regVal));
   // WV_printf("set reg 0x%08x[0x%08x]\r\n",regAdr,regVal);
    //1:SPI_SDO
    // mode[1..0]--0:GPIO_4_0  1:SPI_SDO   2:：UART2_RXD
	//SR[8] --0:no SR  1:SR ctrl
	//DSx[10..9] 0: 4mA  1:3mA :2:1mA  3:1mA  
	//PD[12]  --0: no PD  1:enable PD
	regAdr = HIS_PINMUX_BASEADDR + HIS_PINMUX_SPISDO;
    regVal = 0x71;  
	WV_ASSERT_RET(HI_SYS_WriteRegister(regAdr,regVal));
   // WV_printf("set reg 0x%08x[0x%08x]\r\n",regAdr,regVal);
    //1:SPI_SPI_SDI
    // mode[1..0]--0:GPIO_4_1  1:SPI_SDI  2:UART2_TXD 3:I2C1_SCL
	//SR[8] --0:no SR  1:SR ctrl
	//DSx[10..9] 0: 4mA  1:3mA :2:1mA  3:1mA  
	//PD[12]  --0: no PD  1:enable PD
	regAdr = HIS_PINMUX_BASEADDR + HIS_PINMUX_SPISDI;
	regVal = 0x71;
	WV_ASSERT_RET(HI_SYS_WriteRegister(regAdr,regVal));
    //WV_printf("set reg 0x%08x[0x%08x]\r\n",regAdr,regVal);
    // 1:SPI_CSN0 
    // mode[1..0]--0:GPIO_6_0  1:SPI_CSN0  2:UART2_RTSN 3:I2C1_SDA
	//SR[8] --0:no SR  1:SR ctrl
	//DSx[10..9] 0: 4mA  1:3mA :2:1mA  3:1mA  
	//PD[12]  --0: no PD  1:enable PD;
	regAdr = HIS_PINMUX_BASEADDR + HIS_PINMUX_SPICS;
	regVal = 0x71;
	WV_ASSERT_RET(HI_SYS_WriteRegister(regAdr,regVal));
    //WV_printf("set reg 0x%08x[0x%08x]\r\n",regAdr,regVal);
   //set CPIO6_6  1  ena usb Power
   
   
   return WV_SOK;
}

 


/*********************************************************************************************

WV_S32  HIS_IO_usbPw(WV_U8  ena)

*********************************************************************************************/
WV_S32  HIS_IO_usbPw(WV_U8  ena)
{
  WV_S32  ret =0;
  if(ena == 0)
  {
    WV_RET_ADD(HI_UNF_GPIO_WriteBit(HIS_GPIO_USB_ENA, HI_FALSE),ret);
  }
  else
  {
    WV_RET_ADD(HI_UNF_GPIO_WriteBit(HIS_GPIO_USB_ENA, HI_TRUE),ret);
  }  
  return ret;
}


/*********************************************************************************************

WV_S32  HIS_IO_usbCur(WV_U8 *over)

*********************************************************************************************/
WV_S32  HIS_IO_usbCur(WV_U8 *over)
{
   WV_S32  ret =0; 
    HI_BOOL bHighVolt = HI_FALSE; 
    WV_RET_ADD(HI_UNF_GPIO_ReadBit(HIS_GPIO_USB_CUR, &bHighVolt),ret); 
    if(bHighVolt == HI_FALSE)
      {
       *over = 0; 
      }
     else
     {
      *over = 1; 
     } 
      
  return ret;
}

/*********************************************************************************************

WV_S32  HIS_IO_Init()

*********************************************************************************************/
WV_S32  HIS_IO_Init()
{
  WV_S32  ret =0;
  WV_RET_ADD(HIS_IO_PinInit(),ret);
  WV_RET_ADD(HI_UNF_GPIO_Init (),ret);
  WV_RET_ADD(HI_UNF_GPIO_SetDirBit (HIS_GPIO_USB_ENA, HI_FALSE),ret); // GPIO_6_6 output 
  WV_RET_ADD(HI_UNF_GPIO_SetDirBit (HIS_GPIO_USB_CUR, HI_TRUE),ret); // GPIO_6_7 input  
  WV_RET_ADD(HIS_IO_usbPw(1),ret); 
  
  return ret;
}

/*********************************************************************************************

WV_S32  HIS_IO_DeInit()

*********************************************************************************************/
WV_S32  HIS_IO_DeInit()
{
  WV_S32  ret =0;
  WV_RET_ADD(HI_UNF_GPIO_DeInit (),ret);
  
  
  return ret;
}

/*********************************************************************************************

WV_S32  HIS_IO_UartSel( WV_U32  num)

*********************************************************************************************/

WV_S32  HIS_IO_UartSel( WV_U32  num)
{
  WV_S32  ret =0;
  WV_U32  regAdr;
  WV_U32  regVal;
 
	regAdr = 0xf8000044;
    WV_ASSERT_RET( HI_SYS_ReadRegister(regAdr,&regVal)); 
    if(num == 0)
    {
     regVal &=0xfffffffc;  
    }
    else
    {
     regVal |=0x3;   
    }
     
	WV_ASSERT_RET( HI_SYS_WriteRegister(regAdr,regVal));  
  
  return ret;
}
