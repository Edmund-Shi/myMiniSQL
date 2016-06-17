#ifndef _INDEX_H_
#define _INDEX_H_
#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include "BufferManager.h"

#define BLOCKSIZE 4096
#define MESSAGE 20
#define POINTERLENGTH 20
//block内结构，前20位记录当前节点信息，后键值信息分为四块：第一块为键值，第二块为兄弟键值位置
//第三块为叶子节点独有，记录文件对应位置；对应的内节点记录下一块地址(比自己小的)，第四块为当前编号位置,第五块专门记比自己大的地址
//节点第一部分空出特地用来记录第一个键值位置
//编号均从0开始

using namespace std;

enum nodetype{ Internal, Leaf };
int keylength[3] = { 4, 8, 20 };
BufferManager bf;

class index{
private:
	int Number;
	int maxchild;
	int order;
	int type;//0 int; 1 double; 2 string
	char* fatherBlock;
public:
	string name;
	template<class Keytype>
	index(string filename, Keytype key, int Addr, int ktype) :name(filename)
	{
		ifstream in;
		in.open(name);
		fatherBlock = new char[BLOCKSIZE];

		if (in)
		{
			ofstream out;
			out.open(name);
			char* root = new char[BLOCKSIZE];
			*(int*)(root) = Internal;
			*(int*)(root + 4) = 0;//node position
			*(int*)(root + 8) = -1;//father node position
			*(int*)(root + 12) = 1;//number of keys
			*(int*)(root + 16) = 0;//delete or not
			cout << *(root) << endl;
			int NumOfKeys = *(int*)(root + 12);
			*(int*)(root + MESSAGE + keylength[type]) = NumOfKeys;//first member position
			*(Keytype*)(root + MESSAGE + NumOfKeys*(keylength[type] + POINTERLENGTH)) = key;
			*(int*)(root + MESSAGE + NumOfKeys*(keylength[type] + POINTERLENGTH) + keylength[type]) = -1;
			*(int*)(root + MESSAGE + NumOfKeys*(keylength[type] + POINTERLENGTH) + keylength[type] + 8) = NumOfKeys;
			*(int*)(root + MESSAGE + NumOfKeys*(keylength[type] + POINTERLENGTH) + keylength[type] + 4) = -1;
			*(int*)(root + MESSAGE + NumOfKeys*(keylength[type] + POINTERLENGTH) + keylength[type] + 12) = 1;//leaf node position
			*(int*)(root + MESSAGE + NumOfKeys*(keylength[type] + POINTERLENGTH) + keylength[type] + 16) = 0;//delete or not
			int buffernum = bf.GiveMeABlock(name, 0);
			memcpy(bf.bufferBlock[buffernum].values, root, BLOCKSIZE);
			bf.writeBlock(buffernum);
			bf.flashBack(buffernum);
			//writebuffer
			
			char* newBlock = new char[BLOCKSIZE];
			*(int*)(newBlock) = Leaf;
			*(int*)(newBlock + 4) = 1;//node position
			*(int*)(newBlock + 8) = 0;//father node position
			*(int*)(newBlock + 12) = 1;//number of keys
			*(int*)(newBlock + 16) = 0;//delete or not

			NumOfKeys = *(int*)(newBlock + 12);
			*(int*)(newBlock + MESSAGE + keylength[type]) = NumOfKeys;
			*(Keytype*)(newBlock + MESSAGE + NumOfKeys*(keylength[type] + POINTERLENGTH)) = key;
			*(int*)(newBlock + MESSAGE + NumOfKeys*(keylength[type] + POINTERLENGTH) + keylength[type]) = -1;
			*(int*)(newBlock + MESSAGE + NumOfKeys*(keylength[type] + POINTERLENGTH) + keylength[type] + 8) = NumOfKeys;
			*(int*)(newBlock + MESSAGE + NumOfKeys*(keylength[type] + POINTERLENGTH) + keylength[type] + 4) = Addr;
			*(int*)(newBlock + MESSAGE + NumOfKeys*(keylength[type] + POINTERLENGTH) + keylength[type] + 16) = 0;//delete or not
			buffernum = bf.GiveMeABlock(name, 1);
			memcpy(bf.bufferBlock[buffernum].values, newBlock, BLOCKSIZE);
			bf.writeBlock(buffernum);
			bf.flashBack(buffernum);

			maxchild = 3;
			order = maxchild;
			Number = 2;
			type = ktype;
			delete[] newBlock;
			delete[] root;
		}
		else
		{
			cout << "error!" << endl;
		}

	}
	index(){};
	~index(){};

