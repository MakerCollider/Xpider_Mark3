#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <pthread.h>

#include <mraa.h>
#include <mosquitto.h>

#include "spider.h"
#include "spideraction.h"


/*blink and led related*/
#define LED_PIN 7
mraa_gpio_context gpstled;
int gisblink_running=0;
int gblinktimeout=500;/*in ms*/

/*topic related*/
#define TOPIC_SPIDER_ACTION "spider_action" /*spider action topic*/
#define SPIDER_ID "edison_%d"
typedef struct spider_info_s
{
	bool is_connected;
	int id;
	int port;
	char host[16];
	pthread_t mqtt_thread;
	pthread_t rotate_thread;
	
	SpiderActionQueue *actionQueue;
	SpiderAction *action;
}spider_info;
spider_info g_spider;


int map(int dstmin, int dstmax, int srcmin,int srcmax,int srcval);
void* blink_thread(void * param);
void start_blink();
void stop_blink();

struct mosquitto * init_mqtt(const char* id,void * args);
bool start_mqtt(struct mosquitto* mosq,const char* host, int port);
void close_mqtt(struct mosquitto* mosq);
void* mqtt_thread(void * p);
void mqtt_message_callback(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message);
void mqtt_connect_callback(struct mosquitto *mosq, void *obj, int result);
void mqtt_disconnect_callback(struct mosquitto *, void *, int);
void exit_handler(int sig);

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
void* spider_move_thread(void *p){
	if(!g_spider.action){
			return NULL;
	}
	usleep(g_spider.action->time()*1000);
	spider_move_stop();
	spider_rotate_stop();
	return NULL;
}
void spider_rotate_done(void * p ){
	
	//printf("[%s,%d]here?\n",__FILE__,__LINE__);
	if(!g_spider.action){
		return;
	}
	//pthread_cancel(g_spider.rotate_thread);

	int is_forward=0;
	int speed =  g_spider.action->speed();
	if(speed>0){
		is_forward=1;
	}else{
		is_forward=0;
		speed = -speed;
	}
	//printf("[%s,%d]here?\n",__FILE__,__LINE__);
		
	spider_move(is_forward,speed);
	if(pthread_create(&g_spider.rotate_thread,NULL,spider_move_thread,NULL)){
		printf("[%s,%d]fail to create the spider move thread.\n",__FILE__,__LINE__);
		return;	
	}
}
int map(int dstmin, int dstmax, int srcmin,int srcmax,int srcval)
{
	float tmp=(float)(srcval-srcmin)/(srcmax-srcmin);
	float dst=tmp*(dstmax-dstmin)+dstmin;
	return (int)dst;
}

void* mqtt_thread(void * p)
{
	struct mosquitto *mosq = (struct mosquitto *)p;
	mosquitto_loop_forever(mosq, -1, 1);
}
void mqtt_message_callback(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message)
{
	if(strcmp(message->topic,TOPIC_SPIDER_ACTION)==0){
		/*process the spider_action topic*/
		//message->payload;
		//printf("RX message:%s\n",message->payload);
		if(0>=g_spider.actionQueue->importJson((const char *)message->payload)){
			printf("[%s,%d]fail to parse the spider action info.\n",__FILE__,__LINE__);
			return;
		}
		g_spider.action = g_spider.actionQueue->action(g_spider.id);
		int curDegree = (int)g_spider.action->curDegree()%180;
		int dstDegree = (int)g_spider.action->dstDegree()%180;

		spider_move_stop();
		//printf("[%s,%d]here?\n",__FILE__,__LINE__);
		spider_rotate_degree((dstDegree-curDegree)%360,45,spider_rotate_done,NULL);
		//printf("[%s,%d]here?\n",__FILE__,__LINE__);

	}
}

