#include "wv_common.h"
#include "wv_cmd.h"
#include "wv_tty.h"
#include "wv_telnet.h"
#include "his_sys.h"
#include "his_spi.h"
#include "his_fb.h"
#include "adi_7619.h"
#include "tsk_go.h"
#include <sys/time.h>
#include "wv_file.h"
#include "sys_env.h"
#include "wv_file.h"
#include "PCA9548A.h"
#include "PCA9555.h"
#include "iTE6805_Init.h"
#include "iTE6615_Init.h"
#include "ti1297_init.h"
#include "fpga_conf.h"
#include "sys_ip.h"
#include "tsk_player.h"
#define HELLO_DEBUG

WV_S32 main()
{
	WV_S32 ret;
	WV_U32 rowNum;
	printf("********hello world ********\n");
	//打印版本信息
	printf("The version number of the program is %s\n", SYS_ENV_VERSION_NO); //版本号的宏定义在sys_env.h里
																			 //WV_FILE_PRINTF_VERSION();                  //这里打印的版本信息，是读取conf64.ini里
	WV_CMD_Create();
	SYS_ENV_Open();
	WV_TTY_Create();
	WV_TEL_Open();
	HIS_SYS_Init();
	SYS_IP_Init();

#ifdef HELLO_DEBUG
	PCA9548_Init();
	PCA9555_Init();
	ADV_7619_Init();
	FPGA_CONF_Init();

	TSK_PLAYER_Open();

	ITE6805_Open();
	ITE6615_Open();

	//TI1297_Init();
	TSK_GO_Open();
#endif
	//system("./MultiViewer > /dev/null");
	rowNum = 0;
	while (1)
	{
		ret = WV_CMD_GetExit();
		if (ret == 1)
			break;
		usleep(100000);
	}
#ifdef HELLO_DEBUG
	TSK_GO_Close();
	ITE6615_Close();
	ITE6805_Close();
	TSK_PLAYER_Close();
	FPGA_CONF_DeInit();
#endif

	SYS_IP_DeInit();
	HIS_SYS_DeInit();
	WV_TEL_Close();
	WV_TTY_Destroy();
	SYS_ENV_Close();
	WV_CMD_Destroy();

	printf("********    bye     ********\n");
	return 0;
}
/**/
