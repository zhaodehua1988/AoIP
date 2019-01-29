#include "cJSON.h"
#include "fpga_conf_json.h"
#include "fpga_conf.h"
#include "fpga_sdp.h"
#if 0
int analysisJson(char *json)
{
	char out[4096*4]={0};
	FILE *	fp=NULL;
	fp=fopen(json,"r");
	if(fp == NULL){
		printf("analysis json file %s err\n",json);	
	}
	fread(out,1,4096*4,fp);
	fclose(fp);

	FPGA_CONF_WIN gWin[16]={0};	
    FPGA_SDP_Info gSdp[16]={0};
	cJSON * root,*enaJson,*winJson;
	WV_S32 ena = 0;
    //printf("%s\n",out);
	root = cJSON_Parse(out);
	int i=0,j=0;
	if(root){
		//enaJson = cJSON_GetObjectItem( root, "ena");
		//ena = enaJson->valueint;
		winJson = cJSON_GetObjectItem( root, "win");
		if(winJson != NULL){
			cJSON *client_list=winJson->child;
			while(client_list != NULL){

                ena |= cJSON_GetObjectItem( client_list, "winEna")->valueint << i;
				gWin[i].x = cJSON_GetObjectItem( client_list , "x")->valueint;
				gWin[i].y = cJSON_GetObjectItem( client_list , "y")->valueint;
				gWin[i].w = cJSON_GetObjectItem( client_list , "w")->valueint;
				gWin[i].h = cJSON_GetObjectItem( client_list , "h")->valueint;
				gWin[i].video_type = cJSON_GetObjectItem( client_list , "videoType")->valueint;
				gWin[i].video_ip = cJSON_GetObjectItem( client_list , "videoIp")->valueint;
				gWin[i].audio_ip = cJSON_GetObjectItem( client_list , "audioIp")->valueint;
				gWin[i].sel_in = cJSON_GetObjectItem( client_list , "selIn")->valueint;
                sprintf(gSdp[i].video_colorimetry,"%s",cJSON_GetObjectItem( client_list , "videoColorimetry")->string);
                gSdp[i].video_interlace = cJSON_GetObjectItem( client_list , "videoInterlace")->valueint;
                gSdp[i].video_framerate = cJSON_GetObjectItem( client_list , "videoFramerate")->valueint;
                gSdp[i].video_depth = cJSON_GetObjectItem( client_list , "videoDepth")->valueint;
                sprintf(gSdp[i].video_sampling,"%s",cJSON_GetObjectItem( client_list , "videoSampling")->string);
                gSdp[i].video_width = cJSON_GetObjectItem( client_list , "videoWidth")->valueint;
                gSdp[i].video_height = cJSON_GetObjectItem( client_list , "videoHeight")->valueint;
                gSdp[i].video_pt = cJSON_GetObjectItem( client_list , "videoPt")->valueint;
                gSdp[i].audio_pt = cJSON_GetObjectItem( client_list , "audioPt")->valueint;
                gSdp[i].audio_depth = cJSON_GetObjectItem( client_list , "audioDepth")->valueint;

                cJSON * audioChl=cJSON_GetObjectItem( client_list , "audioChannel");
                if(audioChl != NULL){
                    int audio_chl_size = cJSON_GetArraySize(audioChl);
                    for(j=0;j<audio_chl_size;j++){
                        gSdp[i].audio_chl[j]   = cJSON_GetArrayItem(audioChl, j)->valueint;
                    }
                }

				client_list = client_list->next;
				i++;
				if(i == 16) {
					break;
				}

			}

		}
		
	}
    
	printf("winEna = 0x%X\n",ena);
	for(i=0;i<16;i++){
		printf("x=%d,y=%d,w=%d,h=%d,video_type=%d,video_ip=%d,audio_ip=%d,sel_in=%d\n",\
		gWin[i].x,gWin[i].y,gWin[i].w,gWin[i].h,gWin[i].video_type,gWin[i].video_ip,gWin[i].audio_ip,gWin[i].sel_in);
        printf("videoColorimetry=%s,videoInterlace=%d,videoFramerate=%d,videoDepth=%d,videoSampling=%s,videoWidth=%d,videoHeight=%d,videoPt=%d,audioPt=%d,audioDepth=%d,audioChannel=[",\
        gSdp[i].video_colorimetry,gSdp[i].video_interlace,gSdp[i].video_framerate,gSdp[i].video_depth,gSdp[i].video_sampling,gSdp[i].video_width,gSdp[i].video_height,gSdp[i].video_pt,gSdp[i].audio_pt,gSdp[i].audio_depth);
        for(j=0;j<strlen(gSdp[i].audio_chl);j++){
            printf("%d ",gSdp[i].audio_chl[j]);
        }
        printf("]\n");

	}	

}
#endif
/*****************************************************************
 * WV_S32 FPGA_CONF_WinAnalysisJson(char *json,FPGA_CONF_WIN_T *gWin)
 * ****************************************************************/
