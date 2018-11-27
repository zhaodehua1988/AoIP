///*****************************************
//  Copyright (C) 2009-2017
//  ITE Tech. Inc. All Rights Reserved
//  Proprietary and Confidential
///*****************************************
//   @file   <iTE6805_EDID.c>
//   @author Kuro.Chung@ite.com.tw
//   @date   2017/09/15
//   @fileversion: iTE6805_MCUSRC_1.02
//******************************************/

#include "iTE6805_Global.h"
#include "iTE6805_EDID_Table.h"
#include "iTE6805_DEV_DEFINE.h"

//extern _iTE6805_DATA iTE6805_DATA;
//extern _iTE6805_PARSE3D_STR iTE6805_EDID_Parse3D;
_iTE6805_DATA iTE6805_DATA;
_iTE6805_PARSE3D_STR iTE6805_EDID_Parse3D;

#ifdef 	_HDMI_SWITCH_
iTE_u8  txphyadr[2];	// for CEC function (TX Reveice EDID ABCD transfrom to RX EDID ABCD phy Value, only for HDMI Switch)
#endif

iTE_u8  rxphyadr[2][2];	// for EDID RAM function

void iTE6805_EDID_Init()
{
	// init EDID
#ifdef _ENABLE_EDID_RAM_
	chgbank(0);
	hdmirxwr(0x4B, (ADDR_EDID|0x01));	//[7:1] EDID RAM Slave Adr ,[0]1: Enable access EDID block
	iTE6805_EDID_RAMInitial();
	iTE6805_EDID_ParseVSDB_3Dblock();
	hdmirxset(0xC5, 0x01, 0x00);	// enable PORT0 internal EDID
	chgbank(4);
	hdmirxset(0xC5, 0x01, 0x00);	// enable PORT1 internal EDID
	chgbank(0);

	//come from 6802 for reset EDID start
    hdmirxset(0xC5, 0x10, 0x10);	// reset PORT0
    delay1ms(1);
    hdmirxset(0xC5, 0x10, 0x00);

	chgbank(4);
    hdmirxset(0xC5, 0x10, 0x10);	// reset PORT1
    delay1ms(1);
    hdmirxset(0xC5, 0x10, 0x00);
	chgbank(0);

	//come from 6802 for reset EDID start
#else
	hdmirxset(0xC5, 0x01, 0x01);
	chgbank(4);
	hdmirxset(0xC5, 0x01, 0x01);
	chgbank(0);
#endif

#ifdef _ENABLE_MHL_SUPPORT_3D_
	iTE6805_EDID_Parse3D.bVSDBspport3D = 0;
	iTE6805_EDID_Parse3D.ucVicCnt = 0;
	iTE6805_EDID_Parse3D.ucDtdCnt = 0;
#endif
}


