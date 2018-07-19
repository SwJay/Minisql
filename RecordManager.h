#ifndef _RECORD_MANAGER_
#define _RECORD_MANAGER_
#include <string>
#include <iostream>
#include <cstring>
#include <utility>
#include <vector>
#include <fstream>
#include <exception>
#include <io.h>
#include "MyClass.h"
#include "BufferManager.h"
#include "CatalogManager.h"
#include "IndexManager.h"
using namespace std;

class RecordManager {
private:
	BufferManager *bm= MiniSQL::GetBM();
	CatalogManager *cm= MiniSQL::GetCM();
	IndexManager *im= MiniSQL::GetIM();
public:
	bool InsertRecords(string filename, vector<string>*values);
//	template <typename key>
//	bool deleterecord(string filename, vector<InfoBlock<key>>* V);
	template <typename key>
	string selectrecord(string filename, vector<InfoBlock<key>*>* V);
	string SelectRecords(string table, vector<Condition*> *conditions);
	//bool test(string filename);
	string AllRecords(string table);
	void DeleteRecord(string table, Condition* condition);
	template<typename key>
	bool DeleteRecord(string table, vector<InfoBlock<key>*>*Info);
	template <typename key>
	string SelectRecords(string table, vector<InfoBlock<key>*>* info, vector<Condition*> *conditions);
	void CreateTable(const string &table, vector<Attribute*>*attribute);
	void DropTable(const string &table);
};

template<typename key>
bool RecordManager::DeleteRecord(string table, vector<InfoBlock<key>*>*Info) {
	Block* block[3];
    typename vector<InfoBlock<key>*>::const_iterator it;
	for ( it= Info->begin(); it < Info->end(); it++) {
		int fileoffset = (*it)->file_off;
		int blockoffset = (*it)->block_off;
		//cout << fileoffset << "   " << blockoffset << endl;
		block[1] = bm->FetchBlockHead(table);
		block[0] = bm->FetchRandomBlock(table, fileoffset);
		int oldfileoffset = block[0]->ReadBlock<int>(0);
		int oldblockoffset = block[0]->ReadBlock<int>(5);
        int finalfileoffset = fileoffset;
        int finalblockoffset=blockoffset;
        vector<string> *vfuck = cm->IsTableIndexed(table);
        //block[0] = bm->FetchRandomBlock(table, finalfileoffset);
        block[0]->SetPin(true);
        for (vector<string > ::const_iterator it = vfuck->begin(); it < vfuck->end(); it++) {
            string aaa = *it;
            string finalattribute = cm->GiveAttribute(table, aaa);
            //cout << finalattribute << endl;
            int finaltype = cm->AttributeType(table,finalattribute);
            int finalorder = cm->AttributeOrder(table, finalattribute);
            string whatfuck = (*it);
            //cout << cm->GiveIndex(tablename, finalattribute) << endl;
            string finalindex = cm->GiveIndex(table, finalattribute);

            if (finaltype== 0){
                int finalvalue = block[0]->ReadBlock<int>(finalblockoffset + finalorder);
                InfoBlock<int> * v;
                v = new InfoBlock<int>(finalvalue, finalfileoffset, finalblockoffset);
                vector<InfoBlock<int>*> vv;
                vv.push_back(v);
//            cout << finalvalue << endl;
//            cout << finalfileoffset << endl;
//            cout << finalblockoffset << endl;
                im->Delete<int>(0, finalindex, v->value, v->value, &vv);
                delete v;
            }
            else if (finaltype == -1) {
                float finalvalue = block[0]->ReadBlock<float>(finalblockoffset + finalorder);
                InfoBlock<float> * v;
                v = new InfoBlock<float>(finalvalue, finalfileoffset, finalblockoffset);
                vector<InfoBlock<float>*> vv;
                vv.push_back(v);
                //cout << finalvalue << endl;
                //cout << finalfileoffset << endl;
                //cout << finalblockoffset << endl;
                im->Delete<float>(0, finalindex, v->value, v->value, &vv);
                delete v;
            }
            else {
                string finalvalue = block[0]->ReadBlock(finalblockoffset + finalorder,finaltype);
                InfoBlock<string> * v;
                v = new InfoBlock<string>(finalvalue, finalfileoffset, finalblockoffset);
                vector<InfoBlock<string>*> vv;
                vv.push_back(v);
                //cout << finalvalue << endl;
                //cout << finalfileoffset << endl;
                //cout << finalblockoffset << endl;
                im->Delete<string>(0, finalindex, v->value, v->value, &vv);
                delete v;
            }
        }
		//cout << oldfileoffset << "old" << oldblockoffset << endl;
		block[1]->WriteBlock(0, fileoffset);
		block[1]->WriteBlock(5, blockoffset);
		block[0]->WriteBlock(blockoffset, oldfileoffset);
		block[0]->WriteBlock(blockoffset + 4, (char*)"-",1);
		block[0]->WriteBlock(blockoffset + 5, oldblockoffset);
		block[0]->SetPin(false);
	}
	//cout << block[0]->ReadBlock<int>(0) << block[0]->ReadBlock<int>(5) << block[0]->ReadBlock<int>(9) << block[0]->ReadBlock<int>(14) << block[1]->ReadBlock<int>(0) << block[1]->ReadBlock<int>(5) << block[1]->ReadBlock<int>(9) << block[1]->ReadBlock<int>(14) << endl;
	return true;
}

