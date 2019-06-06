#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <stdio.h>
#include <stdarg.h>
#include "fpga_sdp.h"
#include "his_spi.h"
#include "fpga_conf.h"
#include "fpga_common.h"
extern FPGA_CONF_DEV *gpFpgaConfDev;
#define _FPGA_SDP_DATALEN (2048)
typedef struct FPGA_SDP_DEV
{

    WV_THR_HNDL_T thrHndl;
    WV_U32 open;
    WV_U32 close;
    WV_S8 *pSdpData;
} FPGA_SDP_DEV;

FPGA_SDP_DEV gFpgaSdpDev;

pthread_mutex_t gMutexUpdateSdpInfo;
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
 * int FPGA_SDP_AnalysisSdpInfo(char *pSdpData, FPGA_SDP_Info *pGetInfo)
 * 函数说明：解析sdp信息
 * 参数说明：
 *     pSdpData：输入的sdp报文
 *     pGetInfo：输出为FPGA_SDP_Info结构体信息
 * 返回值： 
 *     0  ：代表解析成功
 *     -1 ：代表解析失败
 * *************************************************************************/
int FPGA_SDP_AnalysisSdpInfo(char *pSdpData, FPGA_SDP_Info *pGetInfo)
{

    if(pSdpData == NULL) return -1;
    struct sdp_payload *sdp;
    sdp = sdp_parse(pSdpData);
    if(sdp == NULL)
    {
        WV_printf("invalid SDP\n");
        return -1;
    }

    size_t i, j;
    char pOut[24] = {0};
    unsigned int data;
    int strLen = 0, videoST2110 = -1, audioST2110 = -1, colorimetry = -1,
        interlace = -1, framerate = -1, videoDepth = -1, sampling = -1,
        videoWidth = -1, videoHight = -1, videoFmtp = -1;
    if (!sdp)
    {

    }

    for (i = 0; i < sdp->medias_count; i++)
    {

        
        struct sdp_media *m = &sdp->medias[i];
        struct sdp_info *info = &m->info;
        if (strcmp(info->type, "video") == 0)
        {
            sprintf(pGetInfo->video_srcIp,"%s",sdp->origin.addr);
            if(sdp->conn.nettype && sdp->conn.addrtype && sdp->conn.address)
            {
                //sprintf(pGetInfo->video_desIp,"%s",sdp->conn.address);
                for(j=0;j<64;j++)
                {
                    if(sdp->conn.address[j] == 0 || sdp->conn.address[j] == '/') break;
                    pGetInfo->video_desIp[j] = sdp->conn.address[j];
                }

            }
            
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
                    colorimetry = 1;
                }
                //get interlace
                if (FPGA_SDP_GetParmUint(m->attributes[j], "interlace", &data) == 0)
                {
                    pGetInfo->video_interlace = data;
                    interlace = 1;
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
                        framerate = 1;
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
                    videoDepth = 1;
                }
                //get sampling
                memset(pOut, 0, sizeof(pOut));
                strLen = 0;
                if (FPGA_SDP_GetParmStr(m->attributes[j], "sampling", pOut, &strLen) == 0)
                {
                    memcpy(pGetInfo->video_sampling, pOut, strLen);
                    sampling = 1;
                }
                //get video width
                if (FPGA_SDP_GetParmUint(m->attributes[j], "width", &data) == 0)
                {
                    pGetInfo->video_width = data;
                    videoWidth = 1;
                }
                //get video hight
                if (FPGA_SDP_GetParmUint(m->attributes[j], "height", &data) == 0)
                {
                    pGetInfo->video_height = data;
                    videoHight = 1;
                }
                //get video pt
                if (FPGA_SDP_GetParmUint(m->attributes[j], "fmtp", &data) == 0)
                {
                    pGetInfo->video_pt = data;
                    videoFmtp = 1;
                }
            }
        }
        else if (strcmp(info->type, "audio"))
        {
            sprintf(pGetInfo->audio_srcIp,"%s",sdp->origin.addr);
            //sprintf(pGetInfo->audio_desIp,"%s",sdp->conn.address);
            if(sdp->conn.nettype && sdp->conn.addrtype && sdp->conn.address)
            {
                //sprintf(pGetInfo->video_desIp,"%s",sdp->conn.address);
                for(j=0;j<64;j++)
                {
                    if(sdp->conn.address[j] == 0 || sdp->conn.address[j] == '/') break;
                    pGetInfo->audio_desIp[j] = sdp->conn.address[j];
                }

            }
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
                            pGetInfo->audio_channel[i] = p[0];
                            p++;
                        }
                    }
                    //WV_printf("\naudio_chl=%s\n", pGetInfo->audio_channel);
                }
            }
        }
    }
    sdp_destroy(sdp);
    if (audioST2110 != 1 && videoST2110 != 1)
    {
        return -1;
    }
    //判断所需的sdp信息是否都已经获取，如果没有获取完整，则说明这个sdp信息时错误的，则返回错误
    if (colorimetry != 1 || interlace != 1 || videoDepth != 1 || sampling != 1 || videoWidth != 1 || videoHight != 1 || videoFmtp != 1)
    {
        return -1;
    }

    return 0;
}

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
int FPGA_SDP_MakeSdpData(FPGA_SDP_Info *pInputInfo,char *pSdpOutData,unsigned int *sdpLen)
{
    int ret = -1;
    
    return ret;
}



#if 0
/********************************************************************
 * void* FPGA_SDP_Proc(void *prm)
 * ******************************************************************/
void* FPGA_SDP_Proc(void *prm)
{
	FPGA_SDP_DEV *pDev = (FPGA_SDP_DEV *)prm;
	pDev->open = 1;
	pDev->close = 0;	
    while( pDev->open = 1 ) {
        memset(pDev->pSdpData,0,_FPGA_SDP_DATALEN+8);
        //通过fpga获取SDP报文
        //提取有用的sdp信息
        struct sdp_payload *sdp;
        sdp_parse(pDev->pSdpData);
        FPGA_SDP_Info info;
        memset(&info,0,sizeof(FPGA_SDP_Info));
        if(FPGA_SDP_GetInfo(sdp,&info) == 0) //需要在获取到sdp信息中判断是否正确，主要时包含必要字段
        {
            //gpFpgaConfDev->win[0].sdpInfo = info;//更新sdp信息
            FPGA_SDP_SetInfo(&info,0,0);
        }
        sdp_destroy(sdp);
    }
    pDev->close = 1;

}
#endif
/********************************************************************
 * void FPGA_SDP_Init()
 * ******************************************************************/
void FPGA_SDP_Init()
{
    //memset(&gFpgaSdpDev,0,sizeof(gFpgaSdpDev));
    //gFpgaSdpDev.pSdpData = (WV_S8 *)malloc(_FPGA_SDP_DATALEN+8);
    //pthread_mutex_init(&gMutexUpdateSdpInfo, NULL);

    //WV_THR_Create(&gFpgaSdpDev.thrHndl,FPGA_SDP_Proc, WV_THR_PRI_DEFAULT, WV_THR_STACK_SIZE_DEFAULT, (void *)&gFpgaSdpDev);
}

/********************************************************************
 * void FPGA_SDP_DeInit()
 * ******************************************************************/
void FPGA_SDP_DeInit()
{
    /*
	if(gFpgaSdpDev.open == 1){
		gFpgaSdpDev.open = 0;
		while(gFpgaSdpDev.close == 0);
		WV_THR_Destroy(&gFpgaSdpDev.thrHndl);
	}
    */
    //pthread_mutex_destroy(&gMutexUpdateSdpInfo);
    //free(gFpgaSdpDev.pSdpData);

}
