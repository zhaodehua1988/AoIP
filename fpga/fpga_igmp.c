#include "fpga_igmp.h"
#include "wv_thr.h"
#include "his_spi.h"
typedef struct FPGA_IGMP_DEV
{
    WV_THR_HNDL_T thrHndl;
    WV_U32 open;
    WV_U32 close;

} FPGA_IGMP_DEV;

FPGA_IGMP_DEV gFpgaIgmpDev;

/****************************************************
*组播协议测试加入
****************************************************/
static WV_S32 gIgmpMode = 0;
void FPGA_IGMP_join()
{
    //加入组播 232.2.2.2 源：169.254.11.254

    WV_U16 regAddr = 0x81;
    WV_U16 baseAddr = 0x100;
    WV_U16  addr;
    int i=0;
    for (i=0; i < 4; i++)
    {
        addr = (((baseAddr >> 8) + i) << 8) | regAddr;
        WV_printf("fgpa igmp join: set [0x%X]\n",addr);
        HIS_SPI_FpgaWd(addr, 0x101); // set spi 0x181 0x101
        HIS_SPI_FpgaWd(addr, 0x000); // set spi 0x181 0x000
        HIS_SPI_FpgaWd(addr, 0x05e); // set spi 0x181 0x05e
        HIS_SPI_FpgaWd(addr, 0x000); // set spi 0x181 0x000
        HIS_SPI_FpgaWd(addr, 0x000); // set spi 0x181 0x000
        HIS_SPI_FpgaWd(addr, 0x016); // set spi 0x181 0x016
        HIS_SPI_FpgaWd(addr, 0x000); // set spi 0x181 0x000
        HIS_SPI_FpgaWd(addr, 0x020); // set spi 0x181 0x020
        HIS_SPI_FpgaWd(addr, 0x0fc); // set spi 0x181 0x0fc
        HIS_SPI_FpgaWd(addr, 0x032); // set spi 0x181 0x032
        HIS_SPI_FpgaWd(addr, 0x08b); // set spi 0x181 0x08b
        HIS_SPI_FpgaWd(addr, 0x0fd); // set spi 0x181 0x0fd
        HIS_SPI_FpgaWd(addr, 0x008); // set spi 0x181 0x008
        HIS_SPI_FpgaWd(addr, 0x000); // set spi 0x181 0x000
        HIS_SPI_FpgaWd(addr, 0x046); // set spi 0x181 0x046
        HIS_SPI_FpgaWd(addr, 0x000); // set spi 0x181 0x000
        HIS_SPI_FpgaWd(addr, 0x000); // set spi 0x181 0x000
        HIS_SPI_FpgaWd(addr, 0x02c); // set spi 0x181 0x02c
        HIS_SPI_FpgaWd(addr, 0x0b5); // set spi 0x181 0x0b5
        HIS_SPI_FpgaWd(addr, 0x0af); // set spi 0x181 0x0af
        HIS_SPI_FpgaWd(addr, 0x000); // set spi 0x181 0x000
        HIS_SPI_FpgaWd(addr, 0x000); // set spi 0x181 0x000
        HIS_SPI_FpgaWd(addr, 0x001); // set spi 0x181 0x001
        HIS_SPI_FpgaWd(addr, 0x002); // set spi 0x181 0x002
        HIS_SPI_FpgaWd(addr, 0x0da); // set spi 0x181 0x0da
        HIS_SPI_FpgaWd(addr, 0x004); // set spi 0x181 0x004
        HIS_SPI_FpgaWd(addr, 0x0a9); // set spi 0x181 0x0a9
        HIS_SPI_FpgaWd(addr, 0x0fe); // set spi 0x181 0x0fe
        HIS_SPI_FpgaWd(addr, 0x00b); // set spi 0x181 0x00b
        HIS_SPI_FpgaWd(addr, 0x003); // set spi 0x181 0x003
        HIS_SPI_FpgaWd(addr, 0x0e0); // set spi 0x181 0x0e0
        HIS_SPI_FpgaWd(addr, 0x000); // set spi 0x181 0x000
        HIS_SPI_FpgaWd(addr, 0x000); // set spi 0x181 0x000
        HIS_SPI_FpgaWd(addr, 0x016); // set spi 0x181 0x016
        HIS_SPI_FpgaWd(addr, 0x094); // set spi 0x181 0x094
        HIS_SPI_FpgaWd(addr, 0x004); // set spi 0x181 0x004
        HIS_SPI_FpgaWd(addr, 0x000); // set spi 0x181 0x000
        HIS_SPI_FpgaWd(addr, 0x000); // set spi 0x181 0x000
        HIS_SPI_FpgaWd(addr, 0x022); // set spi 0x181 0x022
        HIS_SPI_FpgaWd(addr, 0x000); // set spi 0x181 0x000
        HIS_SPI_FpgaWd(addr, 0x03a); // set spi 0x181 0x03a
        HIS_SPI_FpgaWd(addr, 0x0fc); // set spi 0x181 0x0fc
        HIS_SPI_FpgaWd(addr, 0x000); // set spi 0x181 0x000
        HIS_SPI_FpgaWd(addr, 0x000); // set spi 0x181 0x000
        HIS_SPI_FpgaWd(addr, 0x000); // set spi 0x181 0x000
        HIS_SPI_FpgaWd(addr, 0x001); // set spi 0x181 0x001
        HIS_SPI_FpgaWd(addr, 0x003); // set spi 0x181 0x003
        HIS_SPI_FpgaWd(addr, 0x000); // set spi 0x181 0x000
        HIS_SPI_FpgaWd(addr, 0x000); // set spi 0x181 0x000
        HIS_SPI_FpgaWd(addr, 0x001); // set spi 0x181 0x001
        HIS_SPI_FpgaWd(addr, 0x0e8); // set spi 0x181 0x0e8
        HIS_SPI_FpgaWd(addr, 0x002); // set spi 0x181 0x002
        HIS_SPI_FpgaWd(addr, 0x002); // set spi 0x181 0x002
        HIS_SPI_FpgaWd(addr, 0x002); // set spi 0x181 0x002
        HIS_SPI_FpgaWd(addr, 0x0a9); // set spi 0x181 0x0a9
        HIS_SPI_FpgaWd(addr, 0x0fe); // set spi 0x181 0x0fe
        HIS_SPI_FpgaWd(addr, 0x00b); // set spi 0x181 0x00b
        HIS_SPI_FpgaWd(addr, 0x0fe); // set spi 0x181 0x0fe
        HIS_SPI_FpgaWd(addr, 0x000); // set spi 0x181 0x000
        HIS_SPI_FpgaWd(addr, 0x200); // set spi 0x181 0x200

        //加入组播 232    WV_U16 addr = 0x181;.4.4.4 源 192.168.18.20
        HIS_SPI_FpgaWd(addr, 0x501); // set spi 0x181 0x501
        HIS_SPI_FpgaWd(addr, 0x400); // set spi 0x181 0x400
        HIS_SPI_FpgaWd(addr, 0x45e); // set spi 0x181 0x45e
        HIS_SPI_FpgaWd(addr, 0x400); // set spi 0x181 0x400
        HIS_SPI_FpgaWd(addr, 0x400); // set spi 0x181 0x400
        HIS_SPI_FpgaWd(addr, 0x416); // set spi 0x181 0x416
        HIS_SPI_FpgaWd(addr, 0x400); // set spi 0x181 0x400
        HIS_SPI_FpgaWd(addr, 0x420); // set spi 0x181 0x420
        HIS_SPI_FpgaWd(addr, 0x4fc); // set spi 0x181 0x4fc
        HIS_SPI_FpgaWd(addr, 0x432); // set spi 0x181 0x432
        HIS_SPI_FpgaWd(addr, 0x48b); // set spi 0x181 0x48b
        HIS_SPI_FpgaWd(addr, 0x4fd); // set spi 0x181 0x4fd
        HIS_SPI_FpgaWd(addr, 0x408); // set spi 0x181 0x408
        HIS_SPI_FpgaWd(addr, 0x400); // set spi 0x181 0x400
        HIS_SPI_FpgaWd(addr, 0x446); // set spi 0x181 0x446FPGA_IGMP_exit()
        HIS_SPI_FpgaWd(addr, 0x400); // set spi 0x181 0x400
        HIS_SPI_FpgaWd(addr, 0x400); // set spi 0x181 0x400
        HIS_SPI_FpgaWd(addr, 0x200); // set spi 0x181 0x42c
        HIS_SPI_FpgaWd(addr, 0x4b5); // set spi 0x181 0x4b5
        HIS_SPI_FpgaWd(addr, 0x4af); // set spi 0x181 0x4af
        HIS_SPI_FpgaWd(addr, 0x400); // set spi 0x181 0x400
        HIS_SPI_FpgaWd(addr, 0x400); // set spi 0x181 0x400
        HIS_SPI_FpgaWd(addr, 0x401); // set spi 0x181 0x401
        HIS_SPI_FpgaWd(addr, 0x402); // set spi 0x181 0x402
        HIS_SPI_FpgaWd(addr, 0x4da); // set spi 0x181 0x4da
        HIS_SPI_FpgaWd(addr, 0x404); // set spi 0x181 0x404
        HIS_SPI_FpgaWd(addr, 0x4a9); // set spi 0x181 0x4a9
        HIS_SPI_FpgaWd(addr, 0x4fe); // set spi 0x181 0x4fe
        HIS_SPI_FpgaWd(addr, 0x40b); // set spi 0x181 0x40b
        HIS_SPI_FpgaWd(addr, 0x403); // set spi 0x181 0x403
        HIS_SPI_FpgaWd(addr, 0x4e0); // set spi 0x181 0x4e0
        HIS_SPI_FpgaWd(addr, 0x400); // set spi 0x181 0x400
        HIS_SPI_FpgaWd(addr, 0x400); // set spi 0x181 0x400
        HIS_SPI_FpgaWd(addr, 0x416); // set spi 0x181 0x416
        HIS_SPI_FpgaWd(addr, 0x494); // set spi 0x181 0x494
        HIS_SPI_FpgaWd(addr, 0x404); // set spi 0x181 0x404
        HIS_SPI_FpgaWd(addr, 0x400); // set spi 0x181 0x400
        HIS_SPI_FpgaWd(addr, 0x400); // set spi 0x181 0x400
        HIS_SPI_FpgaWd(addr, 0x422); // set spi 0x181 0x422
        HIS_SPI_FpgaWd(addr, 0x400); // set spi 0x181 0x400
        HIS_SPI_FpgaWd(addr, 0x41c); // set spi 0x181 0x41c
        HIS_SPI_FpgaWd(addr, 0x438); // set spi 0x181 0x438
        HIS_SPI_FpgaWd(addr, 0x400); // set spi 0x181 0x400
        HIS_SPI_FpgaWd(addr, 0x400); // set spi 0x181 0x400
        HIS_SPI_FpgaWd(addr, 0x400); // set spi 0x181 0x400
        HIS_SPI_FpgaWd(addr, 0x401); // set spi 0x181 0x401
        HIS_SPI_FpgaWd(addr, 0x403); // set spi 0x181 0x403
        HIS_SPI_FpgaWd(addr, 0x400); // set spi 0x181 0x400
        HIS_SPI_FpgaWd(addr, 0x400); // set spi 0x181 0x400
        HIS_SPI_FpgaWd(addr, 0x401); // set spi 0x181 0x401
        HIS_SPI_FpgaWd(addr, 0x4e8); // set spi 0x181 0x4e8
        HIS_SPI_FpgaWd(addr, 0x404); // set spi 0x181 0x404
        HIS_SPI_FpgaWd(addr, 0x404); // set spi 0x181 0x404
        HIS_SPI_FpgaWd(addr, 0x404); // set spi 0x181 0x404
        HIS_SPI_FpgaWd(addr, 0x4c0); // set spi 0x181 0x4c0
        HIS_SPI_FpgaWd(addr, 0x4a8); // set spi 0x181 0x4a8
        HIS_SPI_FpgaWd(addr, 0x412); // set spi 0x181 0x412
        HIS_SPI_FpgaWd(addr, 0x414); // set spi 0x181 0x414
        HIS_SPI_FpgaWd(addr, 0x400); // set spi 0x181 0x400
        HIS_SPI_FpgaWd(addr, 0x600); // set spi 0x181 0x600

        //加入组播 232.5.5.5 源 168.2.2.4
        HIS_SPI_FpgaWd(addr, 0x901); //set spi 0x181 0x901
        HIS_SPI_FpgaWd(addr, 0x800); //set spi 0x181 0x800
        HIS_SPI_FpgaWd(addr, 0x85e); //set spi 0x181 0x85e
        HIS_SPI_FpgaWd(addr, 0x800); //set spi 0x181 0x800
        HIS_SPI_FpgaWd(addr, 0x800); //set spi 0x181 0x800
        HIS_SPI_FpgaWd(addr, 0x816); //set spi 0x181 0x816
        HIS_SPI_FpgaWd(addr, 0x800); //set spi 0x181 0x800
        HIS_SPI_FpgaWd(addr, 0x820); //set spi 0x181 0x820
        HIS_SPI_FpgaWd(addr, 0x8fc); //set spi 0x181 0x8fc
        HIS_SPI_FpgaWd(addr, 0x832); //set spi 0x181 0x832
        HIS_SPI_FpgaWd(addr, 0x88b); //set spi 0x181 0x88b
        HIS_SPI_FpgaWd(addr, 0x8fd); //set spi 0x181 0x8fd
        HIS_SPI_FpgaWd(addr, 0x808); //set spi 0x181 0x808
        HIS_SPI_FpgaWd(addr, 0x800); //set spi 0x181 0x800
        HIS_SPI_FpgaWd(addr, 0x846); //set spi 0x181 0x846
        HIS_SPI_FpgaWd(addr, 0x800); //set spi 0x181 0x800
        HIS_SPI_FpgaWd(addr, 0x800); //set spi 0x181 0x800
        HIS_SPI_FpgaWd(addr, 0x82c); //set spi 0x181 0x82c
        HIS_SPI_FpgaWd(addr, 0x8b5); //set spi 0x181 0x8b5
        HIS_SPI_FpgaWd(addr, 0x8af); //set spi 0x181 0x8af
        HIS_SPI_FpgaWd(addr, 0x800); //set spi 0x181 0x800
        HIS_SPI_FpgaWd(addr, 0x800); //set spi 0x181 0x800
        HIS_SPI_FpgaWd(addr, 0x801); //set spi 0x181 0x801
        HIS_SPI_FpgaWd(addr, 0x802); //set spi 0x181 0x802
        HIS_SPI_FpgaWd(addr, 0x8da); //set spi 0x181 0x8da
        HIS_SPI_FpgaWd(addr, 0x804); //set spi 0x181 0x804
        HIS_SPI_FpgaWd(addr, 0x8a9); //set spi 0x181 0x8a9
        HIS_SPI_FpgaWd(addr, 0x8fe); //set spi 0x181 0x8fe
        HIS_SPI_FpgaWd(addr, 0x80b); //set spi 0x181 0x80b
        HIS_SPI_FpgaWd(addr, 0x803); //set spi 0x181 0x803
        HIS_SPI_FpgaWd(addr, 0x8e0); //set spi 0x181 0x8e0
        HIS_SPI_FpgaWd(addr, 0x800); //set spi 0x181 0x800
        HIS_SPI_FpgaWd(addr, 0x800); //set spi 0x181 0x800
        HIS_SPI_FpgaWd(addr, 0x816); //set spi 0x181 0x816
        HIS_SPI_FpgaWd(addr, 0x894); //set spi 0x181 0x894
        HIS_SPI_FpgaWd(addr, 0x804); //set spi 0x181 0x804
        HIS_SPI_FpgaWd(addr, 0x800); //set spi 0x181 0x800
        HIS_SPI_FpgaWd(addr, 0x800); //set spi 0x181 0x800
        HIS_SPI_FpgaWd(addr, 0x822); //set spi 0x181 0x822
        HIS_SPI_FpgaWd(addr, 0x800); //set spi 0x181 0x800
        HIS_SPI_FpgaWd(addr, 0x843); //set spi 0x181 0x843
        HIS_SPI_FpgaWd(addr, 0x8ec); //set spi 0x181 0x8ec
        HIS_SPI_FpgaWd(addr, 0x800); //set spi 0x181 0x800
        HIS_SPI_FpgaWd(addr, 0x800); //set spi 0x181 0x800
        HIS_SPI_FpgaWd(addr, 0x800); //set spi 0x181 0x800
        HIS_SPI_FpgaWd(addr, 0x801); //set spi 0x181 0x801
        HIS_SPI_FpgaWd(addr, 0x803); //set spi 0x181 0x803
        HIS_SPI_FpgaWd(addr, 0x800); //set spi 0x181 0x800
        HIS_SPI_FpgaWd(addr, 0x800); //set spi 0x181 0x800
        HIS_SPI_FpgaWd(addr, 0x801); //set spi 0x181 0x801
        HIS_SPI_FpgaWd(addr, 0x8e8); //set spi 0x181 0x8e8
        HIS_SPI_FpgaWd(addr, 0x805); //set spi 0x181 0x805
        HIS_SPI_FpgaWd(addr, 0x805); //set spi 0x181 0x805
        HIS_SPI_FpgaWd(addr, 0x805); //set spi 0x181 0x805
        HIS_SPI_FpgaWd(addr, 0x8a8); //set spi 0x181 0x8a8
        HIS_SPI_FpgaWd(addr, 0x802); //set spi 0x181 0x802
        HIS_SPI_FpgaWd(addr, 0x802); //set spi 0x181 0x802
        HIS_SPI_FpgaWd(addr, 0x804); //set spi 0x181 0x804
        HIS_SPI_FpgaWd(addr, 0x800); //set spi 0x181 0x800
        HIS_SPI_FpgaWd(addr, 0xa00); //set spi 0x181 0xa00

        //加入组播 232.6.6.6 源 169.0.1.4
        HIS_SPI_FpgaWd(addr, 0xd01); //set spi 0x181 0xd01
        HIS_SPI_FpgaWd(addr, 0xc00); //set spi 0x181 0xc00
        HIS_SPI_FpgaWd(addr, 0xc5e); //set spi 0x181 0xc5e
        HIS_SPI_FpgaWd(addr, 0xc00); //set spi 0x181 0xc00
        HIS_SPI_FpgaWd(addr, 0xc00); //set spi 0x181 0xc00
        HIS_SPI_FpgaWd(addr, 0xc16); //set spi 0x181 0xc16
        HIS_SPI_FpgaWd(addr, 0xc00); //set spi 0x181 0xc00
        HIS_SPI_FpgaWd(addr, 0xc20); //set spi 0x181 0xc20
        HIS_SPI_FpgaWd(addr, 0xcfc); //set spi 0x181 0xcfc
        HIS_SPI_FpgaWd(addr, 0xc32); //set spi 0x181 0xc32
        HIS_SPI_FpgaWd(addr, 0xc8b); //set spi 0x181 0xc8b
        HIS_SPI_FpgaWd(addr, 0xcfd); //set spi 0x181 0xcfd
        HIS_SPI_FpgaWd(addr, 0xc08); //set spi 0x181 0xc08
        HIS_SPI_FpgaWd(addr, 0xc00); //set spi 0x181 0xc00
        HIS_SPI_FpgaWd(addr, 0xc46); //set spi 0x181 0xc46
        HIS_SPI_FpgaWd(addr, 0xc00); //set spi 0x181 0xc00
        HIS_SPI_FpgaWd(addr, 0xc00); //set spi 0x181 0xc00
        HIS_SPI_FpgaWd(addr, 0xc2c); //set spi 0x181 0xc2c
        HIS_SPI_FpgaWd(addr, 0xcb5); //set spi 0x181 0xcb5
        HIS_SPI_FpgaWd(addr, 0xcaf); //set spi 0x181 0xcaf
        HIS_SPI_FpgaWd(addr, 0xc00); //set spi 0x181 0xc00
        HIS_SPI_FpgaWd(addr, 0xc00); //set spi 0x181 0xc00
        HIS_SPI_FpgaWd(addr, 0xc01); //set spi 0x181 0xc01
        HIS_SPI_FpgaWd(addr, 0xc02); //set spi 0x181 0xc02
        HIS_SPI_FpgaWd(addr, 0xcda); //set spi 0x181 0xcda
        HIS_SPI_FpgaWd(addr, 0xc04); //set spi 0x181 0xc04
        HIS_SPI_FpgaWd(addr, 0xca9); //set spi 0x181 0xca9
        HIS_SPI_FpgaWd(addr, 0xcfe); //set spi 0x181 0xcfe
        HIS_SPI_FpgaWd(addr, 0xc0b); //set spi 0x181 0xc0b
        HIS_SPI_FpgaWd(addr, 0xc03); //set spi 0x181 0xc03
        HIS_SPI_FpgaWd(addr, 0xce0); //set spi 0x181 0xce0
        HIS_SPI_FpgaWd(addr, 0xc00); //set spi 0x181 0xc00
        HIS_SPI_FpgaWd(addr, 0xc00); //set spi 0x181 0xc00
        HIS_SPI_FpgaWd(addr, 0xc16); //set spi 0x181 0xc16
        HIS_SPI_FpgaWd(addr, 0xc94); //set spi 0x181 0xc94
        HIS_SPI_FpgaWd(addr, 0xc04); //set spi 0x181 0xc04
        HIS_SPI_FpgaWd(addr, 0xc00); //set spi 0x181 0xc00
        HIS_SPI_FpgaWd(addr, 0xc00); //set spi 0x181 0xc00
        HIS_SPI_FpgaWd(addr, 0xc22); //set spi 0x181 0xc22
        HIS_SPI_FpgaWd(addr, 0xc00); //set spi 0x181 0xc00
        HIS_SPI_FpgaWd(addr, 0xc42); //set spi 0x181 0xc42
        HIS_SPI_FpgaWd(addr, 0xcec); //set spi 0x181 0xcec
        HIS_SPI_FpgaWd(addr, 0xc00); //set spi 0x181 0xc00
        HIS_SPI_FpgaWd(addr, 0xc00); //set spi 0x181 0xc00
        HIS_SPI_FpgaWd(addr, 0xc00); //set spi 0x181 0xc00
        HIS_SPI_FpgaWd(addr, 0xc01); //set spi 0x181 0xc01
        HIS_SPI_FpgaWd(addr, 0xc03); //set spi 0x181 0xc03
        HIS_SPI_FpgaWd(addr, 0xc00); //set spi 0x181 0xc00
        HIS_SPI_FpgaWd(addr, 0xc00); //set spi 0x181 0xc00
        HIS_SPI_FpgaWd(addr, 0xc01); //set spi 0x181 0xc01
        HIS_SPI_FpgaWd(addr, 0xce8); //set spi 0x181 0xce8
        HIS_SPI_FpgaWd(addr, 0xc06); //set spi 0x181 0xc06
        HIS_SPI_FpgaWd(addr, 0xc06); //set spi 0x181 0xc06
        HIS_SPI_FpgaWd(addr, 0xc06); //set spi 0x181 0xc06
        HIS_SPI_FpgaWd(addr, 0xca9); //set spi 0x181 0xca9
        HIS_SPI_FpgaWd(addr, 0xc00); //set spi 0x181 0xc00
        HIS_SPI_FpgaWd(addr, 0xc01); //set spi 0x181 0xc01
        HIS_SPI_FpgaWd(addr, 0xc04); //set spi 0x181 0xc04
        HIS_SPI_FpgaWd(addr, 0xc00); //set spi 0x181 0xc00
        HIS_SPI_FpgaWd(addr, 0xe00); //set spi 0x181 0xe00
    }
}

