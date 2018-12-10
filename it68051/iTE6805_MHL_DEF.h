///*****************************************
//  Copyright (C) 2009-2017
//  ITE Tech. Inc. All Rights Reserved
//  Proprietary and Confidential
///*****************************************
//   @file   <iTE6805_MHL_DEF.h>
//   @author Kuro.Chung@ite.com.tw
//   @date   2017/09/15
//   @fileversion: iTE6805_MCUSRC_1.02
//******************************************/
#ifndef _iTE6805_MHL_DEF_H_H_
#define _iTE6805_MHL_DEF_H_H_
#define WAITABORTNUM  			  	(3000)//(300)			//change from 3000 to 5000 for ATC 6.3.6.5


typedef struct _MSC_WB_INFO{
	iTE_u8	*TX_Scrpad;
	iTE_u8	Count;
	iTE_u8	Offset;
}MSC_WB_INFO;


typedef enum _MHL_INT{
	MHL_INT_3D_REQ = 0x10,
	MHL_INT_GNT_WRT = 0x08,
	MHL_INT_REQ_WRT = 0x04,
	MHL_INT_DSCR_CHG = 0x02,
}MHL_INT;

typedef enum _MHL_STS{
	MHL_STS_DCAP_RDY = 0x01,
	MHL_STS_MUTED = 0x10,
	MHL_STS_PATH_EN = 0x08,
	MHL_STS_CLK_24 = 0x03,
	MHL_STS_CLK_PP = 0x02
}MHL_STS;

typedef enum _MSC_MODE{
	MSC_FW_MODE,
	MSC_HW_MODE,
	MSC_WRITE_BURST_MODE
}MSC_MODE;

typedef struct _MSC_CMD{
	iTE_u8	ucOffset;
	iTE_u8	ucValue;
	iTE_u8	ucCnt;
}MSC_CMD, *P_MSC_CMD;


typedef enum _MHL3D_STATE{
	MHL3D_REQ_START,
	MHL3D_REQ_WRT,
	MHL3D_GNT_WRT,
	MHL3D_WRT_BURST,
	MHL3D_REQ_DONE
}MHL3D_STATE;

typedef enum _MSC_PKT_ID{
	MSC_PKT_WRITE_STAT = 0,
	MSC_PKT_SET_INT,
	MSC_PKT_READ_DEVCAP,
	MSC_PKT_GET_STATE,
	MSC_PKT_GET_VENDOR_ID,
	MSC_PKT_SET_HPD,
	MSC_PKT_CLR_HPD,
	MSC_PKT_MSC_MSG,
	MSC_PKT_GET_DDC_ERRORCODE,
	MSC_PKT_GET_MSC_ERRORCODE,
	MSC_PKT_WRITE_BURST,
	MSC_PKT_GET_SC1_ERRORCODE,
	MSC_PKT_GET_SC3_ERRORCODE,
	MSC_PKT_EOF,
	MSC_PKT_ACK,
	MSC_PKT_NACK,
	MSC_PKT_ABORT,
	MSC_PKT_ID_END
}MSC_PKT_ID;


typedef struct {
	iTE_u8		MSC_MSG_TX_DATA[2];
	MSC_WB_INFO	MSC_TX_WB;
}_iTE6805_MHL;


#define MHLVer	(0x21)    // V1.2=0x12, V2.0=0x20

// define in MHL spec page.95
#define MSG_PKT_MSGE	0x02
#define MSG_PKT_RCP 	0x10
#define MSG_PKT_RCPK	0x11
#define MSG_PKT_RCPE	0x12
#define MSG_PKT_RAP 	0x20
#define MSG_PKT_RAPK	0x21
#define MSG_PKT_UCP		0x30
#define MSG_PKT_UCPK	0x31
#define MSG_PKT_UCPE	0x32


// define in MHL spec
#define MSG_PKT_RCPE_NO_ERROR			0x00
#define MSG_PKT_RCPE_INEFFECTIVE_KEY	0x01
#define MSG_PKT_RCPE_BUSY				0x02

#define MSG_PKT_RAPK_NO_ERROR				0x00
#define MSG_PKT_RAPK_UNRECOGN_ACTION_CODE	0x01
#define MSG_PKT_RAPK_UNSUPPORT_ACTION_CODE	0x02
#define MSG_PKT_RAPK_RESPONDER_BUSY			0x03

#define MSG_PKT_UCPE_NO_ERROR		0x00
#define MSG_PKT_UCPE_INEFFECTIVE_KEY	0x01

#define MSG_PKT_RAP_POLL		0x00
#define MSG_PKT_RAP_CONTENT_ON	0x10
#define MSG_PKT_RAP_CONTENT_OFF	0x11

#define MSC_NO_ERROR		0x00
#define MSC_INVALID_CODE	0x01

// MSC Fire, then need wait RX status, this define for RX status
#define	MSC_SUCCESS	0
#define	MSC_FAIL		1
#define	MSC_RX_ABORT	2
#define	MSC_RX_RCVABORT	3
#define	MSC_RX_RCVNACK	4
#define	MSC_RX_ARBLOSE	5

/*-----------RD Init Option-----------*/
// Debug Mode
#define EnCBusDbgMode  FALSE
#define MSCCBusDbgCtrl FALSE
#define DDCCBusDbgCtrl FALSE

