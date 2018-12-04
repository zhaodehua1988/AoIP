#ifndef  _HIS_MODE_SPI_H_H_
#define  _HIS_MODE_SPI_H_H_

#include "wv_common.h" 
WV_S32  HIS_SPI_Init();
WV_S32  HIS_SPI_DeInit();
WV_S32  HIS_SPI_FpgaWd(WV_U16 addr,WV_U16 data);
WV_S32  HIS_SPI_FpgaWd2(WV_U16 addr,WV_U16 data);
WV_S32  HIS_SPI_FpgaRd(WV_U16 addr,WV_U16 *pData);
WV_S32  HIS_SPI_FpgaRd2(WV_U16 addr,WV_U16 *pData);
//WV_S32  HIS_SPI_FpgaRdNum(WV_U16 addr,WV_U16 *pData,WV_U32 dataNum);
WV_S32   HIS_SPI_FpgaReset();

//WV_S32  HIS_SPI_Read(WV_S8 * pbuf,WV_S32 len);
//WV_S32  HIS_SPI_Write(WV_S8 * pbuf,WV_S32 len);

typedef struct{

	union
	{
		struct {
		volatile	WV_U16	frame_number_addr;
		
		
		volatile	WV_U16 splite_start_line_addr;
		volatile	WV_U16 splite_end_line_addr;
		volatile	WV_U16 start_pixel_split1_addr;
		volatile	WV_U16 start_pixel_split2_addr;
		volatile	WV_U16 start_pixel_split3_addr;
		volatile	WV_U16 start_pixel_split4_addr;
		volatile	WV_U16 end_pixel_split1_addr;
		volatile	WV_U16 end_pixel_split2_addr;
		volatile	WV_U16 end_pixel_split3_addr;
		volatile	WV_U16 end_pixel_split4_addr;
		//volatile	WV_U16 overlay_parameter_data_port_addr;
		volatile 	WV_U16	split_start_addr;
		volatile	WV_U16  video_gen_test_addr;
		volatile 	WV_U16	start_addr;
		} S;
		volatile WV_U16 address[14];	
	}ADDR;
	union
	{

		struct {
		  	
		volatile 	WV_U16 frame_number;
		
		
		volatile	WV_U16 splite_start_line;
		volatile	WV_U16 splite_end_line;
		volatile	WV_U16 start_pixel_split1;
		volatile	WV_U16 start_pixel_split2;
		volatile	WV_U16 start_pixel_split3;
		volatile	WV_U16 start_pixel_split4;
		volatile	WV_U16 end_pixel_split1;
		volatile	WV_U16 end_pixel_split2;
		volatile	WV_U16 end_pixel_split3;
		volatile	WV_U16 end_pixel_split4;
		//volatile	WV_U16 overlay_parameter_data_port;
		volatile	WV_U16 split_start;
		volatile	WV_U16 video_gen_test;
		volatile	WV_U16 start;
		} S;
		volatile WV_U16 data[14];
	} DATA;
} OVERLAY_REG;


typedef struct{

	union
	{
		struct {
		
		volatile	WV_U16 freq_l_addr;
		volatile	WV_U16 freq_h_addr;
		volatile	WV_U16 freq_up_addr1;
		volatile	WV_U16 freq_up_addr2;

		volatile	WV_U16 total_pix_addr;
		volatile	WV_U16 total_line_addr;
		volatile	WV_U16 hsync_width_addr;
		volatile	WV_U16 vsync_width_addr;
		volatile	WV_U16 active_hstart_addr;
		volatile	WV_U16 active_hsize_addr;
		volatile	WV_U16 active_vstart_addr;
		volatile	WV_U16 active_vsize_addr;
		//volatile 	WV_U16 test_color_addr;	;
		//volatile 	WV_U16 test_ctrl_addr;
		volatile	WV_U16	ch_en_addr;
		} S;
		volatile WV_U16 address[13];	
	}ADDR;
	union
	{

		struct {
		  	
		
		volatile	WV_U16 freq_l;
		volatile	WV_U16 freq_h;
		volatile	WV_U16 freq_up1;
		volatile	WV_U16 freq_up2;

		volatile	WV_U16 total_pix;
		volatile	WV_U16 total_line;
		volatile	WV_U16 hsync_width;
		volatile	WV_U16 vsync_width;
		volatile	WV_U16 active_hstart;
		volatile	WV_U16 active_hsize;
		volatile	WV_U16 active_vstart;
		volatile	WV_U16 active_vsize;
		//volatile 	WV_U16 test_color;	;
		//volatile 	WV_U16 test_ctrl;
		volatile	WV_U16	ch_en;
		} S;
		volatile WV_U16 data[13];
	} DATA;
} OUTPUT_REG;
typedef union
{
/*
	struct{
		volatile	WV_U64 	video_channel :1;
		volatile	WV_U64	reserved      :3;
		volatile 	WV_U64	outline_channel:4;
		volatile	WV_U64	linenumber     :12;
		volatile	WV_U64  pixelnumber    :12;
		volatile	WV_U64	reserved2      :8;
		volatile	WV_U64  yoffset	       :12;
		volatile	WV_U64  xoffset	       :12;
	}BITS;
*/
	struct{
		volatile	WV_U64  xoffset	       :12;
		volatile	WV_U64  yoffset	       :12;
		volatile	WV_U64	reserved2      :8;
		volatile	WV_U64  pixelnumber    :12;
		volatile	WV_U64	linenumber     :12;
		volatile 	WV_U64	outline_channel:4;
		volatile	WV_U64	reserved      :3;
		volatile	WV_U64 	video_channel :1;
	}BITS;
	volatile	WV_U16  parameter[4];

}   OVERLAY_PARAMETER;
#endif