template <typename key>
std::string RecordManager::SelectRecords(std::string table, std::vector<InfoBlock<key>*>* info, std::vector<Condition*> *conditions) {//未测
    Block * block[2];
    string result;
    typename vector<InfoBlock<key>*>::const_iterator oit;
    for (oit = info->begin(); oit < info->end(); oit++) {
        int blockoffset = (*oit)->block_off;
        int fileoffset = (*oit)->file_off;
        block[0] = bm->FetchRandomBlock(table, fileoffset);
        int flag = 1;
        int order;
        for (vector<Condition*>::const_iterator it = conditions->begin(); it < conditions->end(); it++) {
            order = cm->AttributeOrder(table, (*it)->attribute_name);
            if ((*it)->type == 0) {
                int rvalue = atoi(((*it)->value).c_str());
                int v = block[0]->ReadBlock<int>(blockoffset);
                if ((*it)->operate == 0) {
                    if (v != rvalue) {
                        flag = 0;
                        break;
                    }
                }
                else if ((*it)->operate == -1) {
                    if (v == rvalue) {
                        flag = 0;
                        break;
                    }
                }
                else if ((*it)->operate == 2) {
                    if (v <= rvalue) {
                        flag = 0;
                        break;
                    }
                }
                else if ((*it)->operate == 3) {
                    if (v < rvalue) {
                        flag = 0;
                        break;
                    }
                }
                else if ((*it)->operate == 4) {
                    if (v >= rvalue) {
                        flag = 0;
                        break;
                    }
                }
                else if ((*it)->operate == 5) {
                    if (v > rvalue) {
                        flag = 0;
                        break;
                    }
                }
            }
            else if ((*it)->type == -1) {
                float rvalue = atof(((*it)->value).c_str());
                float v = block[0]->ReadBlock<float>(blockoffset);
                if ((*it)->operate == 0) {
                    if (v != rvalue) {
                        flag = 0;
                        break;
                    }
                }
                else if ((*it)->operate == -1) {
                    if (v == rvalue) {
                        flag = 0;
                        break;
                    }
                }
                else if ((*it)->operate == 2) {
                    if (v <= rvalue) {
                        flag = 0;
                        break;
                    }
                }
                else if ((*it)->operate == 3) {
                    if (v < rvalue) {
                        flag = 0;
                        break;
                    }
                }
                else if ((*it)->operate == 4) {
                    if (v >= rvalue) {
                        flag = 0;
                        break;
                    }
                }
                else if ((*it)->operate == 5) {
                    if (v > rvalue) {
                        flag = 0;
                        break;
                    }
                }
            }
            else {
                string rvalue = ((*it)->value).c_str();
                string v = block[0]->ReadBlock(blockoffset + order, cm->AttributeType(table, (*it)->attribute_name));
                if ((*it)->operate == 0) {
                    if (v != rvalue) {
                        flag = 0;
                        break;
                    }
                }
                else if ((*it)->operate == -1) {
                    if (v == rvalue) {
                        flag = 0;
                        break;
                    }
                }
                else if ((*it)->operate == 2) {
                    if (v <= rvalue) {
                        flag = 0;
                        break;
                    }
                }
                else if ((*it)->operate == 3) {
                    if (v < rvalue) {
                        flag = 0;
                        break;
                    }
                }
                else if ((*it)->operate == 4) {
                    if (v >= rvalue) {
                        flag = 0;
                        break;
                    }
                }
                else if ((*it)->operate == 5) {
                    if (v > rvalue) {
                        flag = 0;
                        break;
                    }
                }
            }
        }
        if (flag == 1) {
            string data = cm->Tabletype(table);
            string::size_type pos1, pos2;
            pos1 = 0;
            pos2 = data.find('-');
            while (std::string::npos != pos2) {
                string temp = data.substr(pos1, pos2 - pos1);
                if (temp == "int") {
                    result += (to_string(block[0]->ReadBlock<int>(blockoffset)) + "\t" + " | ");
                    blockoffset += 4;
                }
                else if (temp == "float") {
                    result += (to_string(block[0]->ReadBlock<float>(blockoffset)) + "\t" + " | ");
                    blockoffset += 4;
                }
                else {
                    int l = atoi(temp.c_str());
                    result += (block[0]->ReadBlock(blockoffset, l)) + "\t" + " | ";
                    blockoffset += l;
                }
                pos1 = pos2 + 1;
                pos2 = data.find('-', pos1);
            }
            result += "\n";
        }
    }
    return result;
}

