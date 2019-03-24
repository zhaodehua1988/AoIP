#include "fpga_update.h"
#include "w25qxx.h"
#include "PCA9555.h"
#define  _D_FPGA_UPDATE_BUFSIZE          (4096)
#define  _D_FPGA_UPDATE_FLASH_ADDR       (0x800000)

typedef struct _S_FPGA_UPDATE
{
    WV_U8 writeBuf[_D_FPGA_UPDATE_BUFSIZE];
    WV_U8 readBuf[_D_FPGA_UPDATE_BUFSIZE];
    WV_U8 tempBuf[_D_FPGA_UPDATE_BUFSIZE];
}S_FPGA_UPDATE_DEV;

S_FPGA_UPDATE_DEV gFpgaUpdateDev;

/************************************************************************************
 * 开始升级fpga程序
 * **********************************************************************************/
WV_S32 FPGA_UPDATE_UpdateStart()
{
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
    return WV_SOK;
}

/************************************************************************************
 * 写入Flash
 * **********************************************************************************/
WV_S32 FPGA_UPDATE_WriteToFlash(WV_U32 addr,WV_S8 *pBinFile)
{

    WV_S32 readLen;
    WV_S32 badNum=0;
    FILE *fp = NULL;
    fp = fopen(pBinFile,"rb");
    if(fp == NULL){
        WV_printf("打开源src_fpga.bin失败\n");
        return 0;            
    }
    while(1){
        readLen = fread(gFpgaUpdateDev.writeBuf,1,_D_FPGA_UPDATE_BUFSIZE,fp);
        if(readLen <= 0 ) break;
        while(1){
            W25QXX_Write(gFpgaUpdateDev.writeBuf,addr,readLen);
            W25QXX_Read(gFpgaUpdateDev.tempBuf,addr,readLen);
            if(memcmp(gFpgaUpdateDev.writeBuf,gFpgaUpdateDev.tempBuf,readLen) == 0){
                break;
            }
            WV_printf("badNum = %d\n",badNum);
            badNum ++;
        }

        addr += readLen;
    }

    fclose(fp);
    fp = NULL;
    return WV_SOK;
}

/************************************************************************************
 * 读取Flash
 * **********************************************************************************/