WV_S32 FPGA_CONF_WinAnalysisJson(char *json,FPGA_CONF_WIN_T *gWin)
{
	printf("analysisJson [%s]\n",json);
#if 1
	char out[4096*4]={0};
	FILE *	fp=NULL;
	fp=fopen(json,"r");
	if(fp == NULL){
		printf("analysis json file %s err\n",json);	
		fclose(fp);
		return WV_EFAIL;
	}
	fread(out,1,4096*4,fp);
	fclose(fp);


	cJSON * root,*dataJson,*winJson;
	WV_S32 ena = 0;
    //printf("%s\n",out);
	root = cJSON_Parse(out);
	int i=0;

	if(root){
		//enaJson = cJSON_GetObjectItem( root, "ena");
		//ena = enaJson->valueint;
		//printf("success=%d\n",cJSON_GetObjectItem( root, "success")->valueint);
		dataJson = cJSON_GetObjectItem( root, "data");
		winJson = cJSON_GetObjectItem( dataJson, "win");
		if(winJson != NULL){

			cJSON *client_list=winJson->child;
			while(client_list != NULL){

                ena |= cJSON_GetObjectItem( client_list, "winEna")->valueint << i;
				gWin[i].win_ena = cJSON_GetObjectItem( client_list , "winEna")->valueint;
				gWin[i].x = cJSON_GetObjectItem( client_list , "x")->valueint;
				gWin[i].y = cJSON_GetObjectItem( client_list , "y")->valueint;
				gWin[i].w = cJSON_GetObjectItem( client_list , "w")->valueint;
				gWin[i].h = cJSON_GetObjectItem( client_list , "h")->valueint;
	
				sprintf(gWin[i].type,"%s",cJSON_GetObjectItem( client_list , "type")->valuestring);
				//printf("win[%d]type=%s",i,gWin[i].type);
				gWin[i].channel = cJSON_GetObjectItem( client_list , "channel")->valueint;
				
				gWin[i].video_ipv6_ena = cJSON_GetObjectItem( client_list , "videoIpv6Ena")->valueint;

				sprintf(gWin[i].video_ip,"%s", cJSON_GetObjectItem( client_list , "videoIp")->valuestring);

				gWin[i].video_port = cJSON_GetObjectItem( client_list , "videoPort")->valueint;
				gWin[i].audio_ipv6_ena = cJSON_GetObjectItem( client_list , "audioIpv6Ena")->valueint;
				sprintf(gWin[i].audio_ip,"%s", cJSON_GetObjectItem( client_list , "audioIp")->valuestring);
				gWin[i].audio_port = cJSON_GetObjectItem( client_list , "audioPort")->valueint;

                sprintf(gWin[i].sdpInfo.video_colorimetry,"%s",cJSON_GetObjectItem( client_list , "videoColorimetry")->valuestring);
                gWin[i].sdpInfo.video_interlace = cJSON_GetObjectItem( client_list , "videoInterlace")->valueint;
                
				sprintf(gWin[i].sdpInfo.video_framerate ,"%s",cJSON_GetObjectItem( client_list , "videoFramerate")->valuestring);
                gWin[i].sdpInfo.video_depth = cJSON_GetObjectItem( client_list , "videoDepth")->valueint;
                sprintf(gWin[i].sdpInfo.video_sampling,"%s",cJSON_GetObjectItem( client_list , "videoSampling")->valuestring);
                gWin[i].sdpInfo.video_width = cJSON_GetObjectItem( client_list , "videoWidth")->valueint;
                gWin[i].sdpInfo.video_height = cJSON_GetObjectItem( client_list , "videoHeight")->valueint;
                gWin[i].sdpInfo.video_pt = cJSON_GetObjectItem( client_list , "videoPt")->valueint;
                gWin[i].sdpInfo.audio_pt = cJSON_GetObjectItem( client_list , "audioPt")->valueint;
                gWin[i].sdpInfo.audio_depth = cJSON_GetObjectItem( client_list , "audioDepth")->valueint;
				sprintf(gWin[i].sdpInfo.audio_chl,"%s",cJSON_GetObjectItem( client_list , "audioChl")->valuestring);

				client_list = client_list->next;
				i++;
				if(i == 16) {
					break;
				}

			}

		}
		
	}
#endif
#if 0
	printf("winEna = 0x%X\n",ena);
	for(i=0;i<16;i++){
		printf("x=%d,y=%d,w=%d,h=%d,type=%s,channel=%d,video_ip=%s,video_port=%d,audio_ip=%s,audio_port=%d,,video_ipv6_ena=%d\n",\
		gWin[i].x,gWin[i].y,gWin[i].w,gWin[i].h,gWin[i].type,gWin[i].channel,gWin[i].video_ip,gWin[i].video_port,gWin[i].audio_ip,gWin[i].audio_port,gWin[i].video_ipv6_ena);
        printf("videoColorimetry=%s,videoInterlace=%d,videoFramerate=%d,videoDepth=%d,videoSampling=%s,videoWidth=%d,videoHeight=%d,videoPt=%d,audioPt=%d,audioDepth=%d,audioChannel=[%s]",\
        gWin[i].sdpInfo.video_colorimetry,gWin[i].sdpInfo.video_interlace,gWin[i].sdpInfo.video_framerate,gWin[i].sdpInfo.video_depth,gWin[i].sdpInfo.video_sampling,gWin[i].sdpInfo.video_width,gWin[i].sdpInfo.video_height,gWin[i].sdpInfo.video_pt,gWin[i].sdpInfo.audio_pt,gWin[i].sdpInfo.audio_depth,gWin[i].sdpInfo.audio_chl);
        printf("-------------------------------------------\n");

	}	
#endif
	return WV_SOK;
}

