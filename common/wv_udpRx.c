
// printf wrapper that can be turned on and off by defining/undefining
#define WV_UDPRX_DEBUG_MODE 
#ifdef WV_UDPRX_DEBUG_MODE 
  #define UDPRX_printf(...)  do { printf("\n\r UDPRX:" __VA_ARGS__); fflush(stdout); } while(0)
#else 
  #define UDPRXprintf(...) 
#endif

#include "wv_udpRx.h"



/***********************************************

WV_S32  WV_UDPRX_Creat(WV_ETH_UDP_RX_E *pDev);

************************************************/

WV_S32  WV_UDPRX_Creat(WV_ETH_UDP_RX_E *pDev)
{
 
	WV_S32         reuse,nSendBufLen;
	socklen_t      len; 
    struct sockaddr_in *pAddr;
    pAddr = & (pDev->addrRx); 
	pDev->socketHndl  = socket(AF_INET, SOCK_DGRAM, 0); 
	
	if ( pDev->socketHndl < 0 ) 
		{
		 WV_printf("cannot open socket \r\n");
		 return WV_EFAIL;
		 }
    //set reuse
	reuse = 1;
	len = sizeof(reuse);
	WV_CHECK_RET(setsockopt(pDev->socketHndl, SOL_SOCKET,SO_REUSEADDR,(char*)&reuse,len));
     //set buffer len   
	nSendBufLen =  WV_COMM_UDPRX_BUF_MAXLEN; 
	len =  sizeof(nSendBufLen);
    //UDPRX_printf("set buf len [%d]\r\n",nSendBufLen);
	//WV_CHECK_RET(setsockopt(pDev->socketHndl, SOL_SOCKET, SO_RCVBUF , (char*)&nSendBufLen, len ));
        
	//socklen_t optlen = sizeof(int); 
	//nSendBufLen = 0;
   	//WV_CHECK_RET(getsockopt(pDev->socketHndl, SOL_SOCKET, SO_RCVBUF,(char*)&nSendBufLen, &optlen)); 
	//if(nSendBufLen !=  WV_COMM_UDPRX_BUF_MAXLEN)
	//{
	//  WV_ERROR("set  buffer len erro:%d \r\n",nSendBufLen);
	//} 
	
	if(bind(pDev->socketHndl, (struct sockaddr *) pAddr, sizeof(struct sockaddr_in) ) < 0 )
	{
		close(pDev->socketHndl);
		pDev->socketHndl= -1;	
		 WV_ERROR("bind erro \r\n");	
		return -1;
	}
	/*
	UDPRX_printf("recv addr[%d:%d:%d:%d]\r\n",pDev->addrRx.sin_addr.s_addr & 0xFF,\
	                                        (pDev->addrRx.sin_addr.s_addr>>8) & 0xFF,\
	                                        (pDev->addrRx.sin_addr.s_addr>>16) & 0xFF,\
	                                        (pDev->addrRx.sin_addr.s_addr>>24) & 0xFF     ); 
    UDPRX_printf("recv port[%d]\r\n", ((pDev->addrRx.sin_port<<8) &   0xFF00 )|((pDev->addrRx.sin_port>>8) &  0xFF) );                                        
	UDPRX_printf("local addr[%d:%d:%d:%d]\r\n",pDev->addrLocal.sin_addr.s_addr & 0xFF,\
	                                        (pDev->addrLocal.sin_addr.s_addr>>8) & 0xFF,\
	                                        (pDev->addrLocal.sin_addr.s_addr>>16) & 0xFF,\
	                                        (pDev->addrLocal.sin_addr.s_addr>>24) & 0xFF     );  
	                                        
   UDPRX_printf("local port[%d]\r\n", ((pDev->addrLocal.sin_port<<8) &   0xFF00 )|((pDev->addrLocal.sin_port>>8) &  0xFF));     	  */                                                                             
	struct ip_mreq mreq;
	mreq.imr_multiaddr.s_addr  = pDev->addrRx.sin_addr.s_addr;
	mreq.imr_interface.s_addr  = pDev->addrLocal.sin_addr.s_addr;
	/*
	UDPRX_printf("multiaddr addr[%d:%d:%d:%d]\r\n",mreq.imr_multiaddr.s_addr & 0xFF,\
	                                        (mreq.imr_multiaddr.s_addr>>8) & 0xFF,\
	                                        (mreq.imr_multiaddr.s_addr>>16) & 0xFF,\
	                                        (mreq.imr_multiaddr.s_addr>>24) & 0xFF ); 
	UDPRX_printf("interface addr[%d:%d:%d:%d]\r\n",mreq.imr_interface.s_addr & 0xFF,\
	                                        (mreq.imr_interface.s_addr>>8) & 0xFF,\
	                                        (mreq.imr_interface.s_addr>>16) & 0xFF,\
	                                        (mreq.imr_interface.s_addr>>24) & 0xFF );                                         
	*/
    WV_CHECK_RET(setsockopt(pDev->socketHndl, SOL_IP,IP_ADD_MEMBERSHIP, &mreq, sizeof( mreq) ));
	return WV_SOK; 
}


/***********************************************

WV_S32  WV_UDPRX_Destroy(WV_ETH_UDP_RX_E *pDev);

************************************************/

WV_S32  WV_UDPRX_Destroy(WV_ETH_UDP_RX_E *pDev) 
{
  close(pDev->socketHndl);
  pDev->socketHndl= -1;
  return WV_SOK; 
}



/***********************************************

WV_S32  WV_UDP_Rx(WV_ETH_UDP_RX_E *pDev,WV_S8 *pBuf, WV_S32 *len);

************************************************/
WV_S32  WV_UDP_Rx(WV_ETH_UDP_RX_E *pDev,WV_S8 *pBuf, WV_S32 *len) 
{
    
	struct sockaddr_in *pAddr;
	WV_S32  recv_addr_len;  
	//get recv ip
	pAddr = & (pDev->addrRx);  
	recv_addr_len  = sizeof(struct sockaddr);
	*len =  recvfrom(pDev->socketHndl,pBuf,2048,0,(struct sockaddr *)pAddr, &recv_addr_len ); 
	//UDPRX_printf("recv data %d\r\n",*len); 
	return WV_SOK; 
}

