/****************************************************************************
 * 
 * zhaodehua create 2019/06/06
 * 
 * **************************************************************************/
#include "RTC_DS3231.h"
#include "PCA9548A.h"
#include <time.h>
#define _D_RTCDS3231_DEV_ADDR 0xD0


/******************************
 * //BCD转换为Byte
 * ****************************/
WV_U8 BCD2HEX(WV_U8 val) //BCD转换为Byte
{
    WV_U8 i;
    i = val & 0x0f;
    val >>= 4;
    val &= 0x0f;
    val *= 10;
    i += val;

    return i;
}
/******************************
 * //B码转换为BCD码
 * ****************************/
WV_U16 B_BCD(WV_U8 val) 
{
    WV_U8 i, j, k;
    i = val / 10;
    j = val % 10;
    k = j + (i << 4);

    return k;
}

/******************************************************************************
 * 修改时间,BCD码输入
 * ****************************************************************************/
WV_S32 RTC_SetTime(WV_U8 year, WV_U8 mon, WV_U8 day, WV_U8 hour, WV_U8 min, WV_U8 sec)
{
    WV_S32 ret = 0;
    WV_U8 temp = 0;
    temp = B_BCD(year);
    ret |= PCA9548_IIC_Write(PCA9548A_IIC_SWID_DS3231, _D_RTCDS3231_DEV_ADDR, 0x06, temp); //修改年

    temp = B_BCD(mon);
    ret |= PCA9548_IIC_Write(PCA9548A_IIC_SWID_DS3231, _D_RTCDS3231_DEV_ADDR, 0x05, temp); //修改月

    temp = B_BCD(day);
    ret |= PCA9548_IIC_Write(PCA9548A_IIC_SWID_DS3231, _D_RTCDS3231_DEV_ADDR, 0x04, temp); //修改日

    temp = B_BCD(hour);
    ret |= PCA9548_IIC_Write(PCA9548A_IIC_SWID_DS3231, _D_RTCDS3231_DEV_ADDR, 0x02, temp); //修改时

    temp = B_BCD(min);
    ret |= PCA9548_IIC_Write(PCA9548A_IIC_SWID_DS3231, _D_RTCDS3231_DEV_ADDR, 0x01, temp); //修改分

    temp = B_BCD(sec);
    ret |= PCA9548_IIC_Write(PCA9548A_IIC_SWID_DS3231, _D_RTCDS3231_DEV_ADDR, 0x00, temp); //修改秒
    if (ret != 0)
    {
        WV_ERROR("modify time err!!\n");
    }
    return ret;
}

/******************************************************************************
 * 获取时间
 * ****************************************************************************/
int RTC_GetTime(WV_U8 *pYear, WV_U8 *pMon, WV_U8 *pDay, WV_U8 *pHour, WV_U8 *pMin, WV_U8 *pSec)
{
    WV_S32 ret = 0;
    WV_U8 temp = 0;

    ret |= PCA9548_IIC_Read(PCA9548A_IIC_SWID_DS3231, _D_RTCDS3231_DEV_ADDR, 0x06, &temp); //读取年
    *pYear = BCD2HEX(temp);

    ret |= PCA9548_IIC_Read(PCA9548A_IIC_SWID_DS3231, _D_RTCDS3231_DEV_ADDR, 0x05, &temp); //读取
    *pMon = BCD2HEX(temp);

    ret |= PCA9548_IIC_Read(PCA9548A_IIC_SWID_DS3231, _D_RTCDS3231_DEV_ADDR, 0x04, &temp); //读取日
    *pDay = BCD2HEX(temp);

    ret |= PCA9548_IIC_Read(PCA9548A_IIC_SWID_DS3231, _D_RTCDS3231_DEV_ADDR, 0x02, &temp); //读取时
    *pHour = BCD2HEX(temp);

    ret |= PCA9548_IIC_Read(PCA9548A_IIC_SWID_DS3231, _D_RTCDS3231_DEV_ADDR, 0x01, &temp); //读取分
    *pMin = BCD2HEX(temp);

    ret |= PCA9548_IIC_Read(PCA9548A_IIC_SWID_DS3231, _D_RTCDS3231_DEV_ADDR, 0x00, &temp); //读取秒
    *pSec = BCD2HEX(temp);

    if (ret != 0)
    {
        WV_ERROR("get ret time err!!\n");
    }
    else
    {
        WV_printf("get rtc time %d-%d-%d %d:%d:%d \n", *pYear, *pMon, *pDay, *pHour, *pMin, *pSec);
    }
    return ret;
}




/*****************************************************************************
 * 同步NTP服务器时间
 * ***************************************************************************/
WV_S32 GetNtpTime(WV_S8 *pNtpServerIP)
{
    WV_S8 cmd[128]={0};
    sprintf("ntpdate %s",pNtpServerIP);
    system(cmd);
    return WV_SOK;
}
/****************************************************************************
 * 设置系统时间
 * **************************************************************************/
