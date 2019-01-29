#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <stdio.h>
#include <stdarg.h>
#include "fpga_sdp.h"
#include "his_spi.h"

//#define FPGA_SDP_DEBUG
static char *load_next_entry(char *p, char *key, char **value)
{
    char *endl;

    if (!p)
        goto fail;

    endl = strstr(p, "\r\n");
    if (!endl)
        endl = strchr(p, '\n');

    if (endl)
        while (*endl == '\r' || *endl == '\n')
            *endl++ = '\0';
    else
        endl = &p[strlen(p)];

    if (!p[0] || p[1] != '=')
        goto fail;

    *key = p[0];
    *value = &p[2];

    return endl;

fail:
    *key = 0;
    *value = NULL;
    return NULL;
}

static char *split_values(char *p, char sep, char *fmt, ...)
{
    va_list va;

    va_start(va, fmt);
    while (*p == sep)
        p++;
    while (*fmt)
    {
        char **s, *tmp;
        int *i;
        long long int *l;
        time_t *t;

        switch (*fmt++)
        {
        case 's':
            s = va_arg(va, char **);
            *s = p;
            tmp = strchr(p, sep);
            if (tmp)
            {
                p = tmp;
                while (*p == sep)
                    *p++ = '\0';
            }
            else
            {
                p = &p[strlen(p)];
            }
            break;
        case 'l':
            l = va_arg(va, long long int *);
            *l = strtoll(p, &tmp, 10);
            if (tmp == p)
                *p = 0;
            else
                p = tmp;
            break;
        case 'i':
            i = va_arg(va, int *);
            *i = strtol(p, &tmp, 10);
            if (tmp == p)
                *p = 0;
            else
                p = tmp;
            break;
        case 't':
            t = va_arg(va, time_t *);
            *t = strtol(p, &tmp, 10);
            if (tmp == p)
            {
                *p = 0;
            }
            else
            {
                p = tmp;
                switch (*p)
                {
                case 'd':
                    *t *= 86400;
                    p++;
                    break;
                case 'h':
                    *t *= 3600;
                    p++;
                    break;
                case 'm':
                    *t *= 60;
                    p++;
                    break;
                }
            }
            break;
        }
        while (*p == sep)
            p++;
    }
    va_end(va);
    return p;
}

#define GET_CONN_INFO(connf_ptr)                                       \
    do                                                                 \
    {                                                                  \
        if (key == 'c')                                                \
        {                                                              \
            struct sdp_connection *c = connf_ptr;                      \
            split_values(value, ' ', "sss", &c->nettype, &c->addrtype, \
                         &c->address);                                 \
            p = load_next_entry(p, &key, &value);                      \
        }                                                              \
    } while (0)

#define GET_BANDWIDTH_INFO(bw)                            \
    do                                                    \
    {                                                     \
        int n;                                            \
        while (key == 'b')                                \
        {                                                 \
            ADD_ENTRY(bw);                                \
            n = bw##_count - 1;                           \
            split_values(value, ':', "ss", &bw[n].bwtype, \
                         &bw[n].bandwidth);               \
            p = load_next_entry(p, &key, &value);         \
        }                                                 \
    } while (0)

#define LOAD_FACULTATIVE_STR(k, field)            \
    do                                            \
    {                                             \
        if (key == k)                             \
        {                                         \
            field = value;                        \
            p = load_next_entry(p, &key, &value); \
        }                                         \
    } while (0)

