#include "RecordManager.h"


RecordManager::~RecordManager()
{
}

bool RecordManager::isSatisfied(Table& tableinfor, tuper row, vector<int> mask, where w)
{
	bool res = true;
	for (int i = 0; i < mask.size();i++){
		if (w.d == NULL){ //不存在where条件
			continue;
		}
		else if (row[mask[i]]->flag == -1) { //int
			switch (w.flag) {
			case eq: if (!(((Datai*)row[mask[i]])->x == ((Datai*)w.d)->x)) return false;break;
			case leq: if (!(((Datai*)row[mask[i]])->x <= ((Datai*)w.d)->x)) return false; break;
			case l: if (!(((Datai*)row[mask[i]])->x < ((Datai*)w.d)->x)) return false; break;
			case geq: if (!(((Datai*)row[mask[i]])->x >= ((Datai*)w.d)->x)) return false; break;
			case g: if (!(((Datai*)row[mask[i]])->x > ((Datai*)w.d)->x)) return false; break;
			case neq: if (!(((Datai*)row[mask[i]])->x != ((Datai*)w.d)->x)) return false; break;
			default: ;
			}
		}
		else if (row[mask[i]]->flag == 0) { //Float
			switch (w.flag) {
			case eq: if (!(((Dataf*)row[mask[i]])->x == ((Dataf*)w.d)->x)) return false; break;
			case leq: if (!(((Dataf*)row[mask[i]])->x <= ((Dataf*)w.d)->x)) return false; break;
			case l: if (!(((Dataf*)row[mask[i]])->x < ((Dataf*)w.d)->x)) return false; break;
			case geq: if (!(((Dataf*)row[mask[i]])->x >= ((Dataf*)w.d)->x)) return false; break;
			case g: if (!(((Dataf*)row[mask[i]])->x >((Dataf*)w.d)->x)) return false; break;
			case neq: if (!(((Dataf*)row[mask[i]])->x != ((Dataf*)w.d)->x)) return false; break;
			default: ;
			}
		}
		else if (row[mask[i]]->flag > 0){ //string
			switch (w.flag) {
			case eq: if (!(((Datac*)row[mask[i]])->x == ((Datac*)w.d)->x)) return false; break;
			case leq: if (!(((Datac*)row[mask[i]])->x <= ((Datac*)w.d)->x)) return false; break;
			case l: if (!(((Datac*)row[mask[i]])->x < ((Datac*)w.d)->x)) return false; break;
			case geq: if (!(((Datac*)row[mask[i]])->x >= ((Datac*)w.d)->x)) return false; break;
			case g: if (!(((Datac*)row[mask[i]])->x >((Datac*)w.d)->x)) return false; break;
			case neq: if (!(((Datac*)row[mask[i]])->x != ((Datac*)w.d)->x)) return false; break;
			default: ;
			}
		}
		else { //just for debug
			cout << "Error in RecordManager in function is satisified!" << endl;
			system("pause");
		}

		//else if (tableinfor.getattribute().flag[i] == -1){ //int
		//	int value = ((Datai*)row[mask[i]])->x;
		//	if (lower_bound[i]==NULL){//不存在下界
		//		if (value>((Datai*)upper_bound[i])->x){
		//			res = false;
		//			return res;
		//		}
		//	}
		//	else if (upper_bound[i] == NULL){//不存在上界
		//		if (value < ((Datai*)lower_bound[i])->x){
		//			res = false;
		//			return res;
		//		}
		//	}
		//	else{
		//		if (value < ((Datai*)lower_bound[i])->x || value>((Datai*)upper_bound[i])->x){
		//			res = false;
		//			return res;
		//		}
		//	}
		//}
		//else if (tableinfor.getattribute().flag[i] == 0){ // float
		//	float value= ((Dataf*)row[mask[i]])->x;
		//	if (lower_bound[i] == NULL){//不存在下界
		//		if (value > ((Dataf*)upper_bound[i])->x){
		//			res = false;
		//			return res;
		//		}
		//	}
		//	else if (upper_bound[i] == NULL){//不存在上界
		//		if (value < ((Dataf*)lower_bound[i])->x){
		//			res = false;
		//			return res;
		//		}
		//	}
		//	else{
		//		if (value < ((Dataf*)lower_bound[i])->x || value>((Dataf*)upper_bound[i])->x){
		//			res = false;
		//			return res;
		//		}
		//	}
		//}
		//else{ //string
		//	string value = ((Datac*)row[mask[i]])->x;
		//	if (value != ((Datac*)lower_bound[i])->x){
		//		res = false; return res;
		//	}
		//}
	}
	return res;
}

