///*****************************************
//  Copyright (C) 2009-2018
//  ITE Tech. Inc. All Rights Reserved
//  Proprietary and Confidential
///*****************************************
//   @file   <iTE6615_CEC_FETURE.c>
//   @author Kuro.Chung@ite.com.tw
//   @date   2018/07/04
//   @fileversion: iTE6615_MCUSRC_0.23
//******************************************/
#include "iTE6615_Global.h"
#include "iTE6615_CEC_DRV.h"
#include "iTE6615_CEC_SYS.h"
#include "iTE6615_CEC_FETURE.h"

#if (_ENABLE_CEC_==TRUE)

extern _iTE6615_CEC *iTE6615_CEC;

// for more CEC Feture setting, plz reference http://lxr.free-electrons.com/source/include/uapi/linux/cec-funcs.h
// In CEC , Give mean Request


u8 CEC6615_POWER_STATUS = CEC_OP_POWER_STATUS_ON;


void iTE6615_CEC_CMD_Feature_decode(pCEC6615_FRAME CEC6615_FRAME)
{
    CEC_DEBUG_PRINTF(("iTE6615_CEC_CMD_Feature_decode!!"));
    // only response
    switch(CEC6615_FRAME->id.OPCODE)
    {

        ///////////////////////////////
        // General Protocol Messages //
        ///////////////////////////////
        case CEC_MSG_FEATURE_ABORT:
            // need to check abort reason is valid or not when need to care CEC_MSG_FEATURE_ABORT
            /* Don't reply with CEC_MSG_FEATURE_ABORT to a CEC_MSG_FEATURE_ABORT message! */
            break;
        case CEC_MSG_ABORT:
            // CEC MSG for TEST, need to reply CEC Feture Abort
            iTE6615_CEC_MSG_Reply_Feture_Abort(CEC6615_FRAME, CEC_OP_ABORT_REFUSED);
            break;

        ////////////////////////////////
        // System Information Feature //
        ////////////////////////////////
        case CEC_MSG_GET_CEC_VERSION:
            iTE6615_CEC_MSG_Reply_CEC_Version(CEC6615_FRAME->id.Initiator, CEC_OP_CEC_VERSION_2_0);
            break;
        case CEC_MSG_CEC_VERSION:
            CEC_DEBUG_PRINTF((" CEC_MSG_CEC_VERSION = "));
            switch(CEC6615_FRAME->id.OPERAND1)
            {
                case CEC_OP_CEC_VERSION_1_3A:
                    CEC_DEBUG_PRINTF(("CEC_OP_CEC_VERSION_1_3A"));
                    break;
                case CEC_OP_CEC_VERSION_1_4:
                    CEC_DEBUG_PRINTF(("CEC_OP_CEC_VERSION_1_4"));
                    break;
                case CEC_OP_CEC_VERSION_2_0:
                    CEC_DEBUG_PRINTF(("CEC_OP_CEC_VERSION_2_0"));
                    break;
            }
            break;

        case CEC_MSG_REPORT_PHYSICAL_ADDR:
            CEC_DEBUG_PRINTF((" CEC_MSG_REPORT_PHYSICAL_ADDR = %02X%02X, Primary Device Type = %02X \r\n",(int) iTE6615_CEC->CEC6615_FRAME_RX.id.OPERAND1,(int) iTE6615_CEC->CEC6615_FRAME_RX.id.OPERAND2,(int) iTE6615_CEC->CEC6615_FRAME_RX.id.OPERAND3));
            break;
        case CEC_MSG_GIVE_PHYSICAL_ADDR:
            iTE6615_CEC_MSG_Future_Report_PA(iTE6615_CEC->MY_PA_LOW, iTE6615_CEC->MY_PA_HIGH, CEC_OP_PRIM_DEVTYPE_TV);
            break;


        //////////////////////////////////////
        // Vendor Specific Commands Feature //
        //////////////////////////////////////
        case CEC_MSG_DEVICE_VENDOR_ID:
            CEC_DEBUG_PRINTF((" CEC_MSG_DEVICE_VENDOR_ID: %02X %02X %02X\r\n",(int) iTE6615_CEC->CEC6615_FRAME_RX.id.OPERAND1,(int) iTE6615_CEC->CEC6615_FRAME_RX.id.OPERAND2,(int) iTE6615_CEC->CEC6615_FRAME_RX.id.OPERAND3));
            break;

        case CEC_MSG_GIVE_DEVICE_VENDOR_ID:
            iTE6615_CEC_MSG_Device_VenderID(CEC_VENDOR_ID);
            break;


        /* Standby Feature */
        case CEC_MSG_STANDBY:
            break;    // do not need to reply, only TV is Initiator

        /* System Information Feature */
        case CEC_MSG_GET_MENU_LANGUAGE:
        case CEC_MSG_SET_MENU_LANGUAGE:
            break;    // dont need to reply
        case CEC_MSG_REPORT_FEATURES:
            break;

        /* Power Status Feature */
        case CEC_MSG_GIVE_DEVICE_POWER_STATUS:
            iTE6615_CEC_MSG_REPORT_POWER_STATUS(DIRECTED, CEC6615_FRAME->id.Initiator);
            break;
        case CEC_MSG_REPORT_POWER_STATUS:
            CEC_DEBUG_PRINTF((" CEC_MSG_REPORT_POWER_STATUS: %02X\r\n",(int) iTE6615_CEC->CEC6615_FRAME_RX.id.OPERAND1));
            break;

        /* Routing Control Feature */
        case CEC_MSG_REQUEST_ACTIVE_SOURCE:
            iTE6615_CEC_MSG_ACTIVE_SOURCE(iTE6615_CEC->MY_PA_LOW, iTE6615_CEC->MY_PA_HIGH);
            break;
        case CEC_MSG_INACTIVE_SOURCE:
            break;
        case CEC_MSG_ROUTING_CHANGE:
            // need to varifty parameter if implement
            break;
        case CEC_MSG_ROUTING_INFORMATION:
        case CEC_MSG_SET_STREAM_PATH:
            break;    // dont need to reply

        /* OSD Display Feature */
        case CEC_MSG_GIVE_OSD_NAME:
            iTE6615_CEC_MSG_SET_OSD_NAME(CEC6615_FRAME->id.Initiator);
            break;
        case CEC_MSG_SET_OSD_STRING:
        case CEC_MSG_SET_OSD_NAME:
            break; // dont need to reply

        case CEC_MSG_USER_CONTROL_PRESSED:
        case CEC_MSG_USER_CONTROL_RELEASED:
            break; // dont need to reply, need to update UI by which key CEC get

        case CEC_MSG_GIVE_AUDIO_STATUS:
        case CEC_MSG_REQUEST_SHORT_AUDIO_DESCRIPTOR:
        case CEC_MSG_GIVE_SYSTEM_AUDIO_MODE_STATUS:
        case CEC_MSG_REPORT_AUDIO_STATUS:
        case CEC_MSG_REPORT_SHORT_AUDIO_DESCRIPTOR:
        case CEC_MSG_SET_SYSTEM_AUDIO_MODE:
        case CEC_MSG_SYSTEM_AUDIO_MODE_REQUEST:
        case CEC_MSG_SYSTEM_AUDIO_MODE_STATUS:
            break; // dont need to reply

        case CEC_MSG_INITIATE_ARC:
        case CEC_MSG_REPORT_ARC_INITIATED:
        case CEC_MSG_REPORT_ARC_TERMINATED:
        case CEC_MSG_REQUEST_ARC_INITIATION:
        case CEC_MSG_REQUEST_ARC_TERMINATION:
        case CEC_MSG_TERMINATE_ARC:
            break;

        /* One Touch Play Feature */
        case CEC_MSG_ACTIVE_SOURCE:
            // need to varifty parameter if implement
            break;
        case CEC_MSG_IMAGE_VIEW_ON:
        case CEC_MSG_TEXT_VIEW_ON:
            break;

        /* Deck Control Feature */
        case CEC_MSG_PLAY:
        case CEC_MSG_DECK_STATUS:
        case CEC_MSG_GIVE_DECK_STATUS:
            break;
        case CEC_MSG_DECK_CONTROL:
            iTE6615_CEC_MSG_Reply_Feture_Abort(CEC6615_FRAME, CEC_OP_ABORT_REFUSED);
            break;

        /* Tuner Control Feature */
        case CEC_MSG_GIVE_TUNER_DEVICE_STATUS:
        case CEC_MSG_SELECT_ANALOGUE_SERVICE:
        case CEC_MSG_SELECT_DIGITAL_SERVICE:
        case CEC_MSG_TUNER_DEVICE_STATUS:
            break;

        /* Vendor Specific Commands Feature */
        case CEC_MSG_VENDOR_COMMAND:
        case CEC_MSG_VENDOR_COMMAND_WITH_ID:
        case CEC_MSG_VENDOR_REMOTE_BUTTON_UP:
        case CEC_MSG_VENDOR_REMOTE_BUTTON_DOWN:
            break;

        case CEC_MSG_RECORD_OFF:
        case CEC_MSG_RECORD_ON:
        case CEC_MSG_RECORD_STATUS:
            break;
        case CEC_MSG_RECORD_TV_SCREEN:
            iTE6615_CEC_MSG_Reply_Feture_Abort(CEC6615_FRAME, CEC_OP_ABORT_REFUSED);
            break;
        default:
            CEC_DEBUG_PRINTF((" CEC - Can't recognize CEC MSG , ignore it\r\n"));
            // If don't recognize CEC MSG, need to ignore this MSG
            // do not reply any MSG
            break;
    }
}




