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
#ifndef CONFIG_H
#define CONFIG_H

//servo related
#define MIN_PULSE_WIDTH         544         // the shortest pulse sent to a servo in us
#define MAX_PULSE_WIDTH         2400        // the longest pulse sent to a servo in us
#define DEFAULT_PULSE_WIDTH     1500        // default pulse width when servo is attached
#define PWM_50Hz                19000000    // in ns

#define SERVO_PIN               11          // SERVO pwm pin
#define SERVO_MIN               15          // Minimum servo angle
#define SERVO_DEFAULT           28
#define SERVO_MAX               45          // Maximum sero angle
#define SERVO_RANGE             120

#define MDRIVE_PWM_PIN          8
#define MDRIVE_DIR_PINA         12
#define MDRIVE_DIR_PINB         13
#define MDRIVE_PWM_MIN          50          // minimum driver-motor pwm percentage(max is 100)

#define MROTATE_PWM_PIN         9
#define MROTATE_DIR_PINA        14
#define MROTATE_DIR_PINB        15
#define MROTATE_ENCODER_PINA    3
#define MROTATE_ENCODER_PINB    4
#define MROTATE_PWM_MIN         10          // minimum rotation-motor pwm percentage(max is 100)
#define MROTATE_COUNT_PERROUND  680         // how many ISR per round

#endif /*CONFIG_H*/
#ifdef __cplusplus
}
#endif

