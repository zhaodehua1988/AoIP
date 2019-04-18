#ifndef _FPGA_SDP_H_H_
#define _FPGA_SDP_H_H_

#include <time.h>
#include"wv_common.h"
struct sdp_connection {
    char *nettype;
    char *addrtype;
    char *address;
};

struct sdp_bandwidth {
    char *bwtype;
    char *bandwidth;
};

struct sdp_payload {
    char *_payload;

    unsigned char proto_version;
    struct sdp_origin {
        char *username;
        long long int sess_id;
        long long int sess_version;
        char *nettype;
        char *addrtype;
        char *addr;
    } origin;
    char *session_name;
    char *information;
    char *uri;
    char **emails;
    size_t emails_count;
    char **phones;
    size_t phones_count;
    struct sdp_connection conn;
    struct sdp_bandwidth *bw;
    size_t bw_count;
    struct sdp_time {
        time_t start_time;
        time_t stop_time;
        struct sdp_repeat {
            time_t interval;
            time_t duration;
            time_t *offsets;
            size_t offsets_count;
        } *repeat;
        size_t repeat_count;
    } *times;
    size_t times_count;
    struct sdp_zone_adjustments {
        time_t adjust;
        time_t offset;
    } *zone_adjustments;
    size_t zone_adjustments_count;
    char *encrypt_key;
    char **attributes;
    size_t attributes_count;
    struct sdp_media {
        struct sdp_info {
            char *type;
            int port;
            int port_n;
            char *proto;
            int *fmt;
            size_t fmt_count;
        } info;
        char *title;
        struct sdp_connection conn;
        struct sdp_bandwidth *bw;
        size_t bw_count;
        char *encrypt_key;
        char **attributes;
        size_t attributes_count;
    } *medias;
    size_t medias_count;
};

typedef struct FPGA_SDP_Info{

    char video_colorimetry[20];  //色彩空间
    unsigned short video_interlace;  //0:逐行 1:隔行
    char video_framerate[24];    //帧率
    unsigned short video_depth;  //视频位宽

    //0：YCbCr:4:4:4；1:YCbCr:4:2:2；2：YCbCr:4:2:0；3：RGB； 4：CLYCbCr-4:4:4 5：CLYCbCr-4:2:2 ；6：CLYCbCr-4:2:0：7：ICtCp-4:4:48：ICtCp-4:2:2   9：ICtCp-4:2:0
    char video_sampling[20];     //视频编码格式
    unsigned short video_width;  //图像宽度
    unsigned short video_height; //图像高度
    unsigned short video_pt;     //视频载荷类型
    unsigned short audio_pt;     //音频载荷类型
    unsigned short audio_depth;  //音频位宽
    char audio_channel[128];
    unsigned short audio_chl_num;//音频通道数

}FPGA_SDP_Info;
struct sdp_payload *sdp_parse(const char *payload);
void sdp_destroy(struct sdp_payload *sdp);
void sdp_dump(struct sdp_payload *sdp);

char *sdp_get_attr(char **attr, size_t nattr, char *key);
int sdp_has_flag_attr(char **attr, size_t nattr, char *flag);

/****************************************************************************
 * int FPGA_SDP_AnalysisSdpInfo(char *pSdpData, FPGA_SDP_Info *pGetInfo)
 * 函数说明：解析sdp信息
 * 参数说明：
 *     pSdpData：输入的sdp报文
 *     pGetInfo：输出为FPGA_SDP_Info结构体信息
 * 返回值： 
 *     0  ：代表解析成功
 *     -1 ：代表解析失败
 * *************************************************************************/
int FPGA_SDP_AnalysisSdpInfo(char *pSdpData, FPGA_SDP_Info *pGetInfo);



/****************************************************************************
 * int FPGA_SDP_MakeSdpData(char *pSdpData, FPGA_SDP_Info *pGetInfo)
 * 函数说明：生成sdp报文
 * 参数说明：
 *     pInputInfo：输入的sdp参数
 *     pSdpOutData：输出的sdp报文
 *     sdpLen：sdp报文长度
 * 返回值： 
 *     0  ：代表生存sdp报文成功
 *     -1 ：代表生成sdp报文失败
 * *************************************************************************/
int FPGA_SDP_MakeSdpData(FPGA_SDP_Info *pInputInfo,char *pSdpOutData,unsigned int *sdpLen);

void FPGA_SDP_Init();
void FPGA_SDP_DeInit();

#endif
