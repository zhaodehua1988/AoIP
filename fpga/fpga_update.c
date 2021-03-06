#include "fpga_update.h"
#include "w25qxx.h"
#include "PCA9555.h"
#include "his_spi.h"
#define _D_FPGA_UPDATE_BUFSIZE (4096)
#define _D_FPGA_UPDATE_FLASH_ADDR (0x800000)

typedef struct _S_FPGA_UPDATE
{
    WV_U8 writeBuf[_D_FPGA_UPDATE_BUFSIZE];
    WV_U8 readBuf[_D_FPGA_UPDATE_BUFSIZE];
    WV_U8 tempBuf[_D_FPGA_UPDATE_BUFSIZE];
} S_FPGA_UPDATE_DEV;

S_FPGA_UPDATE_DEV gFpgaUpdateDev;

/************************************************************************************
 * 开始升级fpga程序
 * **********************************************************************************/
WV_S32 FPGA_UPDATE_UpdateStart()
{
    HIS_SPI_SetFpgaUpdateEna();
    PCA9555_Clr(0x3, 0x2);
    usleep(100000);
    //SPIFlashInit();
    return WV_SOK;
}
/************************************************************************************
 * 结束升级fpga程序
 * **********************************************************************************/
WV_S32 FPGA_UPDATE_UpdateEnd()
{
    PCA9555_Set(0x3, 0x2);
    usleep(100000);
    HIS_SPI_SetFpgaUpdateDisable();
    return WV_SOK;
}

/************************************************************************************
 * 写入Flash
 * **********************************************************************************/
WV_S32 FPGA_UPDATE_WriteToFlash(WV_U32 addr, WV_S8 *pBinFile)
{

    WV_S32 readLen;
    FILE *fp = NULL;
    fp = fopen(pBinFile, "rb");
    if (fp == NULL)
    {
        WV_printf("打开源src_fpga.bin失败\n");
        return 0;
    }
    while (1)
    {
        readLen = fread(gFpgaUpdateDev.writeBuf, 1, _D_FPGA_UPDATE_BUFSIZE, fp);
        if (readLen <= 0)
            break;
        //while (1)
        //{
            W25QXX_Write(gFpgaUpdateDev.writeBuf, addr, readLen);
          //  W25QXX_Read(gFpgaUpdateDev.tempBuf, addr, readLen);
          //  if (memcmp(gFpgaUpdateDev.writeBuf, gFpgaUpdateDev.tempBuf, readLen) == 0)
          //  {
          //      break;
          //  }
          //  WV_printf("badNum = %d\n", badNum);
          //  badNum++;
        //}

        addr += readLen;
    }

    fclose(fp);
    fp = NULL;
    return WV_SOK;
}

/************************************************************************************
 * 读取Flash
 * **********************************************************************************/
WV_S32 FPGA_UPDATE_ReadFromFlash(WV_U32 addr, WV_S8 *pBinFile, WV_U32 fileSize)
{
    WV_U32 lastSize = 0;
    WV_U16 readNum = 0, writeNum = 0;
    FILE *fp = NULL;

    fp = fopen(pBinFile, "wb+");
    if (fp == NULL)
    {
        WV_ERROR("打开%s失败\n", pBinFile);
        return WV_EFAIL;
    }
    lastSize = fileSize;
    while (lastSize)
    {
        if (lastSize > _D_FPGA_UPDATE_BUFSIZE)
        {

            lastSize -= _D_FPGA_UPDATE_BUFSIZE;
            readNum = _D_FPGA_UPDATE_BUFSIZE;
        }
        else
        {
            readNum = lastSize;
            lastSize = 0;
        }
        W25QXX_Read(gFpgaUpdateDev.readBuf, addr, readNum);
        writeNum = fwrite(gFpgaUpdateDev.readBuf, 1, readNum, fp);
        if (writeNum != readNum)
        {
            WV_ERROR("回读flash fpga.bin出错");
            break;
        }
        addr += readNum;
    }
    fclose(fp);
    fp = NULL;
    return WV_SOK;
}

