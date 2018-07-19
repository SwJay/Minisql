#ifndef _CATALOG_MANAGER_
#define _CATALOG_MANAGER_
#include <string>
#include <cstring>
#include <iostream>
#include <utility>
#include <vector>
#include <fstream>
#include <exception>
#include <io.h>
#include "MyClass.h"
#include "BufferManager.h"
using namespace std;

class Attribute;
class Condition;
template <typename key>
class InfoBlock;


struct TableMetadata {
	string firstine;
	vector<string> indexdata;
};

struct Index {
	string tablename;
	string indexname;
};
class CatalogManager {
private:
	BufferManager * bm = MiniSQL::GetBM();
	vector<TableMetadata>* Metadata;
	vector<Index>* index;

public:
	CatalogManager();
	~CatalogManager();
	vector<TableMetadata>* InitMetadata();
	vector<Index>* InitIndex();
	bool IsTable(std::string tablename);
	/*int chechindexwhetherin(std::string tablename, std::string Arrtribute, std::string indexname);*/
	bool checkattributewhetherin(std::string tablename, std::string Arrtribute);
	bool CreateTable(std::string tablename, vector<Attribute*> *attributes);
	bool DropTable(std::string tablename);
	/*
	string primarykeytype(std::string tablename);
	//string getallindex(string tablename);
	*/
	string getArrtribute(std::string tablename);
	string Tabletype(std::string tablename);
	bool IsUnique(std::string tablename, std::string indexname);
	bool IsUnique(const std::string &table, Condition *condition);
	vector<std::string>* IsTableIndexed(std::string table);
	string GivePrimary(std::string tablename);
	string GiveIndex(std::string table, std::string attribute);
	vector<std::string>* GiveUnique(std::string table);
	string GiveAttribute(std::string table, std::string index);
	int AttributeType(std::string table, std::string attribute);
	int AttributeOrder(std::string table, std::string attribute);
	int Attributerank(std::string table, std::string attribute);
	int  Getlength(std::string table);
	vector<int>* GiveTypes(std::string tablename);
	bool deleteIndex(std::string tablename, std::string indexname);
	bool DropIndex(std::string index);
	bool IsIndex(std::string Indexname);
	bool IsIndex(std::string Tablename, std::string Attribute);
	bool AddIndexIntoVagina(std::string tablename, std::string indexname);
	string DeleteIndexFromVagina(std::string indexname);
	vector<InfoBlock<string> *> * CreateIndex(const std::string &index, const std::string &tablename, const std::string &attribute);
	template <typename key>
	std::vector<InfoBlock<key>* >* addIndex(const std::string &tablename, const std::string &Arrtribute, const std::string &indexname);
};
template<typename key>
vector<InfoBlock<key>*>* CatalogManager::addIndex(const string &tablename, const string &Arrtribute, const string &indexname) {//未检测
	for (vector<TableMetadata>::iterator it = Metadata->begin(); it < Metadata->end(); it++) {
		string::size_type newpos;
		newpos = ((*it).firstine).find("-");
		if (((*it).firstine).substr(0, newpos) == tablename) {
			string newtemp = Arrtribute + "-" + indexname;
			((it)->indexdata).push_back(newtemp);
		}
	}
	vector<InfoBlock<key>*> * v;
	v = new vector<InfoBlock<key>*>;
	int blockoffset = 0;
	int fileoffset = 0;

	Block * block[3];
	int length = Getlength(tablename);
	string taildata;

	InfoBlock<key> * in;
	for (int i = 0; i < length; i++) {
		taildata += "-";
	}
	string fuck = "-";
	block[0] = bm->FetchBlockHead(tablename);
	while (block[0]) {
		if (BLOCK_SIZE - blockoffset < length) {
			block[0] = bm->FetchNextBlock(block[0]);
			fileoffset++;
			blockoffset = 0;
		}
		else if (block[0]->ReadBlock(blockoffset, length) == taildata || block[0]->ReadBlock(blockoffset + 4, 1) == fuck) {
			blockoffset += length;
			//cout << "ri";
		}
		else {

			int l = AttributeOrder(tablename, Arrtribute);
			int type = AttributeType(tablename, Arrtribute);
			if (type == 0) {

				int val = block[0]->ReadBlock<int>(blockoffset + l);
				string value = to_string(val);
				in = new InfoBlock<key>(value, fileoffset, blockoffset);
				//in->value = value;
				v->push_back(in);
			}
			else if (type == -1) {
				float val = block[0]->ReadBlock<float>(blockoffset + l);
				string value = to_string(val);
				in = new InfoBlock<string>(value, fileoffset, blockoffset);
				v->push_back(in);
			}
			else {
				string vale = block[0]->ReadBlock(blockoffset + l, type);
				in = new InfoBlock<string>(vale, fileoffset, blockoffset);
				v->push_back(in);
			}
			blockoffset += length;
		}
	}
	return v;
}

#endif