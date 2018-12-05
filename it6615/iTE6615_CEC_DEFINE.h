///*****************************************
//  Copyright (C) 2009-2018
//  ITE Tech. Inc. All Rights Reserved
//  Proprietary and Confidential
///*****************************************
//   @file   <iTE6615_CEC_DEFINE.h>
//   @author Kuro.Chung@ite.com.tw
//   @date   2018/07/04
//   @fileversion: iTE6615_MCUSRC_0.23
//******************************************/
#ifndef _iTE6615_CEC_DEFINE_H_
#define _iTE6615_CEC_DEFINE_H_

#define CEC_MAX_MSG_LEN        (16)    /* 16 blocks */

#define CEC_BCAST_ADDR        (0x0f)
#define CEC_ADDR_MAX        CEC_BCAST_ADDR

#define TX_DONE 1
#define TX_FAIL 0

#define TX_FAIL_RECEIVE_ACK        1
#define TX_FAIL_RECEIVE_NACK    2
#define TX_FAIL_RETRY            4

#define    REG_FOLLOWER_RX_NUM    0x42

#define CEC_DEV_VENDOR_ID_0        (0x00)
#define CEC_DEV_VENDOR_ID_1        (0x00)
#define CEC_DEV_VENDOR_ID_2        (0x00)

#define CEC6615_QUEUE_SIZE    (0x01 << 6)
#define CEC6615_QUEUE_MAX    (CEC6615_QUEUE_SIZE - 1)

#define CEC6615_FRAME_SIZE 21
typedef union {
    struct
    {
        u8    CMD_SIZE;
        u8    HEADER;
        u8    OPCODE;
        u8    OPERAND1;
        u8    OPERAND2;
        u8    OPERAND3;
        u8    OPERAND4;
        u8    OPERAND5;
        u8    OPERAND6;
        u8    OPERAND7;
        u8    OPERAND8;
        u8    OPERAND9;
        u8    OPERAND10;
        u8    OPERAND11;
        u8    OPERAND12;
        u8    OPERAND13;
        u8    OPERAND14;
        u8    OPERAND15;
        u8    OPERAND16;
        u8    Follower;
        u8    Initiator;
    }id;
    u8     CEC_ARY[21];
}CEC6615_FRAME, *pCEC6615_FRAME;


typedef struct{
    u8    Rptr;
    u8    Wptr;
    u8    Queue[CEC6615_QUEUE_SIZE];
}CEC6615_QUEUE;

typedef struct {
    u8        MY_LA;
    u8        MY_PA_LOW, MY_PA_HIGH;
    CEC6615_QUEUE Rx_Queue;    // for Receive Message
    CEC6615_QUEUE Tx_Queue;    // for Receive Message
    u8        Rx_TmpHeader[3];
    CEC6615_FRAME CEC6615_FRAME_RX, CEC6615_FRAME_TX;
    u8        Tx_QueueFull;
    u8        Tx_CECDone;
    u8        Tx_CECFire;
    u8        Tx_CECInitDone;
}_iTE6615_CEC;


 typedef struct {
    u8    PhyicalAddr1;
    u8    PhyicalAddr2;
    u8    Active;
 }CEC6615_Device, *pCEC6615_Device;

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

typedef enum {
    Msg6615_Directly=0,
    Msg6615_Broadcast=1,
    Msg6615_Both=2,
}MessageType;


#define DIRECTED    0x80
#define BCAST1_4    0x40
#define BCAST2_0    0x20    /* broadcast only allowed for >= 2.0 */
#define BCAST        (BCAST1_4 | BCAST2_0)
#define BOTH        (BCAST | DIRECTED)

/*
 * The remainder of this header defines all CEC messages and operands.
 * The format matters since it the cec-ctl utility parses it to generate
 * code for implementing all these messages.
 *
 * Comments ending with 'Feature' group messages for each feature.
 * If messages are part of multiple features, then the "Has also"
 * comment is used to list the previously defined messages that are
 * supported by the feature.
 *
 * Before operands are defined a comment is added that gives the
 * name of the operand and in brackets the variable name of the
 * corresponding argument in the cec-funcs.h function.
 */

/* Messages */

/* One Touch Play Feature */
#define CEC_MSG_ACTIVE_SOURCE                           0x82
#define CEC_MSG_IMAGE_VIEW_ON                           0x04
#define CEC_MSG_TEXT_VIEW_ON                            0x0d

/* Routing Control Feature */

/*
 * Has also:
 *      CEC_MSG_ACTIVE_SOURCE
 */

