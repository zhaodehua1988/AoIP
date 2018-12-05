#include "wv_sqlite3.h"

//C语言实现sqlite3数据库查询的基本方法:callback，gettable和预处理stmt方法
#define WV_SQLITE3_DEBUG_MODE 
#ifdef   WV_SQLITE3_DEBUG_MODE  
#define Sqlite3_printf(...) \
    do {\
    printf("\n\r[%s][%s]-%d:",__FILE__,__FUNCTION__, __LINE__);\
    printf(__VA_ARGS__);\
    fflush(stdout); \
    } while(0)
#else 
#define Sqlite3_printf(...)
#endif  

/******************为SQLite3提供一个ANSI到UTF8的互转函数***********************/
#include<iconv.h> 

/******************************************************************************

int utf8togb2312(const char *sourcebuf,size_t sourcelen,char *destbuf,size_t destlen) ;

******************************************************************************/  
int utf8togb2312(const char *sourcebuf,size_t sourcelen,char *destbuf,size_t destlen) 
{ 

    Sqlite3_printf("change  utf-8 to gb2312 ..,sourceLen=%d,destLen=%d\n",sourcelen,destlen);
    iconv_t cd;

    //if( (cd = iconv_open("gb2312","utf-8")) ==0 )
    if( (cd = iconv_open("gbk","utf-8")) ==0 )
    {
        Sqlite3_printf("iconv_open(gbk,utf-8) error[%d]",errno);
        return -1;
    }
    memset(destbuf,0,destlen);

    const char **source = &sourcebuf;

    char **dest = &destbuf;
    int i;
    if(-1 == iconv(cd,source,&sourcelen,dest,&destlen))
    {
        Sqlite3_printf("change utf-8 to gb2312 error[%d]\n",errno);
        return -1;
    }

    iconv_close(cd);

    return 0;

}
/******************************************************************************

int gb2312toutf8(const char *sourcebuf,size_t sourcelen,char *destbuf,size_t destlen);

******************************************************************************/ 
int gb2312toutf8(const char *sourcebuf,size_t sourcelen,char *destbuf,size_t destlen)
{
    Sqlite3_printf("change   gb2312 to utf-8  ..,sourceLen=%d,destLen=%d\n",sourcelen,destlen);

    iconv_t cd;
    if( (cd = iconv_open("utf-8","gbk")) ==0 )
    {
        printf("in gb2312toutf8 open error!\n");
        return -1;
    }
    memset(destbuf,0,destlen);

    const char **source = &sourcebuf;

    char **dest = &destbuf;
    if(-1 == iconv(cd,source,&sourcelen,dest,&destlen))
    {
        printf("iconv error!\n");
        return -1;
    }
    iconv_close(cd);

    return 0;
}

//判断字符串是否是utf8

#define CHECK_LENGTH 20       //检查是否为utf8编码时所检查的字符长度
int is_utf8_string(char *utf)
{
    int length = strlen(utf);
    int check_sub = 0;
    int i = 0;

    if ( length > CHECK_LENGTH )  //只取前面特定长度的字符来验证即可
    {
        length = CHECK_LENGTH;
    }

    for (i = 0 ; i < length; i ++ )
    {
        if ( check_sub == 0 )
        {
            if ( (utf[i] >> 7) == 0 )         //0xxx xxxx
            {
                continue;
            }
            else if ( (utf[i] & 0xE0) == 0xC0 ) //110x xxxx
            {
                check_sub = 1;
            }
            else if ( (utf[i] & 0xF0) == 0xE0 ) //1110 xxxx
            {
                check_sub = 2;
            }
            else if ( (utf[i] & 0xF8) == 0xF0 ) //1111 0xxx
            {
                check_sub = 3;
            }
            else if ( (utf[i] & 0xFC) == 0xF8 ) //1111 10xx
            {
                check_sub = 4;
            }
            else if ( (utf[i] & 0xFE) == 0xFC ) //1111 110x
            {
                check_sub = 5;
            }
            else
            {
                return 0;
            }
        }
        else
        {
            if ( (utf[i] & 0xC0) != 0x80 )
            {
                return 0;
            }
            check_sub --;
        }
    }
    return 1;
}
/***********************************************************************/

//  
//typedef int (*sqlite3_callback)(   
//    void* data,       /* Data provided in the 4th argument of sqlite3_exec() */   
//    int ncols,        /* The number of columns in row */   
//    char** values,    /* An array of strings representing fields in the row */   
//    char** headers    /* An array of strings representing column names */   
//);   