//EDIDRAMInitial
void iTE6805_EDID_RAMInitial(){

	iTE_u8 *piTE6805EDID = &Default_Edid_Block[0]; // Always 0 , define only one EDID
    iTE_u8 Block0_CheckSum;
    iTE_u8 Block1_CheckSum;
    iTE_u8 VSDB_ADDR = 0 ;
    iTE_u8 PORT0_EDID_Block1_CheckSum;
    iTE_u8 PORT1_EDID_Block1_CheckSum;

    EDID_DEBUG_PRINTF(("EnableEDIDupdata()\n"));
    iTE6805_Set_HPD_Ctrl(PORT0,HPD_LOW);	// port 0, set HPD = 0
    iTE6805_Set_HPD_Ctrl(PORT1,HPD_LOW);	// port 1, set HPD = 0

	// update BLOCK0
	Block0_CheckSum = iTE6805_EDID_UpdateRAM(piTE6805EDID,0);
	chgbank(0);
	hdmirxwr(0xC9,Block0_CheckSum);		//Port 0 Block 0 CheckSum
	chgbank(4);
	hdmirxwr(0xC9,Block0_CheckSum);		//Port 1 Block 0 CheckSum
	chgbank(0);
	// get BLOCK1 CheckSum but not ready
	Block1_CheckSum = iTE6805_EDID_UpdateRAM(piTE6805EDID,1);

	// need to find VSDB ADDR to fill 6805 reg for CEC function
	VSDB_ADDR = iTE6805_EDID_Find_Phyaddress(piTE6805EDID);

	if(VSDB_ADDR == 0) return;

#ifdef 	_HDMI_SWITCH_
	// if HDMI not a HDMI switch , do not need this
	txphyadr[0]=(*(piTE6805EDID+VSDB_ADDR  ));
	txphyadr[1]=(*(piTE6805EDID+VSDB_ADDR+1));
	EDID_DEBUG_PRINTF(("txphyadr[0] = 0x%02X\n",(int)  txphyadr[0]));
	EDID_DEBUG_PRINTF(("txphyadr[1] = 0x%02X\n",(int)  txphyadr[1]));
	iTE6805_EDID_Set_Phyaddress();
#else
	rxphyadr[PORT0][0] = 0x10;
	rxphyadr[PORT0][1] = 0x00;
	rxphyadr[PORT1][0] = 0x20;
	rxphyadr[PORT1][1] = 0x00;
#endif

	// caulate BLOCK1 checksum and setting VSDB AB CD value
    PORT0_EDID_Block1_CheckSum=(Block1_CheckSum + piTE6805EDID[VSDB_ADDR] + piTE6805EDID[VSDB_ADDR+1] - rxphyadr[PORT0][0] - rxphyadr[PORT0][1])%0x100;
    PORT1_EDID_Block1_CheckSum=(Block1_CheckSum + piTE6805EDID[VSDB_ADDR] + piTE6805EDID[VSDB_ADDR+1] - rxphyadr[PORT1][0] - rxphyadr[PORT1][1])%0x100;

	// Update Related EDID REG
	chgbank(0);
    hdmirxwr(0xC6,VSDB_ADDR);						//VSDB Start Address
    hdmirxwr(0xC7,rxphyadr[PORT0][0]);	//Port 0 AB
    hdmirxwr(0xC8,rxphyadr[PORT0][1]);	//Port 0 CD
    hdmirxwr(0xCA,PORT0_EDID_Block1_CheckSum);		//Port 0 Block 1 CheckSum
	chgbank(4);
    hdmirxwr(0xC7,rxphyadr[PORT1][0]);	//Port 1 AB
    hdmirxwr(0xC8,rxphyadr[PORT1][1]);	//Port 1 CD
    hdmirxwr(0xCA,PORT1_EDID_Block1_CheckSum);		//Port 1 Block 1 CheckSum
	chgbank(0);

	EDID_DEBUG_PRINTF((" PORT0_EDID_Block1_CheckSum = %02X\n", (int) PORT0_EDID_Block1_CheckSum));
	EDID_DEBUG_PRINTF((" PORT1_EDID_Block1_CheckSum = %02X\n", (int) PORT1_EDID_Block1_CheckSum));
	EDID_DEBUG_PRINTF((" EDID_VSDB_ADDR = %02X\n" , (int) VSDB_ADDR));
}



iTE_u8 iTE6805_EDID_UpdateRAM(iTE_u8 *pEDID,iTE_u8 BlockNUM)
{
    iTE_u8 i, offset, sum =0;

    if ( BlockNUM == 0x02 ) offset = 0x00+128*0x01;
    else  					offset = 0x00+128*BlockNUM;

    EDID_DEBUG_PRINTF(("block No =%02X offset = %02X\n",(int) BlockNUM,(int) offset));

    for( i=0 ; i<0x7F ; i++ ){
		EDID_DEBUG_PRINTF(("%02X ",(int) *(pEDID+offset)));
        edid_ram_wr(offset,1 ,(pEDID+offset));
        sum += *(pEDID+offset);
        offset ++;
        if((i % 16) == 15){
            EDID_DEBUG_PRINTF(("\n"));
        }
    }
	EDID_DEBUG_PRINTF(("\n"));
    sum = 0x00 - sum;
    return sum;
}


