///*****************************************
//  Copyright (C) 2009-2017
//  ITE Tech. Inc. All Rights Reserved
//  Proprietary and Confidential
///*****************************************
//   @file   <iTE6805_CEC_DRV.c>
//   @author Kuro.Chung@ite.com.tw
//   @date   2017/09/15
//   @fileversion: iTE6805_MCUSRC_1.02
//******************************************/
#include "iTE6805_Global.h"
#include "iTE6805_CEC_DRV.h"
#include "iTE6805_CEC_DEFINE.h"
extern _iTE6805_CEC *iTE6805_CEC;

void iTE6805_CEC_Fire_Tx(void)
{
	cecset(0x08, 0x88, 0x08);         	//FireCmd
	cecset(0x08, 0x88, 0x88);         	//FireCmd
}

void iTE6805_CEC_INIT_CMD(iTE_u8 Initiator,iTE_u8 Follower)
{
	iTE_u8 i;
	for(i = 0 ; i  < CEC_FRAME_SIZE; i++)
	{
		iTE6805_CEC->CEC_FRAME_TX.array[i]=0;
	}
	iTE6805_CEC->CEC_FRAME_TX.id.HEADER=((Initiator<<4)+Follower);
	iTE6805_CEC->CEC_FRAME_TX.id.SIZE=1;
}

iTE_u8 iTE6805_CEC_Check_Fire_Ready()
{
	iTE_u8 Reg44h;
	iTE_u8 Ready_Fire, Bus_Free;

	Reg44h = cecrd(0x44);
	Ready_Fire = Reg44h&BIT6; // Ready_Fire
	Bus_Free = Reg44h&BIT1; // Bus Free

	if (Ready_Fire == 1 && Bus_Free == 1)
	{
		// Ready Fire && Bus Free
		return TRUE;
	}
	else
	{
		return FALSE;
	}

}
iTE_u8 iTE6805_CEC_Check_IsPollingMsg()
{
	iTE_u8	u8Initiator, u8Follower;
	u8Initiator = cecrd(0x10);
	u8Follower = u8Initiator & 0x0F;
	u8Initiator >>= 4;
	if(u8Initiator == u8Follower)
	{
		return TRUE;
	}
	return FALSE;
}

void iTE6805_CEC_Reset_RX_FIFO()
{
	cecwr(0x52, 0x60);
	cecwr(0x52, 0xE0);
	cecwr(0x52, 0x80);
	iTE6805_CEC->Rx_TmpHeader[0] = 0;
}

iTE_u8 iTE6805_CEC_Get_TxFailStatus()
{
	iTE_u8	TxFailStatus = cecrd(0x54)&0x03;
	iTE_u8 temp = 0;
	switch(TxFailStatus)
	{
		case 0x01:
			return TX_FAIL_RECEIVE_ACK;
		case 0x02:
			return TX_FAIL_RECEIVE_NACK;
		case 0x04:
			return TX_FAIL_RETRY;
	}
	return 0;
}

void iTE6805_CEC_Clear_INT()
{
	cecset(0x08, BIT0, BIT0);
	cecset(0x08, BIT0, 0);
}



//////////////////////////////////////////////////////////////////////////////////////////
// CEC Queue
//////////////////////////////////////////////////////////////////////////////////////////



//****************************************************************************
iTE_u8 iTE6805_CEC_RX_CMD_Push(piTE_u8 pHeader)
{
	piTE_u8	pRX_Queue = iTE6805_CEC->Rx_Queue.Queue;
	iTE_u8	Rptr = iTE6805_CEC->Rx_Queue.Rptr;
	iTE_u8	Wptr = iTE6805_CEC->Rx_Queue.Wptr;
	iTE_u8	u8CecSize = (pHeader[2] & 0x1F) + 1; // Reg4F[0:4] -> Command FIFO In_Cnt
	iTE_u8	u8FreeSize;
	iTE_u8	u8Temp;
	// Reg4F[5:6] Error_Status
	if(pHeader[2] & 0x60)
	{
		CEC_DEBUG_PRINTF(("--------------CEC Rx ERROR: %x\n",(int) pHeader[2] & 0x60));
		return TRUE;
	}

	// Get Queue Free Size
	if(Rptr > Wptr)
	{
		u8FreeSize = Rptr - Wptr;
	}
	else
	{
		u8FreeSize = CEC_QUEUE_SIZE + Rptr - Wptr;
	}

	if(u8FreeSize < u8CecSize)
	{
		// If Free Size is less than CEC CMD Size
		return FALSE;
	}

	// push header to Queue
	CEC_DEBUG_PRINTF(("Before RxPush W=0x%02X, R=0x%02X\n", (int) Wptr,(int) Rptr));
	pRX_Queue[Wptr++ & CEC_QUEUE_MAX] = u8CecSize;
	pRX_Queue[Wptr++ & CEC_QUEUE_MAX] = pHeader[0];
	pRX_Queue[Wptr++ & CEC_QUEUE_MAX] = pHeader[1];
	Wptr &= CEC_QUEUE_MAX;
	CEC_DEBUG_PRINTF(("After RxPush W=0x%02X, R=0x%02X\n",(int) Wptr,(int) Rptr));

	// Clear Header
	pHeader[0] = 0;
	u8CecSize -= 3;

	// If it still have operand
	if(u8CecSize > 0)
	{
		if(CEC_QUEUE_SIZE - Wptr >= u8CecSize)
		{
			// Reg50 RxData
			cecbrd(0x50, u8CecSize, &pRX_Queue[Wptr]);
			Wptr += u8CecSize;
		}
		else
		{
			for(u8Temp = 0; u8Temp < u8CecSize; u8Temp++)
			{
				pRX_Queue[Wptr++ & CEC_QUEUE_MAX] = cecrd(0x50);
			}
			Wptr &= CEC_QUEUE_MAX;
		}
	}

	iTE6805_CEC->Rx_Queue.Wptr = Wptr;
	CEC_DEBUG_PRINTF(("oRxPush W=0x%02X, R=0x%02X\n",(int) Wptr,(int) Rptr));

	return TRUE;
}


