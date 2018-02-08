#ifndef SRC_FDMANAGER_H_
#define SRC_FDMANAGER_H_

#include <vector>
#include <string>
#include <map>
#include<unordered_map>

class FdManagerSet;

class FdManager
{
public:
	FdManager(int num);
	virtual ~FdManager();
	int Insert(int fd);
	int Delete(int fd);
	int Find(int fd);
	int ClearAll();
	int GetVal(int num);
	int GetTotalCount();
	void DumpAllData();

private:
	int maxSize_ = 0;
	std::vector<int> fdVec_;
protected:
};

class FdManagerSet
{
public:
	FdManagerSet(int num);
	virtual ~FdManagerSet();
	int CreatManagerSet(const char*,int);
	int CreatManagerSet(std::string&,int);
	int DeleteManagerSet(const char*);
	int DeleteManagerSet(std::string&);
	int insert(std::string&,int);
	int insert(const char*,int);
	//int RegisterFd(std::string &str,int maxNum);
	int DumpAll();
private:
	int maxSize_ = 0;
	//std::multimap<std::string, FdManager*> FdMap_;
	//typedef std::multimap<std::string, FdManager*>::iterator FdMapIter_t;
	std::unordered_map<std::string, FdManager*> FdMap_;
	typedef std::unordered_map<std::string, FdManager*>::iterator FdMapIter_t;
};
#endif