#define LOAD_MULTIPLE_FACULTATIVE_STR(k, field)   \
    do                                            \
    {                                             \
        while (key == k)                          \
        {                                         \
            ADD_ENTRY(field);                     \
            field[field##_count - 1] = value;     \
            p = load_next_entry(p, &key, &value); \
        }                                         \
    } while (0)

#define ADD_ENTRY(field)                                \
    do                                                  \
    {                                                   \
        field##_count++;                                \
        if (!field)                                     \
        {                                               \
            field = calloc(1, sizeof(*field));          \
        }                                               \
        else                                            \
        {                                               \
            int n = field##_count;                      \
            field = realloc(field, sizeof(*field) * n); \
            memset(&field[n - 1], 0, sizeof(*field));   \
        }                                               \
        if (!(field))                                   \
            goto fail;                                  \
    } while (0)

struct sdp_payload *sdp_parse(const char *payload)
{
    struct sdp_payload *sdp = calloc(1, sizeof(*sdp));
    char *p, key, *value;

    if (!sdp)
        goto fail;

    p = sdp->_payload = strdup(payload);
    if (!p)
        goto fail;

    /* Protocol version (mandatory, only 0 supported) */
    p = load_next_entry(p, &key, &value);
    if (key != 'v')
        goto fail;
    sdp->proto_version = value[0] - '0';
    if (sdp->proto_version != 0 || value[1])
        goto fail;

    /* Origin field (mandatory) */
    p = load_next_entry(p, &key, &value);
    if (key != 'o')
        goto fail;
    else
    {
        struct sdp_origin *o = &sdp->origin;
        split_values(value, ' ', "sllsss", &o->username, &o->sess_id,
                     &o->sess_version, &o->nettype, &o->addrtype, &o->addr);
    }

    /* Session name field (mandatory) */
    p = load_next_entry(p, &key, &value);
    if (key != 's')
        goto fail;
    sdp->session_name = value;
    p = load_next_entry(p, &key, &value);

    /* Information field */
    LOAD_FACULTATIVE_STR('i', sdp->information);

    /* URI field */
    LOAD_FACULTATIVE_STR('u', sdp->uri);

    /* Email addresses */
    LOAD_MULTIPLE_FACULTATIVE_STR('e', sdp->emails);

    /* Phone numbers */
    LOAD_MULTIPLE_FACULTATIVE_STR('p', sdp->phones);

    /* Connection information */
    GET_CONN_INFO(&sdp->conn);

    /* Bandwidth fields */
    GET_BANDWIDTH_INFO(sdp->bw);

    /* Time fields (at least one mandatory) */
    do
    {
        struct sdp_time *tf;

        ADD_ENTRY(sdp->times);
        tf = &sdp->times[sdp->times_count - 1];
        split_values(value, ' ', "tt", &tf->start_time, &tf->stop_time);
        p = load_next_entry(p, &key, &value);

        while (key == 'r')
        {
            struct sdp_repeat *rf;

            ADD_ENTRY(tf->repeat);
            rf = &tf->repeat[tf->repeat_count - 1];
            value = split_values(value, ' ', "tt", &rf->interval, &rf->duration);
            while (*value)
            {
                int n = rf->offsets_count;
                ADD_ENTRY(rf->offsets);
                value = split_values(value, ' ', "t", &rf->offsets[n]);
            }
            p = load_next_entry(p, &key, &value);
        }
    } while (key == 't');

    /* Zone adjustments */
    if (key == 'z')
    {
        while (*value)
        {
            int n = sdp->zone_adjustments_count;
            struct sdp_zone_adjustments *za;

            ADD_ENTRY(sdp->zone_adjustments);
            za = &sdp->zone_adjustments[n];
            value = split_values(value, ' ', "tt", &za->adjust, &za->offset);
        }
        p = load_next_entry(p, &key, &value);
    }

    /* Encryption key */
    LOAD_FACULTATIVE_STR('k', sdp->encrypt_key);

    /* Media attributes */
    LOAD_MULTIPLE_FACULTATIVE_STR('a', sdp->attributes);

    /* Media descriptions */
    while (key == 'm')
    {
        struct sdp_media *md;

        ADD_ENTRY(sdp->medias);
        md = &sdp->medias[sdp->medias_count - 1];

        value = split_values(value, ' ', "s", &md->info.type);
        md->info.port = strtol(value, &value, 10);
        md->info.port_n = *value == '/' ? strtol(value + 1, &value, 10) : 0;
        value = split_values(value, ' ', "s", &md->info.proto);
        while (*value)
        {
            ADD_ENTRY(md->info.fmt);
            value = split_values(value, ' ', "i", &md->info.fmt[md->info.fmt_count - 1]);
        }
        p = load_next_entry(p, &key, &value);

        LOAD_FACULTATIVE_STR('i', md->title);
        GET_CONN_INFO(&md->conn);
        GET_BANDWIDTH_INFO(md->bw);
        LOAD_FACULTATIVE_STR('k', md->encrypt_key);
        LOAD_MULTIPLE_FACULTATIVE_STR('a', md->attributes);
    }

    return sdp;

fail:
    sdp_destroy(sdp);
    return NULL;
}

void sdp_destroy(struct sdp_payload *sdp)
{
    size_t i, j;

    if (sdp)
    {
        free(sdp->_payload);
        free(sdp->emails);
        free(sdp->phones);
        free(sdp->bw);
        for (i = 0; i < sdp->times_count; i++)
        {
            for (j = 0; j < sdp->times[i].repeat_count; j++)
                free(sdp->times[i].repeat[j].offsets);
            free(sdp->times[i].repeat);
        }
        free(sdp->times);
        free(sdp->zone_adjustments);
        free(sdp->attributes);
        for (i = 0; i < sdp->medias_count; i++)
        {
            free(sdp->medias[i].info.fmt);
            free(sdp->medias[i].bw);
            free(sdp->medias[i].attributes);
        }
        free(sdp->medias);
    }
    free(sdp);
}

char *sdp_get_attr(char **attr, size_t nattr, char *key)
{
    size_t i, klen = strlen(key);

    for (i = 0; i < nattr; i++)
        if (!strncmp(attr[i], key, klen) && attr[i][klen] == ':')
            return &attr[i][klen + 1];
    return NULL;
}

int sdp_has_flag_attr(char **attr, size_t nattr, char *flag)
{
    size_t i;

    for (i = 0; i < nattr; i++)
        if (!strcmp(attr[i], flag))
            return 1;
    return 0;
}

void sdp_dump(struct sdp_payload *sdp)
{
    size_t i, j, k;

    if (!sdp)
    {
        printf("invalid SDP\n");
        return;
    }

    printf("v=%d\n", sdp->proto_version);
    printf("o=%s %lld %lld %s %s %s\n", sdp->origin.username,
           sdp->origin.sess_id, sdp->origin.sess_version, sdp->origin.nettype,
           sdp->origin.addrtype, sdp->origin.addr);
    printf("s=%s\n", sdp->session_name);

    if (sdp->information)
        printf("i=%s\n", sdp->information);
    if (sdp->uri)
        printf("u=%s\n", sdp->uri);

    for (i = 0; i < sdp->emails_count; i++)
        printf("e=%s\n", sdp->emails[i]);
    for (i = 0; i < sdp->phones_count; i++)
        printf("p=%s\n", sdp->phones[i]);

    if (sdp->conn.nettype && sdp->conn.addrtype && sdp->conn.address)
        printf("c=%s %s %s\n",
               sdp->conn.nettype, sdp->conn.addrtype, sdp->conn.address);

    for (i = 0; i < sdp->bw_count; i++)
        printf("b=%s:%s\n", sdp->bw[i].bwtype, sdp->bw[i].bandwidth);

    for (i = 0; i < sdp->times_count; i++)
    {
        struct sdp_time *t = &sdp->times[i];
        printf("t=%ld %ld\n", t->start_time, t->stop_time);
        for (j = 0; j < t->repeat_count; j++)
        {
            struct sdp_repeat *r = &t->repeat[j];
            printf("r=%ld %ld", r->interval, r->duration);
            for (k = 0; k < r->offsets_count; k++)
                printf(" %ld", r->offsets[k]);
            printf("\n");
        }
    }

    if (sdp->zone_adjustments_count)
    {
        printf("z=");
        for (i = 0; i < sdp->zone_adjustments_count; i++)
            printf("%ld %ld%s", sdp->zone_adjustments[i].adjust,
                   sdp->zone_adjustments[i].offset,
                   i + 1 < sdp->zone_adjustments_count ? " " : "");
        printf("\n");
    }

    if (sdp->encrypt_key)
        printf("k=%s\n", sdp->encrypt_key);

    for (i = 0; i < sdp->attributes_count; i++)
        printf("a=%s\n", sdp->attributes[i]);

    for (i = 0; i < sdp->medias_count; i++)
    {
        struct sdp_media *m = &sdp->medias[i];
        struct sdp_info *info = &m->info;

        printf("m=%s %d", info->type, info->port);
        if (info->port_n)
            printf("/%d", info->port_n);
        printf(" %s", info->proto);
        for (j = 0; j < info->fmt_count; j++)
            printf(" %d", info->fmt[j]);
        printf("\n");

        if (m->title)
            printf("i=%s\n", m->title);
        if (m->conn.nettype && m->conn.addrtype && m->conn.address)
            printf("c=%s %s %s\n",
                   m->conn.nettype, m->conn.addrtype, m->conn.address);
        for (j = 0; j < m->bw_count; j++)
            printf("b=%s:%s\n", m->bw[j].bwtype, m->bw[j].bandwidth);
        if (m->encrypt_key)
            printf("k=%s\n", m->encrypt_key);
        for (j = 0; j < m->attributes_count; j++)
            printf("a=%s\n", m->attributes[j]);
    }
}
/****************************************************************************
 * int FPGA_SDP_GetParmStr(char *pData,char *pItem,char *pOutStr,int *pStrLen)
 * description：从字符出pData中找到某个item的字符串值和字符串长度，成功返回0，其他返回-1；
 * 必须是 查找的字符串格式为：item=outstr或者item:outstr
 * *************************************************************************/
int FPGA_SDP_GetParmStr(char *pData, char *pItem, char *pOutStr, int *pStrLen)
{
    int i = 0;
    char *p = strstr(pData, pItem);
    if (p == NULL)
    {
        return -1;
    }
    p += strlen(pItem);
    if (p[0] != '=' && p[0] != ':')
    {
        printf("can not find %c \n", p[0]);
        return -1;
    }
    p++;
    while (p[i] == ';' || p[i] == ' ' || p[i] == '\r' || p[i] == '\n')
    {
        i++;
        if (i > 20)
        {
            printf("get str err;parm len > 20\n");
            return -1;
        }
    }

    memcpy(pOutStr, p, i);
    *pStrLen = i;
    return 0;
}
/****************************************************************************
 * int FPGA_SDP_GetParmUint(char *pData,char *pItem,unsigned int *pOutUint)
 * description：从字符出pData中找到某个item的数值（整型），成功返回0，其他返回-1；
 * 必须是 查找的字符串格式为：item=outUint或者item:outUint
 * *************************************************************************/
int FPGA_SDP_GetParmUint(char *pData, char *pItem, unsigned int *pOutUint)
{
    char *p = strstr(pData, pItem);
    if (p == NULL)
    {
        return -1;
    }
    p += strlen(pItem);
    if (p[0] != '=' && p[0] != ':')
    {
        printf("can not find = or :// %c \n", p[0]);
        return -1;
    }
    p++;
    if (sscanf(p, "%d", pOutUint) != 1)
    {
        printf("can not get the item(%s) val(uint)\n", pItem);
        return -1;
    }
    return 0;
}
/****************************************************************************
 * int FPGA_SDP_GetInfo(struct sdp_payload *sdp,FPGA_SDP_Info *pGetInfo)
 * description：获取fpga所需要的sdp media info
 * 
 * *************************************************************************/
int FPGA_SDP_GetInfo(struct sdp_payload *sdp, FPGA_SDP_Info *pGetInfo)
{
    size_t i, j;
    char pOut[24] = {0};
    unsigned int data;
    int strLen = 0, videoST2110 = -1, audioST2110 = -1;
    if (!sdp)
    {
        printf("invalid SDP\n");
        return -1;
    }

    for (i = 0; i < sdp->medias_count; i++)
    {

        struct sdp_media *m = &sdp->medias[i];
        struct sdp_info *info = &m->info;
        if (strcmp(info->type, "video") == 0)
        {
            for (j = 0; j < m->attributes_count; j++)
            {

                //printf("a=%s\n", m->attributes[j]);
                //int FPGA_SDP_GetParmStr(char *pData,char *pItem,char *pOutStr,int *pStrLen);
                //get sdd type ST2110
                if (FPGA_SDP_GetParmStr(m->attributes[j], "SSN", pOut, &strLen) == 0)
                {
                    if (strstr(pOut, "ST2110") != NULL)
                        videoST2110 = 1;
                }
                //get colorimetry
                memset(pOut, 0, sizeof(pOut));
                strLen = 0;
                if (FPGA_SDP_GetParmStr(m->attributes[j], "colorimetry", pOut, &strLen) == 0)
                {
                    memcpy(pGetInfo->video_colorimetry, pOut, strLen);
                }
                //get interlace
                if (FPGA_SDP_GetParmUint(m->attributes[j], "interlace", &data) == 0)
                {
                    pGetInfo->video_interlace = data;
                }
                //get framerate
                memset(pOut, 0, sizeof(pOut));
                strLen = 0;
                if (FPGA_SDP_GetParmStr(m->attributes[j], "framerate", pOut, &strLen) == 0)
                {
                    //pOut = 60000/1001;
                    int i, a = 0, b = 0;
                    sscanf(pOut, "%d", &a);
                    char *p = pOut;
                    for (i = 0; i < strLen; i++)
                    {
                        if (p[i] == '/')
                        {
                            p = p + i + 1;
                            sscanf(p, "%d", &b);
                            break;
                        }
                    }
                    if (b != 0)
                    {
                        sprintf(pGetInfo->video_framerate, "%d.%d", a / b, a % b);
                    }
                    else
                    {
                        sprintf(pGetInfo->video_framerate, "%d", 60);
                        //printf("sdp video framerate = %s \n", pGetInfo->video_framerate);
                    }
                }
                //get depth
                if (FPGA_SDP_GetParmUint(m->attributes[j], "depth", &data) == 0)
                {
                    pGetInfo->video_depth = data;
                }
                //get sampling
                memset(pOut, 0, sizeof(pOut));
                strLen = 0;
                if (FPGA_SDP_GetParmStr(m->attributes[j], "sampling", pOut, &strLen) == 0)
                {
                    memcpy(pGetInfo->video_sampling, pOut, strLen);
                }
                //get video width
                if (FPGA_SDP_GetParmUint(m->attributes[j], "width", &data) == 0)
                {
                    pGetInfo->video_width = data;
                }
                //get video hight
                if (FPGA_SDP_GetParmUint(m->attributes[j], "height", &data) == 0)
                {
                    pGetInfo->video_height = data;
                }
                //get video pt
                if (FPGA_SDP_GetParmUint(m->attributes[j], "fmtp", &data) == 0)
                {
                    pGetInfo->video_pt = data;
                }
            }
        }
        else if (strcmp(info->type, "audio"))
        {
            for (j = 0; j < m->attributes_count; j++)
            {
                //get ST2110
                if (FPGA_SDP_GetParmStr(m->attributes[j], "channel-order", pOut, &strLen) == 0)
                {
                    if (strstr(pOut, "SMPTE2110") != NULL)
                        audioST2110 = 1;
                }
                //get audio fmtp
                if (FPGA_SDP_GetParmUint(m->attributes[j], "fmtp", &data) == 0)
                {
                    pGetInfo->audio_pt = data;
                }
                //get audio depth
                if (FPGA_SDP_GetParmUint(m->attributes[j], "depth", &data) == 0)
                {
                    pGetInfo->audio_depth = data;
                }
                //channel-order
                if (FPGA_SDP_GetParmStr(m->attributes[j], "channel-order", pOut, &strLen) == 0)
                {
                    int i, len;
                    char *p = NULL;
                    p = strstr(pOut, "(");
                    if (p != NULL)
                    {
                        p++;
                        len = (int)strlen(p);
                        for (i = 0; len; i++)
                        {
                            if (p[0] == ')')
                            {
                                break;
                            }
                            pGetInfo->audio_chl[i] = p[0];
                            p++;
                        }
                    }
                    printf("\naudio_chl=%s\n", pGetInfo->audio_chl);
                }
            }
        }
    }

    if (audioST2110 != 1 && videoST2110 != 1)
    {
        return -1;
    }

    return 0;
}

/**********************************************************
 * int strstr_cnt(const char *string ,const char *substring)
 * 查询某个字符串在另外一个字符串中出现的次数
**********************************************************/
int strstr_cnt(const char *string ,const char *substring)
{

     int i,j,k,count=0;
     for(i=0;string[i];i++){
        for(j=i,k=0;string[j]==substring[k];j++,k++){
            if(j==strlen(string) ) break;
        
            if(!substring[k+1]){
                count++;
            }
        }
        
     }

     return(count);
}
/**********************************************************
 * int fpga_sdp_getAudioChlNum(char *pAudioChlIn)
 * 查询sdp音频声道数量
**********************************************************/
int fpga_sdp_getAudioChlNum(char *pAudioChlIn)
{
    //printf();
    //audiochl = "ST,51"
    int chlNum = 0;
    int undefinedChl=0,i=0;
    int M=0,DM=0,ST=0,LtRt=0,_51=0,_71=0,_222=0,SGRP=0,U=0;
    M    = strstr_cnt(pAudioChlIn,"M")-strstr_cnt(pAudioChlIn,"DM");
    DM   = strstr_cnt(pAudioChlIn,"DM")*2;
    ST   = strstr_cnt(pAudioChlIn,"ST")*2;
    LtRt = strstr_cnt(pAudioChlIn,"LtRt")*2;
    _51  = strstr_cnt(pAudioChlIn,"51")*6;
    _71  = strstr_cnt(pAudioChlIn,"71")*8;
    _222 = strstr_cnt(pAudioChlIn,"222")*24;
    SGRP = strstr_cnt(pAudioChlIn,"SGRP")*4;

    undefinedChl=strstr_cnt(pAudioChlIn,"U");
    
    char *p=strstr(pAudioChlIn,"U");

    for(i=0;i<undefinedChl;i++)
    {

        if(strlen(p)>=3)
        {
            int num;
            if (sscanf(&p[1], "%d", &num) != 1)
            {
                //printf("can not get the item(%s) val(uint)\n", pItem);
                return -1;
            }else{
                //printf("U%02d = %d |",num,num);
                U+=num;
            }            
        }

    }
    chlNum=M + DM + ST + LtRt + _51 + _71 +_222 + SGRP + U;
 #ifdef FPGA_SDP_DEBUG   
    printf("\r\nchlNum=%d,M=%d, DM=%d, ST=%d, LtRt=%d, 51=%d, 71=%d, 222=%d,SGPR=%d,U=%d\n",chlNum,M,DM,ST,LtRt,_51,_71,_222,SGRP,U);
#endif
    return chlNum;
}
/**********************************************************************************
 * int FPGA_SDP_SetInfo(FPGA_SDP_Info *pSetInfo,unsigned short eth,unsigned short channel)
 * 功能：配置网络接收的sdp信息
 * 参数说明：
 * pSetInfo:sdp数据
 * type: 0=videosdp 1=audiosdp 2=videoAndAudio sdp
 * eth:网卡id【0～3】
 * channel:第几个ip地址
 * *******************************************************************************/
int FPGA_SDP_SetInfo(FPGA_SDP_Info *pSetInfo,WV_U16 eth, WV_U16 channel)
{
    WV_S32 ret=0;
    WV_U16 baseAddr; 
    WV_U16 regAddr; 
    WV_U16 videoInfo = 0,videoWidth=0,videoHight=0,avPt=0,audioInfo=0;
    //video_sampling
    //0：YCbCr:4:4:4    1:YCbCr:4:2:2    ：YCbCr:4:2:0      3：RGB             4：CLYCbCr-4:4:4    5：CLYCbCr-4:2:2
    //6：CLYCbCr-4:2:0  7：ICtCp-4:4:4   8：ICtCp-4:2:2      9：ICtCp-4:2:0    10：XYZ 其他：预留
    // Bit[3:0]
    if (0 == strcmp(pSetInfo->video_sampling, "YCbCr-4:4:4") || 0 == strcmp(pSetInfo->video_sampling, "YCbCr:4:4:4"))
    {
        videoInfo |= 0x0;
    }
    else if (0 == strcmp(pSetInfo->video_sampling, "YCbCr-4:2:2") || 0 == strcmp(pSetInfo->video_sampling, "YCbCr:4:2:2"))
    {
        videoInfo |= 0x1;
    }
    else if (0 == strcmp(pSetInfo->video_sampling, "YCbCr-4:2:0") || 0 == strcmp(pSetInfo->video_sampling, "YCbCr:4:2:0"))
    {
        videoInfo |= 0x2;
    }
    else if (0 == strcmp(pSetInfo->video_sampling, "RGB") || 0 == strcmp(pSetInfo->video_sampling, "rgb"))
    {
        videoInfo |= 0x3;
    }
    else if (0 == strcmp(pSetInfo->video_sampling, "CLYCbCr-4:4:4") || 0 == strcmp(pSetInfo->video_sampling, "CLYCbCr:4:4:4"))
    {
        videoInfo |= 0x4;
    }
    else if (0 == strcmp(pSetInfo->video_sampling, "CLYCbCr-4:2:2") || 0 == strcmp(pSetInfo->video_sampling, "CLYCbCr:4:2:2"))
    {
        videoInfo |= 0x5;
    }
    else if (0 == strcmp(pSetInfo->video_sampling, "CLYCbCr-4:2:0") || 0 == strcmp(pSetInfo->video_sampling, "CLYCbCr:4:2:0"))
    {
        videoInfo |= 0x6;
    }
    else if (0 == strcmp(pSetInfo->video_sampling, "ICtCp-4:4:4") || 0 == strcmp(pSetInfo->video_sampling, "ICtCp:4:4:4"))
    {
        videoInfo |= 0x7;
    }
    else if (0 == strcmp(pSetInfo->video_sampling, "ICtCp-4:2:2") || 0 == strcmp(pSetInfo->video_sampling, "ICtCp:4:2:2"))
    {
        videoInfo |= 0x8;
    }
    else if (0 == strcmp(pSetInfo->video_sampling, "ICtCp-4:2:0") || 0 == strcmp(pSetInfo->video_sampling, "ICtCp:4:2:0"))
    {
        videoInfo |= 0x9;
    }
    else
    {
        videoInfo |= 0x1; //默认YCbCr-4:2:2
    }
    //video_depth
    if (8 == pSetInfo->video_depth)
    {
        videoInfo |= 0x0 << 4;
    }
    else if (10 == pSetInfo->video_depth)
    {
        videoInfo |= 0x1 << 4;
    }
    else if (12 == pSetInfo->video_depth)
    {
        videoInfo |= 0x2 << 4;
    }
    else if (16 == pSetInfo->video_depth)
    {
        videoInfo |= 0x3 << 4;
    }
    else if (24 == pSetInfo->video_depth)
    {
        videoInfo |= 0x4 << 4;
    }else {
        videoInfo |= 0x0 << 4; //默认为 video_depth=8
    }
    //video_framerate
    if(strstr(pSetInfo->video_framerate,"50") != NULL){
        videoInfo |= 0x0 << 7;
    }else if(strstr(pSetInfo->video_framerate,"60") != NULL){
        videoInfo |= 0x1 << 7;
    }else if(strstr(pSetInfo->video_framerate,"23.98") != NULL || strstr(pSetInfo->video_framerate,"23") !=NULL ){
        videoInfo |= 0x2 << 7;
    }else if(strstr(pSetInfo->video_framerate,"24") != NULL){
        videoInfo |= 0x3 << 7;
    }else if(strstr(pSetInfo->video_framerate,"47.95") != NULL || strstr(pSetInfo->video_framerate,"47") !=NULL){
        videoInfo |= 0x4 << 7;
    }else if(strstr(pSetInfo->video_framerate,"25") != NULL){
        videoInfo |= 0x5 << 7;
    }else if(strstr(pSetInfo->video_framerate,"29.97") != NULL || strstr(pSetInfo->video_framerate,"29") !=NULL){
        videoInfo |= 0x6 << 7;
    }else if(strstr(pSetInfo->video_framerate,"30") != NULL){
        videoInfo |= 0x7 << 7;
    }else if(strstr(pSetInfo->video_framerate,"48") != NULL){
        videoInfo |= 0x8 << 7;
    }else if(strstr(pSetInfo->video_framerate,"59.94") != NULL || strstr(pSetInfo->video_framerate,"59") !=NULL){
        videoInfo |= 0x9 << 7;
    }else{
         videoInfo |= 0x1 << 7; //默认60MHZ
    }

    //video_interlace
    //Bit11 b1 隔行 b0逐行

    if(1==pSetInfo->video_interlace){
        videoInfo |= 0x1 << 11;
    }else{
        videoInfo |= 0x0 << 11; //默认逐行
    }
    //video_colorimetry 
    if(0 == strcmp(pSetInfo->video_colorimetry,"BT.709") || 0 == strcmp(pSetInfo->video_colorimetry,"BT709")){
        videoInfo |= 0x0 << 12;
    }else if(0 == strcmp(pSetInfo->video_colorimetry,"BT.2020") || 0 == strcmp(pSetInfo->video_colorimetry,"BT2020")){
        videoInfo |= 0x1 << 12;
    }else if(0 == strcmp(pSetInfo->video_colorimetry,"BT.610") || 0 == strcmp(pSetInfo->video_colorimetry,"BT610")){
        videoInfo |= 0x2 << 12;
    }else{
        videoInfo |= 0x0 << 12;//默认BT.709
    }
     /****************audio info***********************************************/
    /*         Table 1 -- Channel Order Convention Grouping Symbols
    ------------------------------------------------------------------------------------------------------------------
    ID      Channel Grouping |  Quantity of Audio | Description of group   | Order of Audio
            Symbol           |  Channels in group |                        | Channels in group
                                        
    0               M               1                   Mono                    Mono
    1               DM              2                   Dual Mono               M1, M2
    2               ST              2                   Standard Stereo         Left, Right
    3               LtRt            2                   Matrix Stereo           Left Total, Right Total
    4               51              6                   5.1 Surround            L, R, C, LFE, Ls, Rs
    5               71              8                   7.1 Surround            L, R, C, LFE, Lss, Rss, Lrs, Rrs
    6               222             24                  22.2 Surround           Order shall be per SMPTE ST 2036-2,
    7               SGRP            4                   One SDI audio group     1 , 2, 3, 4
    */
   //audio chl 因为fpga文档没有更新，暂时保留

    WV_S32 chl=fpga_sdp_getAudioChlNum(pSetInfo->audio_chl);

    audioInfo = 0xff & chl;
   //audio_depth
   if(8 == pSetInfo->audio_depth){
       audioInfo |= 0x0 << 8;
   }else if(16 == pSetInfo->audio_depth){
       audioInfo |= 0x1 << 8;
   }else{
       audioInfo |= 0x0 << 8;
   }
    /****************video width /hight***********************************************/
    videoWidth = pSetInfo->video_width;
    videoHight = pSetInfo->video_height;
    //avPt
    avPt = (pSetInfo->video_pt << 8) | (pSetInfo->audio_pt & 0xff);
    baseAddr = 0x100;
    regAddr = ((baseAddr >> 8) + eth) << 8;

    ret+=HIS_SPI_FpgaWd(regAddr + 0x46 +channel*5,videoInfo);
    ret+=HIS_SPI_FpgaWd(regAddr + 0x47 +channel*5,videoWidth);
    ret+=HIS_SPI_FpgaWd(regAddr + 0x48 +channel*5,videoHight);
    ret+=HIS_SPI_FpgaWd(regAddr + 0x49 +channel*5,avPt);
    ret+=HIS_SPI_FpgaWd(regAddr + 0x4a +channel*5,audioInfo);


    if(ret != 0 ){
        WV_printf("set sdp info err \n");
    }

#ifdef FPGA_SDP_DEBUG
    printf("--------------sdp---------------------------\n");
    printf("set spi  0x%04x  0x%04x\n",regAddr + 0x46 +channel*5,videoInfo);
    printf("set spi  0x%04x  0x%04x\n",regAddr + 0x47 +channel*5,videoWidth);
    printf("set spi  0x%04x  0x%04x\n",regAddr + 0x48 +channel*5,videoHight);
    printf("set spi  0x%04x  0x%04x\n",regAddr + 0x49 +channel*5,avPt);
    printf("set spi  0x%04x  0x%04x\n",regAddr + 0x4a +channel*5,audioInfo);
    
#endif
    return ret;

}
