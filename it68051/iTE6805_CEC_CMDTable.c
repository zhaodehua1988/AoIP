///*****************************************
//  Copyright (C) 2009-2017
//  ITE Tech. Inc. All Rights Reserved
//  Proprietary and Confidential
///*****************************************
//   @file   <iTE6805_CEC_CMDTable.c>
//   @author Kuro.Chung@ite.com.tw
//   @date   2017/09/15
//   @fileversion: iTE6805_MCUSRC_1.02
//******************************************/

#include "iTE6805_CEC_FETURE.h"
#if 0
#define DIRECTED	0x80
#define BCAST1_4	0x40
#define BCAST2_0	0x20	/* broadcast only allowed for >= 2.0 */
#define BCAST		(BCAST1_4 | BCAST2_0)
#define BOTH		(BCAST | DIRECTED)

/*
 * Specify minimum length and whether the message is directed, broadcast
 * or both. Messages that do not match the criteria are ignored as per
 * the CEC specification.
 */
iTE_u8 cec_msg_size[256] = {0};

void iTE6805_INIT_MSG_CHECK_TABLE()
{
	cec_msg_size[CEC_MSG_ACTIVE_SOURCE] = 4 | BCAST;
	cec_msg_size[CEC_MSG_IMAGE_VIEW_ON] = 2 | DIRECTED;
	cec_msg_size[CEC_MSG_TEXT_VIEW_ON] = 2 | DIRECTED;
	cec_msg_size[CEC_MSG_INACTIVE_SOURCE] = 4 | DIRECTED;
	cec_msg_size[CEC_MSG_REQUEST_ACTIVE_SOURCE] = 2 | BCAST;
	cec_msg_size[CEC_MSG_ROUTING_CHANGE] = 6 | BCAST;
	cec_msg_size[CEC_MSG_ROUTING_INFORMATION] = 4 | BCAST;
	cec_msg_size[CEC_MSG_SET_STREAM_PATH] = 4 | BCAST;
	cec_msg_size[CEC_MSG_STANDBY] = 2 | BOTH;
	cec_msg_size[CEC_MSG_RECORD_OFF] = 2 | DIRECTED;
	cec_msg_size[CEC_MSG_RECORD_ON] = 3 | DIRECTED;
	cec_msg_size[CEC_MSG_RECORD_STATUS] = 3 | DIRECTED;
	cec_msg_size[CEC_MSG_RECORD_TV_SCREEN] = 2 | DIRECTED;
	cec_msg_size[CEC_MSG_CLEAR_ANALOGUE_TIMER] = 13 | DIRECTED;
	cec_msg_size[CEC_MSG_CLEAR_DIGITAL_TIMER] = 16 | DIRECTED;
	cec_msg_size[CEC_MSG_CLEAR_EXT_TIMER] = 13 | DIRECTED;
	cec_msg_size[CEC_MSG_SET_ANALOGUE_TIMER] = 13 | DIRECTED;
	cec_msg_size[CEC_MSG_SET_DIGITAL_TIMER] = 16 | DIRECTED;
	cec_msg_size[CEC_MSG_SET_EXT_TIMER] = 13 | DIRECTED;
	cec_msg_size[CEC_MSG_SET_TIMER_PROGRAM_TITLE] = 2 | DIRECTED;
	cec_msg_size[CEC_MSG_TIMER_CLEARED_STATUS] = 3 | DIRECTED;
	cec_msg_size[CEC_MSG_TIMER_STATUS] = 3 | DIRECTED;
	cec_msg_size[CEC_MSG_CEC_VERSION] = 3 | DIRECTED;
	cec_msg_size[CEC_MSG_GET_CEC_VERSION] = 2 | DIRECTED;
	cec_msg_size[CEC_MSG_GIVE_PHYSICAL_ADDR] = 2 | DIRECTED;
	cec_msg_size[CEC_MSG_GET_MENU_LANGUAGE] = 2 | DIRECTED;
	cec_msg_size[CEC_MSG_REPORT_PHYSICAL_ADDR] = 5 | BCAST;
	cec_msg_size[CEC_MSG_SET_MENU_LANGUAGE] = 5 | BCAST;
	cec_msg_size[CEC_MSG_REPORT_FEATURES] = 6 | BCAST;
	cec_msg_size[CEC_MSG_GIVE_FEATURES] = 2 | DIRECTED;
	cec_msg_size[CEC_MSG_DECK_CONTROL] = 3 | DIRECTED;
	cec_msg_size[CEC_MSG_DECK_STATUS] = 3 | DIRECTED;
	cec_msg_size[CEC_MSG_GIVE_DECK_STATUS] = 3 | DIRECTED;
	cec_msg_size[CEC_MSG_PLAY] = 3 | DIRECTED;
	cec_msg_size[CEC_MSG_GIVE_TUNER_DEVICE_STATUS] = 3 | DIRECTED;
	cec_msg_size[CEC_MSG_SELECT_ANALOGUE_SERVICE] = 6 | DIRECTED;
	cec_msg_size[CEC_MSG_SELECT_DIGITAL_SERVICE] = 9 | DIRECTED;
	cec_msg_size[CEC_MSG_TUNER_DEVICE_STATUS] = 7 | DIRECTED;
	cec_msg_size[CEC_MSG_TUNER_STEP_DECREMENT] = 2 | DIRECTED;
	cec_msg_size[CEC_MSG_TUNER_STEP_INCREMENT] = 2 | DIRECTED;
	cec_msg_size[CEC_MSG_DEVICE_VENDOR_ID] = 5 | BCAST;
	cec_msg_size[CEC_MSG_GIVE_DEVICE_VENDOR_ID] = 2 | DIRECTED;
	cec_msg_size[CEC_MSG_VENDOR_COMMAND] = 2 | DIRECTED;
	cec_msg_size[CEC_MSG_VENDOR_COMMAND_WITH_ID] = 5 | BOTH;
	cec_msg_size[CEC_MSG_VENDOR_REMOTE_BUTTON_DOWN] = 2 | BOTH;
	cec_msg_size[CEC_MSG_VENDOR_REMOTE_BUTTON_UP] = 2 | BOTH;
	cec_msg_size[CEC_MSG_SET_OSD_STRING] = 3 | DIRECTED;
	cec_msg_size[CEC_MSG_GIVE_OSD_NAME] = 2 | DIRECTED;
	cec_msg_size[CEC_MSG_SET_OSD_NAME] = 2 | DIRECTED;
	cec_msg_size[CEC_MSG_MENU_REQUEST] = 3 | DIRECTED;
	cec_msg_size[CEC_MSG_MENU_STATUS] = 3 | DIRECTED;
	cec_msg_size[CEC_MSG_USER_CONTROL_PRESSED] = 3 | DIRECTED;
	cec_msg_size[CEC_MSG_USER_CONTROL_RELEASED] = 2 | DIRECTED;
	cec_msg_size[CEC_MSG_GIVE_DEVICE_POWER_STATUS] = 2 | DIRECTED;
	cec_msg_size[CEC_MSG_REPORT_POWER_STATUS] = 3 | DIRECTED | BCAST2_0;
	cec_msg_size[CEC_MSG_FEATURE_ABORT] = 4 | DIRECTED;
	cec_msg_size[CEC_MSG_ABORT] = 2 | DIRECTED;
	cec_msg_size[CEC_MSG_GIVE_AUDIO_STATUS] = 2 | DIRECTED;
	cec_msg_size[CEC_MSG_GIVE_SYSTEM_AUDIO_MODE_STATUS] = 2 | DIRECTED;
	cec_msg_size[CEC_MSG_REPORT_AUDIO_STATUS] = 3 | DIRECTED;
	cec_msg_size[CEC_MSG_REPORT_SHORT_AUDIO_DESCRIPTOR] = 2 | DIRECTED;
	cec_msg_size[CEC_MSG_REQUEST_SHORT_AUDIO_DESCRIPTOR] = 2 | DIRECTED;
	cec_msg_size[CEC_MSG_SET_SYSTEM_AUDIO_MODE] = 3 | BOTH;
	cec_msg_size[CEC_MSG_SYSTEM_AUDIO_MODE_REQUEST] = 2 | DIRECTED;
	cec_msg_size[CEC_MSG_SYSTEM_AUDIO_MODE_STATUS] = 3 | DIRECTED;
	cec_msg_size[CEC_MSG_SET_AUDIO_RATE] = 3 | DIRECTED;
	cec_msg_size[CEC_MSG_INITIATE_ARC] = 2 | DIRECTED;
	cec_msg_size[CEC_MSG_REPORT_ARC_INITIATED] = 2 | DIRECTED;
	cec_msg_size[CEC_MSG_REPORT_ARC_TERMINATED] = 2 | DIRECTED;
	cec_msg_size[CEC_MSG_REQUEST_ARC_INITIATION] = 2 | DIRECTED;
	cec_msg_size[CEC_MSG_REQUEST_ARC_TERMINATION] = 2 | DIRECTED;
	cec_msg_size[CEC_MSG_TERMINATE_ARC] = 2 | DIRECTED;
	cec_msg_size[CEC_MSG_REQUEST_CURRENT_LATENCY] = 4 | BCAST;
	cec_msg_size[CEC_MSG_REPORT_CURRENT_LATENCY] = 7 | BCAST;
	cec_msg_size[CEC_MSG_CDC_MESSAGE] = 2 | BCAST;
}
#endif
//code CEC_CMD LookUpTable_CEC[]=
//{
    //{OP_FeaturOP_Abort	,		Msg_Directly,		4},		// [Header] + [0x00] + [Feature OpCode] + [Abort Reason]
    //{OP_Abort			,		Msg_Directly,		2},		// [Header] + [0xFF]
    //
    //// One Touch Play Feature
    //{OP_ActiveSource	,		Msg_Broadcast,		4},		// Header + [0x82] + [Physical Address]
    //{OP_ImageViewOn		,		Msg_Directly,		2},		// Header + [0x04]
    //{OP_TextViewOn		,		Msg_Directly,		2},		// Header + [0x0D]
    //
    //// Routing Control Feature
    //{OP_InactiveSource,			Msg_Broadcast,		2},		// Header + [0x9D]
    //{OP_RequestActiveSource,	Msg_Broadcast,		2},		// Header + [0x85]
    //{OP_RoutingChange,			Msg_Broadcast,		6},		// Header + [0x80] + [Org Address] + [New Address]
    //{OP_RoutingInformation,		Msg_Broadcast,		4},		// Header + [0x80] + [Org Address] + [New Address]
    //{OP_SetStreamPath,			Msg_Broadcast,		4},		// Header + [0x86] + [Physical Address]
    //{OP_StandBy,				Msg_Both,			2},		// Header + [0x36]
    //
    //// System Information Feature
    //{OP_CECVersioin,			Msg_Directly,		3},		// Header + [0x82] + [CEC Version]
    //{OP_GetCECVersion,			Msg_Directly,		2},		// Header + [0x9F]
    //{OP_GivePhysicalAddress,	Msg_Directly,		2},		// Header + [0x83]
    //{OP_GetMenuLanguage,		Msg_Directly,		2},		// Header + [0x91]
    //{OP_ReportPhysicalAddress,	Msg_Broadcast,		5},		// Header + [0x84] + [Physical Address]+ [Device Type]
    //{OP_SetMenuLanguage,		Msg_Broadcast,		3},		// Header + [0x32] + [Language]
    //
    //// Vendor Specific Commands Feature
    //{OP_DeviceVendorID,			Msg_Broadcast,		5},		// Header + [0x87] + [Vendor ID]
    //{OP_GiveDeviceVendorID,		Msg_Directly,		2},		// Header + [0x8C]
    //{OP_VendorCommand,			Msg_Directly,		3},		// Header + [0x89] + [Vendor Specific Data]
    //{OP_VendorCommandWithID,	Msg_Both,			6},		// Header + [0xA0] + [Vendor ID]+ [Vendor Specific Data]
    //{OP_RemoteButtonDown,		Msg_Both,			3},		// Header + [0x8A] + [Vendor Specific RC code]
    //{OP_RemoteButtonUp,			Msg_Both,			3},		// Header + [0x8B] + [Vendor Specific RC code]
	//
    //// Remote Control PassThrough Feature
    //{OP_UserPressed,			Msg_Directly,		3},		// Header + [0x44] + [UI command]
    //{OP_UserReleased,			Msg_Directly,		2},		// Header + [0x45]
    //
    //// Power Status Feature
    //{OP_GiveDevicePowerStatus,	Msg_Directly,		2},		// Header + [0x8F]
    //{OP_ReportPowerStatus,		Msg_Directly,		3},		// Header + [0x90] + [Power Status]
    //
    //// System Audio Control Feature
    //{OP_GiveAudioStatus,			Msg_Directly,	2},	// Header + [0x71]
    //{OP_GiveSystemAudIoModeStatus,	Msg_Directly,	2},	// Header + [0x7D]
    //{OP_ReportAudioStatus,			Msg_Directly,	3},	// Header + [0x7A] + [Audio Status]
    //{OP_ReportAudioDescriptor,		Msg_Directly,	4},	// Header + [0xA3] + [Audio Descriptor]
    //{OP_RequestAudioDescriptor,		Msg_Directly,	3},	// Header + [0xA4] + [Audio Format ID and Code]
    //{OP_SetSystemAudioMode	,		Msg_Both,		3},	// Header + [0x72] + [System Audio Status]
    //{OP_AudioModeRequest,			Msg_Directly,	4},	// Header + [0x70] + [Physical Address]
    //{OP_SystemAudIoModeStatus,		Msg_Directly,	3},	// Header + [0x7E] + [System Audio Status]

    // ACR
    //{OP_InitiateARC,				Msg_Directly,	2},	// Header + [0xC0]
    //{OP_ReportARCInitiated,			Msg_Directly,	2},	// Header + [0xC1]
    //{OP_ReportARCTerminated,		Msg_Directly,	2},	// Header + [0xC2]
    //{OP_RequestARCInitiation,		Msg_Directly,	2},	// Header + [0xC3]
    //{OP_RequestARCTermination,		Msg_Directly,	2},	// Header + [0xC4]
    //{OP_TerminateARC,				Msg_Directly,	2},	// Header + [0xC5]

    // HEC
    //{OP_CDC,						Msg_Directly,	3},	// Header + [0xF8]

    // other
    //{OP_Play,						Msg_Directly,	3},	// Header + [0x41] + [Play Mode]

    // Polling Mesage
    //{OP_CDC+1,						Msg_Both,		0},	//end of 	lookup table !!!
 //};