//////////////////////////////////////////////////////////////////////////////////////////
// System Information Feature
//////////////////////////////////////////////////////////////////////////////////////////
#define CEC_OP_CEC_VERSION_1_3A                         4
#define CEC_OP_CEC_VERSION_1_4                          5
#define CEC_OP_CEC_VERSION_2_0                          6
void iTE6615_CEC_MSG_Reply_CEC_Version(u8 TARGET_LA, u8 CEC_Version)
{
    CEC_DEBUG_PRINTF((" CEC iTE6615_CEC_MSG_Reply_CEC_Version\r\n"));
    iTE6615_CEC_INIT_CMD(iTE6615_CEC->MY_LA,TARGET_LA);
    iTE6615_CEC->CEC6615_FRAME_TX.id.CMD_SIZE = 3;
    iTE6615_CEC->CEC6615_FRAME_TX.id.OPCODE = CEC_MSG_CEC_VERSION;
    iTE6615_CEC->CEC6615_FRAME_TX.id.OPERAND1 =  CEC_Version;
    iTE6615_CEC_CMD_Ready_To_Fire();
}
void iTE6615_CEC_MSG_Feture_CEC_Version(u8 TARGET_LA)
{
    CEC_DEBUG_PRINTF((" CEC iTE6615_CEC_MSG_Feture_CEC_Version\r\n"));
    iTE6615_CEC_INIT_CMD(iTE6615_CEC->MY_LA,TARGET_LA);
    iTE6615_CEC->CEC6615_FRAME_TX.id.CMD_SIZE = 2;
    iTE6615_CEC->CEC6615_FRAME_TX.id.OPCODE = CEC_MSG_GET_CEC_VERSION;
    iTE6615_CEC_CMD_Ready_To_Fire();
}