/******************************************************************************

WV_S32 WV_SQLITE_Temp_callback(void* data, int ncols, char** values, char** headers) ;

******************************************************************************/  
WV_S32 WV_SQLITE_Temp_callback(void* data, int ncols, char** values, char** headers)  
{  
    int i;
    int len =0;
    int ll=0;
    for(i=0; i < ncols; i++)
    {
        if(strlen(headers[i])>len)
            len = strlen(headers[i]);
    }

    for(i=0; i < ncols; i++)
    {
        ll = len-strlen(headers[i]);
        while(ll)
        {
            Sqlite3_printf(" \n");
            --ll;
        }
        Sqlite3_printf("[%d]%s: %s\n",i, headers[i], values[i]);
        break;
    }
    Sqlite3_printf("\n");
    return 0;
}  

/******************************************************************************

WV_S32 WV_SQLITE3_SearchBycallback(WV_S8 *db_name,WV_S8 *sql_cmd,WV_SQLITE_callback callback);
//方法1：callback
******************************************************************************/
WV_S32 WV_SQLITE3_SearchBycallback(WV_S8 *db_name,WV_S8 *sql_cmd,WV_SQLITE_callback callback)
{

    int i = 0 ;
    int j = 0 ;
    int nrow = 0, ncolumn = 0;
    char **azResult; //二维数组存放结果
    sqlite3 *db=NULL;
    char *zErrMsg = 0;
    int rc;
    int len=0;

    if(access(db_name, 0) == -1)
    {
        Sqlite3_printf("%s not found\n", db_name);
        return -1;
    }

    rc = sqlite3_open(db_name, &db);

    if( rc != SQLITE_OK)
    {
        Sqlite3_printf("%s open failed: %s\n", db_name,sqlite3_errmsg(db));
        sqlite3_close(db);
        return -1;
    }

    //查询数据
    rc = sqlite3_exec( db,sql_cmd, callback, NULL, &zErrMsg );
    if( rc != SQLITE_OK)
    {
        Sqlite3_printf("%s %s: %s\n", db_name,sql_cmd, sqlite3_errmsg(db));
        if(zErrMsg)
        {
            Sqlite3_printf("ErrMsg = %s \n", zErrMsg);
            sqlite3_free(zErrMsg);
        }
        sqlite3_close(db);
        return -1;
    }

    if(zErrMsg)
    {
        sqlite3_free(zErrMsg);
    }

    //关闭数据库
    sqlite3_close(db);
    return 0;

}

/******************************************************************************

WV_S32 WV_SQLITE3_SearchByTable(WV_S8 *db_name,WV_S8 *sql_cmd);
//方法2：gettable
******************************************************************************/  
WV_S32 WV_SQLITE3_SearchByTable(WV_S8* db_name, WV_S8 *sql_cmd)   
{    
    int i = 0 ;
    int j = 0 ;
    int nrow = 0, ncolumn = 0;
    char **azResult; //二维数组存放结果
    sqlite3 *db=NULL;
    char *zErrMsg = 0;
    int rc;
    int len=0;

    if(access(db_name, 0) == -1)
    {
        fprintf(stderr, "%s not found\n", db_name);
        return -1;
    }

    rc = sqlite3_open(db_name, &db);

    if( rc != SQLITE_OK)
    {
        fprintf(stderr, "%s open failed: %s\n", db_name,sqlite3_errmsg(db));
        sqlite3_close(db);
        return -1;
    }

    //查询数据
    rc = sqlite3_get_table( db , sql_cmd, &azResult , &nrow , &ncolumn , &zErrMsg );
    if( rc != SQLITE_OK)
    {
        fprintf(stderr, "%s %s: %s\n", db_name,sql_cmd, sqlite3_errmsg(db));
        if(zErrMsg)
            fprintf(stderr,"ErrMsg = %s \n", zErrMsg);
        sqlite3_free_table( azResult );
        sqlite3_close(db);
        return -1;
    }

    for(j=0; j < ncolumn; j++)
    {
        if(strlen(azResult[j])>len)
            len = strlen(azResult[j]);
    }

    //从第0索引到第 nColumn - 1索引都是字段名称
    //从第 nColumn 索引开始，后面都是字段值
    for( i = 0 ; i < nrow; i++ )
    {
        for(j=0; j < ncolumn; j++)
        {
            int ll = (len- strlen(azResult[j]));
            while(ll)
            {
                printf(" ");
                --ll;
            }
            printf( "%s: %s\n", azResult[j], azResult[(i+1)*ncolumn+j]);
        }
        printf("\n");
    }

    //与sqlite3_get_table对应，释放掉  azResult 的内存空间
    sqlite3_free_table( azResult );
    //关闭数据库
    sqlite3_close(db);
    return 0;

}   

