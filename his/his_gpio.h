#ifndef  _HIS_GPIO_H_H_
#define  _HIS_GPIO_H_H_
#include "wv_common.h" 
WV_S32  HIS_IO_Init();
WV_S32  HIS_IO_DeInit();
WV_S32  HIS_IO_usbPw(WV_U8  ena);
WV_S32  HIS_IO_usbCur(WV_U8 *over);
WV_S32  HIS_IO_UartSel( WV_U32  num); 
#endif