void mqtt_connect_callback(struct mosquitto *mosq, void *obj, int result)
{
	printf("edison_%d connected to the host:%s:%d\n",
		g_spider.id,
		g_spider.host,
		g_spider.port);
	
	g_spider.is_connected=true;

	printf("edison_%d subscribed topic:%s\n",
		g_spider.id,
		TOPIC_SPIDER_ACTION);
	
	mosquitto_subscribe(mosq,NULL,TOPIC_SPIDER_ACTION,1);
	stop_blink();
}
void mqtt_disconnect_callback(struct mosquitto *mosq,void *obj,int result)
{
	printf("edison_%d disconnected from the host:%s:%d\n",
		g_spider.id,
		g_spider.host,
		g_spider.port);

	g_spider.is_connected=false;
}
struct mosquitto * init_mqtt(const char* id,void * args)
{
	struct mosquitto *mosq = NULL;
	mosquitto_lib_init();
	mosq=mosquitto_new(id,true,args);
	if(mosq){		
		mosquitto_connect_callback_set(mosq, mqtt_connect_callback);
		mosquitto_message_callback_set(mosq, mqtt_message_callback);
		mosquitto_disconnect_callback_set(mosq,mqtt_disconnect_callback);
	}
	
	//create the thread
	if(pthread_create(&(g_spider.mqtt_thread),
		NULL,
		mqtt_thread,
		(void*)mosq)){
		printf("[%s,%d]fail to create the tty rx thread.\n",__FILE__,__LINE__);
		return NULL;	
	}
	
	return mosq;
}
bool start_mqtt(struct mosquitto* mosq,const char* host, int port)
{
	int ret=0;
	if(mosq==NULL && host==NULL){
		return false;
	}
		
	ret = mosquitto_connect(mosq, host, port, 60);
	if(ret){
		printf("Fail to connect to mqtt server %s.\n",host);
		return false;
	}

	return true;
}
void close_mqtt(struct mosquitto* mosq)
{
	mosquitto_destroy(mosq);
	mosquitto_lib_cleanup();
}


void exit_handler(int sig)
{
	//stop the mqtt event loop.
	pthread_kill(g_spider.mqtt_thread,0);
	printf("[%s,%d]exitting the program.\n",__FILE__,__LINE__);
	spider_move_stop();
	spider_rotate_stop();
	spider_close();
	exit(1);
}

int main(int argc, char * argv[])
{
  int is_connected=0;
  char host_ip[32];
  int host_port;
  int id=0;
  char strid[32];
  struct mosquitto* mosq;
  SpiderActionQueue *pstActioQueue = new SpiderActionQueue;

  /*init the input args*/
  if(argc<2){
     printf("usage: spider_json [id] [host ip] [port]\n"
	 	    "eg:spider_json 2 192.168.2.15 1883\n"
	 	    "named your self as ID:2, and connect to host:192.168.2.15:1883\n");
	 exit(1);
  }
  //set id
  id = atol(argv[1]);
  //set host
  if(argc>=3){
  	strcpy(host_ip,argv[2]);
  }else{
  	strcpy(host_ip,"localhost");
  }
  //set host_port
  if(argc>=4){
  	host_port = atol(argv[3]);
  }else{
  	host_port = 1883;
  }
  printf("connecting edison_%d to host[%s]:[%d]...\n",id,host_ip,host_port);

  /*init the g_spider info*/
  memset(&g_spider,0,sizeof(spider_info));
  sprintf(strid,SPIDER_ID,id);
  g_spider.id = id;
  strcpy(g_spider.host,host_ip);
  g_spider.port = host_port;
  g_spider.is_connected=false;
  g_spider.actionQueue = pstActioQueue;
  
  signal(SIGINT, exit_handler);
  signal(SIGTERM,exit_handler);

  /*init the mqtt*/
  mosq=init_mqtt(strid,NULL);
  if(mosq==NULL){
  	printf("fail to init the mqtt to host[%s]:[%d]",host_ip,host_port);
	goto ERROR;
  }
  /*init spider*/
  if(spider_init()){
    printf("fail to init the local spider IO.\n");
	goto ERROR;
  }
  do{
  	if(g_spider.is_connected){
		//sleep 200ms
		usleep(200000);
  	}else{
		//start blink
		start_blink();
		
		if(false==start_mqtt(mosq,host_ip,host_port)){
		  printf("fail to start the mqtt to host[%s]:[%d]\n",host_ip,host_port);
		  sleep(5);
		  continue;
		}
		printf("trying to connect to host:%s:%d.\n",host_ip,host_port);
		sleep(3);/*wait for 500ms*/

  	}
  }while(1);
  return 0;
ERROR:
  close_mqtt(mosq);
  spider_close();
  return 1;	
}
