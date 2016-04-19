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
 
#include <mraa.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "xpiderctl.h"
typedef struct g_spider_setting_s
{
    mraa_gpio_context dmotor_a;
    mraa_gpio_context dmotor_b;
    mraa_pwm_context dmotor_speed;

    mraa_gpio_context rotate_a;
    mraa_gpio_context rotate_b;
    mraa_pwm_context rotate_speed;
    
    mraa_pwm_context head;
    
    mraa_gpio_context encoder_a;
}g_spider_setting;

g_spider_setting gst_spider;

typedef struct rotate_task_s
{
    int counter;
    int total;
    int running;//0 is false
    void (*pfun)(void *);
    void * argv;
}rotate_task;
rotate_task gst_rotate;

void encoder_input_isr(void* p)
{
    if(gst_rotate.running==0)return;
    gst_rotate.counter=gst_rotate.counter+1;
    //printf("enc=%d\n",gst_rotate.counter);
    if(gst_rotate.counter>gst_rotate.total){
        spider_rotate_brake();
        if(gst_rotate.pfun){
            gst_rotate.pfun(gst_rotate.argv);
        }
        gst_rotate.running=0;
        
    }
}

static int map(int dstmin, int dstmax, 
                 int srcmin,int srcmax,
                 int srcval)
{
    float tmp=(float)(srcval-srcmin)/(srcmax-srcmin);
    float dst=tmp*(dstmax-dstmin)+dstmin;
    return (int)dst;
}

void spider_close()
{
    if(gst_spider.dmotor_a)
        mraa_gpio_close(gst_spider.dmotor_a);
    if(gst_spider.dmotor_b)
        mraa_gpio_close(gst_spider.dmotor_b);
    if(gst_spider.dmotor_speed)
        mraa_pwm_close(gst_spider.dmotor_speed);

    if(gst_spider.rotate_a)
        mraa_gpio_close(gst_spider.rotate_a);
    if(gst_spider.rotate_b)
        mraa_gpio_close(gst_spider.rotate_b);
    if(gst_spider.rotate_speed)
        mraa_pwm_close(gst_spider.rotate_speed);

    if(gst_spider.head)
        mraa_pwm_close(gst_spider.head);

    if(gst_spider.encoder_a)
    {
        mraa_gpio_isr_exit(gst_spider.encoder_a);
        mraa_gpio_close(gst_spider.encoder_a);
    }
}

int spider_init()
{
    /*motor drive*/
    gst_spider.dmotor_a = mraa_gpio_init(MDRIVE_DIR_PINA);
    gst_spider.dmotor_b = mraa_gpio_init(MDRIVE_DIR_PINB);
    if(gst_spider.dmotor_a==NULL || 
        gst_spider.dmotor_b==NULL){
        printf("Fail to init the driver motor io.\n");
        goto error;
    }
    mraa_gpio_dir(gst_spider.dmotor_a,MRAA_GPIO_OUT);
    mraa_gpio_dir(gst_spider.dmotor_b,MRAA_GPIO_OUT);
    
    gst_spider.dmotor_speed=mraa_pwm_init(MDRIVE_PWM_PIN);
    if(gst_spider.dmotor_speed==NULL){
        printf("Fail to init the driver speed io.\n");
        goto error;
    }
    mraa_pwm_enable(gst_spider.dmotor_speed,1);
    
    /*motor rotate*/
    gst_spider.rotate_a = mraa_gpio_init(MROTATE_DIR_PINA);
    gst_spider.rotate_b = mraa_gpio_init(MROTATE_DIR_PINB);
    if(gst_spider.rotate_a==NULL || 
        gst_spider.rotate_b==NULL){
        printf("Fail to init the driver motor io.\n");
        goto error;
    }
    mraa_gpio_dir(gst_spider.rotate_a,MRAA_GPIO_OUT);
    mraa_gpio_dir(gst_spider.rotate_b,MRAA_GPIO_OUT);
    
    gst_spider.rotate_speed=mraa_pwm_init(MROTATE_PWM_PIN);
    if(gst_spider.dmotor_speed==NULL){
        printf("Fail to init the driver speed io.\n");
        goto error;
    }
    mraa_pwm_enable(gst_spider.rotate_speed,1);

    /*init servo*/
    gst_spider.head = mraa_pwm_init(SERVO_PIN);
    if(gst_spider.head==NULL){
        printf("Fail to init the pwm pin data.\n");
        goto error;
    }
    //init period 
    mraa_pwm_period_us(gst_spider.head,PWM_50Hz/1000);//period is ns
    mraa_pwm_enable(gst_spider.head,1);

    //init encoder
    gst_spider.encoder_a = mraa_gpio_init(MROTATE_ENCODER_PINA);
    if(gst_spider.encoder_a==NULL){
        printf("Fail to init the ratation encoder_a.\n");
        goto error;
    }
    mraa_gpio_dir(gst_spider.encoder_a,MRAA_GPIO_IN);
    mraa_gpio_isr(gst_spider.encoder_a,MRAA_GPIO_EDGE_BOTH,encoder_input_isr,(void*)(&gst_rotate));

    return 0;
error:
    spider_close();
    return -1;
}