/************************************************************************************
 * 函   数::WV_S32 fpga_update_md5Checkout(WV_S8 *pMd5,WV_S8 *pFile)
 * 参数说明::
 *         pMd5 :传入的Md5值
 *         pFile:要校验的文件
 * 返回值 ::
 *         0:校验成功，文件md5值相同
 *        -1:校验失败
 * **********************************************************************************/
WV_S32 fpga_update_md5Checkout(WV_S8 *pMd5, WV_S8 *pFile)
{

    if (WV_FILE_Access(pFile) != 0)
    {
        WV_ERROR("can not find %s \n", pFile);
        return WV_EFAIL;
    }
    WV_S8 cmd[256] = {0};
    WV_S8 temp[] = "temp.md5";
    WV_S8 buf[128] = {0};
    WV_S8 md5[32] = {0};
    WV_S32 readLen;
    sprintf(cmd, "md5sum %s > %s ", pFile, temp);
    system(cmd);
    FILE *fp = NULL;
    fp = fopen(pFile, "r");
    if (fp == NULL)
    {
        WV_ERROR("can not open %s \n", pFile);
        return WV_EFAIL;
    }
    readLen = fread(buf, 1, 128, fp);
    if (readLen == 128 || readLen < 32)
    {
        fclose(fp);
        WV_ERROR("read %s md5 err!! \n", pFile);
        return WV_EFAIL;
    }

    if (memcpy(md5, buf, 32) != 0)
    {
        fclose(fp);
        WV_ERROR("md5 checkout false \n");
        return WV_EFAIL;
    }
    fclose(fp);
    fp = NULL;
    remove(temp);
    return WV_SOK;
}
/********************************************************************
 * 获取md5值(32个字节)
 * ******************************************************************/
WV_S32 fpga_update_GetMd5(WV_S8 *pMd5Out, WV_S8 *pFile)
{

    if (WV_FILE_Access(pFile) != 0)
    {
        WV_ERROR("can not find %s \n", pFile);
        return WV_EFAIL;
    }
    WV_S8 cmd[256] = {0};
    WV_S8 temp[] = "temp.md5";
    WV_S32 readLen;
    sprintf(cmd, "md5sum %s > %s ", pFile, temp);
    system(cmd);
    FILE *fp = NULL;
    fp = fopen(temp, "r");
    if (fp == NULL)
    {
        WV_ERROR("can not open %s \n", pFile);
        return WV_EFAIL;
    }
    readLen = fread(pMd5Out, 1, 32, fp);
    if (readLen != 32)
    {
        fclose(fp);
        remove(temp);
        WV_ERROR("read %s md5 err!! \n", pFile);
        return WV_EFAIL;
    }
    fclose(fp);
    remove(temp);
    WV_printf("\nget %s md5 = %s \n", pFile, pMd5Out);

    return WV_SOK;
}


