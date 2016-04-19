/*
  Copyright (c) 2016 Song Yunpeng
 
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:
 
  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.
 
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/

#pragma once
#ifndef SPIDER_INFO_H
#define SPIDER_INFO_H

#ifdef __cplusplus
extern "C"{
#endif
#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>
#include <math.h>
#include "cJSON.h"

#define _USE_MATH_DEFINES 

#ifdef __cplusplus
}
#endif

#include <vector>

#define JSON_SPIDER_ID "id"
#define JSON_SPIDER_CUR_DEGREE "cur_degree"
#define JSON_SPIDER_DST_DEGREE "dst_degree"
#define JSON_SPIDER_SPEED "speed"
#define JSON_SPIDER_TIME "time"

class SpiderActionQueue;
class SpiderAction
{
	friend class SpiderActionQueue;
public:
	SpiderAction();
	~SpiderAction();
	
	uint32_t id(){return m_id;}
	float curDegree(){return m_curDegree;}
	float dstDegree(){return m_dstDegree;}
	int speed(){return m_speed;}
	int time(){return m_time;}

protected:		
	/************************************************************
	*purpose: import from json string into the class members.
	*input:strJson, parse the json str into SpiderInfo struct.
	*return: true, success
	*************************************************************/
	bool importJson(cJSON* p);
	bool importJson(const char* str);

protected:
	uint32_t m_id;
	int m_curDegree; /*[0,360]*/
	int m_dstDegree; /*[0,360]*/
	int m_speed;/*[-100,100]*/
	int m_time;/*keep the current action for [time] ms */
};

#define SPIDER_CACHE_SIZE 32
#define SPIDER_CACHE_MAX 64
#define SPIDER_CACHE_MIN 8

class SpiderActionQueue
{
public:
	SpiderActionQueue(int size=SPIDER_CACHE_SIZE);
	~SpiderActionQueue();
	
	/************************************************************
	*purpose: import from json string into the class members.
	*input:strJson, parse the json str into SpiderInfo struct.
	*return: the new valid count of the updated queue
	*************************************************************/
	int importJson(const char* strJson);
	SpiderAction* action(uint32_t id);
	
protected:
	int setupActionCache(int size);
	
	/*clean up the queue and release the memory*/
	void reset();
	
protected:
	//spider cache, used to save all memory pointers.
	std::vector<SpiderAction*> m_stSpiderCache;
	//update cache, just save those updated spider info.
	std::vector<SpiderAction*> m_stUpdateCache;
};
#endif