WV_S32 FPGA_UPDATE_ReadFromFlash(WV_U32 addr,WV_S8 *pBinFile,WV_U32 fileSize)
{
    WV_U32 lastSize=0;
    WV_U16 readNum=0,writeNum=0;
    FILE *fp = NULL;

    fp = fopen(pBinFile,"wb+");
    if(fp == NULL){
        WV_ERROR("打开%s失败\n",pBinFile);
        return WV_EFAIL;            
    }
    lastSize = fileSize;
    while(lastSize){
        if(lastSize > _D_FPGA_UPDATE_BUFSIZE){
            
            lastSize -= _D_FPGA_UPDATE_BUFSIZE;
            readNum = _D_FPGA_UPDATE_BUFSIZE;
        }else{
            readNum = lastSize;
            lastSize = 0 ;
        }
        W25QXX_Read(gFpgaUpdateDev.readBuf,addr,readNum);        
        writeNum = fwrite(gFpgaUpdateDev.readBuf,1,readNum,fp);
        if(writeNum != readNum ){
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
WV_S32 fpga_update_md5Checkout(WV_S8 *pMd5,WV_S8 *pFile)
{

    if(WV_FILE_Access(pFile) != 0 )
    {
        WV_ERROR("can not find %s \n",pFile);
        return WV_EFAIL;
    }
    WV_S8 cmd[256]={0};
    WV_S8 temp[]="temp.md5";
    WV_S8  buf[128] = {0};
    WV_S8  md5[32] = {0};
    WV_S32 readLen;
    sprintf(cmd,"md5sum %s > %s ",pFile,temp);
    system(cmd);
    FILE *fp = NULL;
    fp = fopen(pFile,"r") ;
    if(fp == NULL){
        WV_ERROR("can not open %s \n",pFile);
        return WV_EFAIL;
    }
    readLen = fread(buf,1,128,fp);
    if(readLen == 128 || readLen <32){
        fclose(fp);
        WV_ERROR("read %s md5 err!! \n",pFile);
        return WV_EFAIL;
    }

    if(memcpy(md5,buf,32) != 0 )
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
WV_S32 fpga_update_GetMd5(WV_S8 *pMd5Out,WV_S8 *pFile)
{

    if(WV_FILE_Access(pFile) != 0 )
    {
        WV_ERROR("can not find %s \n",pFile);
        return WV_EFAIL;
    }
    WV_S8 cmd[256]={0};
    WV_S8 temp[]="temp.md5";
    WV_S32 readLen;
    sprintf(cmd,"md5sum %s > %s ",pFile,temp);
    system(cmd);
    FILE *fp = NULL;
    fp = fopen(temp,"r") ;
    if(fp == NULL){
        WV_ERROR("can not open %s \n",pFile);
        return WV_EFAIL;
    }
    readLen = fread(pMd5Out,1,32,fp);
    if(readLen != 32){
        fclose(fp);
        remove(temp);
        WV_ERROR("read %s md5 err!! \n",pFile);
        return WV_EFAIL;
    }
    fclose(fp);
    remove(temp);
    WV_printf("\nget %s md5 = %s \n",pFile,pMd5Out);

    return WV_SOK;
}

/************************************************************************************
 * WV_S32 FPGA_UPDATE_Update()
 * 调用升级fpga接口
 * **********************************************************************************/
#if 0
WV_S32 FPGA_UPDATE_Update(WV_S8 *pMd5)
{

    WV_printf("md5 val:%s\n",pMd5);
    struct timeval start,end;
    gettimeofday(&start,NULL);

    WV_S32 i;
    WV_U16 flashID;
    WV_U32 fpgaBinLen;
    if(fpga_update_md5Checkout(pMd5,_D_FPGA_UPDATE_SRC_FILEPATH) != 0 )
    {
        return WV_EFAIL;
    }
    //
    FPGA_UPDATE_UpdateStart();
    //fpga开始升级以后，可以查询到flash的id，循环查询2s，如果查询不到id，则返回错误
    for(i=0;i<200;i++){
        
        flashID = W25QXX_ReadID();
        if(flashID == W25Q128) break;
        usleep(10000);
    }
    if(flashID != W25Q128){
        WV_ERROR("FPGA update: cannot get w25q128 id,err!!!\n");
        return WV_EFAIL;
    }
    //从起始地址按块擦除8MB，块大小时64k，如果不是块大小不是整数倍，从整数倍地址擦除
    WV_S32 blockAddr;
    blockAddr = _D_FPGA_UPDATE_FLASH_ADDR/(64*1024);
    if(_D_FPGA_UPDATE_FLASH_ADDR%(64*1024) != 0 ){
        blockAddr +=1;
    }
    for(i=blockAddr;i<256;i++){
        W25QXX_Erase_Block(i);
    }
    if(WV_FILE_GetLen(_D_FPGA_UPDATE_SRC_FILEPATH,(WV_S32 *)&fpgaBinLen) != 0){
        WV_ERROR("FPGA update: cannot get %s len,err!!!\n",_D_FPGA_UPDATE_SRC_FILEPATH);
        return WV_EFAIL;
    }

    FPGA_UPDATE_WriteToFlash(_D_FPGA_UPDATE_FLASH_ADDR,_D_FPGA_UPDATE_SRC_FILEPATH);
    FPGA_UPDATE_ReadFromFlash(_D_FPGA_UPDATE_FLASH_ADDR,_D_FPGA_UPDATE_SRC_FILEPATH,fpgaBinLen);
    FPGA_UPDATE_UpdateEnd();
    
    if(fpga_update_md5Checkout(pMd5,_D_FPGA_UPDATE_SRC_FILEPATH) != 0 )
    {
        return WV_EFAIL;
    }

    gettimeofday(&end,NULL);

    //suseconds_t msec=end.tv_usec - start.tv_usec;
    time_t sec = end.tv_sec - start.tv_sec;
    WV_printf("update fpga use time :%u m.%u s\n",(WV_U32)sec/60,(WV_U32)sec%60);
    return WV_SOK;
}
#endif 
WV_S32 FPGA_UPDATE_Update(WV_S8 *pFpgaBin)
{

    struct timeval start,end;
    gettimeofday(&start,NULL);

    WV_S32 i;
    WV_U16 flashID;
    WV_U32 fpgaBinLen;
    WV_S8 srcMd5[32+4]={0};
    WV_S8 desMd5[32+4]={0};
    WV_S8 desFile[256]={0};
    
    sprintf(desFile,"%s_read",pFpgaBin);
    //获取fpgabin的md5值
    if(fpga_update_GetMd5(srcMd5,pFpgaBin) != 0 )
    {
        return WV_EFAIL;        
    }
    //
    FPGA_UPDATE_UpdateStart();
    //fpga开始升级以后，可以查询到flash的id，循环查询2s，如果查询不到id，则返回错误
    for(i=0;i<200;i++){
        
        flashID = W25QXX_ReadID();
        if(flashID == W25Q128) break;
        usleep(10000);
    }
    if(flashID != W25Q128){
        WV_ERROR("FPGA update: cannot get w25q128 id,err!!!\n");
        return WV_EFAIL;
    }
    //从起始地址按块擦除8MB，块大小时64k，如果不是块大小不是整数倍，从整数倍地址擦除
    WV_S32 blockAddr;
    blockAddr = _D_FPGA_UPDATE_FLASH_ADDR/(64*1024);
    if(_D_FPGA_UPDATE_FLASH_ADDR%(64*1024) != 0 ){
        blockAddr +=1;
    }
    for(i=blockAddr;i<(blockAddr+128);i++){
        W25QXX_Erase_Block(i);
    }
    if(WV_FILE_GetLen(pFpgaBin,(WV_S32 *)&fpgaBinLen) != 0){
        WV_ERROR("FPGA update: cannot get %s len,err!!!\n",pFpgaBin);
        return WV_EFAIL;
    }

    FPGA_UPDATE_WriteToFlash(_D_FPGA_UPDATE_FLASH_ADDR,pFpgaBin);
    FPGA_UPDATE_ReadFromFlash(_D_FPGA_UPDATE_FLASH_ADDR,desFile,fpgaBinLen);
    FPGA_UPDATE_UpdateEnd();
    //获取flash里的md5值
    if(fpga_update_GetMd5(desMd5,desFile) != 0 )
    {
        return WV_EFAIL;
    }
    if(strcmp(srcMd5,desMd5) != 0 )
    {
        WV_ERROR("update fpga :md5 val err srcmd5[%s] fpgamd5[%s]\n",srcMd5,desMd5);
        return WV_EFAIL;
    }

    remove(desFile);
    gettimeofday(&end,NULL);

    time_t sec = end.tv_sec - start.tv_sec;
    WV_printf("update fpga use time :%u m.%u s\n",(WV_U32)sec/60,(WV_U32)sec%60);
    return WV_SOK;
}
/************************************************************************************
 * WV_S32 FPGA_UPDATE_SetCmd(WV_S32 argc, WV_S8 **argv, WV_S8 *prfBuff)
 * **********************************************************************************/
static WV_S32 FPGA_UPDATE_SetCmd(WV_S32 argc, WV_S8 **argv, WV_S8 *prfBuff)
{

    if (argc < 1)
    {
        prfBuff += sprintf(prfBuff, "set flash <cmd>;//cmd like: start/stop/update\r\n");
        return 0;
    }
    if (strcmp(argv[0], "start") == 0){
        
        prfBuff += sprintf(prfBuff, "set fpgaf start\r\n");
        FPGA_UPDATE_UpdateStart();

    }else if (strcmp(argv[0], "stop") == 0){

        prfBuff += sprintf(prfBuff, "set fpgaf stop\r\n");
        FPGA_UPDATE_UpdateEnd();

    }else if (strcmp(argv[0], "update") == 0){
        if(argc < 2){
            prfBuff += sprintf(prfBuff, "set flash update <fpgaUpdate.bin>\r\n");
            return WV_SOK;
        }
        
        FPGA_UPDATE_Update(argv[1]);
        
        return WV_SOK;
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
        prfBuff += sprintf(prfBuff, "get flash <cmd>;//cmd like:id/ \r\n");
        return 0;
    }
    else if (strcmp(argv[0], "id") == 0){
        FPGA_UPDATE_UpdateStart();
        WV_U16 id=0xffff;
        int i;
        for(i=0;i<100;i++){
            id=W25QXX_ReadID();
            if(id != 0xffff) break;
            usleep(100000);
        }

        prfBuff += sprintf(prfBuff, "id = 0x%X\r\n",id);
        FPGA_UPDATE_UpdateEnd();
    }
    return WV_SOK;
}
/************************************************************************************
 * WV_S32 FPGA_UPDATE_Init()
 * 初始化
 * **********************************************************************************/
WV_S32 FPGA_UPDATE_Init()
{
    //FPGA_UPDATE_UpdateStart();
    //SPIFlashInit();
    WV_CMD_Register("set", "flash", "set fpgaup", FPGA_UPDATE_SetCmd);
    WV_CMD_Register("get", "flash", "get fpgaup", FPGA_UPDATE_GetCmd);

    return WV_SOK;
}