#define CEC_MSG_INACTIVE_SOURCE                         0x9d
#define CEC_MSG_REQUEST_ACTIVE_SOURCE                   0x85
#define CEC_MSG_ROUTING_CHANGE                          0x80
#define CEC_MSG_ROUTING_INFORMATION                     0x81
#define CEC_MSG_SET_STREAM_PATH                         0x86

/* Standby Feature */
#define CEC_MSG_STANDBY                                 0x36

/* One Touch Record Feature */
#define CEC_MSG_RECORD_OFF                              0x0b
#define CEC_MSG_RECORD_ON                               0x09
/* Record Source Type Operand (rec_src_type) */
#define CEC_OP_RECORD_SRC_OWN                           1
#define CEC_OP_RECORD_SRC_DIGITAL                       2
#define CEC_OP_RECORD_SRC_ANALOG                        3
#define CEC_OP_RECORD_SRC_EXT_PLUG                      4
#define CEC_OP_RECORD_SRC_EXT_PHYS_ADDR                 5
/* Service Identification Method Operand (service_id_method) */
#define CEC_OP_SERVICE_ID_METHOD_BY_DIG_ID              0
#define CEC_OP_SERVICE_ID_METHOD_BY_CHANNEL             1
/* Digital Service Broadcast System Operand (dig_bcast_system) */
#define CEC_OP_DIG_SERVICE_BCAST_SYSTEM_ARIB_GEN        0x00
#define CEC_OP_DIG_SERVICE_BCAST_SYSTEM_ATSC_GEN        0x01
#define CEC_OP_DIG_SERVICE_BCAST_SYSTEM_DVB_GEN         0x02
#define CEC_OP_DIG_SERVICE_BCAST_SYSTEM_ARIB_BS         0x08
#define CEC_OP_DIG_SERVICE_BCAST_SYSTEM_ARIB_CS         0x09
#define CEC_OP_DIG_SERVICE_BCAST_SYSTEM_ARIB_T          0x0a
#define CEC_OP_DIG_SERVICE_BCAST_SYSTEM_ATSC_CABLE      0x10
#define CEC_OP_DIG_SERVICE_BCAST_SYSTEM_ATSC_SAT        0x11
#define CEC_OP_DIG_SERVICE_BCAST_SYSTEM_ATSC_T          0x12
#define CEC_OP_DIG_SERVICE_BCAST_SYSTEM_DVB_C           0x18
#define CEC_OP_DIG_SERVICE_BCAST_SYSTEM_DVB_S           0x19
#define CEC_OP_DIG_SERVICE_BCAST_SYSTEM_DVB_S2          0x1a
#define CEC_OP_DIG_SERVICE_BCAST_SYSTEM_DVB_T           0x1b
/* Analogue Broadcast Type Operand (ana_bcast_type) */
#define CEC_OP_ANA_BCAST_TYPE_CABLE                     0
#define CEC_OP_ANA_BCAST_TYPE_SATELLITE                 1
#define CEC_OP_ANA_BCAST_TYPE_TERRESTRIAL               2
/* Broadcast System Operand (bcast_system) */
#define CEC_OP_BCAST_SYSTEM_PAL_BG                      0x00
#define CEC_OP_BCAST_SYSTEM_SECAM_LQ                    0x01 /* SECAM L' */
#define CEC_OP_BCAST_SYSTEM_PAL_M                       0x02
#define CEC_OP_BCAST_SYSTEM_NTSC_M                      0x03
#define CEC_OP_BCAST_SYSTEM_PAL_I                       0x04
#define CEC_OP_BCAST_SYSTEM_SECAM_DK                    0x05
#define CEC_OP_BCAST_SYSTEM_SECAM_BG                    0x06
#define CEC_OP_BCAST_SYSTEM_SECAM_L                     0x07
#define CEC_OP_BCAST_SYSTEM_PAL_DK                      0x08
#define CEC_OP_BCAST_SYSTEM_OTHER                       0x1f
/* Channel Number Format Operand (channel_number_fmt) */
#define CEC_OP_CHANNEL_NUMBER_FMT_1_PART                0x01
#define CEC_OP_CHANNEL_NUMBER_FMT_2_PART                0x02

