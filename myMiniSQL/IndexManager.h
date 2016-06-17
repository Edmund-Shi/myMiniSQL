#ifndef _INDEXMANAGER_H_
#define _INDEXMANAGER_H_
#include<string>
#include<iostream>
#include<fstream>
#include"bptree.h"
#include"base.h"

class IndexManager{
private:
	int index_num;
	string* filename;
	index* ind;
public:
	IndexManager();
	void Establish(string file, Data* key, int Addr);
	void Insert(string file, Data* key, int Addr);
	void Delete(string file, Data* key);
	int Find(string file, Data* key);
	~IndexManager();
};

#endif