WV_S32 _fpga_update_update(WV_U32 flashAddr,WV_S8 *pFpgaBin)
{

    FPGA_UPDATE_UpdateStart();

    struct timeval start, end;
    gettimeofday(&start, NULL);

    WV_S32 i,ret;
    WV_U16 flashID;
    WV_U32 fpgaBinLen;
    WV_S8 srcMd5[32 + 4] = {0};
    WV_S8 desMd5[32 + 4] = {0};
    WV_S8 desFile[256] = {0};
    sprintf(desFile, "%s_read", pFpgaBin);
    //获取fpgabin的md5值
    if (fpga_update_GetMd5(srcMd5, pFpgaBin) != 0)
    {
        ret = WV_EFAIL;
        goto fpgaUpdateEnd;
    }
    

    //fpga开始升级以后，可以查询到flash的id，循环查询2s，如果查询不到id，则返回错误
    for (i = 0; i < 200; i++)
    {

        flashID = W25QXX_ReadID();
        if (flashID == W25Q128)
            break;
        usleep(10000);
    }
    if (flashID != W25Q128)
    {
        WV_ERROR("FPGA update: cannot get w25q128 id,err!!!\n");
        ret = WV_EFAIL;
        goto fpgaUpdateEnd;
    }
    //从起始地址按块擦除8MB，块大小时64k，如果不是块大小不是整数倍，从整数倍地址擦除
    WV_S32 blockAddr;
    blockAddr = flashAddr / (64 * 1024);
    if (flashAddr % (64 * 1024) != 0)
    {
        blockAddr += 1;
    }
    for (i = blockAddr; i < (blockAddr + 128); i++)
    {
        W25QXX_Erase_Block(i);
    }
    if (WV_FILE_GetLen(pFpgaBin, (WV_S32 *)&fpgaBinLen) != 0)
    {
        WV_ERROR("FPGA update: cannot get %s len,err!!!\n", pFpgaBin);
        ret = WV_EFAIL;
        goto fpgaUpdateEnd;
    }

    FPGA_UPDATE_WriteToFlash(flashAddr, pFpgaBin);
    FPGA_UPDATE_ReadFromFlash(flashAddr, desFile, fpgaBinLen);

    //获取flash里的md5值
    if (fpga_update_GetMd5(desMd5, desFile) != 0)
    {
        remove(desFile);
        ret = WV_EFAIL;
        goto fpgaUpdateEnd;
    }
    if (strcmp(srcMd5, desMd5) != 0)
    {
        WV_ERROR("update fpga :md5 val err srcmd5[%s] fpgamd5[%s]\n", srcMd5, desMd5);
        remove(desFile);
        ret = WV_EFAIL;
        goto fpgaUpdateEnd;
    }
    remove(desFile);
    gettimeofday(&end, NULL);

    time_t sec = end.tv_sec - start.tv_sec;
    WV_printf("update fpga use time :%u m.%u s\n", (WV_U32)sec / 60, (WV_U32)sec % 60);
fpgaUpdateEnd:    
    FPGA_UPDATE_UpdateEnd();
    return WV_SOK;
}

/************************************************************************************
 * WV_S32 FPGA_UPDATE_SetCmd(WV_S32 argc, WV_S8 **argv, WV_S8 *prfBuff)
 * addr::读取地址
 * srcfile::源文件，用于获取读取多长
 * desfile::目标文件，把读取的fpgabin保存为目标文件
 * **********************************************************************************/
WV_S32 FPGA_UPDATE_GetBin(WV_U32 addr, WV_S8 *pSrcFile, WV_S8 *pDesFile)
{
    WV_U32 fpgaBinLen;
    FPGA_UPDATE_UpdateStart();
    WV_S32 i;
    for(i=0;i<100;i++){
        if(W25QXX_ReadID() == 0xEF17){
            WV_printf("get id = 0x%X\n",W25QXX_ReadID());
            break;
        }
    }


    if (WV_FILE_GetLen(pSrcFile, (WV_S32 *)&fpgaBinLen) != 0)
    {
        WV_ERROR("FPGA update: cannot get %s len,err!!!\n", pSrcFile);
        FPGA_UPDATE_UpdateEnd();
        return WV_EFAIL;
    }

    FPGA_UPDATE_ReadFromFlash(addr, pDesFile, fpgaBinLen);
    FPGA_UPDATE_UpdateEnd();
    return WV_SOK;
}
/************************************************************************************
 * WV_S32 FPGA_UPDATE_SetCmd(WV_S32 argc, WV_S8 **argv, WV_S8 *prfBuff)
 * **********************************************************************************/