#define CEC_MSG_RECORD_STATUS                           0x0a
/* Record Status Operand (rec_status) */
#define CEC_OP_RECORD_STATUS_CUR_SRC                    0x01
#define CEC_OP_RECORD_STATUS_DIG_SERVICE                0x02
#define CEC_OP_RECORD_STATUS_ANA_SERVICE                0x03
#define CEC_OP_RECORD_STATUS_EXT_INPUT                  0x04
#define CEC_OP_RECORD_STATUS_NO_DIG_SERVICE             0x05
#define CEC_OP_RECORD_STATUS_NO_ANA_SERVICE             0x06
#define CEC_OP_RECORD_STATUS_NO_SERVICE                 0x07
#define CEC_OP_RECORD_STATUS_INVALID_EXT_PLUG           0x09
#define CEC_OP_RECORD_STATUS_INVALID_EXT_PHYS_ADDR      0x0a
#define CEC_OP_RECORD_STATUS_UNSUP_CA                   0x0b
#define CEC_OP_RECORD_STATUS_NO_CA_ENTITLEMENTS         0x0c
#define CEC_OP_RECORD_STATUS_CANT_COPY_SRC              0x0d
#define CEC_OP_RECORD_STATUS_NO_MORE_COPIES             0x0e
#define CEC_OP_RECORD_STATUS_NO_MEDIA                   0x10
#define CEC_OP_RECORD_STATUS_PLAYING                    0x11
#define CEC_OP_RECORD_STATUS_ALREADY_RECORDING          0x12
#define CEC_OP_RECORD_STATUS_MEDIA_PROT                 0x13
#define CEC_OP_RECORD_STATUS_NO_SIGNAL                  0x14
#define CEC_OP_RECORD_STATUS_MEDIA_PROBLEM              0x15
#define CEC_OP_RECORD_STATUS_NO_SPACE                   0x16
#define CEC_OP_RECORD_STATUS_PARENTAL_LOCK              0x17
#define CEC_OP_RECORD_STATUS_TERMINATED_OK              0x1a
#define CEC_OP_RECORD_STATUS_ALREADY_TERM               0x1b
#define CEC_OP_RECORD_STATUS_OTHER                      0x1f

#define CEC_MSG_RECORD_TV_SCREEN                        0x0f

/* Timer Programming Feature */
#define CEC_MSG_CLEAR_ANALOGUE_TIMER                    0x33
/* Recording Sequence Operand (recording_seq) */
#define CEC_OP_REC_SEQ_SUNDAY                           0x01
#define CEC_OP_REC_SEQ_MONDAY                           0x02
#define CEC_OP_REC_SEQ_TUESDAY                          0x04
#define CEC_OP_REC_SEQ_WEDNESDAY                        0x08
#define CEC_OP_REC_SEQ_THURSDAY                         0x10
#define CEC_OP_REC_SEQ_FRIDAY                           0x20
#define CEC_OP_REC_SEQ_SATERDAY                         0x40
#define CEC_OP_REC_SEQ_ONCE_ONLY                        0x00

#define CEC_MSG_CLEAR_DIGITAL_TIMER                     0x99

#define CEC_MSG_CLEAR_EXT_TIMER                         0xa1
/* External Source Specifier Operand (ext_src_spec) */
#define CEC_OP_EXT_SRC_PLUG                             0x04
#define CEC_OP_EXT_SRC_PHYS_ADDR                        0x05

#define CEC_MSG_SET_ANALOGUE_TIMER                      0x34
#define CEC_MSG_SET_DIGITAL_TIMER                       0x97
#define CEC_MSG_SET_EXT_TIMER                           0xa2

#define CEC_MSG_SET_TIMER_PROGRAM_TITLE                 0x67
#define CEC_MSG_TIMER_CLEARED_STATUS                    0x43
/* Timer Cleared Status Data Operand (timer_cleared_status) */
#define CEC_OP_TIMER_CLR_STAT_RECORDING                 0x00
#define CEC_OP_TIMER_CLR_STAT_NO_MATCHING               0x01
#define CEC_OP_TIMER_CLR_STAT_NO_INFO                   0x02
#define CEC_OP_TIMER_CLR_STAT_CLEARED                   0x80