iTE_u8 iTE6805_CEC_RX_CMD_Pull()
{
	iTE_u8	u8Temp, FillSize, i;
	iTE_u8	CMD_Size;
	piTE_u8	pu8CmdBuf = (piTE_u8)&(iTE6805_CEC->CEC_FRAME_RX.id.SIZE);

	piTE_u8	pRX_Queue = iTE6805_CEC->Rx_Queue.Queue;
	iTE_u8	Rptr = iTE6805_CEC->Rx_Queue.Rptr;
	iTE_u8	Wptr = iTE6805_CEC->Rx_Queue.Wptr;

	for(i = 0 ; i  < CEC_FRAME_SIZE; i++)
	{
		iTE6805_CEC->CEC_FRAME_RX.array[i] = 0;
	}

	// Get CMD Queue Filled Size
	if(Wptr >= Rptr)
	{
		FillSize = Wptr - Rptr;
	}
	else
	{
		FillSize = CEC_QUEUE_SIZE + Wptr - Rptr;
	}

	// Get Command, FillSize > 3 Because need Size, Header, OpCode
	if((iTE6805_CEC->Tx_QueueFull==TRUE) || (FillSize < 3))
	{
		return FALSE;
	}

	CEC_DEBUG_PRINTF(("iRxPull W=0x%02X, R=0x%02X\n",(int) Wptr,(int) Rptr));
	CMD_Size = pu8CmdBuf[0] = pRX_Queue[Rptr++]; // set pu8CmdBuf = CEC_FRAME.id.SIZE

	if(CMD_Size < 3)
	{
		// Size+Header+OpCode = 3
		// only polling is 2, but will be trigger RX Fail INT, so if we get < 3 CMD, that's an error
		CEC_DEBUG_PRINTF(("**ERROR: Rx Cmd size < 3\n"));
	}

	if(FillSize < CMD_Size) // Queue Size < CMD_Size
	{
		CMD_Size = FillSize;
		CEC_DEBUG_PRINTF(("**ERROR: Rx Cmd Buffer ERROR\n"));
	}

	FillSize -= CMD_Size;
	CMD_Size--; // Size already getted

	CEC_DEBUG_PRINTF(("CMD_Size =0x%02X\n",(int) CMD_Size));
	// get out CMD from Queue
	CEC_DEBUG_PRINTF(("RX_CMD => \n"));
	for(u8Temp = 1; u8Temp <= CMD_Size; u8Temp++)
	{
		pu8CmdBuf[u8Temp] = pRX_Queue[Rptr++ & CEC_QUEUE_MAX];
		CEC_DEBUG_PRINTF(("\t[%02d]=0x%02X\n",(int) u8Temp,(int) pu8CmdBuf[u8Temp]));
	}

	Rptr &= CEC_QUEUE_MAX;
	iTE6805_CEC->Rx_Queue.Rptr = Rptr;

	pu8CmdBuf[0]--;
	iTE6805_CEC->CEC_FRAME_RX.id.Follower = iTE6805_CEC->CEC_FRAME_RX.id.HEADER & 0x0F;
	iTE6805_CEC->CEC_FRAME_RX.id.Initiator= iTE6805_CEC->CEC_FRAME_RX.id.HEADER >> 4;
	CEC_DEBUG_PRINTF(("CEC Decoding -> Initiator = 0x%02X, Follower = 0x%02X \n", (int)iTE6805_CEC->CEC_FRAME_RX.id.Initiator,(int) iTE6805_CEC->CEC_FRAME_RX.id.Follower));

	return TRUE;

}
