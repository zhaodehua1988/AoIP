#include "wv_common.h"
#include "wv_cmd.h"
#include "wv_tty.h"
#include "his_sys.h"
#include "his_spi.h"
#include "his_fb.h"
#include "adi_7619.h"
#include "fpga_conf.h"
#include <sys/time.h>
#include "wv_file.h"
#include "sys_env.h"
#include "wv_file.h"
#include "iTE6805_Init.h"
#include "PCA9548A.h"
#include "PCA9555.h"
//#include "iTE6615_Init.h"
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
	HIS_SYS_Init();
	SYS_IP_Init();
	PCA9548_Init();
	PCA9555_Init();
	//ADV_7619_Init();
	//FPGA_Init();
	ITE6805_Init();
	//iTE6615_Init();
	rowNum = 0;
	while (1)
	{
		ret = WV_CMD_GetExit();
		if (ret == 1)
			break;
		usleep(100000);
	}
	PCA9555_DeInit();
	PCA9548_DeInit();
	//FPGA_DeInit();
	SYS_IP_DeInit();
	HIS_SYS_DeInit();
	WV_TTY_Destroy();
	SYS_ENV_Close();
	WV_CMD_Destroy();

	printf("********    bye     ********\n");
	return 0;
}
/**/