//template <typename key>//还没检测
//bool RecordManager::deleterecord(string filename, vector<InfoBlock<key>* >*V){
//	for (vector<string>::const_iterator it = V->begin(); it<V->end(); ++it) {
//		int fileoffset = (*it)->file_off;
//		int blockoffset = (*it)->block_off;
//		Block *block[3];
//		block[0] = bm->FetchBlockHead(filename);
//		int oldfileoffset = block[0]->ReadBlock<int>(0);
//		int oldblockoffset = block[0]->ReadBlock<int>(5);
//		block[0]->WriteBlock(0, fileoffset);
//		block[0]->WriteBlock(5, blockoffset);
//		block[1] = bm->FetchRandomBlock(filename, fileoffset);
//		block[1]->WriteBlock(blockoffset, oldfileoffset);
//		block[1]->WriteBlock(blockoffset + 4, (char *)"-", 1);
//		block[1]->WriteBlock(blockoffset + 5, oldblockoffset);
//
//	}
//}

template <typename key> 
string RecordManager::selectrecord(string filename, vector<InfoBlock<key>* > *V) {
	string result ;
    typename vector<InfoBlock<key>*>::const_iterator it;
	for (it = V->begin(); it<V->end(); ++it) {
		int fileoffset = (*it)->file_off;
		int blockoffset = (*it)->block_off;
		Block *block[3];
		block[0] = bm->FetchRandomBlock(filename,fileoffset);
		string data = cm->Tabletype(filename);
		string::size_type pos1, pos2;
		pos1 = 0;
		pos2 = data.find('-');
		while (string::npos != pos2) {
			string temp = data.substr(pos1, pos2 - pos1);
			if (temp == "int") {
				result += to_string(block[0]->ReadBlock<int>(blockoffset))+"|";
				blockoffset += 4;
			}
			else if (temp == "float") {
				result += to_string(block[0]->ReadBlock<float>(blockoffset)) + "|";
			}
			else {
				int l = atoi((temp).c_str());
				result += block[0]->ReadBlock(blockoffset, l)+"|";
				blockoffset += l;
			}
			pos1 = pos2 + 1;
			pos2 = data.find('-', pos1);
		}
		result += "\n";
	}
	return result;
}

#endif