static WV_S32 FPGA_UPDATE_SetCmd(WV_S32 argc, WV_S8 **argv, WV_S8 *prfBuff)
{

    if (argc < 1)
    {
        prfBuff += sprintf(prfBuff, "set flash <cmd>;//cmd like: start/stop/update/write\r\n");
        return 0;
    }
    if (strcmp(argv[0], "start") == 0)
    {

        prfBuff += sprintf(prfBuff, "set fpgaf start\r\n");
        FPGA_UPDATE_UpdateStart();
    }
    else if (strcmp(argv[0], "stop") == 0)
    {

        prfBuff += sprintf(prfBuff, "set fpgaf stop\r\n");
        FPGA_UPDATE_UpdateEnd();
    }
    else if (strcmp(argv[0], "update") == 0)
    {
        if (argc < 3)
        {
            prfBuff += sprintf(prfBuff, "set flash update <addr> <fpgaUpdate.bin>\r\n");
            return WV_SOK;
        }

        WV_U32 addr;
        WV_STR_S2v(argv[1], &addr);

        //FPGA_UPDATE_Update(argv[2]);
        _fpga_update_update(addr,argv[2]);

        return WV_SOK;
    }
    else if (strcmp(argv[0], "write") == 0)
    {

        if (argc < 3)
        {
            prfBuff += sprintf(prfBuff, "set flash write <addr> \r\n");
            return WV_SOK;
        }
        WV_U32 addr;
        WV_STR_S2v(argv[1], &addr);
        WV_U8 buf[5];
        WV_S32 i;
        for (i = 0; i < 5; i++)
        {
            buf[i] = 0xff;
        }

        W25QXX_Write(buf, addr, 5);
        W25QXX_Read(buf, addr, 5);
        for (i = 0; i < 5; i++)
        {
            if (buf[i] != 0xff)
            {
                prfBuff += sprintf(prfBuff, "set flash write err\r\n");
                return WV_SOK;
            }
        }
    }
    return WV_SOK;
}
/************************************************************************************
 * WV_S32 FPGA_UPDATE_GetCmd(WV_S32 argc, WV_S8 **argv, WV_S8 *prfBuff)
 * **********************************************************************************/
static WV_S32 FPGA_UPDATE_GetCmd(WV_S32 argc, WV_S8 **argv, WV_S8 *prfBuff)
{

    if (argc < 1)
    {
        prfBuff += sprintf(prfBuff, "get flash <cmd>;//cmd like:id/bin \r\n");
        return 0;
    }
    else if (strcmp(argv[0], "id") == 0)
    {
        FPGA_UPDATE_UpdateStart();
        WV_U16 id = 0xffff;
        int i;
        for (i = 0; i < 100; i++)
        {
            id = W25QXX_ReadID();
            if (id != 0xffff)
                break;
            usleep(100000);
        }

        prfBuff += sprintf(prfBuff, "id = 0x%X\r\n", id);
        FPGA_UPDATE_UpdateEnd();
    }
    else if (strcmp(argv[0], "bin") == 0)
    {
        if (argc < 4)
        {
            prfBuff += sprintf(prfBuff, "get flash bin <addr> <srcfile> <desfile>;\r\n");
            return 0;
        }
        WV_U32 addr;
        WV_STR_S2v(argv[1], &addr);
        FPGA_UPDATE_GetBin(addr, argv[2], argv[3]);
    }
    return WV_SOK;
}

/*******************************************************************
 * WV_S32 FPGA_UPDATE_Update(WV_S8 *pFpgaBin)
 * FPGA 升级，外部接口
 * *****************************************************************/
WV_S32 FPGA_UPDATE_Update(WV_S8 *pFpgaBin)
{
    return _fpga_update_update(_D_FPGA_UPDATE_FLASH_ADDR,pFpgaBin);
}


/************************************************************************************
 * WV_S32 FPGA_UPDATE_Init()
 * 初始化
 * **********************************************************************************/
WV_S32 FPGA_UPDATE_Init()
{

    WV_CMD_Register("set", "flash", "set fpgaup", FPGA_UPDATE_SetCmd);
    WV_CMD_Register("get", "flash", "get fpgaup", FPGA_UPDATE_GetCmd);

    return WV_SOK;
}

/************************************************************************************
 * WV_S32 FPGA_UPDATE_DeInit()
 * 去初始化
 * **********************************************************************************/
WV_S32 FPGA_UPDATE_DeInit()
{
    return WV_SOK;
}