Table RecordManager::Select(Table& tableIn, vector<int>attrSelect, vector<int>mask, vector<where> w)
{
	if (mask.size() == 0){
		return Select(tableIn,attrSelect);
	}
	string stringRow;
	
	string filename = tableIn.getname() + ".table";
	int length = tableIn.dataSize() + 1;
	const int recordNum =BLOCKSIZE/length;
	for (int blockOffset = 0; blockOffset < tableIn.blockNum; blockOffset++){
		int bufferNum = buf_ptr->getIfIsInBuffer(filename, blockOffset);
		if (bufferNum == -1){
			bufferNum = buf_ptr->getEmptyBuffer();
			buf_ptr->readBlock(filename, blockOffset, bufferNum);
		}
		for (int offset = 0; offset < recordNum;offset++){
			int position = offset * length;
			stringRow = buf_ptr->bufferBlock[bufferNum].getvalues(position, position + length);
			if (stringRow.c_str()[0] == EMPTY) continue;//该行是空的
			int c_pos = 1;//当前在数据流中指针的位置，0表示该位是否有效，因此数据从第一位开始
			tuper *temp_tuper = new tuper;
			for (int attr_index = 0; attr_index < tableIn.getattribute().num; attr_index++){
				if (tableIn.getattribute().flag[attr_index] == -1){//是一个整数
					int value;
					memcpy(&value, &(stringRow.c_str()[c_pos]), sizeof(int));
					c_pos += sizeof(int);
					temp_tuper->addData(new Datai(value));
				}
				else if (tableIn.getattribute().flag[attr_index] == 0){//float
					float value;
					memcpy(&value, &(stringRow.c_str()[c_pos]), sizeof(float));
					c_pos += sizeof(float);
					temp_tuper->addData(new Dataf(value));
				}
				else{
					char value[MAXSTRINGLEN];
					int strLen = tableIn.getattribute().flag[attr_index]+1;
					memcpy(value, &(stringRow.c_str()[c_pos]), strLen);
					c_pos += strLen;
					temp_tuper->addData(new Datac(string(value)));
				}
			}//以上内容先从文件中生成一行tuper，一下判断是否满足要求
            bool flag = true;
            for(int k=0;k<w.size();k++)
                if(!isSatisfied(tableIn, *temp_tuper, mask, w[k]))
                    flag = false;
			if (flag){
				tableIn.addData(temp_tuper); //可能会存在问题;solved!
			}
		}
	}
	return SelectProject(tableIn,attrSelect);
}

Table RecordManager::Select(Table& tableIn, vector<int>attrSelect)
{
	string stringRow;
	string filename = tableIn.getname() + ".table";
	tuper* temp_tuper;
	int length = tableIn.dataSize() + 1; //一个元组的信息在文档中的长度
	const int recordNum = BLOCKSIZE / length; //一个block中存储的记录条数
	for (int blockOffset = 0; blockOffset < tableIn.blockNum;blockOffset++){//读取整个文件中的所有内容
		int bufferNum = buf_ptr->getIfIsInBuffer(filename, blockOffset);
		if (bufferNum == -1){ //该块不再内存中，读取之
			bufferNum = buf_ptr->getEmptyBuffer();
			buf_ptr->readBlock(filename, blockOffset, bufferNum);
		}
		for (int offset = 0; offset < recordNum;offset++){
			int position = offset * length;
			stringRow = buf_ptr->bufferBlock[bufferNum].getvalues(position, position + length);
			if(stringRow.c_str()[0]==EMPTY) continue;//该行是空的
			int c_pos = 1;//当前在数据流中指针的位置，0表示该位是否有效，因此数据从第一位开始
            temp_tuper = new tuper;
			for (int attr_index = 0; attr_index < tableIn.getattribute().num;attr_index++){
				if (tableIn.getattribute().flag[attr_index] == -1){//是一个整数
					int value;
					memcpy(&value, &(stringRow.c_str()[c_pos]), sizeof(int));
					c_pos += sizeof(int);
					temp_tuper->addData(new Datai(value));
				}
				else if (tableIn.getattribute().flag[attr_index]==0){//float
					float value;
					memcpy(&value, &(stringRow.c_str()[c_pos]), sizeof(float));
					c_pos += sizeof(float);
					temp_tuper->addData(new Dataf(value));
				}
				else{
					char value[MAXSTRINGLEN];
					int strLen = tableIn.getattribute().flag[attr_index]+1;
					memcpy(value, &(stringRow.c_str()[c_pos]), strLen);
					c_pos += strLen;
					temp_tuper->addData(new Datac(string(value)));
				}
			}
			tableIn.addData(temp_tuper); //可能会存在问题;solved!
		}
	}
	return SelectProject( tableIn, attrSelect);
}

