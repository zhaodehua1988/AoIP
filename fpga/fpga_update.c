#include "fpga_update.h"
//#include "his_spi.h"
#include "w25qxx.h"
#include "PCA9555.h"
#define  _D_FPGA_UPDATE_BUFSIZE          (4096)
#define  _D_FPGA_UPDATE_SRC_FILEPATH     "./src_fpga.bin"
#define  _D_FPGA_UPDATE_DES_FILEPATH     "./des_fpga.bin"
#define  _D_FPGA_UPDATE_FLASH_ADDR       (0x800000)
#define  _D_FPGA_UPDATE_SRC_MD5_FILEPATH  "./src_fpga.md5"
#define  _D_FPGA_UPDATE_DES_MD5_FILEPATH  "./des_fpga.md5"

typedef struct _S_FPGA_UPDATE
{
    WV_U8 writeBuf[_D_FPGA_UPDATE_BUFSIZE];
    WV_U8 readBuf[_D_FPGA_UPDATE_BUFSIZE];
}S_FPGA_UPDATE_DEV;

S_FPGA_UPDATE_DEV gFpgaUpdateDev;


/************************************************************************************
 * 开始校验
 * **********************************************************************************/
WV_S32 FPGA_UPDATE_md5Checkout()
{
    

    
    
    return WV_SOK;
}

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
static WV_U8 BufRead[8*1024];
static WV_U8 BufWrite[8*1024];
static WV_U32 badNum=0;
/************************************************************************************
 * 写入Flash
 * **********************************************************************************/
WV_S32 FPGA_UPDATE_WriteToFlash(WV_S8 *pFile)
{

    WV_U8 buf[8*1024]={0};
    WV_S32 readLen;
    WV_U32 addr = _D_FPGA_UPDATE_FLASH_ADDR;
    FILE *fp = NULL;
    fp = fopen("/opt/app/src_fpga.bin","rb");
    if(fp == NULL){
        WV_printf("打开源src_fpga.bin失败\n");
        return 0;            
    }
    while(1){
        readLen = fread(buf,1,8*1024,fp);
        if(readLen <= 0 ) break;
        while(1){
            W25QXX_Write(buf,addr,readLen);
            memset(BufRead,0,sizeof(BufRead));
            W25QXX_Read(BufRead,addr,readLen);
            if(memcmp(buf,BufRead,sizeof(BufRead)) == 0){
                break;
            }
            WV_printf("badNum = %d\n",badNum);
            badNum ++;
        }

        addr += readLen;
    }

    fclose(fp);
    return WV_SOK;
}

/************************************************************************************
 * 读取Flash
 * **********************************************************************************/
WV_S32 FPGA_UPDATE_ReadFromFlash()
{
    WV_U32 fpgaBinSize;
    WV_U32 lastSize=0;
    WV_U16 readNum=0,writeNum=0;
    WV_S32 i;
    WV_U8 buf[4*1024]={0};
    WV_S32 readLen;
    FILE *fp = NULL;

    fp = fopen("/opt/app/src_fpga.bin","rb");
    if(fp == NULL){
        WV_printf("打开源src_fpga.bin失败\n");
        return 0;            
    }else{
        fseek(fp, 0, SEEK_END);//移向END
        fpgaBinSize = ftell(fp);
        lastSize = fpgaBinSize;
    }   
    fclose(fp);

    fp = NULL;
    fp = fopen("/opt/app/des_fpga.bin","wb+");
    if(fp == NULL){
        WV_printf("打开源src_fpga.bin失败\n");
        return 0;            
    }
    WV_U32 addr = _D_FPGA_UPDATE_FLASH_ADDR;//0x80000
    while(lastSize){
        if(lastSize > 4*1024){
            
            lastSize -= 4*1024;
            readNum = 4*1024;
        }else{
            readNum = lastSize;
            lastSize = 0 ;
        }
        //memset();
        W25QXX_Read(buf,addr,readNum);        
        writeNum = fwrite(buf,1,readNum,fp);
        if(writeNum != readNum ){
            WV_ERROR("回读flash fpga.bin出错");
            break;
        }
        addr += readNum;
        
        //WV_printf("");
    }
    fclose(fp);
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
    WV_S32 readLen,i;
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
        WV_ERROR("md5 checkout false \n",pFile);
        return WV_EFAIL;      
    }

    return WV_SOK;
}

/************************************************************************************
 * WV_S32 FPGA_UPDATE_Update()
 * 调用升级fpga接口
 * **********************************************************************************/
WV_S32 FPGA_UPDATE_Update(WV_S8 *pMd5)
{

    WV_S32 i;
    WV_U16 flashID;
    if(fpga_update_md5Checkout(pMd5,_D_FPGA_UPDATE_SRC_MD5_FILEPATH) != 0 )
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
    //FPGA_UPDATE_WriteToFlash();
    FPGA_UPDATE_ReadFromFlash();
    FPGA_UPDATE_UpdateEnd();

}