WV_S32 SetSysTime(WV_S32 year,WV_S32 mon,WV_S32 day,WV_S32 hour,WV_S32 min,WV_S32 sec)
{

    struct tm _tm;
    time_t timep;
    struct timeval tv;
    _tm.tm_year = year;
    _tm.tm_mon = mon-1;
    _tm.tm_mday = day;
    _tm.tm_hour = hour;
    _tm.tm_min = min;
    _tm.tm_sec = sec;
    timep = mktime(&_tm);
    tv.tv_sec = timep;
    tv.tv_usec = 0;
    if(settimeofday(&tv,(struct timezone*)0) < 0 )
    {
        WV_ERROR("set system datatime err!!\n");
        return WV_EFAIL;
    }

    return WV_SOK;
}
/****************************************************************************
 * 获取系统时间
 * 参数：timeZone 时区
 * **************************************************************************/
WV_S32 GetSysTime(int timeZone)
{

    WV_printf("get systime :");
    time_t timep;
    struct tm *p;
    time(&timep);
    p=gmtime(&timep);//此函数返回的时间日期未经时区转换，而是UTC 时间。
    WV_printf("%d-%d-%d %d:%d:%d \n",p->tm_year+1900,p->tm_mon+1,p->tm_mday,p->tm_hour+timeZone,p->tm_min,p->tm_sec);
    return WV_SOK;
}
/****************************************************************************

WV_S32 RTC_GetTimeCmd(WV_S32 argc, WV_S8 **argv, WV_S8 *prfBuff)

****************************************************************************/
WV_S32 RTC_GetTimeCmd(WV_S32 argc, WV_S8 **argv, WV_S8 *prfBuff)
{
    WV_U8 year, mon, day, hour, min, sec;
    RTC_GetTime(&year, &mon, &day, &hour, &min, &sec);
    prfBuff += sprintf(prfBuff, "\nget rtc time %d-%d-%d %d:%d:%d \r\n", year, mon, day, hour, min, sec);
    GetSysTime(8);//获取东8区的时间
    return WV_SOK;
}

/****************************************************************************

WV_S32 RTC_SetTimeCmd(WV_S32 argc, WV_S8 **argv,WV_S8 *prfBuff)

****************************************************************************/
WV_S32 RTC_SetTimeCmd(WV_S32 argc, WV_S8 **argv, WV_S8 *prfBuff)
{
    WV_U32 year, mon, day, hour, min, sec;
    WV_S32 ret;

    if (argc < 6)
    {

        prfBuff += sprintf(prfBuff, "set rtc <year> <mon> <day> <hour> <min> <sec>\r\n");
        return 0;
    }
    //year
    ret = WV_STR_S2v(argv[0], &year);
    if (ret != WV_SOK)
    {
        prfBuff += sprintf(prfBuff, "input erro!\r\n");
        return WV_SOK;
    }

    //mon
    ret = WV_STR_S2v(argv[1], &mon);
    if (ret != WV_SOK)
    {
        prfBuff += sprintf(prfBuff, "input erro!\r\n");
        return WV_SOK;
    }
    //day
    ret = WV_STR_S2v(argv[2], &day);
    if (ret != WV_SOK)
    {
        prfBuff += sprintf(prfBuff, "input erro!\r\n");
        return WV_SOK;
    }
    //hour
    ret = WV_STR_S2v(argv[3], &hour);
    if (ret != WV_SOK)
    {
        prfBuff += sprintf(prfBuff, "input erro!\r\n");
        return WV_SOK;
    }

    //min
    ret = WV_STR_S2v(argv[4], &min);
    if (ret != WV_SOK)
    {
        prfBuff += sprintf(prfBuff, "input erro!\r\n");
        return WV_SOK;
    }

    //sec
    ret = WV_STR_S2v(argv[5], &sec);
    if (ret != WV_SOK)
    {
        prfBuff += sprintf(prfBuff, "input erro!\r\n");
        return WV_SOK;
    }

    ret = RTC_SetTime((WV_U8)year, (WV_U8)mon, (WV_U8)day, (WV_U8)hour, (WV_U8)min, (WV_U8)sec);
    if(ret != 0)
    {
        WV_ERROR("set rtc cmd err!!");
    }else{
        prfBuff +=  sprintf(prfBuff,"set rtc %d-%d-%d %d:%d:%d \n",year, mon, day, hour, min, sec);
    }

    return WV_SOK;
}

/**********************************************
 * rtc时钟初始化
 * *******************************************/
WV_S32 RTC_Init()
{
    WV_CMD_Register("get", "rtc", "get rtc time", RTC_GetTimeCmd);
    WV_CMD_Register("set", "rtc", "set ret time", RTC_SetTimeCmd);
    WV_S32 year,mon,day,hour,min,sec;

    RTC_GetTime((WV_U8 *)&year);

    return WV_SOK;
}
/**********************************************
 * rtc时钟去初始化
 * *******************************************/
WV_S32 RTC_DeInit()
{
    return WV_SOK;
}