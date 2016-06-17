#include "API.h"


API::~API()
{
}

Table API::Select(Table& tableIn, vector<int> attrSelect,vector<int>mask, vector<where> w)
{
//    throw std::bad_alloc();
	return rm.Select(tableIn, attrSelect,mask, w);
}

int API::Delete(Table& tableIn, vector<int>mask, vector<where> w)
{
	int res;
	res = rm.Delete(tableIn, mask, w);
	return res;
}

void API::Insert(Table& tableIn, tuper singleTuper)
{
	rm.Insert(tableIn, singleTuper);
}

bool API::DropTable(Table& tableIn)
{
	bool res;
	res = rm.DropTable(tableIn);
	return res;
}

bool API::CreateTable(Table& tableIn)
{
	bool res;
	res = rm.CreateTable(tableIn);
	return res;
}