#define EnPPGBSwap	TRUE
#define PPHDCPOpt   TRUE
#define PPHDCPOpt2  TRUE
#define T10usSrcSel TRUE	//FALSE: 100ms calibration , TRUR: 27MHz Crystal(only IT6802)
#define CDDGSel		3

//Discovery
#define Reg5VStableTSel      3 //0-3
#define RegWKpulseLAdj       FALSE
#define RegWKpulseSAdj       FALSE
#define RegWKFailThr         2 //0-3
#define RegDisvOpt           FALSE // must false in ACT
#define RegAutoWakeUpFailTrg TRUE
#define RegEnWakeUpFailTog   FALSE
#define RegEnWakeUpFailFlt   TRUE
#define RegWakeUpFailTogSel  1 //0-3
// CBUS INput Option
#define EnCBusDeGlitch TRUE

#define EnMSCBurstWr    TRUE
#define MSCBurstWrID    TRUE   // TRUE: from MHL5E/MHL5F
#define MSCBurstWrOpt   FALSE  // TRUE: Not write Adopter ID into ScratchPad
#define EnPktFIFOBurst  TRUE
#define EnMSCHwRty      FALSE
#define EnHWPathEn      FALSE
#define MSCRxUCP2Nack   TRUE
// MHL CTS
#define RegBurstWrTOSel 2		//0: 320ms, 1: 340ms, 2: 360ms   (ATC)
#define Reg100msTOAdj	2		//00: 100ms, 01: 99ms, 10: 101ms (ATC)

// Link Layer Option
#define DisCBusBurst FALSE	// TRUE: for CBUS single PKT mode
#define AckHigh      0x0B
#define AckLow       0x01

// DDC Option
#define EnDDCSendAbort TRUE	// Send ABORT after segment write with EOF

// CBUS Capability
#define MHLVersion   0x22 	// V1.2=0x12, V2.0=0x20
#define DEV_TYPE	(0x01)	//Sink:0x01, Source:0x02, Dongle:0x03
#define POW			(0x01)
#define PLIM		(0x01)	//500mA:0x00, 900mA:0x01, 1500mA:0x10
#define DEV_CAT		((PLIM<<5)|(POW<<4)|(DEV_TYPE))
#define ADOPTER_ID_H 0x02	// !!!!!!!!!!!!!!!!!!!! need to using IT ADOPTER ID !!!!!!!!!!!!!!!!!!!!
#define ADOPTER_ID_L 0x45	// !!!!!!!!!!!!!!!!!!!! need to using IT ADOPTER ID !!!!!!!!!!!!!!!!!!!!

#define SUPP_VGA					(TRUE)
#define SUPP_ISLANDS				(TRUE)
#define SUPP_PPIXEL					(TRUE)
#define SUPP_YCBCR422				(TRUE)
#define SUPP_YCBCR444				(TRUE)
#define SUPP_RGB444					(TRUE)
#define SUPP_VID_LINK				((SUPP_VGA<<5)|(SUPP_ISLANDS<<4)|(SUPP_PPIXEL<<3)|(SUPP_YCBCR422<<2)|(SUPP_YCBCR444<<1)|(SUPP_RGB444))

#define AUD_8CH						(TRUE)
#define AUD_2CH						(TRUE)
#define AUD_CH						((AUD_8CH<<1)|(AUD_2CH))

#define SUPP_VT						(FALSE)
#define VT_GAME						(FALSE)
#define VT_CINEMA					(FALSE)
#define VT_PHOTO					(FALSE)
#define VT_GRAPHICS					(FALSE)
#define VIDEO_TYPE					((SUPP_VT<<7)|(VT_GAME<<3)|(VT_CINEMA<<2)|(VT_PHOTO<<1)|(VT_GRAPHICS))


// define in MHL spec page119.
#define LD_GUI						(FALSE)
#define LD_SPEAKE					(TRUE)
#define LD_RECORD					(FALSE)
#define LD_TUNER					(FALSE)
#define LD_MEDIA					(FALSE)
#define LD_AUDIO					(FALSE)
#define LD_VIDEO					(FALSE)
#define LD_DISPLAY					(TRUE)
#define LOG_DEV_MAP					((LD_GUI<<7)|(LD_SPEAKE<<6)|(LD_RECORD<<5)|(LD_TUNER<<4)|(LD_MEDIA<<3)||(LD_AUDIO<<2)|(LD_VIDEO<<1)|(LD_DISPLAY))

#define BANDWIDTH					(0x0F)

#define UCP_RECV_SUPP				(TRUE)
#define UCP_SEND_SUPP				(TRUE)
#define SP_SUPPORT					(TRUE)
#define RAP_SUPPORT					(TRUE)
#define RCP_SUPPORT					(TRUE)
#define FEATURE_FLAG				((UCP_RECV_SUPP<<4)|(UCP_SEND_SUPP<<3)|(SP_SUPPORT<<2)|(RAP_SUPPORT<<1)|(RCP_SUPPORT))

#define DEVICE_ID_H  0x68
#define DEVICE_ID_L  0x05

#define SCRATCHPAD_SIZE				(0x10)
#define INT_STA_SIZE				(0x33)
/*-----------RD Init Option-----------*/

#endif