void RecordManager::Insert(Table& tableIn, tuper singleTuper)
{
	char *charTuper;
	charTuper = Tuper2Char(tableIn, singleTuper);//把一个元组转换成字符串
	//判断是否unique
	insertPos iPos = buf_ptr->getInsertPosition(tableIn);//获取插入位置
	buf_ptr->bufferBlock[iPos.bufferNUM].values[iPos.position] = NOTEMPTY;
	memcpy(&(buf_ptr->bufferBlock[iPos.bufferNUM].values[iPos.position + 1]), charTuper, tableIn.dataSize());
	buf_ptr->writeBlock(iPos.bufferNUM);
}

char* RecordManager::Tuper2Char(Table& tableIn, tuper singleTuper)
{
	char* ptrRes;
	int pos = 0;//当前的插入位置
	ptrRes = (char*)malloc((tableIn.dataSize() + 1)*sizeof(char));//额外的最后一位是0
	for (int i = 0; i < tableIn.getattribute().num;i++){
		if (tableIn.getattribute().flag[i] == -1){ //int
			int value = ((Datai*)singleTuper[i])->x;
			memcpy(ptrRes + pos, &value, sizeof(int));
			pos += sizeof(int);
		}
		else if (tableIn.getattribute().flag[i] == 0){
			float value = ((Dataf*)singleTuper[i])->x;
			memcpy(ptrRes + pos, &value, sizeof(float));
			pos += sizeof(float);
		}
		else{ //string
			string value(((Datac*)singleTuper[i])->x);
			int strLen = tableIn.getattribute().flag[i] + 1;
			memcpy(ptrRes + pos, value.c_str(), strLen);//多加1，拷贝最后的'\0';
			pos += strLen;
		}
	}
	ptrRes[tableIn.dataSize()] = '\0';
	return ptrRes;
}

int RecordManager::Delete(Table& tableIn, vector<int>mask, vector<where> w)
{
	string filename = tableIn.getname() + ".table";
	string stringRow;
	
	int count = 0;
	int length = tableIn.dataSize() + 1;
	const int recordNum = BLOCKSIZE / length;
	for (int blockOffset = 0; blockOffset < tableIn.blockNum; blockOffset++){
		int bufferNum = buf_ptr->getIfIsInBuffer(filename, blockOffset);
		if (bufferNum == -1){
			bufferNum = buf_ptr->getEmptyBuffer();
			buf_ptr->readBlock(filename, blockOffset, bufferNum);
		}
		for (int offset = 0; offset < recordNum; offset++){
			int position = offset * length;
			stringRow = buf_ptr->bufferBlock[bufferNum].getvalues(position, position + length);
			if (stringRow.c_str()[0] == EMPTY) continue;//该行是空的
			int c_pos = 1;//当前在数据流中指针的位置，0表示该位是否有效，因此数据从第一位开始
			tuper *temp_tuper = new tuper;
			for (int attr_index = 0; attr_index < tableIn.getattribute().num; attr_index++){
				if (tableIn.getattribute().flag[attr_index] == -1){//是一个整数
					int value;
					memcpy(&value, &(stringRow.c_str()[c_pos]), sizeof(int));
					c_pos += sizeof(int);
					temp_tuper->addData(new Datai(value));
				}
				else if (tableIn.getattribute().flag[attr_index] == 0){//float
					float value;
					memcpy(&value, &(stringRow.c_str()[c_pos]), sizeof(float));
					c_pos += sizeof(float);
					temp_tuper->addData(new Dataf(value));
				}
				else{
					char value[MAXSTRINGLEN];
					int strLen = tableIn.getattribute().flag[attr_index] + 1;
					memcpy(value, &(stringRow.c_str()[c_pos]), strLen);
					c_pos += strLen;
					temp_tuper->addData(new Datac(string(value)));
				}
			}//以上内容先从文件中生成一行tuper，一下判断是否满足要求
            
            bool flag = true;
            for(int k=0;k<w.size();k++)
                if(!isSatisfied(tableIn, *temp_tuper, mask, w[k]))
                    flag = false;
            
			if (flag){
				buf_ptr->bufferBlock[bufferNum].values[position] = DELETED; //DELETED==EMYTP
				buf_ptr->writeBlock(bufferNum);
				count++;
			}
		}
	}
	return count;
}

