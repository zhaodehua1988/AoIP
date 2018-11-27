
// printf wrapper that can be turned on and off by defining/undefining
#define WV_UDPTX_DEBUG_MODE 
#ifdef WV_UDPTX_DEBUG_MODE 
  #define UDPTX_printf(...)  do { printf("\n\r UDPRX:" __VA_ARGS__); fflush(stdout); } while(0)
#else 
  #define UDPTXprintf(...) 
#endif

#include "wv_udpTx.h"


/***********************************************

WV_S32  WV_UDPTX_Creat(WV_ETH_UDP_TX_E *pSock);

************************************************/

WV_S32  WV_UDPTX_Creat(WV_ETH_UDP_TX_E *pSock)
{
 
	WV_S32         reuse,nSendBufLen;
	socklen_t      len; 
    struct sockaddr_in *pAddr;
    pAddr = & (pSock->addrTx); 
	pSock->socketHndl  = socket(AF_INET, SOCK_DGRAM, 0);  
	if ( pSock->socketHndl < 0 ) 
		{
		 WV_printf("cannot open socket \r\n");
		 return WV_EFAIL;
		 }
    //set reuse
	reuse = 1;
	len = sizeof(reuse);
	WV_CHECK_RET(setsockopt(pSock->socketHndl, SOL_SOCKET,SO_REUSEADDR,(char*)&reuse,len));
     //set buffer len 
   /*
	nSendBufLen =  WV_COMM_UDPTX_BUF_MAXLEN; 
	len =  sizeof(nSendBufLen);
   // UDPTX_printf("set buf len [%d]\r\n",nSendBufLen);
	WV_CHECK_RET(setsockopt(pSock->socketHndl, SOL_SOCKET, SO_SNDBUF , (char*)&nSendBufLen, len )); 
	
	socklen_t optlen = sizeof(int); 
	nSendBufLen = 0;
   	WV_CHECK_RET(getsockopt(pSock->socketHndl, SOL_SOCKET, SO_SNDBUF,(char*)&nSendBufLen, &optlen)); 
	if(nSendBufLen !=  WV_COMM_UDPRX_BUF_MAXLEN)
	{
	  WV_ERROR("set  buffer len erro:%d \r\n",nSendBufLen);
	} 
	*/
     pAddr = & (pSock->addrLocal); 
	if(bind(pSock->socketHndl, (struct sockaddr *) pAddr, sizeof(struct sockaddr_in) ) < 0 )
	{
		close(pSock->socketHndl);
		pSock->socketHndl= -1;	
	    WV_ERROR("bind erro \r\n");	
		return -1;
	}
	/*
	UDPTX_printf("recv addr[%d:%d:%d:%d]\r\n",pSock->addrTx.sin_addr.s_addr & 0xFF,\
	                                        (pSock->addrTx.sin_addr.s_addr>>8) & 0xFF,\
	                                        (pSock->addrTx.sin_addr.s_addr>>16) & 0xFF,\
	                                        (pSock->addrTx.sin_addr.s_addr>>24) & 0xFF     ); 
    UDPTX_printf("recv port[%d]\r\n", ((pSock->addrTx.sin_port<<8) &   0xFF00 )|((pSock->addrTx.sin_port>>8) &  0xFF) );                                        
	UDPTX_printf("local addr[%d:%d:%d:%d]\r\n",pSock->addrLocal.sin_addr.s_addr & 0xFF,\
	                                        (pSock->addrLocal.sin_addr.s_addr>>8) & 0xFF,\
	                                        (pSock->addrLocal.sin_addr.s_addr>>16) & 0xFF,\
	                                        (pSock->addrLocal.sin_addr.s_addr>>24) & 0xFF     );  
	                                        
     UDPTX_printf("local port[%d]\r\n", ((pSock->addrLocal.sin_port<<8) &   0xFF00 )|((pSock->addrLocal.sin_port>>8) &  0xFF));    
	 */
	return WV_SOK; 
}


/***********************************************

WV_S32  WV_UDPTX_Destroy(WV_ETH_UDP_TX_E *pSock);

************************************************/

WV_S32  WV_UDPTX_Destroy(WV_ETH_UDP_TX_E *pSock)
{
  close(pSock->socketHndl);
  pSock->socketHndl= -1;
  return WV_SOK; 
}

/***********************************************

WV_S32  WV_UDP_Tx(WV_ETH_UDP_TX_E *pSock,WV_U8 *pBuf, WV_S32 len);

************************************************/
WV_S32  WV_UDP_Tx(WV_ETH_UDP_TX_E *pSock,WV_U8 *pBuf, WV_S32 len) 
{
    
	struct sockaddr_in *pAddr;
	WV_S32  send_addr_len,sendLen;  
	//get tx ip
	pAddr = & (pSock->addrTx);  
	send_addr_len  = sizeof(struct sockaddr); 
	sendLen = sendto(pSock->socketHndl,pBuf,len,0,(struct sockaddr*)pAddr,send_addr_len); 
   if(sendLen != len)			
     { 
       pSock-> erroCnt++; 
      return WV_EFAIL;        
     } 
	return WV_SOK; 
}


