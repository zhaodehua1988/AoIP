///*****************************************
//  Copyright (C) 2009-2017
//  ITE Tech. Inc. All Rights Reserved
//  Proprietary and Confidential
///*****************************************
//   @file   <iTE6805_CEC_DEFINE.h>
//   @author Kuro.Chung@ite.com.tw
//   @date   2017/09/15
//   @fileversion: iTE6805_MCUSRC_1.02
//******************************************/
#ifndef _ITE6805_CEC_DEFINE_H_H
#define  _ITE6805_CEC_DEFINE_H_H

#include "iTE6805_typedef.h"
#define CEC_MAX_MSG_LEN		(16)	/* 16 blocks */

#define CEC_BCAST_ADDR		(0x0f)
#define CEC_ADDR_MAX		CEC_BCAST_ADDR

#define TX_DONE 1
#define TX_FAIL 0

#define TX_FAIL_RECEIVE_ACK		1
#define TX_FAIL_RECEIVE_NACK	2
#define TX_FAIL_RETRY			4

#define	REG_FOLLOWER_RX_NUM	0x42

#define CEC_DEV_VENDOR_ID_0		(0x00)
#define CEC_DEV_VENDOR_ID_1		(0x00)
#define CEC_DEV_VENDOR_ID_2		(0x00)

#define CEC_QUEUE_SIZE	(0x01 << 6)
#define CEC_QUEUE_MAX	(CEC_QUEUE_SIZE - 1)

#define CEC_FRAME_SIZE 21
typedef union {
	struct
	{
		iTE_u8	SIZE;
		iTE_u8	HEADER;
		iTE_u8	OPCODE;
		iTE_u8	OPERAND1;
		iTE_u8	OPERAND2;
		iTE_u8	OPERAND3;
		iTE_u8	OPERAND4;
		iTE_u8	OPERAND5;
		iTE_u8	OPERAND6;
		iTE_u8	OPERAND7;
		iTE_u8	OPERAND8;
		iTE_u8	OPERAND9;
		iTE_u8	OPERAND10;
		iTE_u8	OPERAND11;
		iTE_u8	OPERAND12;
		iTE_u8	OPERAND13;
		iTE_u8	OPERAND14;
		iTE_u8	OPERAND15;
		iTE_u8	OPERAND16;
		iTE_u8	Follower;
		iTE_u8	Initiator;
	}id;
	iTE_u8	 array[21];

}CEC_FRAME, *pCEC_FRAME;


typedef struct{
	iTE_u8	Rptr;
	iTE_u8	Wptr;
	iTE_u8	Queue[CEC_QUEUE_SIZE];
}CEC_QUEUE;

typedef struct {
	iTE_u8		MY_LA;
	iTE_u8		MY_PA_LOW, MY_PA_HIGH;
	CEC_QUEUE	Rx_Queue;	// for Receive Message
	CEC_QUEUE	Tx_Queue;	// for Receive Message
	iTE_u8		Rx_TmpHeader[3];
	CEC_FRAME	CEC_FRAME_RX, CEC_FRAME_TX;
	iTE_u8		Tx_QueueFull;
	iTE_u8		Tx_CECDone;
	iTE_u8		Tx_CECFire;
	iTE_u8		Tx_CECInitDone;
}_iTE6805_CEC;


 typedef struct {
    iTE_u8	PhyicalAddr1;
    iTE_u8	PhyicalAddr2;
    iTE_u8	Active;
 }CEC_Device, *pCEC_Device;

/* The logical addresses defined by CEC 2.0 */
#define CEC_LOG_ADDR_TV                 0
#define CEC_LOG_ADDR_RECORD_1           1
#define CEC_LOG_ADDR_RECORD_2           2
#define CEC_LOG_ADDR_TUNER_1            3
#define CEC_LOG_ADDR_PLAYBACK_1         4
#define CEC_LOG_ADDR_AUDIOSYSTEM        5
#define CEC_LOG_ADDR_TUNER_2            6
#define CEC_LOG_ADDR_TUNER_3            7
#define CEC_LOG_ADDR_PLAYBACK_2         8
#define CEC_LOG_ADDR_RECORD_3           9
#define CEC_LOG_ADDR_TUNER_4            10
#define CEC_LOG_ADDR_PLAYBACK_3         11
#define CEC_LOG_ADDR_BACKUP_1           12
#define CEC_LOG_ADDR_BACKUP_2           13
#define CEC_LOG_ADDR_SPECIFIC           14
#define CEC_LOG_ADDR_UNREGISTERED       15 /* as initiator address */
#define CEC_LOG_ADDR_BROADCAST          15 /* as destination address */