/* Primary Device Type Operand (prim_devtype) */
#define CEC_OP_PRIM_DEVTYPE_TV                0
#define CEC_OP_PRIM_DEVTYPE_RECORD            1
#define CEC_OP_PRIM_DEVTYPE_TUNER            3
#define CEC_OP_PRIM_DEVTYPE_PLAYBACK        4
#define CEC_OP_PRIM_DEVTYPE_AUDIOSYSTEM        5
#define CEC_OP_PRIM_DEVTYPE_SWITCH            6
#define CEC_OP_PRIM_DEVTYPE_PROCESSOR        7
void iTE6615_CEC_MSG_Future_Report_PA(u8 PA_LOW,u8 PA_HIGH, u8 prim_devtype)
{
    CEC_DEBUG_PRINTF((" CEC iTE6615_CEC_MSG_Future_Report_PA\r\n"));
    iTE6615_CEC_INIT_CMD(iTE6615_CEC->MY_LA,CEC_LOG_ADDR_BROADCAST);
    iTE6615_CEC->CEC6615_FRAME_TX.id.CMD_SIZE = 5;
    iTE6615_CEC->CEC6615_FRAME_TX.id.OPCODE = CEC_MSG_REPORT_PHYSICAL_ADDR;
    iTE6615_CEC->CEC6615_FRAME_TX.id.OPERAND1 =  PA_HIGH;
    iTE6615_CEC->CEC6615_FRAME_TX.id.OPERAND2 =  PA_LOW;
    iTE6615_CEC->CEC6615_FRAME_TX.id.OPERAND2 =  prim_devtype;
    iTE6615_CEC_CMD_Ready_To_Fire();
}

