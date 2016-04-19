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
#ifndef SPIDER_H
#define SPIDER_H

int spider_init();
void spider_close();

/*********************************************************
*purpose: move the spider's drive motor.
*input:is_foward, 1 is foward, 0 backward
*      pwm, 100 is full, 0 is stop
**********************************************************/
void spider_move(int is_foward,int pwm);
void spider_rotate(int is_clockwise,int pwm);
void spider_move_stop();
void spider_rotate_stop();
void spider_rotate_brake();

/*********************************************************
*rotates the spider x degrees relatively.
*degree, rotates x degree relatively. [-360,360]
*pwm, 100 is full, 0 is stop
**********************************************************/
void spider_rotate_degree(int degree,int pwm,void (*pfun)(void *),void * argv);

/*********************************************************
*purpose: move the spider's head.
*input:degree, [SERVO_MIN,SERVO_MAX]
**********************************************************/
void spider_head(int degree);

#endif //SPIDER_H
#ifdef __cplusplus
}
#endif