#define CEC_MSG_TIMER_STATUS                            0x35
/* Timer Overlap Warning Operand (timer_overlap_warning) */
#define CEC_OP_TIMER_OVERLAP_WARNING_NO_OVERLAP         0
#define CEC_OP_TIMER_OVERLAP_WARNING_OVERLAP            1
/* Media Info Operand (media_info) */
#define CEC_OP_MEDIA_INFO_UNPROT_MEDIA                  0
#define CEC_OP_MEDIA_INFO_PROT_MEDIA                    1
#define CEC_OP_MEDIA_INFO_NO_MEDIA                      2
/* Programmed Indicator Operand (prog_indicator) */
#define CEC_OP_PROG_IND_NOT_PROGRAMMED                  0
#define CEC_OP_PROG_IND_PROGRAMMED                      1
/* Programmed Info Operand (prog_info) */
#define CEC_OP_PROG_INFO_ENOUGH_SPACE                   0x08
#define CEC_OP_PROG_INFO_NOT_ENOUGH_SPACE               0x09
#define CEC_OP_PROG_INFO_MIGHT_NOT_BE_ENOUGH_SPACE      0x0b
#define CEC_OP_PROG_INFO_NONE_AVAILABLE                 0x0a
/* Not Programmed Error Info Operand (prog_error) */
#define CEC_OP_PROG_ERROR_NO_FREE_TIMER                 0x01
#define CEC_OP_PROG_ERROR_DATE_OUT_OF_RANGE             0x02
#define CEC_OP_PROG_ERROR_REC_SEQ_ERROR                 0x03
#define CEC_OP_PROG_ERROR_INV_EXT_PLUG                  0x04
#define CEC_OP_PROG_ERROR_INV_EXT_PHYS_ADDR             0x05
#define CEC_OP_PROG_ERROR_CA_UNSUPP                     0x06
#define CEC_OP_PROG_ERROR_INSUF_CA_ENTITLEMENTS         0x07
#define CEC_OP_PROG_ERROR_RESOLUTION_UNSUPP             0x08
#define CEC_OP_PROG_ERROR_PARENTAL_LOCK                 0x09
#define CEC_OP_PROG_ERROR_CLOCK_FAILURE                 0x0a
#define CEC_OP_PROG_ERROR_DUPLICATE                     0x0e

/* System Information Feature */
#define CEC_MSG_CEC_VERSION                             0x9e
/* CEC Version Operand (cec_version) */
#define CEC_OP_CEC_VERSION_1_3A                         4
#define CEC_OP_CEC_VERSION_1_4                          5
#define CEC_OP_CEC_VERSION_2_0                          6

#define CEC_MSG_GET_CEC_VERSION                         0x9f
#define CEC_MSG_GIVE_PHYSICAL_ADDR                      0x83
#define CEC_MSG_GET_MENU_LANGUAGE                       0x91
#define CEC_MSG_REPORT_PHYSICAL_ADDR                    0x84
/* Primary Device Type Operand (prim_devtype) */
#define CEC_OP_PRIM_DEVTYPE_TV                          0
#define CEC_OP_PRIM_DEVTYPE_RECORD                      1
#define CEC_OP_PRIM_DEVTYPE_TUNER                       3
#define CEC_OP_PRIM_DEVTYPE_PLAYBACK                    4
#define CEC_OP_PRIM_DEVTYPE_AUDIOSYSTEM                 5
#define CEC_OP_PRIM_DEVTYPE_SWITCH                      6
#define CEC_OP_PRIM_DEVTYPE_PROCESSOR                   7

#define CEC_MSG_SET_MENU_LANGUAGE                       0x32
#define CEC_MSG_REPORT_FEATURES                         0xa6    /* HDMI 2.0 */
/* All Device Types Operand (all_device_types) */
#define CEC_OP_ALL_DEVTYPE_TV                           0x80
#define CEC_OP_ALL_DEVTYPE_RECORD                       0x40
#define CEC_OP_ALL_DEVTYPE_TUNER                        0x20
#define CEC_OP_ALL_DEVTYPE_PLAYBACK                     0x10
#define CEC_OP_ALL_DEVTYPE_AUDIOSYSTEM                  0x08
#define CEC_OP_ALL_DEVTYPE_SWITCH                       0x04
/*
 * And if you wondering what happened to PROCESSOR devices: those should
 * be mapped to a SWITCH.
 */

/* Valid for RC Profile and Device Feature operands */
#define CEC_OP_FEAT_EXT                                 0x80    /* Extension bit */
/* RC Profile Operand (rc_profile) */
#define CEC_OP_FEAT_RC_TV_PROFILE_NONE                  0x00
#define CEC_OP_FEAT_RC_TV_PROFILE_1                     0x02
#define CEC_OP_FEAT_RC_TV_PROFILE_2                     0x06
#define CEC_OP_FEAT_RC_TV_PROFILE_3                     0x0a
#define CEC_OP_FEAT_RC_TV_PROFILE_4                     0x0e
#define CEC_OP_FEAT_RC_SRC_HAS_DEV_ROOT_MENU            0x50
#define CEC_OP_FEAT_RC_SRC_HAS_DEV_SETUP_MENU           0x48
#define CEC_OP_FEAT_RC_SRC_HAS_CONTENTS_MENU            0x44
#define CEC_OP_FEAT_RC_SRC_HAS_MEDIA_TOP_MENU           0x42
#define CEC_OP_FEAT_RC_SRC_HAS_MEDIA_CONTEXT_MENU       0x41
/* Device Feature Operand (dev_features) */
#define CEC_OP_FEAT_DEV_HAS_RECORD_TV_SCREEN            0x40
#define CEC_OP_FEAT_DEV_HAS_SET_OSD_STRING              0x20
#define CEC_OP_FEAT_DEV_HAS_DECK_CONTROL                0x10
#define CEC_OP_FEAT_DEV_HAS_SET_AUDIO_RATE              0x08
#define CEC_OP_FEAT_DEV_SINK_HAS_ARC_TX                 0x04
#define CEC_OP_FEAT_DEV_SOURCE_HAS_ARC_RX               0x02