void FPGA_IGMP_exit()
{
    //离开 232.2.2.2
    WV_U16 regAddr = 0x81;
    WV_U16 baseAddr = 0x100;
    WV_U16  addr;
    WV_S32 i;
    for (i = 0; i < 4; i++)
    {
        addr = (((baseAddr >> 8) + i) << 8 )| regAddr;
        WV_printf("fgpa igmp exit: set [0x%X]\n",addr);
        HIS_SPI_FpgaWd(addr, 0x101); //set spi 0x181 0x101
        HIS_SPI_FpgaWd(addr, 0x000); //set spi 0x181 0x000
        HIS_SPI_FpgaWd(addr, 0x05e); //set spi 0x181 0x05e
        HIS_SPI_FpgaWd(addr, 0x000); //set spi 0x181 0x000
        HIS_SPI_FpgaWd(addr, 0x000); //set spi 0x181 0x000
        HIS_SPI_FpgaWd(addr, 0x016); //set spi 0x181 0x016
        HIS_SPI_FpgaWd(addr, 0x000); //set spi 0x181 0x000
        HIS_SPI_FpgaWd(addr, 0x020); //set spi 0x181 0x020
        HIS_SPI_FpgaWd(addr, 0x0fc); //set spi 0x181 0x0fc
        HIS_SPI_FpgaWd(addr, 0x032); //set spi 0x181 0x032
        HIS_SPI_FpgaWd(addr, 0x08b); //set spi 0x181 0x08b
        HIS_SPI_FpgaWd(addr, 0x0fd); //set spi 0x181 0x0fd
        HIS_SPI_FpgaWd(addr, 0x008); //set spi 0x181 0x008
        HIS_SPI_FpgaWd(addr, 0x000); //set spi 0x181 0x000
        HIS_SPI_FpgaWd(addr, 0x046); //set spi 0x181 0x046
        HIS_SPI_FpgaWd(addr, 0x000); //set spi 0x181 0x000
        HIS_SPI_FpgaWd(addr, 0x000); //set spi 0x181 0x000
        HIS_SPI_FpgaWd(addr, 0x028); //set spi 0x181 0x028
        HIS_SPI_FpgaWd(addr, 0x0b5); //set spi 0x181 0x0b5
        HIS_SPI_FpgaWd(addr, 0x0af); //set spi 0x181 0x0af
        HIS_SPI_FpgaWd(addr, 0x000); //set spi 0x181 0x000
        HIS_SPI_FpgaWd(addr, 0x000); //set spi 0x181 0x000
        HIS_SPI_FpgaWd(addr, 0x001); //set spi 0x181 0x001
        HIS_SPI_FpgaWd(addr, 0x002); //set spi 0x181 0x002
        HIS_SPI_FpgaWd(addr, 0x0da); //set spi 0x181 0x0da
        HIS_SPI_FpgaWd(addr, 0x008); //set spi 0x181 0x008
        HIS_SPI_FpgaWd(addr, 0x0a9); //set spi 0x181 0x0a9
        HIS_SPI_FpgaWd(addr, 0x0fe); //set spi 0x181 0x0fe
        HIS_SPI_FpgaWd(addr, 0x00b); //set spi 0x181 0x00b
        HIS_SPI_FpgaWd(addr, 0x003); //set spi 0x181 0x003
        HIS_SPI_FpgaWd(addr, 0x0e0); //set spi 0x181 0x0e0
        HIS_SPI_FpgaWd(addr, 0x000); //set spi 0x181 0x000
        HIS_SPI_FpgaWd(addr, 0x000); //set spi 0x181 0x000
        HIS_SPI_FpgaWd(addr, 0x016); //set spi 0x181 0x016
        HIS_SPI_FpgaWd(addr, 0x094); //set spi 0x181 0x094
        HIS_SPI_FpgaWd(addr, 0x004); //set spi 0x181 0x004
        HIS_SPI_FpgaWd(addr, 0x000); //set spi 0x181 0x000
        HIS_SPI_FpgaWd(addr, 0x000); //set spi 0x181 0x000
        HIS_SPI_FpgaWd(addr, 0x022); //set spi 0x181 0x022
        HIS_SPI_FpgaWd(addr, 0x000); //set spi 0x181 0x000
        HIS_SPI_FpgaWd(addr, 0x0f0); //set spi 0x181 0x0f0
        HIS_SPI_FpgaWd(addr, 0x0f9); //set spi 0x181 0x0f9
        HIS_SPI_FpgaWd(addr, 0x000); //set spi 0x181 0x000
        HIS_SPI_FpgaWd(addr, 0x000); //set spi 0x181 0x000
        HIS_SPI_FpgaWd(addr, 0x000); //set spi 0x181 0x000
        HIS_SPI_FpgaWd(addr, 0x001); //set spi 0x181 0x001
        HIS_SPI_FpgaWd(addr, 0x003); //set spi 0x181 0x003
        HIS_SPI_FpgaWd(addr, 0x000); //set spi 0x181 0x000
        HIS_SPI_FpgaWd(addr, 0x000); //set spi 0x181 0x000
        HIS_SPI_FpgaWd(addr, 0x000); //set spi 0x181 0x000
        HIS_SPI_FpgaWd(addr, 0x0e8); //set spi 0x181 0x0e8
        HIS_SPI_FpgaWd(addr, 0x002); //set spi 0x181 0x002
        HIS_SPI_FpgaWd(addr, 0x002); //set spi 0x181 0x002
        HIS_SPI_FpgaWd(addr, 0x002); //set spi 0x181 0x002
        HIS_SPI_FpgaWd(addr, 0x000); //set spi 0x181 0x000
        HIS_SPI_FpgaWd(addr, 0x000); //set spi 0x181 0x000
        HIS_SPI_FpgaWd(addr, 0x000); //set spi 0x181 0x000
        HIS_SPI_FpgaWd(addr, 0x000); //set spi 0x181 0x000
        HIS_SPI_FpgaWd(addr, 0x000); //set spi 0x181 0x000
        HIS_SPI_FpgaWd(addr, 0x200); //set spi 0x181 0x200

        //离开 232.4.4.4
        HIS_SPI_FpgaWd(addr, 0x501); //set spi 0x181 0x501
        HIS_SPI_FpgaWd(addr, 0x400); //set spi 0x181 0x400
        HIS_SPI_FpgaWd(addr, 0x45e); //set spi 0x181 0x45e
        HIS_SPI_FpgaWd(addr, 0x400); //set spi 0x181 0x400
        HIS_SPI_FpgaWd(addr, 0x400); //set spi 0x181 0x400
        HIS_SPI_FpgaWd(addr, 0x416); //set spi 0x181 0x416
        HIS_SPI_FpgaWd(addr, 0x400); //set spi 0x181 0x400
        HIS_SPI_FpgaWd(addr, 0x420); //set spi 0x181 0x420
        HIS_SPI_FpgaWd(addr, 0x4fc); //set spi 0x181 0x4fc
        HIS_SPI_FpgaWd(addr, 0x432); //set spi 0x181 0x432
        HIS_SPI_FpgaWd(addr, 0x48b); //set spi 0x181 0x48b
        HIS_SPI_FpgaWd(addr, 0x4fd); //set spi 0x181 0x4fd
        HIS_SPI_FpgaWd(addr, 0x408); //set spi 0x181 0x408
        HIS_SPI_FpgaWd(addr, 0x400); //set spi 0x181 0x400
        HIS_SPI_FpgaWd(addr, 0x446); //set spi 0x181 0x446
        HIS_SPI_FpgaWd(addr, 0x400); //set spi 0x181 0x400
        HIS_SPI_FpgaWd(addr, 0x400); //set spi 0x181 0x400
        HIS_SPI_FpgaWd(addr, 0x428); //set spi 0x181 0x428
        HIS_SPI_FpgaWd(addr, 0x4b5); //set spi 0x181 0x4b5
        HIS_SPI_FpgaWd(addr, 0x4af); //set spi 0x181 0x4af
        HIS_SPI_FpgaWd(addr, 0x400); //set spi 0x181 0x400
        HIS_SPI_FpgaWd(addr, 0x400); //set spi 0x181 0x400
        HIS_SPI_FpgaWd(addr, 0x401); //set spi 0x181 0x401
        HIS_SPI_FpgaWd(addr, 0x402); //set spi 0x181 0x402
        HIS_SPI_FpgaWd(addr, 0x4da); //set spi 0x181 0x4da
        HIS_SPI_FpgaWd(addr, 0x408); //set spi 0x181 0x408
        HIS_SPI_FpgaWd(addr, 0x4a9); //set spi 0x181 0x4a9
        HIS_SPI_FpgaWd(addr, 0x4fe); //set spi 0x181 0x4fe
        HIS_SPI_FpgaWd(addr, 0x40b); //set spi 0x181 0x40b
        HIS_SPI_FpgaWd(addr, 0x403); //set spi 0x181 0x403
        HIS_SPI_FpgaWd(addr, 0x4e0); //set spi 0x181 0x4e0
        HIS_SPI_FpgaWd(addr, 0x400); //set spi 0x181 0x400
        HIS_SPI_FpgaWd(addr, 0x400); //set spi 0x181 0x400
        HIS_SPI_FpgaWd(addr, 0x416); //set spi 0x181 0x416
        HIS_SPI_FpgaWd(addr, 0x494); //set spi 0x181 0x494
        HIS_SPI_FpgaWd(addr, 0x404); //set spi 0x181 0x404
        HIS_SPI_FpgaWd(addr, 0x400); //set spi 0x181 0x400
        HIS_SPI_FpgaWd(addr, 0x400); //set spi 0x181 0x400
        HIS_SPI_FpgaWd(addr, 0x422); //set spi 0x181 0x422
        HIS_SPI_FpgaWd(addr, 0x400); //set spi 0x181 0x400
        HIS_SPI_FpgaWd(addr, 0x4ee); //set spi 0x181 0x4ee
        HIS_SPI_FpgaWd(addr, 0x4f5); //set spi 0x181 0x4f5
        HIS_SPI_FpgaWd(addr, 0x400); //set spi 0x181 0x400
        HIS_SPI_FpgaWd(addr, 0x400); //set spi 0x181 0x400
        HIS_SPI_FpgaWd(addr, 0x400); //set spi 0x181 0x400
        HIS_SPI_FpgaWd(addr, 0x401); //set spi 0x181 0x401
        HIS_SPI_FpgaWd(addr, 0x403); //set spi 0x181 0x403
        HIS_SPI_FpgaWd(addr, 0x400); //set spi 0x181 0x400
        HIS_SPI_FpgaWd(addr, 0x400); //set spi 0x181 0x400
        HIS_SPI_FpgaWd(addr, 0x400); //set spi 0x181 0x400
        HIS_SPI_FpgaWd(addr, 0x4e8); //set spi 0x181 0x4e8
        HIS_SPI_FpgaWd(addr, 0x404); //set spi 0x181 0x404
        HIS_SPI_FpgaWd(addr, 0x404); //set spi 0x181 0x404
        HIS_SPI_FpgaWd(addr, 0x404); //set spi 0x181 0x404
        HIS_SPI_FpgaWd(addr, 0x400); //set spi 0x181 0x400
        HIS_SPI_FpgaWd(addr, 0x400); //set spi 0x181 0x400
        HIS_SPI_FpgaWd(addr, 0x400); //set spi 0x181 0x400
        HIS_SPI_FpgaWd(addr, 0x400); //set spi 0x181 0x400
        HIS_SPI_FpgaWd(addr, 0x400); //set spi 0x181 0x400
        HIS_SPI_FpgaWd(addr, 0x600); //set spi 0x181 0x600

        //离开232.5.5.5
        HIS_SPI_FpgaWd(addr, 0x901); //set spi 0x181 0x901
        HIS_SPI_FpgaWd(addr, 0x800); //set spi 0x181 0x800
        HIS_SPI_FpgaWd(addr, 0x85e); //set spi 0x181 0x85e
        HIS_SPI_FpgaWd(addr, 0x800); //set spi 0x181 0x800
        HIS_SPI_FpgaWd(addr, 0x800); //set spi 0x181 0x800
        HIS_SPI_FpgaWd(addr, 0x816); //set spi 0x181 0x816
        HIS_SPI_FpgaWd(addr, 0x800); //set spi 0x181 0x800
        HIS_SPI_FpgaWd(addr, 0x820); //set spi 0x181 0x820
        HIS_SPI_FpgaWd(addr, 0x8fc); //set spi 0x181 0x8fc
        HIS_SPI_FpgaWd(addr, 0x832); //set spi 0x181 0x832
        HIS_SPI_FpgaWd(addr, 0x88b); //set spi 0x181 0x88b
        HIS_SPI_FpgaWd(addr, 0x8fd); //set spi 0x181 0x8fd
        HIS_SPI_FpgaWd(addr, 0x808); //set spi 0x181 0x808
        HIS_SPI_FpgaWd(addr, 0x800); //set spi 0x181 0x800
        HIS_SPI_FpgaWd(addr, 0x846); //set spi 0x181 0x846
        HIS_SPI_FpgaWd(addr, 0x800); //set spi 0x181 0x800
        HIS_SPI_FpgaWd(addr, 0x800); //set spi 0x181 0x800
        HIS_SPI_FpgaWd(addr, 0x828); //set spi 0x181 0x828
        HIS_SPI_FpgaWd(addr, 0x8b5); //set spi 0x181 0x8b5
        HIS_SPI_FpgaWd(addr, 0x8af); //set spi 0x181 0x8af
        HIS_SPI_FpgaWd(addr, 0x800); //set spi 0x181 0x800
        HIS_SPI_FpgaWd(addr, 0x800); //set spi 0x181 0x800
        HIS_SPI_FpgaWd(addr, 0x801); //set spi 0x181 0x801
        HIS_SPI_FpgaWd(addr, 0x802); //set spi 0x181 0x802
        HIS_SPI_FpgaWd(addr, 0x8da); //set spi 0x181 0x8da
        HIS_SPI_FpgaWd(addr, 0x808); //set spi 0x181 0x808
        HIS_SPI_FpgaWd(addr, 0x8a9); //set spi 0x181 0x8a9
        HIS_SPI_FpgaWd(addr, 0x8fe); //set spi 0x181 0x8fe
        HIS_SPI_FpgaWd(addr, 0x80b); //set spi 0x181 0x80b
        HIS_SPI_FpgaWd(addr, 0x803); //set spi 0x181 0x803
        HIS_SPI_FpgaWd(addr, 0x8e0); //set spi 0x181 0x8e0
        HIS_SPI_FpgaWd(addr, 0x800); //set spi 0x181 0x800
        HIS_SPI_FpgaWd(addr, 0x800); //set spi 0x181 0x800
        HIS_SPI_FpgaWd(addr, 0x816); //set spi 0x181 0x816
        HIS_SPI_FpgaWd(addr, 0x894); //set spi 0x181 0x894
        HIS_SPI_FpgaWd(addr, 0x804); //set spi 0x181 0x804
        HIS_SPI_FpgaWd(addr, 0x800); //set spi 0x181 0x800
        HIS_SPI_FpgaWd(addr, 0x800); //set spi 0x181 0x800
        HIS_SPI_FpgaWd(addr, 0x822); //set spi 0x181 0x822
        HIS_SPI_FpgaWd(addr, 0x800); //set spi 0x181 0x800
        HIS_SPI_FpgaWd(addr, 0x8ed); //set spi 0x181 0x8ed
        HIS_SPI_FpgaWd(addr, 0x8f3); //set spi 0x181 0x8f3
        HIS_SPI_FpgaWd(addr, 0x800); //set spi 0x181 0x800
        HIS_SPI_FpgaWd(addr, 0x800); //set spi 0x181 0x800
        HIS_SPI_FpgaWd(addr, 0x800); //set spi 0x181 0x800
        HIS_SPI_FpgaWd(addr, 0x801); //set spi 0x181 0x801
        HIS_SPI_FpgaWd(addr, 0x803); //set spi 0x181 0x803
        HIS_SPI_FpgaWd(addr, 0x800); //set spi 0x181 0x800
        HIS_SPI_FpgaWd(addr, 0x800); //set spi 0x181 0x800
        HIS_SPI_FpgaWd(addr, 0x800); //set spi 0x181 0x800
        HIS_SPI_FpgaWd(addr, 0x8e8); //set spi 0x181 0x8e8
        HIS_SPI_FpgaWd(addr, 0x805); //set spi 0x181 0x805
        HIS_SPI_FpgaWd(addr, 0x805); //set spi 0x181 0x805
        HIS_SPI_FpgaWd(addr, 0x805); //set spi 0x181 0x805
        HIS_SPI_FpgaWd(addr, 0x800); //set spi 0x181 0x800
        HIS_SPI_FpgaWd(addr, 0x800); //set spi 0x181 0x800
        HIS_SPI_FpgaWd(addr, 0x800); //set spi 0x181 0x800
        HIS_SPI_FpgaWd(addr, 0x800); //set spi 0x181 0x800
        HIS_SPI_FpgaWd(addr, 0x800); //set spi 0x181 0x800
        HIS_SPI_FpgaWd(addr, 0xa00); //set spi 0x181 0xa00

        //离开组播232.6.6.6
        HIS_SPI_FpgaWd(addr, 0xd01); //set spi 0x181 0xd01
        HIS_SPI_FpgaWd(addr, 0xc00); //set spi 0x181 0xc00
        HIS_SPI_FpgaWd(addr, 0xc5e); //set spi 0x181 0xc5e
        HIS_SPI_FpgaWd(addr, 0xc00); //set spi 0x181 0xc00
        HIS_SPI_FpgaWd(addr, 0xc00); //set spi 0x181 0xc00
        HIS_SPI_FpgaWd(addr, 0xc16); //set spi 0x181 0xc16
        HIS_SPI_FpgaWd(addr, 0xc00); //set spi 0x181 0xc00
        HIS_SPI_FpgaWd(addr, 0xc20); //set spi 0x181 0xc20
        HIS_SPI_FpgaWd(addr, 0xcfc); //set spi 0x181 0xcfc
        HIS_SPI_FpgaWd(addr, 0xc32); //set spi 0x181 0xc32
        HIS_SPI_FpgaWd(addr, 0xc8b); //set spi 0x181 0xc8b
        HIS_SPI_FpgaWd(addr, 0xcfd); //set spi 0x181 0xcfd
        HIS_SPI_FpgaWd(addr, 0xc08); //set spi 0x181 0xc08
        HIS_SPI_FpgaWd(addr, 0xc00); //set spi 0x181 0xc00
        HIS_SPI_FpgaWd(addr, 0xc46); //set spi 0x181 0xc46
        HIS_SPI_FpgaWd(addr, 0xc00); //set spi 0x181 0xc00
        HIS_SPI_FpgaWd(addr, 0xc00); //set spi 0x181 0xc00
        HIS_SPI_FpgaWd(addr, 0xc28); //set spi 0x181 0xc28
        HIS_SPI_FpgaWd(addr, 0xcb5); //set spi 0x181 0xcb5
        HIS_SPI_FpgaWd(addr, 0xcaf); //set spi 0x181 0xcaf
        HIS_SPI_FpgaWd(addr, 0xc00); //set spi 0x181 0xc00
        HIS_SPI_FpgaWd(addr, 0xc00); //set spi 0x181 0xc00
        HIS_SPI_FpgaWd(addr, 0xc01); //set spi 0x181 0xc01
        HIS_SPI_FpgaWd(addr, 0xc02); //set spi 0x181 0xc02
        HIS_SPI_FpgaWd(addr, 0xcda); //set spi 0x181 0xcda
        HIS_SPI_FpgaWd(addr, 0xc08); //set spi 0x181 0xc08
        HIS_SPI_FpgaWd(addr, 0xca9); //set spi 0x181 0xca9
        HIS_SPI_FpgaWd(addr, 0xcfe); //set spi 0x181 0xcfe
        HIS_SPI_FpgaWd(addr, 0xc0b); //set spi 0x181 0xc0b
        HIS_SPI_FpgaWd(addr, 0xc03); //set spi 0x181 0xc03
        HIS_SPI_FpgaWd(addr, 0xce0); //set spi 0x181 0xce0
        HIS_SPI_FpgaWd(addr, 0xc00); //set spi 0x181 0xc00
        HIS_SPI_FpgaWd(addr, 0xc00); //set spi 0x181 0xc00
        HIS_SPI_FpgaWd(addr, 0xc16); //set spi 0x181 0xc16
        HIS_SPI_FpgaWd(addr, 0xc94); //set spi 0x181 0xc94
        HIS_SPI_FpgaWd(addr, 0xc04); //set spi 0x181 0xc04
        HIS_SPI_FpgaWd(addr, 0xc00); //set spi 0x181 0xc00
        HIS_SPI_FpgaWd(addr, 0xc00); //set spi 0x181 0xc00
        HIS_SPI_FpgaWd(addr, 0xc22); //set spi 0x181 0xc22
        HIS_SPI_FpgaWd(addr, 0xc00); //set spi 0x181 0xc00
        HIS_SPI_FpgaWd(addr, 0xcec); //set spi 0x181 0xcec
        HIS_SPI_FpgaWd(addr, 0xcf1); //set spi 0x181 0xcf1
        HIS_SPI_FpgaWd(addr, 0xc00); //set spi 0x181 0xc00
        HIS_SPI_FpgaWd(addr, 0xc00); //set spi 0x181 0xc00
        HIS_SPI_FpgaWd(addr, 0xc00); //set spi 0x181 0xc00
        HIS_SPI_FpgaWd(addr, 0xc01); //set spi 0x181 0xc01
        HIS_SPI_FpgaWd(addr, 0xc03); //set spi 0x181 0xc03
        HIS_SPI_FpgaWd(addr, 0xc00); //set spi 0x181 0xc00
        HIS_SPI_FpgaWd(addr, 0xc00); //set spi 0x181 0xc00
        HIS_SPI_FpgaWd(addr, 0xc00); //set spi 0x181 0xc00
        HIS_SPI_FpgaWd(addr, 0xce8); //set spi 0x181 0xce8
        HIS_SPI_FpgaWd(addr, 0xc06); //set spi 0x181 0xc06
        HIS_SPI_FpgaWd(addr, 0xc06); //set spi 0x181 0xc06
        HIS_SPI_FpgaWd(addr, 0xc06); //set spi 0x181 0xc06
        HIS_SPI_FpgaWd(addr, 0xc00); //set spi 0x181 0xc00
        HIS_SPI_FpgaWd(addr, 0xc00); //set spi 0x181 0xc00
        HIS_SPI_FpgaWd(addr, 0xc00); //set spi 0x181 0xc00
        HIS_SPI_FpgaWd(addr, 0xc00); //set spi 0x181 0xc00
        HIS_SPI_FpgaWd(addr, 0xc00); //set spi 0x181 0xc00
        HIS_SPI_FpgaWd(addr, 0xe00); //set spi 0x181 0xe00
    }
}
//设置下发报文数据时4k的还是1080p的
void FPGA_IGMP_SetMode(WV_S32 mode)
{
    gIgmpMode = mode;
    FPGA_IGMP_enable();
}
//下发报文数据
void FPGA_IGMP_enable()
{

    WV_U16 baseAddr = 0x100, regAddr = 0;
    WV_S32 i;
    for (i = 0; i < 4; i++)
    {
        regAddr = (((baseAddr >> 8) + i) << 8) | 0x82;
        if (gIgmpMode == 0)
        {
            HIS_SPI_FpgaWd(regAddr, 0xf);
            //printf("igmp 1080 set [0x%0X] = [0x%0X]\n", regAddr, 0xf);
        }
        else
        {
            HIS_SPI_FpgaWd(regAddr, 0x1);
            //printf("igmp 4K set [0x%0X] = [0x%0X]\n", regAddr, 0x1);
        }
    }
}