/******************************************************************************

WV_S32 WV_SQLITE3_SearchByStmt(WV_S8 *db_name,WV_S8 *sql_cmd);
//方法3：预处理stmt方法
******************************************************************************/
WV_S32 WV_SQLITE3_SearchByStmt(WV_S8 *db_name,WV_S8 *sql_cmd)
{  
    sqlite3 *db=NULL;
    sqlite3_stmt* stmt = 0;
    int ncolumn = 0;
    const char *column_name;
    int vtype , i;
    int rc;

    if(access(db_name, 0) == -1)
    {
        Sqlite3_printf("%s not found\n", db_name);
        return -1;
    }

    rc = sqlite3_open(db_name, &db);

    if( rc != SQLITE_OK)
    {
        Sqlite3_printf("%s open failed: %s\n", db_name,sqlite3_errmsg(db));
        sqlite3_close(db);
        return -1;
    }

    //查询数据
    rc = sqlite3_prepare_v2(db, sql_cmd, -1, &stmt, 0);
    if (rc != SQLITE_OK)
    {
        Sqlite3_printf("%s %s: %s\n", db_name,sql_cmd, sqlite3_errmsg(db));
        sqlite3_close(db);
        return -1;
    }

    ncolumn = sqlite3_column_count(stmt);

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        for(i = 0 ; i < ncolumn ; i++ )
        {
            vtype = sqlite3_column_type(stmt , i);
            column_name = sqlite3_column_name(stmt , i);
            switch( vtype )
            {
            case SQLITE_NULL:
                Sqlite3_printf( "%s: null\n", column_name);
                break;
            case SQLITE_INTEGER:
                Sqlite3_printf("%s: %d\n", column_name,sqlite3_column_int(stmt,i));
                break;
            case SQLITE_FLOAT:
                Sqlite3_printf( "%s: %f\n", column_name,sqlite3_column_double(stmt,i));
                break;
            case SQLITE_BLOB: /* arguably fall through... */
                Sqlite3_printf("%s: BLOB\n", column_name);
                break;
            case SQLITE_TEXT:
                Sqlite3_printf("%s: %s\n", column_name,sqlite3_column_text(stmt,i));
                break;
            default:
                Sqlite3_printf("%s: ERROR [%s]\n", column_name, sqlite3_errmsg(db));
                break;
            }
        }
    }

    sqlite3_finalize(stmt);

    //关闭数据库
    sqlite3_close(db);
    return 0;

}  


/****************************************************************************

WV_S32 WV_SQLITE_SearchVal(WV_S8 *db_name,WV_S8 *sql_cmd,WV_SQLITE_callback callback)

****************************************************************************/
WV_S32 WV_SQLITE_SearchVal(WV_S8 *db_name,WV_S8 *sql_cmd,WV_SQLITE_callback callback)
{

    return WV_SQLITE3_SearchBycallback(db_name,sql_cmd,callback);
}

/****************************************************************************

WV_S32 WV_SQLITE_GetTable(WV_S32 argc, WV_S8 **argv,WV_S8 *prfBuff)

****************************************************************************/
WV_S32 WV_SQLITE_GetTable(WV_S32 argc, WV_S8 **argv, WV_S8 * prfBuff)
{

    WV_S8 sql[128];
    memset(sql,0,sizeof(sql));

    if(argc <1 )
    {
        prfBuff += sprintf(prfBuff,"sqlite get <SongID> \r\n");
        return WV_SOK;
    }


    sprintf(sql,"select Class from SongClass where ID = '%s' ",argv[0]);
    Sqlite3_printf("\n%s \n",sql);
    //WV_SQLITE3_SearchByStmt("test.db",sql);
    //Sqlite3_printf("------------------------------------------------\n");
    WV_SQLITE3_SearchBycallback("test.db",sql,WV_SQLITE_Temp_callback);
    return WV_SOK;
}

/******************************************************************************
WV_S32 WV_SQLITE3_Init()
******************************************************************************/
WV_S32 WV_SQLITE3_Init()
{

    WV_CMD_Register("sqlite",NULL,"sqlite comand ",NULL);
    WV_CMD_Register("sqlite","get","sqlite get val",WV_SQLITE_GetTable);

}