#define CEC_MSG_GIVE_FEATURES                           0xa5    /* HDMI 2.0 */

/* Deck Control Feature */
#define CEC_MSG_DECK_CONTROL                            0x42
/* Deck Control Mode Operand (deck_control_mode) */
#define CEC_OP_DECK_CTL_MODE_SKIP_FWD                   1
#define CEC_OP_DECK_CTL_MODE_SKIP_REV                   2
#define CEC_OP_DECK_CTL_MODE_STOP                       3
#define CEC_OP_DECK_CTL_MODE_EJECT                      4

#define CEC_MSG_DECK_STATUS                             0x1b
/* Deck Info Operand (deck_info) */
#define CEC_OP_DECK_INFO_PLAY                           0x11
#define CEC_OP_DECK_INFO_RECORD                         0x12
#define CEC_OP_DECK_INFO_PLAY_REV                       0x13
#define CEC_OP_DECK_INFO_STILL                          0x14
#define CEC_OP_DECK_INFO_SLOW                           0x15
#define CEC_OP_DECK_INFO_SLOW_REV                       0x16
#define CEC_OP_DECK_INFO_FAST_FWD                       0x17
#define CEC_OP_DECK_INFO_FAST_REV                       0x18
#define CEC_OP_DECK_INFO_NO_MEDIA                       0x19
#define CEC_OP_DECK_INFO_STOP                           0x1a
#define CEC_OP_DECK_INFO_SKIP_FWD                       0x1b
#define CEC_OP_DECK_INFO_SKIP_REV                       0x1c
#define CEC_OP_DECK_INFO_INDEX_SEARCH_FWD               0x1d
#define CEC_OP_DECK_INFO_INDEX_SEARCH_REV               0x1e
#define CEC_OP_DECK_INFO_OTHER                          0x1f

#define CEC_MSG_GIVE_DECK_STATUS                        0x1a
/* Status Request Operand (status_req) */
#define CEC_OP_STATUS_REQ_ON                            1
#define CEC_OP_STATUS_REQ_OFF                           2
#define CEC_OP_STATUS_REQ_ONCE                          3

#define CEC_MSG_PLAY                                    0x41
/* Play Mode Operand (play_mode) */
#define CEC_OP_PLAY_MODE_PLAY_FWD                       0x24
#define CEC_OP_PLAY_MODE_PLAY_REV                       0x20
#define CEC_OP_PLAY_MODE_PLAY_STILL                     0x25
#define CEC_OP_PLAY_MODE_PLAY_FAST_FWD_MIN              0x05
#define CEC_OP_PLAY_MODE_PLAY_FAST_FWD_MED              0x06
#define CEC_OP_PLAY_MODE_PLAY_FAST_FWD_MAX              0x07
#define CEC_OP_PLAY_MODE_PLAY_FAST_REV_MIN              0x09
#define CEC_OP_PLAY_MODE_PLAY_FAST_REV_MED              0x0a
#define CEC_OP_PLAY_MODE_PLAY_FAST_REV_MAX              0x0b
#define CEC_OP_PLAY_MODE_PLAY_SLOW_FWD_MIN              0x15
#define CEC_OP_PLAY_MODE_PLAY_SLOW_FWD_MED              0x16
#define CEC_OP_PLAY_MODE_PLAY_SLOW_FWD_MAX              0x17
#define CEC_OP_PLAY_MODE_PLAY_SLOW_REV_MIN              0x19
#define CEC_OP_PLAY_MODE_PLAY_SLOW_REV_MED              0x1a
#define CEC_OP_PLAY_MODE_PLAY_SLOW_REV_MAX              0x1b

/* Tuner Control Feature */
#define CEC_MSG_GIVE_TUNER_DEVICE_STATUS                0x08
#define CEC_MSG_SELECT_ANALOGUE_SERVICE                 0x92
#define CEC_MSG_SELECT_DIGITAL_SERVICE                  0x93
#define CEC_MSG_TUNER_DEVICE_STATUS                     0x07
/* Recording Flag Operand (rec_flag) */
#define CEC_OP_REC_FLAG_USED                            0
#define CEC_OP_REC_FLAG_NOT_USED                        1
/* Tuner Display Info Operand (tuner_display_info) */
#define CEC_OP_TUNER_DISPLAY_INFO_DIGITAL               0
#define CEC_OP_TUNER_DISPLAY_INFO_NONE                  1
#define CEC_OP_TUNER_DISPLAY_INFO_ANALOGUE              2

