#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <pthread.h>

#include <mraa.h>

#include <xpiderctl/xpiderctl.h>
#include <xpiderctl/config.h>

#include "mi_joystick.h"

//#define MAC "1C:96:5A:FA:73:96"
#define LED_PIN 7

mraa_gpio_context gpstled;
int gisblink_running=0;
int gblinktimeout=500;/*in ms*/
void* blink_thread(void * param)
{
	int state=0;
	while(gisblink_running){
		state=!state;
		mraa_gpio_write(gpstled,state);
		usleep(gblinktimeout*1000);
	}
	return NULL;
}
void start_blink()
{
	gpstled=mraa_gpio_init(LED_PIN);
	if(gpstled==NULL){
		return;
	}
	
	mraa_gpio_dir(gpstled,MRAA_GPIO_OUT);
	mraa_gpio_write(gpstled,0);
	gisblink_running = 1;
	pthread_t t;
	if(pthread_create(&t,NULL,blink_thread,NULL)){
		printf("[%s,%d]fail to create the tty rx thread.\n",__FILE__,__LINE__);
		return;	
	}
}
void stop_blink()
{
	gisblink_running=0;
	mraa_gpio_write(gpstled,0);
	mraa_gpio_close(gpstled);
}

void connect_joy_stick(const char* mac)
{
	char cmdbuffer[64];
	memset(cmdbuffer,0,64);
	sprintf(cmdbuffer,"echo connect %s | bluetoothctl",mac);
	//system("rfkill unblock bluetooth");
	system(cmdbuffer);
}
int map(int dstmin, int dstmax, int srcmin,int srcmax,int srcval)
{
	float tmp=(float)(srcval-srcmin)/(srcmax-srcmin);
	float dst=tmp*(dstmax-dstmin)+dstmin;
	return (int)dst;
}

void move(unsigned char key_value)
{
	int val = (int)(key_value)-128;
	int dir=0;
	int speed=0;
	if(val<0){
		dir=1;
		speed=map(0,100,0,128,-val);
	}else{
		dir=0;
		speed=map(0,100,0,128,val);
	}
	if(val!=0){
		spider_move(dir,speed);
	}else{
		spider_move_stop();
	}

}
void rotate(unsigned char key_value)
{
	int val = (int)(key_value)-128;
	int dir=0;
	int speed=0;
	if(val<0){
		dir=1;
		speed=map(0,100,0,128,-val);
	}else{
		dir=0;
		speed=map(0,100,0,128,val);
	}
	if(abs(val)>90){
		spider_rotate(dir,speed);
	}else{
		spider_rotate_stop();
	}

}
void head(unsigned char key_value)
{
	int val = map(SERVO_MIN,SERVO_MAX,0,255,key_value);
	//printf("key_val=%d,val=%d\n",key_value,val);
	spider_head(val);
}
void mi_keys_status_changed(unsigned char* keys,int len)
{
	#if 0
	int i=0;
	for(i=0;i<len;i++){
		printf("%d\t",keys[i]);
	}
	printf("\n");
	#endif
	move(keys[MI_KEY_JOYA_V]);
	rotate(keys[MI_KEY_JOYB_H]);
	head(keys[MI_KEY_JOYB_V]);

	//printf("val=%d\n",keys[MI_KEY_JOYA_V]);
}
void close_handler(int sig)
{
	mi_close();
	exit(1);
}
int main(int argc, char * argv[])
{
  int is_connected=0;
#if 1
  if(argc<2){
	printf("usages:spider_joy_demo [bluetooth mac]\n"
               "eg:spider_joy_demo 1C:96:5A:FA:73:96\n");
	return -1;
  }

  if(!mi_init(mi_keys_status_changed) && !spider_init()){
  	is_connected=1;
  }
  signal(SIGINT, close_handler);
  //mi_wait_joystick_input();
  if(is_connected==0){
  		start_blink();
  }
  
  while(1){
  	if(is_connected){
		if(mi_wait_joystick_input()){
			mi_close();
			spider_close();
			is_connected=0;
			start_blink();
		}
  	}else{
		if(!mi_init(mi_keys_status_changed) && !spider_init()){
			is_connected=1;
			stop_blink();
			continue;
		}
  		connect_joy_stick(argv[1]);		
		sleep(5);		
  	}
  }
#endif

  return 0;	
}
