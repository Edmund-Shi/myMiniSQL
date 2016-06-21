#include "API.h"
#include "IndexManager.h"

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

void API::Insert(Table& tableIn, tuper& singleTuper)
{
	IndexManager indexMA;
	
	rm.InsertWithIndex(tableIn, singleTuper);
}

bool API::DropTable(Table& tableIn)
{
	bool res;
	res = rm.DropTable(tableIn);
	return res;
}

bool API::CreateTable(Table& tableIn)
{
	IndexManager indexMA;
	bool res;
	int i;
	res = rm.CreateTable(tableIn);
	for ( i = 0; i < tableIn.attr.num;i++) {
		if (tableIn.attr.unique[i] == 1){ //create index
			break;
		}
	}
	if (i < tableIn.attr.num) {
		
		CreateIndex(tableIn, i);
	}
	return res;
}

bool API::CreateIndex(Table& tableIn, int attr)
{
	IndexManager indexMA;
	string file_name;
	file_name = tableIn.getname() + ".index";
	indexMA.Establish(file_name);
	vector<int> attrSelect;
	attrSelect.push_back(attr);
	Table tableForindex(rm.Select(tableIn, attrSelect));
	for (int i = 0; i < tableForindex.T.size();i++) {
		indexMA.Insert(file_name, tableForindex.T[i]->data[attr], i);
	}
	return true;
}