#define CEC_MSG_TUNER_STEP_DECREMENT                    0x06
#define CEC_MSG_TUNER_STEP_INCREMENT                    0x05

/* Vendor Specific Commands Feature */

/*
 * Has also:
 *      CEC_MSG_CEC_VERSION
 *      CEC_MSG_GET_CEC_VERSION
 */
#define CEC_MSG_DEVICE_VENDOR_ID                        0x87
#define CEC_MSG_GIVE_DEVICE_VENDOR_ID                   0x8c
#define CEC_MSG_VENDOR_COMMAND                          0x89
#define CEC_MSG_VENDOR_COMMAND_WITH_ID                  0xa0
#define CEC_MSG_VENDOR_REMOTE_BUTTON_DOWN               0x8a
#define CEC_MSG_VENDOR_REMOTE_BUTTON_UP                 0x8b

/* OSD Display Feature */
#define CEC_MSG_SET_OSD_STRING                          0x64
/* Display Control Operand (disp_ctl) */
#define CEC_OP_DISP_CTL_DEFAULT                         0x00
#define CEC_OP_DISP_CTL_UNTIL_CLEARED                   0x40
#define CEC_OP_DISP_CTL_CLEAR                           0x80

/* Device OSD Transfer Feature */
#define CEC_MSG_GIVE_OSD_NAME                           0x46
#define CEC_MSG_SET_OSD_NAME                            0x47

/* Device Menu Control Feature */
#define CEC_MSG_MENU_REQUEST                            0x8d
/* Menu Request Type Operand (menu_req) */
#define CEC_OP_MENU_REQUEST_ACTIVATE                    0x00
#define CEC_OP_MENU_REQUEST_DEACTIVATE                  0x01
#define CEC_OP_MENU_REQUEST_QUERY                       0x02

#define CEC_MSG_MENU_STATUS                             0x8e
/* Menu State Operand (menu_state) */
#define CEC_OP_MENU_STATE_ACTIVATED                     0x00
#define CEC_OP_MENU_STATE_DEACTIVATED                   0x01

#define CEC_MSG_USER_CONTROL_PRESSED                    0x44
/* UI Broadcast Type Operand (ui_bcast_type) */
#define CEC_OP_UI_BCAST_TYPE_TOGGLE_ALL                 0x00
#define CEC_OP_UI_BCAST_TYPE_TOGGLE_DIG_ANA             0x01
#define CEC_OP_UI_BCAST_TYPE_ANALOGUE                   0x10
#define CEC_OP_UI_BCAST_TYPE_ANALOGUE_T                 0x20
#define CEC_OP_UI_BCAST_TYPE_ANALOGUE_CABLE             0x30
#define CEC_OP_UI_BCAST_TYPE_ANALOGUE_SAT               0x40
#define CEC_OP_UI_BCAST_TYPE_DIGITAL                    0x50
#define CEC_OP_UI_BCAST_TYPE_DIGITAL_T                  0x60
#define CEC_OP_UI_BCAST_TYPE_DIGITAL_CABLE              0x70
#define CEC_OP_UI_BCAST_TYPE_DIGITAL_SAT                0x80
#define CEC_OP_UI_BCAST_TYPE_DIGITAL_COM_SAT            0x90
#define CEC_OP_UI_BCAST_TYPE_DIGITAL_COM_SAT2           0x91
#define CEC_OP_UI_BCAST_TYPE_IP                         0xa0
/* UI Sound Presentation Control Operand (ui_snd_pres_ctl) */
#define CEC_OP_UI_SND_PRES_CTL_DUAL_MONO                0x10
#define CEC_OP_UI_SND_PRES_CTL_KARAOKE                  0x20
#define CEC_OP_UI_SND_PRES_CTL_DOWNMIX                  0x80
#define CEC_OP_UI_SND_PRES_CTL_REVERB                   0x90
#define CEC_OP_UI_SND_PRES_CTL_EQUALIZER                0xa0
#define CEC_OP_UI_SND_PRES_CTL_BASS_UP                  0xb1
#define CEC_OP_UI_SND_PRES_CTL_BASS_NEUTRAL             0xb2
#define CEC_OP_UI_SND_PRES_CTL_BASS_DOWN                0xb3
#define CEC_OP_UI_SND_PRES_CTL_TREBLE_UP                0xc1
#define CEC_OP_UI_SND_PRES_CTL_TREBLE_NEUTRAL           0xc2
#define CEC_OP_UI_SND_PRES_CTL_TREBLE_DOWN              0xc3