/*****************************************************************
 * WV_S32 FPGA_CONF_EthAnalysisJson(char *json,FPGA_CONF_WIN_T *gWin)
 * ****************************************************************/
WV_S32 FPGA_CONF_EthAnalysisJson(char *json,FPGA_CONF_ETH_T *gEth)
{	
	printf("eth analysis json\n");
	char out[4096*4]={0};
	FILE *	fp=NULL;
	fp=fopen(json,"r");
	if(fp == NULL){
		printf("analysis json file %s err\n",json);
		fclose(fp);
		return WV_EFAIL;	
	}
	fread(out,1,4096*4,fp);
	fclose(fp);

	//FPGA_CONF_WIN_T gWin[16]={0};	
    //FPGA_SDP_Info gSdp[16]={0};
	cJSON * root,*dataJson,*ethJson;
	WV_S32 ena = 0;
    printf("%s\n",out);
	root = cJSON_Parse(out);
	int i=0;

	if(root){
		//enaJson = cJSON_GetObjectItem( root, "ena");
		//ena = enaJson->valueint;
		printf("success=%d\n",cJSON_GetObjectItem( root, "success")->valueint);
		dataJson = cJSON_GetObjectItem( root, "data");
		ethJson = cJSON_GetObjectItem( dataJson, "eth");
		//printf("11111111111111111111111\n");
		if(ethJson != NULL){

			cJSON *client_list=ethJson->child;
			while(client_list != NULL){

				gEth[i].ipv6Ena = cJSON_GetObjectItem(client_list,"ipv6Ena")->valueint;
				sprintf(gEth[i].ipv6,"%s",cJSON_GetObjectItem(client_list,"ip")->valuestring);
				sprintf(gEth[i].subnet_mask,"%s",cJSON_GetObjectItem(client_list,"netMask")->valuestring);
				sprintf(gEth[i].getway,"%s",cJSON_GetObjectItem(client_list,"getWay")->valuestring);
				sprintf(gEth[i].mac,"%s",cJSON_GetObjectItem(client_list,"mac")->valuestring);
				client_list = client_list->next;
				i++;
				if(i == 16) {
					break;
				}

			}

		}
		
	}
	
	return WV_SOK;
}