//typedef enum#endif
//{
	//OP_FeaturOP_Abort = 0x00,

	// One Touch Play Feature
	//OP_ActiveSource = 0x82,	// follower:TV, switch	--> Broadcst , Directly address
	//OP_ImageViewOn	= 0x04,	// follower:TV, switch	--> Broadcst
	//OP_TextViewOn	= 0x0D,	// follower:TV

	// Routing Control Feature
	//OP_RoutingChange 		= 0x80,	// follower:TV, switch	--> Broadcst
	//OP_RoutingInformation 	= 0x81,	// follower:switch		--> Broadcst
	//OP_RequestActiveSource	= 0x85,	// follower:switch		--> Broadcst 	, AVR to request active source when Audio ocntrol feature is active
	//OP_SetStreamPath		= 0x86,	// follower:switch		--> Broadcst
	//OP_InactiveSource		= 0x9D,	//
	//OP_StandBy				= 0x36,	// follower:All			--> Broadcst

	// System Information Feature
	//OP_CECVersioin			= 0x9E,
	//OP_GetCECVersion		= 0x9F,
	//OP_GivePhysicalAddress	= 0x83,	// follower:All
	//OP_GetMenuLanguage		= 0x91,	// follower:TV
	//OP_ReportPhysicalAddress= 0x84,	// follower:TV
	//OP_SetMenuLanguage		= 0x32,	// follower:All,		Initiator:TV

	// Vendor Specific Commands Feature
	//OP_DeviceVendorID		= 0x87,
	//OP_GiveDeviceVendorID	= 0x8C,
	//OP_VendorCommand		= 0x89,
	//OP_GiveDevicePowerStatus= 0x8F,
	//OP_ReportPowerStatus	= 0x90,
	//OP_VendorCommandWithID	= 0xA0,

	// other
	//OP_Play						= 0x41,
	//OP_UserPressed				= 0x44,
	//OP_UserReleased				= 0x45,
	//OP_AudioModeRequest			= 0x70,
	//OP_GiveAudioStatus			= 0x71,
	//OP_SetSystemAudioMode		= 0x72,
	//OP_ReportAudioStatus		= 0x7A,
	//OP_GiveSystemAudIoModeStatus= 0x7D,
	//OP_SystemAudIoModeStatus	= 0x7E,
	//OP_RemoteButtonDown 		= 0x8A,
	//OP_RemoteButtonUp			= 0x8B,
	//OP_ReportAudioDescriptor	= 0xA3,
	//OP_RequestAudioDescriptor	= 0xA4,

	// ACR 6805 do not support ACR anymore
	//OP_InitiateARC				= 0xC0,
	//OP_ReportARCInitiated		= 0xC1,
	//OP_ReportARCTerminated		= 0xC2,
	//OP_RequestARCInitiation		= 0xC3,
	//OP_RequestARCTermination	= 0xC4,
	//OP_TerminateARC				= 0xC5,

	// HEC
	//OP_CDC = 0xF8,

	// Polling Mesage
	//OP_PollingMessage,
	//OP_Abort = 0xff,
	//OP_CECNone,

//} CEC_OP;

typedef enum {
    Msg_Directly=0,
    Msg_Broadcast=1,
    Msg_Both=2,
}MessageType;

struct cec_enum_values {
	char *type_name;
	iTE_u8 value;
};


//typedef struct {
//    CEC_OP		OPCODE;
//    MessageType MSG_Type;
//    iTE_u8		SIZE;
//} CEC_CMD;


#endif
