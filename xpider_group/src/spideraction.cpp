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

#include <algorithm>
#include <numeric>
#include <stdio.h>

#ifdef __cplusplus
extern "C"{
#endif
#include <stdio.h>
#ifdef __cplusplus
}
#endif

#include "spideraction.h"

using namespace std;

SpiderAction::SpiderAction()
{
	m_id =0;
	m_curDegree = 0;
	m_dstDegree = 0;
	m_speed = 0;
	m_time=0;
}
SpiderAction::~SpiderAction()
{
}

bool SpiderAction::importJson(cJSON* p)
{
	if(p==NULL)return false;
	m_id = (uint32_t)cJSON_GetObjectItem(p,JSON_SPIDER_ID)->valueint;
	m_curDegree=(int)cJSON_GetObjectItem(p,JSON_SPIDER_CUR_DEGREE)->valueint;
	m_dstDegree=(int)cJSON_GetObjectItem(p,JSON_SPIDER_DST_DEGREE)->valueint;
	m_speed=(int)cJSON_GetObjectItem(p,JSON_SPIDER_SPEED)->valueint;
	m_time=(int)cJSON_GetObjectItem(p,JSON_SPIDER_TIME)->valueint;
	return true;
}
bool SpiderAction::importJson(const char* str)
{
	if(str==NULL)
		return false;	
	return importJson(cJSON_Parse(str));
}

SpiderActionQueue::SpiderActionQueue(int size)
{
	m_stUpdateCache.clear();
	m_stSpiderCache.clear();
	setupActionCache(size);
}
SpiderActionQueue::~SpiderActionQueue()
{
	reset();
}
void SpiderActionQueue::reset()
{
	vector<SpiderAction*>::iterator iter = m_stSpiderCache.begin();
	for(iter=m_stSpiderCache.begin();iter<m_stSpiderCache.end();iter++)
	{
		SpiderAction* p = *iter;
		if(p){
			delete p;
		}
	}
}
int SpiderActionQueue::importJson(const char * strJson)
{
	cJSON* root;
	int size=0;
	int count=0;
	SpiderAction* pstSpider;
	if(strJson==NULL){
		return 0;
	}

	root = cJSON_Parse(strJson);
	if(!root){
		return 0;
	}
	size = cJSON_GetArraySize(root);
	
	m_stUpdateCache.clear();
	vector<SpiderAction*>::iterator iter=m_stSpiderCache.begin();
	for(int i=0 ;i<size;i++){
		cJSON * p = cJSON_GetArrayItem(root,i);
		pstSpider = *(iter++);
		if(pstSpider->importJson(p)){
			count++;
		}
	}
	
	//printf("root_array_size=%d/%d\n",size,count);
	return count;
}
SpiderAction* SpiderActionQueue::action(uint32_t id)
{
	SpiderAction* ret = NULL;
	vector<SpiderAction*>::iterator iter = m_stSpiderCache.begin();
	for(iter=m_stSpiderCache.begin();iter<m_stSpiderCache.end();iter++)
	{
		SpiderAction* p = *iter;
		if(p->id()==id)return p;
	}
	return NULL;
}
int SpiderActionQueue::setupActionCache(int size)
{
	if(size<SPIDER_CACHE_MIN){
		size = SPIDER_CACHE_MIN;
	}
	reset();
	
	if(m_stSpiderCache.size()){
		m_stSpiderCache.clear();
	}
	for(int i=0;i<size;i++)
	{
		SpiderAction * p = new SpiderAction();
		m_stSpiderCache.push_back(p);
	}
	return size;
}


