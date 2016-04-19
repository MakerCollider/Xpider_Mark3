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

#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#ifndef MI_JOSTICK_H
#define MI_JOSTICK_H

#define MI_HIDPATH "/dev/hidraw0"
#define MI_BYTES 16
/*defination of joy keys*/
enum MI_KEY
{
	MI_KEY_INVALID=-1,
	MI_KEY_L1=0,
	MI_KEY_L2=1,
	MI_KEY_R1=2,
	MI_KEY_R2,
	MI_KEY_A,
	MI_KEY_B,
	MI_KEY_X,
	MI_KEY_Y,
	MI_KEY_BACK,
	MI_KEY_MENU,
	MI_KEY_JOYA_H,
	MI_KEY_JOYA_V,
	MI_KEY_JOYB_H,
	MI_KEY_JOYB_V,
	MI_KEY_LEFT,
	MI_KEY_RIGHT,
	MI_KEY_UP,
	MI_KEY_DOWN,
	MI_KEY_NUM
};

typedef void (*mi_key_status_changed)(unsigned char* keys,int len);
/****************************************************************
*purpose:register a callback function to the mi_daemon.
*return: 0 is success.
*****************************************************************/
int mi_init(mi_key_status_changed pfncallback);
/****************************************************************
*purpose:block the process until mi_joystick has input.
*return: 0 is success.
*****************************************************************/
int mi_wait_joystick_input();
/****************************************************************
*purpose:close the mi joystick daemon
*****************************************************************/
void mi_close();

#endif //MI_JOSTICK_H

#ifdef __cplusplus
}
#endif

