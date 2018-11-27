#ifndef _WV_COMMON_TYPE_H_
#define _WV_COMMON_TYPE_H_
 
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <stdlib.h> 
#include <sys/stat.h> 
#include <sys/mount.h> 
#include <dirent.h>  

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/time.h> 
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h> 
#include <sys/statfs.h>



typedef char              	WV_S8;
typedef short             	WV_S16;
typedef int               	WV_S32;
typedef long long 		  	WV_S64;

typedef unsigned char     	WV_U8;
typedef unsigned short    	WV_U16;
typedef unsigned int      	WV_U32;
typedef unsigned long long 	WV_U64; 


#ifndef NULL
#define NULL             	0L
#endif

#define WV_NULL          	0L
#define WV_NULL_PTR      	0L

#define WV_SOK       		 0
#define WV_EFAIL     		(-1)

#endif 
