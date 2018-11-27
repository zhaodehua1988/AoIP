#ifndef _WV_UDP_TX_H_
#define _WV_UDP_TX_H_
#include "wv_common.h" 
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h> 
#include <netinet/in.h>
#include <arpa/inet.h>
typedef struct  WV_ETH_UDP_TX_E
{

 WV_S32  socketHndl;
 struct sockaddr_in	addrTx;
 struct sockaddr_in	addrLocal;
 WV_U32 erroCnt;
  
}  WV_ETH_UDP_TX_E;

WV_S32  WV_UDPTX_Creat(WV_ETH_UDP_TX_E *pDev);
WV_S32  WV_UDPTX_Destroy(WV_ETH_UDP_TX_E *pDev);
WV_S32  WV_UDP_Tx(WV_ETH_UDP_TX_E *pSock,WV_U8 *pBuf, WV_S32 len); 

#endif