iTE_u8 iTE6805_EDID_Find_Phyaddress(iTE_u8 *pEDID)
{
    iTE_u8 AddStart = 128;	// EDID Block1 offset
    iTE_u8 tag, length, i;		// EDID Tag, and length
    iTE_u8 offset,End;
    iTE_u8 VSDB_ADDR = 0;

#ifdef _ENABLE_MHL_SUPPORT_3D_
	iTE_u8 ADDR_3DPresent, ucTemp;
#endif

	// CEA Extension Revision Number
    if((*(pEDID+AddStart))!=0x2 || (*(pEDID+AddStart+1))!=0x3)
        return 0;

    End = (*(pEDID+AddStart+2));

	// AddStart+0x04 for ignore CEA Extension Header
    for(offset=(AddStart+0x04); offset < (AddStart+End); )
    {
        tag=(*(pEDID+offset))>>5;
        length=(*(pEDID+offset)) & 0x1f;

        EDID_DEBUG_PRINTF(("offset = %X , Tag = %X , length =%X\n", (int) offset,(int)  tag, (int) length));

        offset++;

		switch(tag)
		{
			// HDMI VSDB Block of EDID
			case 0x03 :
				EDID_DEBUG_PRINTF(("HDMI VSDB Block address = %X\n",(int)  offset));

				if(	(*(pEDID+offset  ))==0x03 &&    // 24-bit IEEE Registrator Identifier = 0x000C03
					(*(pEDID+offset+1))==0x0C &&
					(*(pEDID+offset+2))==0x0    )
				{
					VSDB_ADDR=offset+3;
#ifdef _ENABLE_MHL_SUPPORT_3D_
					// Paser EDID 3D block in VSDB if it have for using in MSC Command
					// 3D block in EDID VSDB Block in HDMI spec v1.4a

					if (length < 7)		// VSDB block length < 7, no 3D support !!!
						return VSDB_ADDR;

					ADDR_3DPresent = offset + 7;
					ucTemp = *(pEDID + offset + 7);

					if (ucTemp & 0x80)				// If it have 'Video and Audio Latency present', need add offset
						ADDR_3DPresent += 2;

					if (ucTemp & 0x40)				// If it have 'Interlaced Video and Audio Latency present', need add offset
						ADDR_3DPresent += 2;

					if (ucTemp & 0x20)				// If it have 'HDMI additional video format present' need add offset
					{
						ADDR_3DPresent++;
					}

					iTE6805_EDID_Parse3D.uc3DEdidStart = ADDR_3DPresent;
					if ((*(pEDID + ADDR_3DPresent)) & 0x80)
					{
						// 3D_present = 1 = EDID support 3D , only it condition needed support 3D
						iTE6805_EDID_Parse3D.bVSDBspport3D = TRUE;
					}
#endif
					return VSDB_ADDR;
				}
				break;


#if 0 // mark useless block
			//HDMI Video Data Block (include one or more Short Video Description)
			case 0x02 :
				iTE6805_EDID_Parse3D.ucVicCnt = length;
				for(i = 0 ; i < length ; i++)
				{
					ucTemp = (*(pEDID+offset+i) ) & 0x7F;
					iTE6805_EDID_Parse3D.SVD_LIST[i] = ucTemp; // short video decription
				}
				break;
#endif

			default:
				break;
        }


        offset=offset+length;
    }
    return 0;
}

