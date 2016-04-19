/*
 * Author: Yunpeng Song <turtle_syp@163.com>
 * Copyright (c) 2016 Maker Collider Corporation.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include<sys/time.h>
#include<sys/types.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>

#include "mi_joystick.h"
typedef struct global_setting_s
{
	int hidfd;
	int is_running;
	mi_key_status_changed pfnkey_status_change;
	unsigned char keys[MI_KEY_NUM];/*must be the same size as the */
}g_setting;

/*global settings are all here.*/
static g_setting gstSetting=
	{.hidfd=-1,
	 .is_running=0,
	 .pfnkey_status_change=NULL
	};

/****************************************************************
*purpose:categorize the keys according to the buffer from the HID file.
*input:  b1, the second byte from the HID device file.
*        pstkey, the output key dict. it is always the same length of MI_KEY_NUM
*return: 0 is success.
*****************************************************************/
static void parser_key_b1(unsigned char b1,unsigned char *keys)
{
	unsigned char code[6]={0x01,0x02,0x08,0x10,0x40,0x80};
	enum MI_KEY key[]={MI_KEY_A,
		           MI_KEY_B,
		           MI_KEY_X,
		           MI_KEY_Y,
		           MI_KEY_L1,
		           MI_KEY_R1};
	int i=0;
	int index=0;
	for(i=0;i<6;i++)
	{
		index = key[i];
		if(b1&code[i]){
			keys[index] = 1;
		}else{
			keys[index] = 0;			
		}
	}
}


/****************************************************************
*purpose:categorize the keys according to the buffer from the HID file.
*input:  b2, the 3rd byte from the HID device file.
*        pstkey, the output key dict. it is always the same length of MI_KEY_NUM
*return: 0 is success.
*****************************************************************/
static void parser_key_b2(unsigned char b2,unsigned char* keys)
{
	unsigned char code[]={0x04,0x08};
	enum MI_KEY key[]={MI_KEY_BACK,MI_KEY_MENU};
	int i=0;
	int index=0;
	/*if b2==0, all keys set to 0*/
	for(i=0;i<2;i++)
	{
		index = key[i];
		if(b2&code[i]){
			keys[index] = 1;
		}else{
			keys[index] = 0;			
		}
	}
}

/****************************************************************
*purpose:categorize the keys according to the buffer from the HID file.
*input:  b4, the 5th byte from the HID device file.
*        pstkey, the output key dict. it is always the same length of MI_KEY_NUM
*return: 0 is success.
*****************************************************************/
static void parser_key_b4(unsigned char b4,unsigned char *keys)
{
	unsigned char code[]={0x00,0x02,0x04,0x06};
	enum MI_KEY key[]={MI_KEY_UP,MI_KEY_RIGHT,MI_KEY_DOWN,MI_KEY_LEFT};
	int i=0;
	int index=0;
	for(i=0;i<4;i++)
	{
		index = key[i];
		if(code[i]==b4){
			keys[index] = 1;
			break;
		}else{
			keys[index] = 0;
		}
	}
}

/****************************************************************
*purpose:categorize the keys according to the buffer from the HID file.
*input:  b4, the 5th byte from the HID device file.
*        pstkey, the output key dict. it is always the same length of MI_KEY_NUM
*return: 0 is success.
*****************************************************************/
static void parser_key_b5(unsigned char b5,unsigned char *keys)
{	
	keys[MI_KEY_JOYA_H]=b5;
}
/****************************************************************
*purpose:categorize the keys according to the buffer from the HID file.
*input:  b4, the 5th byte from the HID device file.
*        pstkey, the output key dict. it is always the same length of MI_KEY_NUM
*return: 0 is success.
*****************************************************************/
static void parser_key_b6(unsigned char b6,unsigned char *keys)
{	
	keys[MI_KEY_JOYA_V]=b6;
}
/****************************************************************
*purpose:categorize the keys according to the buffer from the HID file.
*input:  b4, the 5th byte from the HID device file.
*        pstkey, the output key dict. it is always the same length of MI_KEY_NUM
*return: 0 is success.
*****************************************************************/
static void parser_key_b7(unsigned char b7,unsigned char *keys)
{	
	keys[MI_KEY_JOYB_H]=b7;
}
/****************************************************************
*purpose:categorize the keys according to the buffer from the HID file.
*input:  b4, the 5th byte from the HID device file.
*        pstkey, the output key dict. it is always the same length of MI_KEY_NUM
*return: 0 is success.
*****************************************************************/
static void parser_key_b8(unsigned char b8,unsigned char *keys)
{	
	keys[MI_KEY_JOYB_V]=b8;
}