void spider_move(int is_foward,int pwm)
{
    int val_a=0;
    int val_b=0;
    float speed=0;
    if(gst_spider.dmotor_a==NULL ||
        gst_spider.dmotor_b==NULL ||
        gst_spider.dmotor_speed==NULL){
        return;
    }
    if(is_foward){
        val_a=1;
        val_b=0;
    }else{
        val_a=0;
        val_b=1;
    }
    if(pwm<MDRIVE_PWM_MIN){
        pwm=0;
    }
    speed=(float)pwm/100.0f;
    
    //printf("val_a=%d,val_b=%d,speed=%f\n",val_a,val_b,speed); 
    mraa_pwm_write(gst_spider.dmotor_speed,speed);
    mraa_gpio_write(gst_spider.dmotor_a,val_a);
    mraa_gpio_write(gst_spider.dmotor_b,val_b);
}

void spider_rotate(int is_clockwise,int pwm)
{
    int val_a=0;
    int val_b=0;
    float speed=0;
    if(gst_spider.dmotor_a==NULL ||
        gst_spider.dmotor_b==NULL ||
        gst_spider.dmotor_speed==NULL){
        return;
    }
    if(is_clockwise){
        val_a=1;
        val_b=0;
    }else{
        val_a=0;
        val_b=1;
    }
    if(pwm<MROTATE_PWM_MIN){
        pwm=MROTATE_PWM_MIN;
    }
    speed=(float)pwm/100.0f;
    
    //printf("val_a=%d,val_b=%d,speed=%f\n",val_a,val_b,speed);
    mraa_gpio_write(gst_spider.rotate_a,val_a);
    mraa_gpio_write(gst_spider.rotate_b,val_b);
    mraa_pwm_write(gst_spider.rotate_speed,speed);
}
void spider_move_stop()
{
    mraa_gpio_write(gst_spider.dmotor_a,0);
    mraa_gpio_write(gst_spider.dmotor_b,0);
    mraa_pwm_write(gst_spider.dmotor_speed,0.0f);
}
void spider_rotate_stop()
{
    mraa_gpio_write(gst_spider.rotate_a,0);
    mraa_gpio_write(gst_spider.rotate_b,0);
    mraa_pwm_write(gst_spider.rotate_speed,0.0f);
}
void spider_rotate_brake()
{
    mraa_gpio_write(gst_spider.rotate_a,1);
    mraa_gpio_write(gst_spider.rotate_b,1);
    mraa_pwm_write(gst_spider.rotate_speed,0.0f);
}

void spider_rotate_degree(int degree,int pwm,void (*pfun)(void *),void * argv)
{
    if(degree==0 ||  degree>360 || degree<-360){
        return;
    }

    int total = (int)(((float)abs(degree)*MROTATE_COUNT_PERROUND)/360.0);
    int is_clockwise=1;
    if(degree>0){
        is_clockwise=0;
    }
    //printf("[%s,%d]total=%d\n",__FILE__,__LINE__,total);

    gst_rotate.running = 0;
    spider_rotate_stop();
    
    memset(&gst_rotate,0,sizeof(gst_rotate));
    gst_rotate.total = total;
    gst_rotate.running = 1;
    gst_rotate.pfun = pfun;
    gst_rotate.argv=argv;
    
    spider_rotate(is_clockwise,pwm);
}
void spider_head(int degree)
{
    int pulse = map(MIN_PULSE_WIDTH,MAX_PULSE_WIDTH,0,SERVO_RANGE,degree);
    mraa_pwm_pulsewidth_us(gst_spider.head,pulse);
    //printf("Pulse width=%d us,degree=%d\n",pulse,degree);
}