/************************************************************************************
 * WV_S32 FPGA_UPDATE_SetCmd(WV_S32 argc, WV_S8 **argv, WV_S8 *prfBuff)
 * **********************************************************************************/
static WV_S32 FPGA_UPDATE_SetCmd(WV_S32 argc, WV_S8 **argv, WV_S8 *prfBuff)
{
    WV_U32 data, addr;
    WV_U16 x = 0, y = 0, w = 0, h = 0;
    WV_S32 ret = 0,i,j;
    WV_U8 buf[8192]={0};
    if (argc < 1)
    {
        prfBuff += sprintf(prfBuff, "set fpgaf <cmd>;//cmd like: start/addr/write/flash/all/erase\r\n");
        return 0;
    }
    if (strcmp(argv[0], "start") == 0){
        
        prfBuff += sprintf(prfBuff, "set fpgaf start\r\n");
        FPGA_UPDATE_UpdateStart();

    }else if (strcmp(argv[0], "stop") == 0){

        prfBuff += sprintf(prfBuff, "set fpgaf stop\r\n");
        FPGA_UPDATE_UpdateEnd();
    }
    else if (strcmp(argv[0], "addr") == 0)
    {

        if (argc < 2)
        {
            prfBuff += sprintf(prfBuff, "set fpgaf addr <data>\r\n");
            return WV_EFAIL;
        }
        ret = WV_STR_S2v(argv[1], &addr);
        if(addr <0x800000){
            prfBuff += sprintf(prfBuff, "sectorErase err!! addr = %d\n",addr);
            return 0;
        }      

    }else if ((strcmp(argv[0], "write") == 0))
    {
        if (argc < 2)
        {
            prfBuff += sprintf(prfBuff, "set fpgaf write <addr>\r\n");
            return WV_EFAIL;
        }
        ret = WV_STR_S2v(argv[1], &addr);
        if(addr <0x800000){
            prfBuff += sprintf(prfBuff, "sectorErase err!! addr = %d\n",addr);
            return 0;
        }
        //prfBuff += sprintf(prfBuff, "sectorErase start!! addr = %d[0x%X]\n",addr,addr);
        j=0;
        for(i=0;i<8192;i++){
            
            buf[i] = j;
            j++;
            if(j>255) j=0;
        }

        W25QXX_Write(buf,addr,128);       
        prfBuff += sprintf(prfBuff, "\r\nwrite end !addr = 0x%X\n",addr);

    }else if((strcmp(argv[0], "flash") == 0))
    {
        FPGA_UPDATE_WriteToFlash();
    }else if((strcmp(argv[0], "all") == 0)){
        W25QXX_Erase_Chip();
    }else if(strcmp(argv[0],"erase") == 0){
        
        int i=128;
        for(i=128;i<256;i++){
            W25QXX_Erase_Block(i);
        }

    }

    return WV_SOK;
}
/************************************************************************************
 * WV_S32 FPGA_UPDATE_GetCmd(WV_S32 argc, WV_S8 **argv, WV_S8 *prfBuff)
 * **********************************************************************************/
static WV_S32 FPGA_UPDATE_GetCmd(WV_S32 argc, WV_S8 **argv, WV_S8 *prfBuff)
{
    WV_U32 data, addr;
    WV_S32 ret = 0,i=0;
    if (argc < 1)
    {
        prfBuff += sprintf(prfBuff, "get fpgaf <cmd>;//cmd like:addr/id/flash \r\n");
        return 0;
    }

    if (strcmp(argv[0], "addr") == 0)
    {

        if (argc < 2)
        {
            prfBuff += sprintf(prfBuff, "get fpgaf addr <RdAddr>\r\n");
            return WV_EFAIL;
        }
        ret = WV_STR_S2v(argv[1], &addr);
        WV_U16 buflen = 256;
        WV_U8 buf[256]={0};
        W25QXX_Read(buf,addr,buflen);
        prfBuff += sprintf(prfBuff, "\r\n");
        for(i=0;i<buflen;i++){
            if((i+1)%10 == 0) prfBuff += sprintf(prfBuff, "\r\n");
            prfBuff += sprintf(prfBuff, "%02x ",buf[i]);
        }
        prfBuff += sprintf(prfBuff, "\r\n");

    }
    else if (strcmp(argv[0], "id") == 0){
        WV_U16 id;
        id=W25QXX_ReadID();
        prfBuff += sprintf(prfBuff, "id = 0x%X\r\n",id);
        
    }
    else if (strcmp(argv[0], "flash") == 0){
        FPGA_UPDATE_ReadFromFlash();
        
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
    WV_CMD_Register("set", "fpgaf", " set fpga", FPGA_UPDATE_SetCmd);
    WV_CMD_Register("get", "fpgaf", "pca9555 get reg", FPGA_UPDATE_GetCmd);

    return WV_SOK;
}