//#define     SizeOfLookUpTable_CEC    (sizeof(LookUpTable_CEC)/sizeof(CEC_CMD))

//_CODE struct cec_enum_values type_ui_cmd[] = {
//	{ "Select", 0x00 },
//	{ "Up", 0x01 },
//	{ "Down", 0x02 },
//	{ "Left", 0x03 },
//	{ "Right", 0x04 },
//	{ "Right-Up", 0x05 },
//	{ "Right-Down", 0x06 },
//	{ "Left-Up", 0x07 },
//	{ "Left-Down", 0x08 },
//	{ "Device Root Menu", 0x09 },
//	{ "Device Setup Menu", 0x0a },
//	{ "Contents Menu", 0x0b },
//	{ "Favorite Menu", 0x0c },
//	{ "Back", 0x0d },
//	{ "Media Top Menu", 0x10 },
//	{ "Media Context-sensitive Menu", 0x11 },
//	{ "Number Entry Mode", 0x1d },
//	{ "Number 11", 0x1e },
//	{ "Number 12", 0x1f },
//	{ "Number 0 or Number 10", 0x20 },
//	{ "Number 1", 0x21 },
//	{ "Number 2", 0x22 },
//	{ "Number 3", 0x23 },
//	{ "Number 4", 0x24 },
//	{ "Number 5", 0x25 },
//	{ "Number 6", 0x26 },
//	{ "Number 7", 0x27 },
//	{ "Number 8", 0x28 },
//	{ "Number 9", 0x29 },
//	{ "Dot", 0x2a },
//	{ "Enter", 0x2b },
//	{ "Clear", 0x2c },
//	{ "Next Favorite", 0x2f },
//	{ "Channel Up", 0x30 },
//	{ "Channel Down", 0x31 },
//	{ "Previous Channel", 0x32 },
//	{ "Sound Select", 0x33 },
//	{ "Input Select", 0x34 },
//	{ "Display Information", 0x35 },
//	{ "Help", 0x36 },
//	{ "Page Up", 0x37 },
//	{ "Page Down", 0x38 },
//	{ "Power", 0x40 },
//	{ "Volume Up", 0x41 },
//	{ "Volume Down", 0x42 },
//	{ "Mute", 0x43 },
//	{ "Play", 0x44 },
//	{ "Stop", 0x45 },
//	{ "Pause", 0x46 },
//	{ "Record", 0x47 },
//	{ "Rewind", 0x48 },
//	{ "Fast forward", 0x49 },
//	{ "Eject", 0x4a },
//	{ "Skip Forward", 0x4b },
//	{ "Skip Backward", 0x4c },
//	{ "Stop-Record", 0x4d },
//	{ "Pause-Record", 0x4e },
//	{ "Angle", 0x50 },
//	{ "Sub picture", 0x51 },
//	{ "Video on Demand", 0x52 },
//	{ "Electronic Program Guide", 0x53 },
//	{ "Timer Programming", 0x54 },
//	{ "Initial Configuration", 0x55 },
//	{ "Select Broadcast Type", 0x56 },
//	{ "Select Sound Presentation", 0x57 },
//	{ "Audio Description", 0x58 },
//	{ "Internet", 0x59 },
//	{ "3D Mode", 0x5a },
//	{ "Play Function", 0x60 },
//	{ "Pause-Play Function", 0x61 },
//	{ "Record Function", 0x62 },
//	{ "Pause-Record Function", 0x63 },
//	{ "Stop Function", 0x64 },
//	{ "Mute Function", 0x65 },
//	{ "Restore Volume Function", 0x66 },
//	{ "Tune Function", 0x67 },
//	{ "Select Media Function", 0x68 },
//	{ "Select A/V Input Function", 0x69 },
//	{ "Select Audio Input Function", 0x6a },
//	{ "Power Toggle Function", 0x6b },
//	{ "Power Off Function", 0x6c },
//	{ "Power On Function", 0x6d },
//	{ "F1 (Blue)", 0x71 },
//	{ "F2 (Red)", 0x72 },
//	{ "F3 (Green)", 0x73 },
//	{ "F4 (Yellow)", 0x74 },
//	{ "F5", 0x75 },
//	{ "Data", 0x76 },
//};