#ifdef _HDMI_SWITCH_
void iTE6805_EDID_Set_Phyaddress()
{
	iTE_u8 rxphyA, rxphyB, rxphyC, rxphyD, rxcurport;			// for CEC function
	iTE_u8 txphyA, txphyB, txphyC, txphyD, txphylevel;
    txphyA = (txphyadr[0]&0xF0)>>4;
    txphyB = (txphyadr[0]&0x0F);
    txphyC = (txphyadr[1]&0xF0)>>4;
    txphyD = (txphyadr[1]&0x0F);

    if( txphyA==0 && txphyB==0 && txphyC==0 && txphyD==0 )
        txphylevel = 0;
    else if( txphyB==0 && txphyC==0 && txphyD==0 )
        txphylevel = 1;
    else if( txphyC==0 && txphyD==0 )
        txphylevel = 2;
    else if( txphyD==0 )
        txphylevel = 3;
    else
        txphylevel = 4;

    rxcurport = 0;
    switch( txphylevel )
    {
        case 0:
            rxphyadr[0][0] = 0x10;
            rxphyadr[0][1] = 0x00;
            rxphyadr[1][0] = 0x20;
            rxphyadr[1][1] = 0x00;
            break;
        case 1:
            rxphyadr[0][0] = (txphyA<<4)+0x01;
            rxphyadr[0][1] = 0x00;
            rxphyadr[1][0] = (txphyA<<4)+0x02;
            rxphyadr[1][1] = 0x00;
            break;
        case 2:
            rxphyadr[0][0] = txphyadr[0];
            rxphyadr[0][1] = 0x10;
            rxphyadr[1][0] = txphyadr[0];
            rxphyadr[1][1] = 0x20;
            break;
        case 3:
            rxphyadr[0][0] = txphyadr[0];
            rxphyadr[0][1] = (txphyC<<4)+0x01;
            rxphyadr[1][0] = txphyadr[0];
            rxphyadr[1][1] = (txphyC<<4)+0x02;
            break;
        default:
            rxphyadr[0][0] = 0xFF;
            rxphyadr[0][1] = 0xFF;
            rxphyadr[1][0] = 0xFF;
            rxphyadr[1][1] = 0xFF;
            break;
    }
    EDID_DEBUG_PRINTF(("\nDnStrm PhyAdr = %X, %X, %X, %X\n", (int) txphyA,(int)  txphyB, (int) txphyC, (int) txphyD));

    rxphyA = (rxphyadr[0][0]&0xF0)>>4;
    rxphyB = (rxphyadr[0][0]&0x0F);
    rxphyC = (rxphyadr[0][1]&0xF0)>>4;
    rxphyD = (rxphyadr[0][1]&0x0F);
    EDID_DEBUG_PRINTF((" PortA PhyAdr = %X, %X, %X, %X\n",(int)  rxphyA, (int) rxphyB, (int) rxphyC,(int)  rxphyD));

    rxphyA = (rxphyadr[1][0]&0xF0)>>4;
    rxphyB = (rxphyadr[1][0]&0x0F);
    rxphyC = (rxphyadr[1][1]&0xF0)>>4;
    rxphyD = (rxphyadr[1][1]&0x0F);
    EDID_DEBUG_PRINTF((" PortB PhyAdr = %X, %X, %X, %X\n", (int) rxphyA,(int)  rxphyB,(int)  rxphyC,(int)  rxphyD));
}
#endif