bool RecordManager::DropTable(Table& tableIn)
{
	string filename = tableIn.getname() + ".table";
	if (remove(filename.c_str()) != 0){
		throw TableException("Can't delete the file!\n");
	}
	else{
		buf_ptr->setInvalid(filename);
	}
	return true;
}

bool RecordManager::CreateTable(Table& tableIn)
{

	string filename = tableIn.getname() + ".table";
	fstream fout(filename.c_str(), ios::out);
	fout.close();
	return true;
}

Table RecordManager::SelectProject(Table& tableIn, vector<int>attrSelect)
{
	Attribute attrOut;
	tuper *ptrTuper=NULL;
	attrOut.num = attrSelect.size();
	for (int i = 0; i < attrSelect.size();i++){
		attrOut.flag[i] = tableIn.getattribute().flag[attrSelect[i]];
		attrOut.name[i] = tableIn.getattribute().name[attrSelect[i]];
		attrOut.unique[i] = tableIn.getattribute().unique[attrSelect[i]];
	}
	Table tableOut(tableIn.getname(), attrOut, tableIn.blockNum);
    int k;
	for (int i = 0; i < tableIn.T.size(); i++){//tuper的个数
		ptrTuper = new tuper;
		for (int j = 0; j < attrSelect.size();j++){
            k = attrSelect[j];
			Data *resadd =NULL;
			if (tableIn.T[i]->operator [](k)->flag == -1) {
				resadd = new Datai((*((Datai*)tableIn.T[i]->operator [](k))).x);
			}
			else if(tableIn.T[i]->operator [](k)->flag == 0){
				resadd = new Dataf((*((Dataf*)tableIn.T[i]->operator [](k))).x);
			}
			else if (tableIn.T[i]->operator [](k)->flag>0) {
				resadd = new Datac((*((Datac*)tableIn.T[i]->operator [](k))).x);
			}
			
			ptrTuper->addData(resadd);//bug

		}
		tableOut.addData(ptrTuper);
	}

	return tableOut;
}

tuper RecordManager::String2Tuper(Table& tableIn, string stringRow)
{
	tuper temp_tuper;
	if (stringRow.c_str()[0] == EMPTY) return temp_tuper;//该行是空的
	int c_pos = 1;//当前在数据流中指针的位置，0表示该位是否有效，因此数据从第一位开始
	for (int attr_index = 0; attr_index < tableIn.getattribute().num; attr_index++){
		if (tableIn.getattribute().flag[attr_index] == -1){//是一个整数
			int value;
			memcpy(&value, &(stringRow.c_str()[c_pos]), sizeof(int));
			c_pos += sizeof(int);
			temp_tuper.addData(new Datai(value));
		}
		else if (tableIn.getattribute().flag[attr_index] == 0){//float
			float value;
			memcpy(&value, &(stringRow.c_str()[c_pos]), sizeof(float));
			c_pos += sizeof(float);
			temp_tuper.addData(new Dataf(value));
		}
		else{
			char value[MAXSTRINGLEN];
			int strLen = tableIn.getattribute().flag[attr_index] + 1;
			memcpy(value, &(stringRow.c_str()[c_pos]), strLen);
			c_pos += strLen;
			temp_tuper.addData(new Datac(string(value)));
		}
	}//以上内容先从文件中生成一行tuper
	return temp_tuper;
}