void *FPGA_IGMP_Proc(void *prm)
{

    FPGA_IGMP_DEV *pDev = (FPGA_IGMP_DEV *)prm;
    pDev->open = 1;
    pDev->close = 0;
    //iTE6615_fsm();

    while (pDev->open == 1)
    {
        FPGA_IGMP_enable();
        sleep(2);
    }
}

/************************************************************
 * void FPGA_IGMP_Open()
 * *********************************************************/
void FPGA_IGMP_Open()
{

    FPGA_IGMP_join();
    WV_THR_Create(&gFpgaIgmpDev.thrHndl, FPGA_IGMP_Proc, WV_THR_PRI_DEFAULT, WV_THR_STACK_SIZE_DEFAULT, &gFpgaIgmpDev);
    WV_printf("fpga igmp init end \n");
}
/************************************************************
 * void FPGA_IGMP_Close()
 * *********************************************************/
void FPGA_IGMP_Close()
{

    if (gFpgaIgmpDev.open == 1)
    {
        gFpgaIgmpDev.open = 0;
        while (gFpgaIgmpDev.close == 1)
            ;
        WV_THR_Destroy(&gFpgaIgmpDev.thrHndl);
    }
    FPGA_IGMP_exit();
    WV_printf("fpga igmp deinit ok..");
}