/****************************************************************
*purpose:categorize the keys according to the buffer from the HID file.
*input:  b4, the 5th byte from the HID device file.
*        pstkey, the output key dict. it is always the same length of MI_KEY_NUM
*return: 0 is success.
*****************************************************************/
static void parser_key_b11(unsigned char b11,unsigned char *keys)
{	
	keys[MI_KEY_L2]=b11;
}

/****************************************************************
*purpose:categorize the keys according to the buffer from the HID file.
*input:  b4, the 5th byte from the HID device file.
*        pstkey, the output key dict. it is always the same length of MI_KEY_NUM
*return: 0 is success.
*****************************************************************/
static void parser_key_b12(unsigned char b12,unsigned char *keys)
{	
	keys[MI_KEY_R2]=b12;
}

/****************************************************************
*purpose:categorize the keys according to the buffer from the HID file.
*input:  buffer, the unsigned char line from the HID device file.
*        pstkeylist, the final output key list. it is always the same length of MI_KEY_NUM
*        len, the length of the output pstkeylist.
*return: 0 is success.
*****************************************************************/
static int parser_keys(unsigned char* buffer,unsigned char *keys)
{
	if(buffer==NULL || keys==NULL){
		return -1;
	}
	
	parser_key_b1(buffer[1],keys);
	parser_key_b2(buffer[2],keys);
	parser_key_b4(buffer[4],keys);
	parser_key_b5(buffer[5],keys);
	parser_key_b6(buffer[6],keys);
	parser_key_b7(buffer[7],keys);
	parser_key_b8(buffer[8],keys);
	parser_key_b11(buffer[11],keys);
	parser_key_b12(buffer[12],keys);
	return 0;	
}
/****************************************************************
*purpose:block the process until mi_joystick has input.
*input:  key_buffer,a buffer used to read all keys' status & values
*        len, the size of the input key_buffer. MUST BE BIGGER THAN MI_KEY_NUM
*return: 0 is success. -1 fail
*****************************************************************/
int mi_wait_joystick_input()
{
	fd_set hidset;
	unsigned char buffer[24];
	g_setting* pstSetting = &gstSetting;
	int ret=0;
	if(!pstSetting->is_running){
		printf("[%s,%d]mi joystick is not initialized.\n",__FILE__,__LINE__);
		return -1;
	}
	
	do{
		/*prepare the "select" operation*/
		FD_ZERO(&hidset);
	  	FD_SET(pstSetting->hidfd,&hidset);
		select(pstSetting->hidfd+1,&hidset,NULL,NULL,NULL);
		lseek(pstSetting->hidfd,SEEK_SET,0);
		
		/*read the key pressed buffer*/
		memset(buffer,0,sizeof(buffer));
		if(read(pstSetting->hidfd,buffer,sizeof(buffer))<=0){
			printf("[%s,%d] bluetooth connection lost.\n",__FILE__,__LINE__);
			mi_close();
			ret = -1;
			break;
		}
		
		if(parser_keys(buffer,pstSetting->keys)==0 && pstSetting->pfnkey_status_change){
			pstSetting->pfnkey_status_change(pstSetting->keys,MI_KEY_NUM);
		}
	}while(0);
	return ret;
}

int mi_init(mi_key_status_changed pfncallback)
{
	int fd=-1;
	
	/*check if it is running*/
	if(gstSetting.is_running){
		printf("[%s,%d]mi joy thread is running. fail to regist.\n",__FILE__,__LINE__);
		goto error;
	}

	/*open the hid path.*/
	fd = open(MI_HIDPATH,O_RDONLY);
	if(fd<0){
		printf("[%s,%d]fail to open the %s.\n",
			  __FILE__,__LINE__,
			  MI_HIDPATH);
		goto error;
	}
	
	/*init global setting*/
	gstSetting.hidfd = fd;
	gstSetting.is_running=1;
	gstSetting.pfnkey_status_change=pfncallback;
	memset(gstSetting.keys,0, MI_KEY_NUM*sizeof(unsigned char));

	return 0;
error:
	mi_close();
	return -1;
}
void mi_close()
{
	if(gstSetting.hidfd>0){
		close(gstSetting.hidfd);
	}
	
	/*clear the settings*/
	gstSetting.hidfd=-1;
	gstSetting.is_running=0;
	memset(gstSetting.keys,0, MI_KEY_NUM*sizeof(unsigned char));
}

#ifdef __cplusplus
}
#endif

