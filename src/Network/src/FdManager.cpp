/*
 *	FdManager.cpp
 *
 *  Created on: Dec 26, 2017
 *      Author: Daniel yuan
 */

#include <algorithm>
#include "logging.h"
#include "FdManager.h"

//-------------------------FdMamager class---------------------------------
//-------------------------------------------------------------------------
#define LOG_TAG "FdManager"
#define LOG_LEVEL LOG_NONE
#define LOGGING(...) log_print(LOG_LEVEL,LOG_TAG,__VA_ARGS__)

FdManager::FdManager(int num):maxSize_(num)
{
}

FdManager::~FdManager()
{
	fdVec_.clear();	
}

int FdManager::Insert(int fd)
{
	LOGGING("%s %d %d\r\n",__func__,fdVec_.size(),maxSize_);
	if(fdVec_.size() <= maxSize_) {
		fdVec_.push_back(fd);
		LOGGING("size:%d\r\n",fdVec_.size());
	} else {
		LOGGING("Max Size reached!\r\n");
		return -1;
	}
	return 0;
}

int FdManager::Delete(int fd)
{
	LOGGING("%s (%d)\r\n",__func__,fd);
	
	std::vector<int>::iterator iter;
	iter = std::find(fdVec_.begin(),fdVec_.end(),fd);
	if(fdVec_.end() == iter) {
		LOGGING("can not find fd:%d\r\n",fd);
		return -1;
	} else {
		fdVec_.erase(iter);
	}
	return 0;
}

int FdManager::Find(int fd)
{
	LOGGING("%s (%d)\r\n",__func__,fd);
	
	std::vector<int>::iterator iter;
	iter = std::find(fdVec_.begin(),fdVec_.end(),fd);
	if(fdVec_.end() == iter) {
		LOGGING("can not find fd:%d\r\n",fd);
		return -1;
	} else {
		return *iter;
	}
}

int FdManager::ClearAll()
{
	LOGGING("%s\r\n",__func__);
	fdVec_.clear();
}

int FdManager::GetVal(int num)
{
	return fdVec_[num];
}

int FdManager::GetTotalCount()
{
	return fdVec_.size();
}

void FdManager::DumpAllData()
{
	printf("**********%s*********\r\n",__func__);

	std::vector<int>::iterator iter;
	for(iter = fdVec_.begin(); iter != fdVec_.end(); ++iter) {
		printf("[fd]:%d\r\n",*iter);	
	}

	printf("******End of %s******\r\n",__func__);
}


//class FdManagerSet
#define LOG_TAG2 "FdManagerSet"
#define LOG_LEVEL2 LOG_PRINT
#define LOGGING2(...) log_print(LOG_LEVEL2,LOG_TAG2,__VA_ARGS__)

//find first para of the map
template<typename K, typename V>
typename std::unordered_map<K, V>::const_iterator findPair(const std::unordered_map<K, V>& map, const std::pair<K, V>& pair)
{
	std::pair<typename std::unordered_map<K, V>::const_iterator, typename std::unordered_map<K, V>::const_iterator> ret;

	ret = map.equal_range(pair.first);

	for (typename std::unordered_map<K, V>::const_iterator it=ret.first; it!=ret.second; ++it) {
		//if ((*it).second == pair.second)
		if ((*it).first == pair.first)
			return it;
	}

	return map.end();
}

FdManagerSet::FdManagerSet(int num):maxSize_(num)
{
}

FdManagerSet::~FdManagerSet()
{
}

int FdManagerSet::CreatManagerSet(std::string &str,int num)
{
	FdManager *pFdManager = new FdManager(num);
	
	std::pair<std::string, FdManager*> newPair = std::make_pair(str, pFdManager);
	
	if (findPair(FdMap_, newPair) == FdMap_.end()) {
		LOGGING2("insert [%s]\r\n",str.c_str());		
		FdMap_.insert(newPair);		
	} else {
		LOGGING2("not insert [%s] \r\n",str.c_str());		
	}
	return 0;
}

int FdManagerSet::CreatManagerSet(const char *pName,int num)
{
	if(NULL == pName) {
		return -1;	
	}
	std::string str;
	str.assign(pName);
	CreatManagerSet(str,num);
	return 0;
}

int FdManagerSet::DeleteManagerSet(const char *pName)
{
	if(NULL == pName) {
		return -1;	
	}
	std::string str;
	str.assign(pName);
	DeleteManagerSet(str);
	return 0;
}

int FdManagerSet::DeleteManagerSet(std::string &str)
{
	for (FdMapIter_t it=FdMap_.begin(); it!=FdMap_.end(); /*it++*/) {
		if((*it).first == str) {
			//LOGGING2("find [%s]!delete it!\r\n",str.c_str());		
			delete (*it).second;				
			FdMap_.erase(it++);
			//LOGGING2("find it!delete2!\r\n");		
		} else
			it++;
	}
	return 0;
}

int FdManagerSet::insert(const char *pName,int fd)
{
	if(NULL == pName) {
		return -1;	
	}
	std::string str;
	str.assign(pName);
	int ret = insert(str,fd);
	return ret;
}

int FdManagerSet::insert(std::string &str,int fd)
{
	FdMapIter_t iter=FdMap_.begin();

	for ( ;iter!=FdMap_.end(); ) {
		if((*iter).first == str) {		
			//delete (*it).second;				
			//FdMap_.erase(it++);
			(*iter++).second->Insert(fd);
			//iter++;
			return 0;
		} else
			iter++;
	}
	return -1;
}

int FdManagerSet::DumpAll()
{
	LOGGING2("Start dump\r\n");
	
	for(FdMapIter_t iter = FdMap_.begin();iter !=  FdMap_.end(); ++iter) {
		printf("[%s]:\r\n",(*iter).first.c_str());
		(*iter).second->DumpAllData();
	}
	
	LOGGING2("Stop dump\r\n");
}
/*int FdManagerSet::RegisterFd(std::string &str,int maxNum)
{
	return 0;
}*/