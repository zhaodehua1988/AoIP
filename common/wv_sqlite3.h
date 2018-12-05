#ifndef  _WV_SQLITE3_H_H_
#define  _WV_SQLITE3_H_H_
#include "wv_common.h"  
#include "sqlite3.h"

typedef int (*WV_SQLITE_callback)(void* para,int columnCount,char** columnValue,char** columnName);
WV_S32 WV_SQLITE_SearchVal(WV_S8 *db_name,WV_S8 *sql_cmd,WV_SQLITE_callback callback);

int utf8togb2312(const char *sourcebuf,size_t sourcelen,char *destbuf,size_t destlen);
int gb2312toutf8(const char *sourcebuf,size_t sourcelen,char *destbuf,size_t destlen);
int is_utf8_string(char *utf);
#endif