void iTE6615_CEC_MSG_Future_Give_PA(void)
{
    CEC_DEBUG_PRINTF((" CEC iTE6615_CEC_MSG_Future_Give_PA\r\n"));
    iTE6615_CEC_INIT_CMD(iTE6615_CEC->MY_LA,CEC_LOG_ADDR_BROADCAST);
    iTE6615_CEC->CEC6615_FRAME_TX.id.CMD_SIZE = 2;
    iTE6615_CEC->CEC6615_FRAME_TX.id.OPCODE = CEC_MSG_GIVE_PHYSICAL_ADDR;
    iTE6615_CEC_CMD_Ready_To_Fire();
}


//////////////////////////////////////////////////////////////////////////////////////////
// General Protocol Messages
//////////////////////////////////////////////////////////////////////////////////////////
//    The whole structure is zeroed, the len field is set to 1 (i.e. a poll
//    message) and the initiator and destination are filled in.
void iTE6615_CEC_MSG_Future_Polling(u8 TARGET_LA)
{
    CEC_DEBUG_PRINTF((" CEC iTE6615_CEC_MSG_Future_Polling\r\n"));
    iTE6615_CEC_INIT_CMD(iTE6615_CEC->MY_LA,TARGET_LA);
    iTE6615_CEC_CMD_Ready_To_Fire();
}

/* Abort Reason Operand (reason) */
#define CEC_OP_ABORT_UNRECOGNIZED_OP                    0
#define CEC_OP_ABORT_INCORRECT_MODE                     1
#define CEC_OP_ABORT_NO_SOURCE                          2
#define CEC_OP_ABORT_INVALID_OP                         3
#define CEC_OP_ABORT_REFUSED                            4
#define CEC_OP_ABORT_UNDETERMINED                       5
void iTE6615_CEC_MSG_Feture_Abort(u8 TARGET_LA, u8 CEC_RXCMD, u8 Abort_Reason)
{
    CEC_DEBUG_PRINTF((" CEC iTE6615_CEC_MSG_Feture_Abort\r\n"));
    iTE6615_CEC_INIT_CMD(iTE6615_CEC->MY_LA,TARGET_LA);
    iTE6615_CEC->CEC6615_FRAME_TX.id.CMD_SIZE = 4;
    iTE6615_CEC->CEC6615_FRAME_TX.id.OPCODE = CEC_MSG_FEATURE_ABORT;
    iTE6615_CEC->CEC6615_FRAME_TX.id.OPERAND1 =  CEC_RXCMD;
    iTE6615_CEC->CEC6615_FRAME_TX.id.OPERAND2 =  Abort_Reason;
    iTE6615_CEC_CMD_Ready_To_Fire();
}