#define CEC_MSG_USER_CONTROL_RELEASED                   0x45

/* Remote Control Passthrough Feature */

/*
 * Has also:
 *      CEC_MSG_USER_CONTROL_PRESSED
 *      CEC_MSG_USER_CONTROL_RELEASED
 */

/* Power Status Feature */
#define CEC_MSG_GIVE_DEVICE_POWER_STATUS                0x8f
#define CEC_MSG_REPORT_POWER_STATUS                     0x90
/* Power Status Operand (pwr_state) */
#define CEC_OP_POWER_STATUS_ON                          0
#define CEC_OP_POWER_STATUS_STANDBY                     1
#define CEC_OP_POWER_STATUS_TO_ON                       2
#define CEC_OP_POWER_STATUS_TO_STANDBY                  3

/* General Protocol Messages */
#define CEC_MSG_FEATURE_ABORT                           0x00
/* Abort Reason Operand (reason) */
#define CEC_OP_ABORT_UNRECOGNIZED_OP                    0
#define CEC_OP_ABORT_INCORRECT_MODE                     1
#define CEC_OP_ABORT_NO_SOURCE                          2
#define CEC_OP_ABORT_INVALID_OP                         3
#define CEC_OP_ABORT_REFUSED                            4
#define CEC_OP_ABORT_UNDETERMINED                       5

#define CEC_MSG_ABORT                                   0xff

/* System Audio Control Feature */

/*
 * Has also:
 *      CEC_MSG_USER_CONTROL_PRESSED
 *      CEC_MSG_USER_CONTROL_RELEASED
 */
#define CEC_MSG_GIVE_AUDIO_STATUS                       0x71
#define CEC_MSG_GIVE_SYSTEM_AUDIO_MODE_STATUS           0x7d
#define CEC_MSG_REPORT_AUDIO_STATUS                     0x7a
/* Audio Mute Status Operand (aud_mute_status) */
#define CEC_OP_AUD_MUTE_STATUS_OFF                      0
#define CEC_OP_AUD_MUTE_STATUS_ON                       1

#define CEC_MSG_REPORT_SHORT_AUDIO_DESCRIPTOR           0xa3
#define CEC_MSG_REQUEST_SHORT_AUDIO_DESCRIPTOR          0xa4
#define CEC_MSG_SET_SYSTEM_AUDIO_MODE                   0x72
/* System Audio Status Operand (sys_aud_status) */
#define CEC_OP_SYS_AUD_STATUS_OFF                       0
#define CEC_OP_SYS_AUD_STATUS_ON                        1

#define CEC_MSG_SYSTEM_AUDIO_MODE_REQUEST               0x70
#define CEC_MSG_SYSTEM_AUDIO_MODE_STATUS                0x7e
/* Audio Format ID Operand (audio_format_id) */
#define CEC_OP_AUD_FMT_ID_CEA861                        0
#define CEC_OP_AUD_FMT_ID_CEA861_CXT                    1

/* Audio Rate Control Feature */
#define CEC_MSG_SET_AUDIO_RATE                          0x9a
/* Audio Rate Operand (audio_rate) */
#define CEC_OP_AUD_RATE_OFF                             0
#define CEC_OP_AUD_RATE_WIDE_STD                        1
#define CEC_OP_AUD_RATE_WIDE_FAST                       2
#define CEC_OP_AUD_RATE_WIDE_SLOW                       3
#define CEC_OP_AUD_RATE_NARROW_STD                      4
#define CEC_OP_AUD_RATE_NARROW_FAST                     5
#define CEC_OP_AUD_RATE_NARROW_SLOW                     6

/* Audio Return Channel Control Feature */
#define CEC_MSG_INITIATE_ARC                            0xc0
#define CEC_MSG_REPORT_ARC_INITIATED                    0xc1
#define CEC_MSG_REPORT_ARC_TERMINATED                   0xc2
#define CEC_MSG_REQUEST_ARC_INITIATION                  0xc3
#define CEC_MSG_REQUEST_ARC_TERMINATION                 0xc4
#define CEC_MSG_TERMINATE_ARC                           0xc5

