#ifndef _D_FPGA_VOLUME_H_H_
#define _D_FPGA_VOLUME_H_H_
#include "fpga_common.h"

#define _D_FPGA_VOLUME_CHLNUM (8)

/**************************************************************************
 * void FPGA_VOLUME_Get(WV_U16 ethID,WV_U16 ipID,WV_U8 volume )
 * ***********************************************************************/
void FPGA_VOLUME_Get(WV_U16 ethID,WV_U16 ipID,WV_U16 volume[] );


/**************************************************************************
 * void FPGA_VOLUME_Get_dB(WV_U16 ethID,WV_U16 ipID,WV_U8 volume )
 * ***********************************************************************/
void FPGA_VOLUME_Get_dB(WV_U16 ethID, WV_U16 ipID, float volume[]);


/**************************************************************************
 * void FPGA_VOLUME_Open()
 * ************************************************************************/
void FPGA_VOLUME_Open();
/**************************************************************************
 * void FPGA_VOLUME_Close()
 * ************************************************************************/
void FPGA_VOLUME_Close();
#endif