void iTE6805_EDID_ParseVSDB_3Dblock()
{

    iTE_u8	ucTemp;
    iTE_u8	uc3DMulti;
    iTE_u8	uc3DEdidEnd = 0xFF;
    iTE_u8	ucRdPtr = iTE6805_EDID_Parse3D.uc3DEdidStart; // start from 3D_present address

    PARSE3D_STA	e3DEdidState = PARSE3D_START;

    //check with HDMI VSDB block of EDID, If 3D_present = 1 , MHL 3D supported
    if(iTE6805_EDID_Parse3D.bVSDBspport3D == 0x00)
    {
        iTE6805_EDID_Parse3D.ucVicCnt=0;
        return;
    }
    // Re-initial bVSDBspport3D =0 then check with 3D_Structure and 3D_MASK at HDMI VSDB block of EDID
    iTE6805_EDID_Parse3D.bVSDBspport3D = 0x00;

    MHL3D_DEBUG_PRINTF(("***   EDID_ParseVSDB_3Dblock   ***\n"));
    MHL3D_DEBUG_PRINTF(("MHL 3D [2]LR [1]TB [0]FS\n"));

    if(ucRdPtr ==0)
        return;

	for(;ucRdPtr <= uc3DEdidEnd;){

		switch(e3DEdidState)
		{
			case PARSE3D_START:
				uc3DMulti = (Default_Edid_Block[ucRdPtr++]&0x60);
				e3DEdidState = PARSE3D_LEN;
				break;

			case PARSE3D_LEN:
				// EDID End = currentPtr + VIC LEN + 3D LEN
				uc3DEdidEnd = ucRdPtr + (Default_Edid_Block[ucRdPtr] >> 5) + (Default_Edid_Block[ucRdPtr] & 0x1F) ;
				ucRdPtr += (Default_Edid_Block[ucRdPtr] >> 5) + 1; // Parse 3D struct, Add CurrentPtr to VIC End
				e3DEdidState = PARSE3D_STRUCT_H;
				break;

			case PARSE3D_STRUCT_H: // parse 3D_Struct high byte
				switch (uc3DMulti)
				{
					case	0x20:
					case	0x40:
						// 3D_Structure_ALL_8, Sink support "Side By Side" 3D format
						if (Default_Edid_Block[ucRdPtr++] & 0x01)
						{
							uc3DMulti |= MHL_3D_Support_SideBySide;
						}
						e3DEdidState = PARSE3D_STRUCT_L;
						break;
					default:
						e3DEdidState = PARSE3D_VIC;
						break;
				}
				break;
			case PARSE3D_STRUCT_L:
				ucTemp = Default_Edid_Block[ucRdPtr++];

				// 3D_Structure_ALL_6, Sink support "Top and Bottom" 3D format
				if (ucTemp & BIT6)
				{
					uc3DMulti |= MHL_3D_Support_TopBottom;
				}

				// 3D_Structure_ALL_0, Sink support "Frame Packing" 3D format
				if (ucTemp & BIT0)
				{
					uc3DMulti |= MHL_3D_Support_FramePacking;
				}

				uc3DMulti &= MHL_3D_Mask;	// set 3D Multi to 0, only left 3D_Support Info

				if ((uc3DMulti & 0x60) == 0x20)
				{
					e3DEdidState = PARSE3D_VIC;
					// If 3D_Multi_present = 01 is present and assigns 3D formats to all of the VICs listed
					// in the first 16 entries in the EDID. 3D_MASK_15 is not present.
					// Assign to all Vic one times
					for (ucTemp = 0; ucTemp<16; ucTemp++)
					{
						iTE6805_EDID_Parse3D.uc3DInfor[ucTemp] = uc3DMulti;
						MHL3D_DEBUG_PRINTF(("VSD[%d]=0x%x\n", (int)ucTemp, (int)uc3DMulti));
					}
				}
				else
				{
					e3DEdidState = PARSE3D_MASK_H;
				}
				break;

			case PARSE3D_MASK_H:
				if (Default_Edid_Block[ucRdPtr])
				{
					iTE6805_EDID_Parse3D.bVSDBspport3D = 0x01;	//for identify 3D_MASK have Short Video Descriptor (SVD) support 3D format
				}

				for (ucTemp = 0; ucTemp<8; ucTemp++)
				{
					if (Default_Edid_Block[ucRdPtr] & (1 << ucTemp))
					{
						iTE6805_EDID_Parse3D.uc3DInfor[ucTemp + 8] = uc3DMulti;
						MHL3D_DEBUG_PRINTF(("VSD[%d]=0x%x\n", (int)ucTemp + 8, (int)uc3DMulti));
					}
					else
					{
						iTE6805_EDID_Parse3D.uc3DInfor[ucTemp + 8] = 0;
					}
				}
				ucRdPtr++;
				e3DEdidState = PARSE3D_MASK_L;
				break;

			case PARSE3D_MASK_L:
				if (Default_Edid_Block[ucRdPtr])
				{
					iTE6805_EDID_Parse3D.bVSDBspport3D = 0x01;	//for identify 3D_MASK have SVD support 3D format
				}

				for (ucTemp = 0; ucTemp<8; ucTemp++)
				{
					if (Default_Edid_Block[ucRdPtr] & (1 << ucTemp))
					{
						iTE6805_EDID_Parse3D.uc3DInfor[ucTemp] = uc3DMulti;
						MHL3D_DEBUG_PRINTF(("VSD[%d]=0x%x\n", (int)ucTemp, (int)uc3DMulti));
					}
					else
					{
						iTE6805_EDID_Parse3D.uc3DInfor[ucTemp] = 0;
					}
				}
				ucRdPtr++;
				e3DEdidState = PARSE3D_VIC;
				break;
			case	PARSE3D_VIC:
				ucRdPtr += 1;
				break;
			default:
				break;
		}
	}
}