/* Dynamic Audio Lipsync Feature */
/* Only for CEC 2.0 and up */
#define CEC_MSG_REQUEST_CURRENT_LATENCY                 0xa7
#define CEC_MSG_REPORT_CURRENT_LATENCY                  0xa8
/* Low Latency Mode Operand (low_latency_mode) */
#define CEC_OP_LOW_LATENCY_MODE_OFF                     0
#define CEC_OP_LOW_LATENCY_MODE_ON                      1
/* Audio Output Compensated Operand (audio_out_compensated) */
#define CEC_OP_AUD_OUT_COMPENSATED_NA                   0
#define CEC_OP_AUD_OUT_COMPENSATED_DELAY                1
#define CEC_OP_AUD_OUT_COMPENSATED_NO_DELAY             2
#define CEC_OP_AUD_OUT_COMPENSATED_PARTIAL_DELAY        3

/* Capability Discovery and Control Feature */
#define CEC_MSG_CDC_MESSAGE                             0xf8
/* Ethernet-over-HDMI: nobody ever does this... */
#define CEC_MSG_CDC_HEC_INQUIRE_STATE                   0x00
#define CEC_MSG_CDC_HEC_REPORT_STATE                    0x01

/* HEC Functionality State Operand (hec_func_state) */
#define CEC_OP_HEC_FUNC_STATE_NOT_SUPPORTED             0
#define CEC_OP_HEC_FUNC_STATE_INACTIVE                  1
#define CEC_OP_HEC_FUNC_STATE_ACTIVE                    2
#define CEC_OP_HEC_FUNC_STATE_ACTIVATION_FIELD          3

/* Host Functionality State Operand (host_func_state) */
#define CEC_OP_HOST_FUNC_STATE_NOT_SUPPORTED            0
#define CEC_OP_HOST_FUNC_STATE_INACTIVE                 1
#define CEC_OP_HOST_FUNC_STATE_ACTIVE                   2

/* ENC Functionality State Operand (enc_func_state) */
#define CEC_OP_ENC_FUNC_STATE_EXT_CON_NOT_SUPPORTED     0
#define CEC_OP_ENC_FUNC_STATE_EXT_CON_INACTIVE          1
#define CEC_OP_ENC_FUNC_STATE_EXT_CON_ACTIVE            2

/* CDC Error Code Operand (cdc_errcode) */
#define CEC_OP_CDC_ERROR_CODE_NONE                      0
#define CEC_OP_CDC_ERROR_CODE_CAP_UNSUPPORTED           1
#define CEC_OP_CDC_ERROR_CODE_WRONG_STATE               2
#define CEC_OP_CDC_ERROR_CODE_OTHER                     3

/* HEC Support Operand (hec_support) */
#define CEC_OP_HEC_SUPPORT_NO                           0
#define CEC_OP_HEC_SUPPORT_YES                          1

/* HEC Activation Operand (hec_activation) */
#define CEC_OP_HEC_ACTIVATION_ON                        0
#define CEC_OP_HEC_ACTIVATION_OFF                       1

#define CEC_MSG_CDC_HEC_SET_STATE_ADJACENT              0x02
#define CEC_MSG_CDC_HEC_SET_STATE                       0x03

/* HEC Set State Operand (hec_set_state) */
#define CEC_OP_HEC_SET_STATE_DEACTIVATE                 0
#define CEC_OP_HEC_SET_STATE_ACTIVATE                   1

#define CEC_MSG_CDC_HEC_REQUEST_DEACTIVATION            0x04
#define CEC_MSG_CDC_HEC_NOTIFY_ALIVE                    0x05
#define CEC_MSG_CDC_HEC_DISCOVER                        0x06

/* Hotplug Detect messages */
#define CEC_MSG_CDC_HPD_SET_STATE                       0x10

/* HPD State Operand (hpd_state) */
#define CEC_OP_HPD_STATE_CP_EDID_DISABLE                0
#define CEC_OP_HPD_STATE_CP_EDID_ENABLE                 1
#define CEC_OP_HPD_STATE_CP_EDID_DISABLE_ENABLE         2
#define CEC_OP_HPD_STATE_EDID_DISABLE                   3
#define CEC_OP_HPD_STATE_EDID_ENABLE                    4
#define CEC_OP_HPD_STATE_EDID_DISABLE_ENABLE            5
#define CEC_MSG_CDC_HPD_REPORT_STATE                    0x11

/* HPD Error Code Operand (hpd_error) */
#define CEC_OP_HPD_ERROR_NONE                           0
#define CEC_OP_HPD_ERROR_INITIATOR_NOT_CAPABLE          1
#define CEC_OP_HPD_ERROR_INITIATOR_WRONG_STATE          2
#define CEC_OP_HPD_ERROR_OTHER                          3
#define CEC_OP_HPD_ERROR_NONE_NO_VIDEO                  4

/* End of Messages */

#endif