	template <class Keytype>
	int find(Keytype key)
	{
		char *currentBlock = new char[BLOCKSIZE];
		Keytype NowKey;

		int buffernum = bf.GiveMeABlock(name, 0);
		bf.useBlock(buffernum);
		memcpy(currentBlock, bf.bufferBlock[buffernum].values, BLOCKSIZE);
		//readbuffer;
		int LeafType = *(int*)(currentBlock);
		int Brother = *(int*)(currentBlock + MESSAGE + keylength[type]);
		int tempBro, position;

		while (LeafType == Internal)
		{
			int NumOfKeys = *(int*)(currentBlock + 12);
			int i = 0;

			for (i = 0; i < NumOfKeys; i++)
			{
				NowKey = *(Keytype*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH));
				tempBro = Brother;
				position = *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type] + 4);
				Brother = *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type]);
				if (compare(key, NowKey) < 0)
					break;
				else continue;
			}
			if (i == NumOfKeys)
				position = *(int*)(currentBlock + MESSAGE + tempBro*(keylength[type] + POINTERLENGTH) + keylength[type] + 12);
			int buffernum = bf.GiveMeABlock(name, position);
			bf.useBlock(buffernum);
			memcpy(currentBlock, bf.bufferBlock[buffernum].values, BLOCKSIZE);
			LeafType = *(int*)(currentBlock);
			Brother = *(int*)(currentBlock + MESSAGE + keylength[type]);
			//readbuffer
		}

		int NumOfKeys = *(int*)(currentBlock + 12);
		Brother = *(int*)(currentBlock + MESSAGE + keylength[type]);
		for (int i = 0; i < NumOfKeys; i++)
		{
			NowKey = *(Keytype*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH));
			tempBro = Brother;
			int Addr = *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type] + 4);
			Brother = *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type]);
			if (compare(key, NowKey) == 0&&*(int*)(currentBlock+MESSAGE+Brother*(keylength[type]+POINTERLENGTH)+keylength[type]+16)!=1)
			{
				delete[]currentBlock;
				return Addr;
			}
		}
		return -1;
		delete[]currentBlock;
	}

	template <class Keytype>
	int compare(Keytype k1, Keytype k2)
	{
		if (type != 2)
		{
			if (k1 == k2)
				return 0;
			else if (k1 > k2)
				return 1;
			else return -1;
		}
		else
		{
			return 1;//compare the string
		}
	}

	template <class Keytype>
	void insert(Keytype key, int Addr)
	{
		char *currentBlock = new char[BLOCKSIZE];
		Keytype NowKey;

		int buffernum = bf.GiveMeABlock(name, 0);
		bf.useBlock(buffernum);
		memcpy(currentBlock, bf.bufferBlock[buffernum].values, BLOCKSIZE);
		//readbuffer;
		int LeafType = *(int*)(currentBlock);
		int Brother = *(int*)(currentBlock + MESSAGE + keylength[type]);
		int tempBro, position;

		while (LeafType == Internal)
		{
			int NumOfKeys = *(int*)(currentBlock + 12);
			int i = 0;

			for (i = 0; i < NumOfKeys; i++)
			{
				NowKey = *(Keytype*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH));
				tempBro = Brother;
				position = *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type] + 4);
				Brother = *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type]);
				if (compare(key, NowKey) < 0)
					break;
				else continue;
			}
			if (i == NumOfKeys)
				position = *(int*)(currentBlock + MESSAGE + tempBro*(keylength[type] + POINTERLENGTH) + keylength[type] + 12);
			if (position == -1)
			{
				char* newBlock = new char[BLOCKSIZE];
				*(int*)(newBlock) = Leaf;
				*(int*)(newBlock + 4) = Number++;
				*(int*)(newBlock + 8) = *(int*)(currentBlock + 4);
				*(int*)(newBlock + 12) = 1;
				*(int*)(newBlock + 16) = 0;

				*(int*)(newBlock + MESSAGE + keylength[type]) = *(int*)(newBlock + 12);
				*(Keytype*)(newBlock + MESSAGE + keylength[type] + POINTERLENGTH) = key;
				*(int*)(newBlock + MESSAGE + keylength[type] + POINTERLENGTH + keylength[type]) = -1;
				*(int*)(newBlock + MESSAGE + keylength[type] + POINTERLENGTH + keylength[type] + 4) = Addr;
				*(int*)(newBlock + MESSAGE + keylength[type] + POINTERLENGTH + keylength[type] + 8) = 1;
				*(int*)(newBlock + MESSAGE + (keylength[type] + POINTERLENGTH) + keylength[type] + 16) = 0;//delete or not
				position = Number - 1;
				int buffernum = bf.GiveMeABlock(name, position);
				memcpy(bf.bufferBlock[buffernum].values, newBlock, BLOCKSIZE);
				bf.writeBlock(buffernum);
				bf.flashBack(buffernum);

				*(int*)(currentBlock + MESSAGE + tempBro*(keylength[type] + POINTERLENGTH) + keylength[type] + 4) = Number - 1;
				buffernum = bf.GiveMeABlock(name, *(int*)(currentBlock + 4));
				memcpy(bf.bufferBlock[buffernum].values, currentBlock, BLOCKSIZE);
				bf.writeBlock(buffernum);
				bf.flashBack(buffernum);
				//writebuffer
				return;
			}
			int buffernum = bf.GiveMeABlock(name, position);
			bf.useBlock(buffernum);
			memcpy(currentBlock, bf.bufferBlock[buffernum].values, BLOCKSIZE);
			LeafType = *(int*)(currentBlock);
			Brother = *(int*)(currentBlock + MESSAGE + keylength[type]);
			//readbuffer;
		}

		int NumOfKeys = *(int*)(currentBlock + 12);
		Brother = *(int*)(currentBlock + MESSAGE + keylength[type]);
		if (NumOfKeys < maxchild)
		{
			*(int*)(currentBlock + 12) += 1;
			NumOfKeys++;
			int LastBro = 0;
			int i = 0;

			for (i = 0; i < NumOfKeys - 1; i++)
			{
				NowKey = *(Keytype*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH));
				if (compare(key, NowKey) < 0)
				{
					*(Keytype*)(currentBlock + MESSAGE + NumOfKeys*(keylength[type] + POINTERLENGTH)) = key;
					*(int*)(currentBlock + MESSAGE + NumOfKeys*(keylength[type] + POINTERLENGTH) + keylength[type]) =
						*(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type] + 8);
					*(int*)(currentBlock + MESSAGE + NumOfKeys*(keylength[type] + POINTERLENGTH) + keylength[type] + 8) =
						NumOfKeys;
					*(int*)(currentBlock + MESSAGE + NumOfKeys*(keylength[type] + POINTERLENGTH) + keylength[type] + 4) =
						Addr;
					*(int*)(currentBlock + MESSAGE + LastBro*(keylength[type] + POINTERLENGTH) + keylength[type]) =
						NumOfKeys;
					break;
				}
				LastBro = Brother;
				Brother = *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type]);
			}
			if (i == NumOfKeys-1)
			{
				*(Keytype*)(currentBlock + MESSAGE + NumOfKeys*(keylength[type] + POINTERLENGTH)) = key;
				*(int*)(currentBlock + MESSAGE + NumOfKeys*(keylength[type] + POINTERLENGTH) + keylength[type]) = -1;
				*(int*)(currentBlock + MESSAGE + NumOfKeys*(keylength[type] + POINTERLENGTH) + keylength[type] + 8) =
					NumOfKeys;
				*(int*)(currentBlock + MESSAGE + NumOfKeys*(keylength[type] + POINTERLENGTH) + keylength[type] + 4) =
					Addr;
				*(int*)(currentBlock + MESSAGE + LastBro*(keylength[type] + POINTERLENGTH) + keylength[type]) =
					NumOfKeys;
			}
			int buffernum = bf.GiveMeABlock(name, *(int*)(currentBlock + 4));
			memcpy(bf.bufferBlock[buffernum].values, currentBlock, BLOCKSIZE);
			bf.writeBlock(buffernum);
			bf.flashBack(buffernum);
			//writebuffer
		}
		else
		{
			Keytype mid = Middle(currentBlock,key);
			split(currentBlock, mid, key, Addr, 0, 0);
		}

		delete[]currentBlock;
	}

	template <class Keytype>
	void split(char* currentBlock, Keytype mid, Keytype key, int Addr, int leftpos, int rightpos)
	{
		int NumOfKeys = *(int*)(currentBlock + 12);
		int LeafType = *(int*)(currentBlock);
		if ((*(int*)(currentBlock + 8) == -1) && NumOfKeys >= maxchild - 1)
		{
			char* newBlock1 = new char[BLOCKSIZE];
			char* newBlock2 = new char[BLOCKSIZE];
			SplitInternal(newBlock1, newBlock2, currentBlock, mid, leftpos, rightpos);
			leftpos = *(int*)(newBlock1 + 4);
			rightpos = *(int*)(newBlock2 + 4);

			mid = *(Keytype*)(newBlock2 + MESSAGE + keylength[type] + POINTERLENGTH + keylength[type]);
			int buffernum = bf.GiveMeABlock(name, *(int*)(newBlock1 + 4));
			memcpy(bf.bufferBlock[buffernum].values, newBlock1, BLOCKSIZE);
			bf.writeBlock(buffernum);
			bf.flashBack(buffernum);
			//writebuffer
			buffernum = bf.GiveMeABlock(name, *(int*)(newBlock2 + 4));
			memcpy(bf.bufferBlock[buffernum].values, newBlock2, BLOCKSIZE);
			bf.writeBlock(buffernum);
			bf.flashBack(buffernum);
			//writebuffer
			char* root = new char[BLOCKSIZE];
			*(int*)(root) = Internal;
			*(int*)(root + 4) = 0;//node position
			*(int*)(root + 8) = -1;//father node position
			*(int*)(root + 12) = 1;//number of keys
			*(int*)(root + 16) = 0;//delete or not

			int NumOfKeys = *(int*)(root + 12);
			*(int*)(root + MESSAGE + keylength[type]) = NumOfKeys;
			*(Keytype*)(root + MESSAGE + NumOfKeys*(keylength[type] + POINTERLENGTH)) = mid;
			*(int*)(root + MESSAGE + NumOfKeys*(keylength[type] + POINTERLENGTH) + keylength[type]) = -1;
			*(int*)(root + MESSAGE + NumOfKeys*(keylength[type] + POINTERLENGTH) + keylength[type] + 8) = NumOfKeys;
			*(int*)(root + MESSAGE + NumOfKeys*(keylength[type] + POINTERLENGTH) + keylength[type] + 4) = leftpos;
			*(int*)(root + MESSAGE + NumOfKeys*(keylength[type] + POINTERLENGTH) + keylength[type] + 12) = rightpos;
			*(int*)(root + MESSAGE + NumOfKeys*(keylength[type] + POINTERLENGTH) + keylength[type] + 16) = 0;//delete or not

			buffernum = bf.GiveMeABlock(name, *(int*)(root + 4));
			memcpy(bf.bufferBlock[buffernum].values, root, BLOCKSIZE);
			bf.writeBlock(buffernum);
			bf.flashBack(buffernum);
			//writebuffer;root
			delete[]root;
			delete[]newBlock1;
			delete[]newBlock2;
			return;
		}
		else
		{
			if (LeafType == Leaf&&NumOfKeys >= maxchild)
			{
				char* newBlock1 = new char[BLOCKSIZE];
				char* newBlock2 = new char[BLOCKSIZE];
				SplitLeaf(newBlock1, newBlock2, currentBlock, key, Addr);
				*(int*)(currentBlock + 16) = 1;
				leftpos = *(int*)(newBlock1 + 4);
				rightpos = *(int*)(newBlock2 + 4);

				int buffernum = bf.GiveMeABlock(name, *(int*)(newBlock1 + 4));
				memcpy(bf.bufferBlock[buffernum].values, newBlock1, BLOCKSIZE);
				bf.writeBlock(buffernum);
				bf.flashBack(buffernum);
				//writebuffer
				buffernum = bf.GiveMeABlock(name, *(int*)(newBlock2 + 4));
				memcpy(bf.bufferBlock[buffernum].values, newBlock2, BLOCKSIZE);
				bf.writeBlock(buffernum);
				bf.flashBack(buffernum);
				//writebuffer
				buffernum = bf.GiveMeABlock(name, *(int*)(currentBlock + 4));
				memcpy(bf.bufferBlock[buffernum].values, currentBlock, BLOCKSIZE);
				bf.writeBlock(buffernum);
				bf.flashBack(buffernum);
				//writebuffer

				buffernum = bf.GiveMeABlock(name, *(int*)(currentBlock + 8));
				bf.useBlock(buffernum);
				memcpy(fatherBlock, bf.bufferBlock[buffernum].values, BLOCKSIZE);
				//readbuffer;

				split(fatherBlock, mid, key, Addr, leftpos, rightpos);
				delete[]newBlock1;
				delete[]newBlock2;
			}
			else if (LeafType == Internal&&NumOfKeys >= maxchild - 1)
			{
				char*newBlock1 = new char[BLOCKSIZE];
				char*newBlock2 = new char[BLOCKSIZE];
				SplitInternal(newBlock1, newBlock2, currentBlock, mid, leftpos, rightpos);
				leftpos = *(int*)(newBlock1 + 4);
				rightpos = *(int*)(newBlock2 + 4);
				*(int*)(currentBlock + 16) = 1;

				mid = *(Keytype*)(newBlock2 + MESSAGE + keylength[type] + POINTERLENGTH + keylength[type]);
				int buffernum = bf.GiveMeABlock(name, *(int*)(newBlock1 + 4));
				memcpy(bf.bufferBlock[buffernum].values, newBlock1, BLOCKSIZE);
				bf.writeBlock(buffernum);
				bf.flashBack(buffernum);
				//writebuffer
				buffernum = bf.GiveMeABlock(name, *(int*)(newBlock2 + 4));
				memcpy(bf.bufferBlock[buffernum].values, newBlock2, BLOCKSIZE);
				bf.writeBlock(buffernum);
				bf.flashBack(buffernum);
				//writebuffer
				buffernum = bf.GiveMeABlock(name, *(int*)(currentBlock + 4));
				memcpy(bf.bufferBlock[buffernum].values, currentBlock, BLOCKSIZE);
				bf.writeBlock(buffernum);
				bf.flashBack(buffernum);
				//writebuffer

				buffernum = bf.GiveMeABlock(name, *(int*)(currentBlock + 8));
				bf.useBlock(buffernum);
				memcpy(fatherBlock, bf.bufferBlock[buffernum].values, BLOCKSIZE);
				//readbuffer;
				split(fatherBlock, mid, key, Addr, leftpos, rightpos);
				delete[]newBlock1;
				delete[]newBlock2;
			}
			else if (LeafType == Internal&&NumOfKeys < maxchild - 1)
			{
				*(int*)(currentBlock + 12) += 1;

				Internal_insert(currentBlock, mid, leftpos, rightpos);//insert into internal

				int buffernum = bf.GiveMeABlock(name, *(int*)(currentBlock + 4));
				memcpy(bf.bufferBlock[buffernum].values, currentBlock, BLOCKSIZE);
				bf.writeBlock(buffernum);
				bf.flashBack(buffernum);
				//writebuffer
			}
		}
	}

	template <class Keytype>
	void Internal_insert(char* currentBlock, Keytype mid, int leftpos, int rightpos)
	{
		int NumOfKeys = *(int*)(currentBlock + 12);
		int Brother = *(int*)(currentBlock + MESSAGE + keylength[type]);
		int LastBro = 0;
		int i = 0;

		for (i = 0; i < NumOfKeys - 1; i++)
		{
			Keytype NowKey = *(Keytype*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH));
			if (compare(mid, NowKey) < 0)
			{
				*(Keytype*)(currentBlock + MESSAGE + NumOfKeys*(keylength[type] + POINTERLENGTH)) = mid;
				*(int*)(currentBlock + MESSAGE + NumOfKeys*(keylength[type] + POINTERLENGTH) + keylength[type]) = Brother;
				*(int*)(currentBlock + MESSAGE + NumOfKeys*(keylength[type] + POINTERLENGTH) + keylength[type] + 4) = leftpos;
				*(int*)(currentBlock + MESSAGE + NumOfKeys*(keylength[type] + POINTERLENGTH) + keylength[type] + 8) = NumOfKeys;
				*(int*)(currentBlock + MESSAGE + NumOfKeys*(keylength[type] + POINTERLENGTH) + keylength[type] + 12) = rightpos;
				*(int*)(currentBlock + MESSAGE + LastBro*(keylength[type] + POINTERLENGTH) + keylength[type]) = NumOfKeys;
				*(int*)(currentBlock + MESSAGE + LastBro*(keylength[type] + POINTERLENGTH) + keylength[type] + 12) = leftpos;
				*(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type] + 4) = rightpos;
				break;
			}
			LastBro = Brother;
			Brother = *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type]);
		}
		if (i == NumOfKeys-1)
		{
			*(Keytype*)(currentBlock + MESSAGE + NumOfKeys*(keylength[type] + POINTERLENGTH)) = mid;
			*(int*)(currentBlock + MESSAGE + NumOfKeys*(keylength[type] + POINTERLENGTH) + keylength[type]) = -1;
			*(int*)(currentBlock + MESSAGE + NumOfKeys*(keylength[type] + POINTERLENGTH) + keylength[type] + 4) = leftpos;
			*(int*)(currentBlock + MESSAGE + NumOfKeys*(keylength[type] + POINTERLENGTH) + keylength[type] + 8) = NumOfKeys;
			*(int*)(currentBlock + MESSAGE + NumOfKeys*(keylength[type] + POINTERLENGTH) + keylength[type] + 12) = rightpos;
			*(int*)(currentBlock + MESSAGE + LastBro*(keylength[type] + POINTERLENGTH) + keylength[type]) = NumOfKeys;
			*(int*)(currentBlock + MESSAGE + LastBro*(keylength[type] + POINTERLENGTH) + keylength[type] + 12) = leftpos;
		}
	}

	template <class Keytype>
	Keytype Middle(char* currentBlock,Keytype key)
	{
		int NumOfKeys = *(int*)(currentBlock + 12);
		int Brother = *(int*)(currentBlock + MESSAGE + keylength[type]);
		Keytype mid;
		int fin = (NumOfKeys + 1) / 2 + 1;
		mid = *(Keytype*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH));
		for (int i = 0; i < fin; i++)
		{
			if (compare(key,mid)>0)
			{
				mid = *(Keytype*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH));
				Brother = *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type]);
			}
			else mid = key;
		}

		return mid;
	}

	template <class Keytype>
	void SplitLeaf(char* block1, char*block2, char* currentBlock, Keytype key, int Addr)
	{
		int NumOfKeys = *(int*)(currentBlock + 12);
		int Brother = *(int*)(currentBlock + MESSAGE + keylength[type]);
		int Address, flag = 1;
		Keytype key1;
		int i, position = 1;

		NumOfKeys++;
		*(int*)(block1) = Leaf;
		*(int*)(block1 + 4) = Number++;
		*(int*)(block1 + 8) = *(int*)(currentBlock + 8);
		*(int*)(block1 + 12) = NumOfKeys / 2;
		*(int*)(block1 + 16) = 0;

		*(int*)(block2) = Leaf;
		*(int*)(block2 + 4) = Number++;
		*(int*)(block2 + 8) = *(int*)(currentBlock + 8);
		*(int*)(block2 + 12) = NumOfKeys - (int)(NumOfKeys / 2);
		*(int*)(block2 + 16) = 0;

		*(int*)(block1 + MESSAGE + keylength[type]) = 1;
		*(int*)(block2 + MESSAGE + keylength[type]) = 1;

		for (i = 0; i < NumOfKeys / 2; i++)
		{
			key1 = *(Keytype*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH));
			Address = *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type] + 4);
			if (compare(key, key1) < 0 && flag)
			{
				*(Keytype*)(block1 + MESSAGE + position*(keylength[type] + POINTERLENGTH)) = key;
				*(int*)(block1 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type]) = position + 1;
				*(int*)(block1 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type] + 4) = Addr;
				*(int*)(block1 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type] + 8) = position++;
				*(int*)(block1 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type] + 16) = 0;//delete or not
				flag = 0;
			}
			else
			{
				*(Keytype*)(block1 + MESSAGE + position*(keylength[type] + POINTERLENGTH)) = key1;
				*(int*)(block1 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type]) = position + 1;
				*(int*)(block1 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type] + 4) = Address;
				*(int*)(block1 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type] + 8) = position++;
				*(int*)(block1 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type] + 16) = 0;//delete or not
				Brother = *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type]);
			}
		}
		*(int*)(block1 + MESSAGE + (position - 1)*(keylength[type] + POINTERLENGTH) + keylength[type]) = -1;

		position = 1;
		flag = 1;
		for (; i < NumOfKeys-1; i++)
		{
			key1 = *(Keytype*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH));
			Address = *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type] + 4);
			if (compare(key, key1) < 0 && flag)
			{
				*(Keytype*)(block2 + MESSAGE + position*(keylength[type] + POINTERLENGTH)) = key;
				*(int*)(block2 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type]) = position + 1;
				*(int*)(block2 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type] + 4) = Addr;
				*(int*)(block2 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type] + 8) = position++;
				*(int*)(block2 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type] + 16) = 0;//delete or not
				flag = 0;
			}
			else
			{
				*(Keytype*)(block2 + MESSAGE + position*(keylength[type] + POINTERLENGTH)) = key1;
				*(int*)(block2 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type]) = position + 1;
				*(int*)(block2 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type] + 4) = Address;
				*(int*)(block2 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type] + 8) = position++;
				*(int*)(block2 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type] + 16) = 0;//delete or not
				Brother = *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type]);
			}
		}
		if (flag)
		{
			*(Keytype*)(block2 + MESSAGE + position*(keylength[type] + POINTERLENGTH)) = key;
			*(int*)(block2 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type]) = -1;
			*(int*)(block2 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type] + 4) = Addr;
			*(int*)(block2 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type] + 16) = 0;//delete or not
			*(int*)(block2 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type] + 8) = position++;
		}
		*(int*)(block2 + MESSAGE + (position - 1)*(keylength[type] + POINTERLENGTH) + keylength[type]) = -1;
	}

	template <class Keytype>
	void SplitInternal(char* block1, char*block2, char* currentBlock, Keytype mid, int leftpos, int rightpos)
	{
		*(int*)(currentBlock + 12)+=1;
		int NumOfKeys = *(int*)(currentBlock + 12);
		int Brother = *(int*)(currentBlock + MESSAGE + keylength[type]);
		int Addr1, Addr2;
		Keytype key;
		int i, position = 1, flag = 1, LastBro = 0;

		*(int*)(block1) = Internal;
		*(int*)(block1 + 4) = Number++;
		*(int*)(block1 + 8) = *(int*)(currentBlock + 8);
		*(int*)(block1 + 12) = NumOfKeys / 2;
		*(int*)(block1 + 16) = 0;

		*(int*)(block2) = Internal;
		*(int*)(block2 + 4) = Number++;
		*(int*)(block2 + 8) = *(int*)(currentBlock + 8);
		*(int*)(block2 + 12) = NumOfKeys - (int)(NumOfKeys / 2);
		*(int*)(block2 + 16) = 0;

		*(int*)(block1 + MESSAGE + keylength[type]) = 1;
		*(int*)(block2 + MESSAGE + keylength[type]) = 1;

		for (i = 0; i < NumOfKeys / 2; i++)
		{
			key = *(Keytype*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH));
			Addr1 = *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type] + 4);
			Addr2 = *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type] + 12);
			if (compare(mid, key) < 0 && flag)
			{
				*(Keytype*)(block1 + MESSAGE + position*(keylength[type] + POINTERLENGTH)) = mid;
				*(int*)(block1 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type]) = position + 1;
				*(int*)(block1 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type] + 4) = leftpos;
				*(int*)(block1 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type] + 8) = position++;
				*(int*)(block1 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type] + 12) = rightpos;
				*(int*)(block1 + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type] + 4) = rightpos;
				*(int*)(block1 + MESSAGE + LastBro*(keylength[type] + POINTERLENGTH) + keylength[type] + 12) = leftpos;
				*(int*)(block1 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type] + 16) = 0;//delete or not
				flag = 0;
			}
			else
			{
				*(Keytype*)(block1 + MESSAGE + position*(keylength[type] + POINTERLENGTH)) = key;
				*(int*)(block1 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type]) = position + 1;
				*(int*)(block1 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type] + 4) = Addr1;
				*(int*)(block1 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type] + 8) = position++;
				*(int*)(block1 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type] + 12) = Addr2;
				*(int*)(block1 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type] + 16) = 0;//delete or not
				LastBro = Brother;
				Brother = *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type]);
			}
		}

		position = 1;
		flag = 1;
		for (; i < NumOfKeys-1; i++)
		{
			key = *(Keytype*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH));
			Addr1 = *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type] + 4);
			Addr2 = *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type] + 12);
			if (compare(mid, key) < 0 && flag)
			{
				*(Keytype*)(block2 + MESSAGE + position*(keylength[type] + POINTERLENGTH)) = mid;
				*(int*)(block2 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type]) = position + 1;
				*(int*)(block2 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type] + 4) = Addr1;
				*(int*)(block2 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type] + 8) = position++;
				*(int*)(block2 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type] + 12) = Addr2;
				*(int*)(block2 + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type] + 4) = rightpos;
				*(int*)(block2 + MESSAGE + LastBro*(keylength[type] + POINTERLENGTH) + keylength[type] + 12) = leftpos;
				*(int*)(block2 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type] + 16) = 0;//delete or not
				flag = 0;
			}
			else
			{
				*(Keytype*)(block2 + MESSAGE + position*(keylength[type] + POINTERLENGTH)) = key;
				*(int*)(block2 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type]) = position + 1;
				*(int*)(block2 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type] + 4) = leftpos;
				*(int*)(block2 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type] + 8) = position++;
				*(int*)(block2 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type] + 12) = rightpos;
				*(int*)(block2 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type] + 16) = 0;//delete or not
				LastBro = Brother;
				Brother = *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type]);
			}
		}
		if (flag)
		{
			*(Keytype*)(block2 + MESSAGE + position*(keylength[type] + POINTERLENGTH)) = key;
			*(int*)(block2 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type]) = -1;
			*(int*)(block2 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type] + 4) = Addr1;
			*(int*)(block2 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type] + 8) = position++;
			*(int*)(block2 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type] + 16) = 0;//delete or not
			*(int*)(block2 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type] + 12) = Addr2;
		}
	}

	template<class Keytype>
	void Delete(Keytype key)
	{
		char *currentBlock = new char[BLOCKSIZE];
		Keytype NowKey;

		int buffernum = bf.GiveMeABlock(name, 0);
		bf.useBlock(buffernum);
		memcpy(currentBlock, bf.bufferBlock[buffernum].values, BLOCKSIZE);
		//readbuffer;
		int LeafType = *(int*)(currentBlock);
		int Brother = *(int*)(currentBlock + MESSAGE + keylength[type]);
		int tempBro, position;

		while (LeafType == Internal)
		{
			int NumOfKeys = *(int*)(currentBlock + 12);
			int i = 0;
			for (i = 0; i < NumOfKeys; i++)
			{
				NowKey = *(Keytype*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH));
				tempBro = Brother;
				position = *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type] + 4);
				Brother = *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type]);
				if (compare(key, NowKey) < 0)
					break;
				else continue;
			}
			if (i == NumOfKeys)
				position = *(int*)(currentBlock + MESSAGE + tempBro*(keylength[type] + POINTERLENGTH) + keylength[type] + 12);
			int buffernum = bf.GiveMeABlock(name, position);
			bf.useBlock(buffernum);
			memcpy(currentBlock, bf.bufferBlock[buffernum].values, BLOCKSIZE);
			LeafType = *(int*)(currentBlock);
			Brother = *(int*)(currentBlock + MESSAGE + keylength[type]);
			//readbuffer
		}

		int NumOfKeys = *(int*)(currentBlock + 12);
		Brother = *(int*)(currentBlock + MESSAGE + keylength[type]);
		for (int i = 0; i < NumOfKeys; i++)
		{
			NowKey = *(Keytype*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH));
			tempBro = Brother;
			int Addr = *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type] + 4);
			Brother = *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type]);
			if (compare(key, NowKey) == 0)
			{
				*(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type] + 16) = 1;
				int buffernum = bf.GiveMeABlock(name, *(int*)(currentBlock + 4));
				memcpy(bf.bufferBlock[buffernum].values, currentBlock, BLOCKSIZE);
				//writebuffer
			}
		}
		delete[]currentBlock;
	}
};

#endif