/* This changes the current message into a feature abort message */
// some as  iTE6615_CEC_MSG_Feture_Abort function, wrap it
void iTE6615_CEC_MSG_Reply_Feture_Abort(pCEC6615_FRAME RX_CEC6615_FRAME, u8 Abort_Reason)
{
    CEC_DEBUG_PRINTF((" CEC iTE6615_CEC_MSG_Reply_Feture_Abort\r\n"));
    iTE6615_CEC_INIT_CMD(iTE6615_CEC->MY_LA,RX_CEC6615_FRAME->id.Initiator); // reply to Initiator
    iTE6615_CEC->CEC6615_FRAME_TX.id.CMD_SIZE = 4;
    iTE6615_CEC->CEC6615_FRAME_TX.id.OPCODE = CEC_MSG_FEATURE_ABORT;
    iTE6615_CEC->CEC6615_FRAME_TX.id.OPERAND1 =  RX_CEC6615_FRAME->id.OPERAND1;
    iTE6615_CEC->CEC6615_FRAME_TX.id.OPERAND2 =  Abort_Reason;
    iTE6615_CEC_CMD_Ready_To_Fire();
}

void iTE6615_CEC_MSG_Abort(u8 TARGET_LA)
{
    CEC_DEBUG_PRINTF((" CEC iTE6615_CEC_MSG_Abort\r\n"));
    iTE6615_CEC_INIT_CMD(iTE6615_CEC->MY_LA,TARGET_LA);
    iTE6615_CEC->CEC6615_FRAME_TX.id.CMD_SIZE = 2;
    iTE6615_CEC->CEC6615_FRAME_TX.id.OPCODE = CEC_MSG_ABORT;
    iTE6615_CEC_CMD_Ready_To_Fire();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Vendor Specific Commands Feature
//////////////////////////////////////////////////////////////////////////////////////////
 void iTE6615_CEC_MSG_Device_VenderID(u32 vendor_id)
{
    CEC_DEBUG_PRINTF((" CEC iTE6615_CEC_MSG_Device_VenderID\r\n"));
    iTE6615_CEC_INIT_CMD(iTE6615_CEC->MY_LA,CEC_LOG_ADDR_BROADCAST);
    iTE6615_CEC->CEC6615_FRAME_TX.id.CMD_SIZE = 5;
    iTE6615_CEC->CEC6615_FRAME_TX.id.OPCODE = CEC_MSG_DEVICE_VENDOR_ID;
    iTE6615_CEC->CEC6615_FRAME_TX.id.OPERAND1 =  vendor_id >> 16;
    iTE6615_CEC->CEC6615_FRAME_TX.id.OPERAND2 =  (vendor_id >> 8) & 0xff;
    iTE6615_CEC->CEC6615_FRAME_TX.id.OPERAND3  = vendor_id & 0xff;
    iTE6615_CEC_CMD_Ready_To_Fire();
}

void iTE6615_CEC_MSG_Give_Deive_VendorID(void)
{
    CEC_DEBUG_PRINTF((" CEC iTE6615_CEC_MSG_Give_Deive_VendorID\r\n"));
    iTE6615_CEC_INIT_CMD(iTE6615_CEC->MY_LA,CEC_LOG_ADDR_BROADCAST);
    iTE6615_CEC->CEC6615_FRAME_TX.id.CMD_SIZE = 2;
    iTE6615_CEC->CEC6615_FRAME_TX.id.OPCODE = CEC_MSG_GIVE_DEVICE_VENDOR_ID;
    iTE6615_CEC_CMD_Ready_To_Fire();
}

//////////////////////////////////////////////////////////////////////////////////////////
// Standby Feature
//////////////////////////////////////////////////////////////////////////////////////////
// only for root TV using for all device to standby mode, need trigger by TV
void iTE6615_CEC_MSG_STANDBY(void)
{
    CEC_DEBUG_PRINTF((" CEC iTE6615_CEC_MSG_Give_Deive_VendorID\r\n"));
    iTE6615_CEC_INIT_CMD(iTE6615_CEC->MY_LA,CEC_LOG_ADDR_BROADCAST);
    iTE6615_CEC->CEC6615_FRAME_TX.id.CMD_SIZE = 2;
    iTE6615_CEC->CEC6615_FRAME_TX.id.OPCODE = CEC_MSG_STANDBY;
    iTE6615_CEC_CMD_Ready_To_Fire();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Power Status Feature
//////////////////////////////////////////////////////////////////////////////////////////
void iTE6615_CEC_MSG_REPORT_POWER_STATUS(u8 CASTING_MODE, u8 TARGET_LA)
{
    CEC_DEBUG_PRINTF((" CEC iTE6615_CEC_MSG_REPORT_POWER_STATUS\r\n"));
    if(CASTING_MODE == BCAST)
    {
        iTE6615_CEC_INIT_CMD(iTE6615_CEC->MY_LA,CEC_LOG_ADDR_BROADCAST);
    }
    else
    {
        iTE6615_CEC_INIT_CMD(iTE6615_CEC->MY_LA,TARGET_LA);
    }

    iTE6615_CEC->CEC6615_FRAME_TX.id.CMD_SIZE = 3;
    iTE6615_CEC->CEC6615_FRAME_TX.id.OPCODE = CEC_MSG_REPORT_POWER_STATUS;
    iTE6615_CEC->CEC6615_FRAME_TX.id.OPERAND1 = CEC6615_POWER_STATUS;
    iTE6615_CEC_CMD_Ready_To_Fire();
}

//////////////////////////////////////////////////////////////////////////////////////////
// Device OSD Transfer Feature
//////////////////////////////////////////////////////////////////////////////////////////
void iTE6615_CEC_MSG_SET_OSD_NAME(u8 TARGET_LA)
{
    CEC_DEBUG_PRINTF((" CEC iTE6615_CEC_MSG_REPORT_POWER_STATUS\r\n"));

    iTE6615_CEC_INIT_CMD(iTE6615_CEC->MY_LA,TARGET_LA);
    iTE6615_CEC->CEC6615_FRAME_TX.id.CMD_SIZE = 4;
    iTE6615_CEC->CEC6615_FRAME_TX.id.OPCODE = CEC_MSG_SET_OSD_NAME;
    iTE6615_CEC->CEC6615_FRAME_TX.id.OPERAND1 = 0x68;
    iTE6615_CEC->CEC6615_FRAME_TX.id.OPERAND2 = 0x05;
    iTE6615_CEC_CMD_Ready_To_Fire();
}

void iTE6615_CEC_MSG_ACTIVE_SOURCE(u8 PA_LOW,u8 PA_HIGH)
{
    CEC_DEBUG_PRINTF((" CEC iTE6615_CEC_MSG_REPORT_POWER_STATUS\r\n"));

    iTE6615_CEC_INIT_CMD(iTE6615_CEC->MY_LA,CEC_LOG_ADDR_BROADCAST);
    iTE6615_CEC->CEC6615_FRAME_TX.id.CMD_SIZE = 4;
    iTE6615_CEC->CEC6615_FRAME_TX.id.OPCODE = CEC_MSG_ACTIVE_SOURCE;
    iTE6615_CEC->CEC6615_FRAME_TX.id.OPERAND1 = PA_HIGH;
    iTE6615_CEC->CEC6615_FRAME_TX.id.OPERAND2 = PA_LOW;
    iTE6615_CEC_CMD_Ready_To_Fire();
}

#endif
