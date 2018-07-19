#ifndef _B_PLUS_TREE_
#define _B_PLUS_TREE_
#include "MyClass.h"
#include "BufferManager.h"
#include "CatalogManager.h"
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <sstream>

using namespace std;

#define FLOAT (-1)
#define INT 0

#define EQUAL 0
#define UNEQUAL (-1)
#define GREATER 2
#define GREATER_E 3
#define LESS 4
#define LESS_E 5

template <typename key>
class BxNode;

class BxTree{
private:
	string ind_name;
	Block* block;
	int FirstEmpty;
	int Root;
	//int FirstLeaf; always the 1st block
	int level;
	int Node_num;
	int Key_num;
	int FullNodeNum;
	int key_type;
	int count;//record the offset
	BufferManager* BM= MiniSQL::GetBM();
	CatalogManager* CM= MiniSQL::GetCM();
public:
    explicit BxTree(const string &ind_name);
	BxTree(const string &ind_name, const string &table_name, const string &attr_name, int FullNodeNum, int key_type);
	~BxTree();
	string GetIndexName(){return ind_name;};
	int GetKeyNum(){return Key_num;};
	template <typename key>
    void CreateIndex(const string& ind_name, const string& table_name, const string& attr_name);
	void Drop();
	template <typename key>
	vector<InfoBlock<key>* >* Find(int f_type, key lvalue, key rvalue, vector<InfoBlock<key>* >* Query_list);
	template <typename key>
	bool Insert(InfoBlock<key>* loc);
	template <typename key>
	bool Delete(key value);
	template <typename key>
	void Print();
private:
	//int min(int a, int b){return (a<b)?a:b};
	template <typename key>
	bool AfterInsert(BxNode<key>* target);
	template <typename key>
	bool AfterDelete(BxNode<key>* target);
	int GetNewID();
	bool EraseID(int ID);
};

template <typename key>
class BxNode{
    friend class BxTree;
private:
	string ind_name;
    bool type;//internal or leaf
    int ID;
    int now_use;//the key count
    int Parent;
    int LastBrother;
    int NextBrother;
    int key_type;//judge the data type
    int count;
    int FullNodeNum;
    vector<key> key_list;//internal
    vector<int> child_list;//internal
    vector<InfoBlock<key>* > ptr_list;//leaf used
	BufferManager* BM= MiniSQL::GetBM();
public:
    BxNode(Block* block, int key_type, string ind_name, int FullNodeNum);
    BxNode(bool type, int Parent, int key_type, string ind_name, int ID, int FullNodeNum):type(type),Parent(Parent),key_type(key_type),ind_name(std::move(ind_name)),ID(ID),FullNodeNum(FullNodeNum),now_use(0),LastBrother(0),NextBrother(0){};
    ~BxNode();
    int FindChild(key value);//internal used
    vector<InfoBlock<key>* >* FindKey(int f_type, key lvalue, key rvalue, vector<InfoBlock<key>* >* Query_list);//leaf used
    bool InsertKey(key value, InfoBlock<key>* loc);//leaf used
    bool DeleteKey(key value);
    BxNode* Split(int new_ID);
};

template <typename key>
BxNode<key>::BxNode(Block* block, int key_type, string ind_name, int FullNodeNum):key_type(key_type),ind_name(ind_name),FullNodeNum(FullNodeNum){
	count=0;
	type= block->ReadBlock<bool>(count);
	count+= sizeof(bool);
	ID= block->ReadBlock<int>(count);
	count+= sizeof(int);
	now_use= block->ReadBlock<int>(count);
	count+= sizeof(int);
	Parent= block->ReadBlock<int>(count);
	count+= sizeof(int);
	LastBrother= block->ReadBlock<int>(count);
	count+= sizeof(int);
	NextBrother= block->ReadBlock<int>(count);
	count+= sizeof(int);
	if(type){//leaf
		count+= sizeof(int);
		if(key_type <= 0){
			for(int i=0; i<now_use; i++){
				key value= block->ReadBlock<key>(count);
				count+= sizeof(key);
				int file_off= block->ReadBlock<int>(count);
				count+= sizeof(int);
				int block_off= block->ReadBlock<int>(count);
				count+= sizeof(int);
				InfoBlock<key>* temp= new InfoBlock<key>(value, file_off, block_off);
				ptr_list.push_back(temp);
				key_list.push_back(value);
			}
		}
		else{//string
			for(int i=0; i<now_use; i++){
				key value;
				string value_s= block->ReadBlock(count, key_type);
				stringstream ss;
				ss << value_s;
				ss >> value;
				count+= key_type;
				int file_off= block->ReadBlock<int>(count);
				count+= sizeof(int);
				int block_off= block->ReadBlock<int>(count);
				count+= sizeof(int);
				InfoBlock<key>* temp= new InfoBlock<key>(value, file_off, block_off);
				ptr_list.push_back(temp);
				key_list.push_back(value);
			}
		}
	}
	else{//internal
		child_list.push_back(block->ReadBlock<int>(count));
		count+= sizeof(int);
		if(key_type <= 0){
			for(int i=0; i<now_use; i++){
				key_list.push_back(block->ReadBlock<key>(count));
				count+= sizeof(key);
				child_list.push_back(block->ReadBlock<int>(count));
				count+= 2*sizeof(int);
			}
		}
		else{//string
			for(int i=0; i<now_use; i++){
				key value;
			    string value_s= block->ReadBlock(count, key_type);
			    stringstream ss;
			    ss << value_s;
			    ss >> value;
				key_list.push_back(value);
				count+= key_type;
				child_list.push_back(block->ReadBlock<int>(count));
				count+= 2*sizeof(int);
			}
		}
	}
}

template <typename key>
BxNode<key>::~BxNode(){
	Block* block= BM->FetchRandomBlock(ind_name, ID);
	count=0;
	block->WriteBlock<bool>(count, type);
	count+= sizeof(bool);
	block->WriteBlock<int>(count, ID);
	count+= sizeof(int);
	block->WriteBlock<int>(count, now_use);
	count+= sizeof(int);
	block->WriteBlock<int>(count, Parent);
	count+= sizeof(int);
	block->WriteBlock<int>(count, LastBrother);
	count+= sizeof(int);
	block->WriteBlock<int>(count, NextBrother);
	count+= sizeof(int);
	if(type){//leaf
	    count+= sizeof(int);
		if(key_type <= 0){
		    auto p= ptr_list.begin();
			for(int i=0; i<now_use; i++, p++){
				block->WriteBlock<key>(count, (*p)->value);
				count+= sizeof(key);
				block->WriteBlock<int>(count, (*p)->file_off);
				count+= sizeof(int);
				block->WriteBlock<int>(count, (*p)->block_off);
				count+= sizeof(int);
				delete *p;
			}
		}
		else{//string
		    auto p= ptr_list.begin();
			for(int i=0; i<now_use; i++, p++){
                string temp;
                stringstream ss;
                ss << (*p)->value;
                ss >> temp;
				block->WriteBlock(count, (char*)(temp.c_str()), key_type);
				count+= key_type; 
				block->WriteBlock<int>(count, (*p)->file_off);
				count+= sizeof(int);
				block->WriteBlock<int>(count, (*p)->block_off);
				count+= sizeof(int);
				delete *p;
			}
		}
	}
	else{//internal
        auto p= child_list.begin();
		block->WriteBlock<int>(count, *p);
		p++;
		count+= sizeof(int);
		if(key_type <= 0){
            auto q= key_list.begin();
			for(int i=0; i<now_use; i++, p++, q++){
				block->WriteBlock<key>(count, *q);
				count+= sizeof(key);
				block->WriteBlock<int>(count, *p);
				count+= 2*sizeof(int);
			}
		}
		else{//string
            auto q= key_list.begin();
			for(int i=0; i<now_use; i++, q++, p++){
                string temp;
                stringstream ss;
                ss << (*q);
                ss >> temp;
                block->WriteBlock(count, (char*)(temp.c_str()), key_type);
				count+= key_type;
				block->WriteBlock<int>(count, *(p));
				count+= 2*sizeof(int);
			}
		}
	}
}

template <typename key>
int BxNode<key>::FindChild(key value){
    int i;
	typename vector<key>::iterator p= key_list.begin();
	for(i=0; i<now_use; i++, p++){
		if(value < *p)
			break;
	}
	return child_list.at(static_cast<unsigned long long int>(i));
}

template <typename key>
vector<InfoBlock<key>* >* BxNode<key>::FindKey(int f_type, key lvalue, key rvalue, vector<InfoBlock<key>* >* Query_list){
	typename vector<InfoBlock<key>* >::iterator p= ptr_list.begin();
	if(f_type == LESS || f_type == LESS_E || f_type == EQUAL){
		while(p != ptr_list.end() && (*p)->value < rvalue) {
		    if(f_type != EQUAL) {
                InfoBlock<key> *temp = new InfoBlock<key>((*p)->value, (*p)->file_off, (*p)->block_off);
                Query_list->push_back(temp);
            }
            p++;
        }
		if(p != ptr_list.end() && (*p)->value == rvalue && (f_type == LESS_E || f_type == EQUAL)) {
            InfoBlock<key>* temp= new InfoBlock<key>((*p)->value, (*p)->file_off, (*p)->block_off);
		    Query_list->push_back(temp);
        }
		return Query_list;
	}
	else{
		while(p != ptr_list.end() && (*p)->value < lvalue)
			p++;
		if(p != ptr_list.end() && (*p)->value == lvalue){
			if(f_type == GREATER_E) {
                InfoBlock<key>* temp= new InfoBlock<key>((*p)->value, (*p)->file_off, (*p)->block_off);
                Query_list->push_back(temp);
            }
			p++;
		}
		while(p != ptr_list.end()){
            InfoBlock<key>* temp= new InfoBlock<key>((*p)->value, (*p)->file_off, (*p)->block_off);
            Query_list->push_back(temp);
			p++; 
		}
		return Query_list;
	}
}

template <typename key>
bool BxNode<key>::InsertKey(key value, InfoBlock<key>* loc){
	typename vector<key>::iterator p= key_list.begin();
	typename vector<InfoBlock<key>* >::iterator q= ptr_list.begin();
	if(!now_use){//empty root
		ptr_list.push_back(loc);
		key_list.push_back(value);
		now_use++;
		return true;
	}
	else if(type){//leaf
		while(q != ptr_list.end() && (*q)->value < value){
			p++;
			q++;
		}
		if(q == ptr_list.end()){
			ptr_list.push_back(loc);
			key_list.push_back(value);
			now_use++;
			return true;
		}
		else if((*q)->value == value)
			return false;
		else{
			ptr_list.insert(q, loc);
			key_list.insert(p, value);
			now_use++;
			return true;
		}
	}
	else{//internal
		while(p != key_list.end() && *p < value)
			p++;
		if(p == key_list.end()){
			key_list.push_back(value);
			now_use++;
			return true;
		}
		else if(*p == value)
			return false;
		else{
			key_list.insert(p, value);
			now_use++;
			return true;
		}
	}
}

template <typename key>
bool BxNode<key>::DeleteKey(key value){
	typename vector<key>::iterator p= key_list.begin();
	typename vector<InfoBlock<key>* >::iterator q= ptr_list.begin();
	if(!now_use)
		return false;
	else if(type){//leaf
		while(q != ptr_list.end() && (*q)->value <value){
			p++;
			q++;
		}
		if(q == ptr_list.end() || (*q)->value > value)
			return false;
		else{
			delete *q;
			ptr_list.erase(q);
			key_list.erase(p);
			now_use--;
			return true;
		}
	}
	else{//internal
		while(p != key_list.end() && *p <value)
			p++;
		if(p == key_list.end() || *p > value)
			return false;
		else{
			key_list.erase(p);
			now_use--;
			return true;
		}
	}
}

template <typename key>
BxNode<key>* BxNode<key>::Split(int new_ID){
	BxNode<key>* born= new BxNode<key>(type, Parent, key_type, ind_name, new_ID, FullNodeNum);
	BxNode<key>* temp;
	Block* block= nullptr;
	if(NextBrother)
		block= BM->FetchRandomBlock(ind_name, NextBrother);
	if(type){//leaf
        born->now_use= FullNodeNum- (FullNodeNum+1)/2+1;
        now_use= (FullNodeNum+1)/2;
		born->NextBrother= NextBrother;
		NextBrother= new_ID;
		born->LastBrother= ID;
		if(block){
			temp= new BxNode<key>(block, key_type, ind_name, FullNodeNum);
			temp->LastBrother= new_ID;
			delete temp;
		}
		for(int i=0; i<born->now_use; i++){
			born->key_list.push_back(key_list.at(i+now_use));
			born->ptr_list.push_back(ptr_list.at(i+now_use));
		}
		for(int i=0; i<born->now_use; i++){
			key_list.pop_back();
			ptr_list.pop_back();
		}
	}
	else{//internal
        born->now_use= FullNodeNum- FullNodeNum/2+ 1;
	    now_use= FullNodeNum/2;
        born->NextBrother= NextBrother;
        NextBrother= new_ID;
        born->LastBrother= ID;
        if(block){
            temp= new BxNode<key>(block, key_type, ind_name, FullNodeNum);
            temp->LastBrother= new_ID;
            delete temp;
        }
		for(int i=0; i<born->now_use; i++){
			born->key_list.push_back(key_list.at(i+now_use));
			block= BM->FetchRandomBlock(ind_name, child_list.at(i+now_use+1));
			temp= new BxNode<key>(block, key_type, ind_name, FullNodeNum);
			temp->Parent= new_ID;
			delete temp;
			born->child_list.push_back(child_list.at(i+now_use+1));
		}
		for(int i=0; i<born->now_use; i++){
			key_list.pop_back();
			child_list.pop_back();
		}
	}
	return born;
}

///////////////////////////////////////////////////////////////////////////

template <typename key>
void BxTree::CreateIndex(const string& ind_name, const string& table_name, const string& attr_name){
	vector<InfoBlock<string>* >* add_list= CM->CreateIndex(ind_name, table_name, attr_name);
	vector<InfoBlock<string>* >::iterator p= add_list->begin();
	InfoBlock<key>* change;
	key sss;
	for(; p != add_list->end(); p++){
	    stringstream ss;
	    ss << (*p)->value;
	    ss >> sss;
		change= new InfoBlock<key>(sss, (*p)->file_off, (*p)->block_off);
		Insert<key>(change);
		delete *p;
	}
	delete add_list;
}

template <typename key>
vector<InfoBlock<key>* >* BxTree::Find(int f_type, key lvalue, key rvalue, vector<InfoBlock<key>* >* Query_list){
	int i= level-1;
	int child;
	BxNode<key>* searcher;
	BxNode<key>* temp;
	if(f_type == LESS || f_type == LESS_E){
		block= BM->FetchRandomBlock(ind_name, 1);
		searcher= new BxNode<key>(block, key_type, ind_name, FullNodeNum);
		while(searcher){
			if((*(searcher->ptr_list.begin()))->value > rvalue)
				break;
			else
				Query_list= searcher->FindKey(f_type, lvalue, rvalue, Query_list);
			if(!searcher->NextBrother)
				break;
			block= BM->FetchRandomBlock(ind_name, searcher->NextBrother);
			delete searcher;
			searcher= new BxNode<key>(block, key_type, ind_name, FullNodeNum);
		}
		delete searcher;
		return Query_list;
	}
	else{
		block= BM->FetchRandomBlock(ind_name, Root);
		searcher= new BxNode<key>(block, key_type, ind_name, FullNodeNum);
		if(level == 1)
			return searcher->FindKey(f_type, lvalue, rvalue, Query_list);
		while(i--){
			child= searcher->FindChild(lvalue);
			block= BM->FetchRandomBlock(ind_name, child);
			temp= new BxNode<key>(block, key_type, ind_name, FullNodeNum);
			delete searcher;
			searcher= temp;
		}
		while(searcher){
			Query_list= searcher->FindKey(f_type, lvalue, rvalue, Query_list);
			if(!searcher->NextBrother)
				break;
			block= BM->FetchRandomBlock(ind_name, searcher->NextBrother);
			delete searcher;
			searcher= new BxNode<key>(block, key_type, ind_name, FullNodeNum);
		}
		delete searcher;
		return Query_list;
	}
}

template <typename key>
bool BxTree::Insert(InfoBlock<key>* loc){
	int i= level-1;
	int child;
	block= BM->FetchRandomBlock(ind_name, Root);
	BxNode<key>* searcher= new BxNode<key>(block, key_type, ind_name, FullNodeNum);
	BxNode<key>* temp;
	if(level == 1) {
	    Key_num++;
        searcher->InsertKey(loc->value, loc);
        return AfterInsert(searcher);
    }
	while(i--){
		child= searcher->FindChild(loc->value);
		block= BM->FetchRandomBlock(ind_name, child);
		temp= new BxNode<key>(block, key_type, ind_name, FullNodeNum);
		delete searcher;
		searcher= temp;
	}
	if(searcher->InsertKey(loc->value, loc)){
		Key_num++;
		return AfterInsert(searcher);
	}
	delete searcher;
	return false;
}

template <typename key>
bool BxTree::Delete(key value){
	int i= level-1;
	int child;
	block= BM->FetchRandomBlock(ind_name, Root);
	BxNode<key>* searcher= new BxNode<key>(block, key_type, ind_name, FullNodeNum);
	BxNode<key>* temp;
	if(level == 1) {
	    Key_num--;
        searcher->DeleteKey(value);
        return AfterDelete(searcher);
    }
	while(i--){
		child= searcher->FindChild(value);
		block= BM->FetchRandomBlock(ind_name, child);
		temp= new BxNode<key>(block, key_type, ind_name, FullNodeNum);
		delete searcher;
		searcher= temp;
	}
	if(searcher->DeleteKey(value)){
		Key_num--;
		return AfterDelete(searcher);
	}
	delete searcher;
	return false;
}

template <typename key>
bool BxTree::AfterInsert(BxNode<key>* target){
	BxNode<key>* finder;
	BxNode<key>* new_node;
	BxNode<key>* temp= target;
	if(target->now_use <= FullNodeNum){
		delete target;
		return true;
	}
	else if(!target->Parent){//root split
		finder= target->Split(GetNewID());
		new_node= new BxNode<key>(false, 0, key_type, ind_name, GetNewID(), FullNodeNum);
		new_node->key_list.push_back(finder->key_list.at(0));
		target->Parent= finder->Parent= new_node->ID;
		new_node->child_list.push_back(target->ID);
		new_node->child_list.push_back(finder->ID);
		new_node->now_use++;
		Node_num+= 2;
		Root= new_node->ID;
		level++;
		if(level != 2) {
            finder->key_list.erase(finder->key_list.begin());
            finder->now_use--;
        }
        delete target;
        delete finder;
        delete new_node;
		return true;
	}
	else if(target->ID != 1){
		block= BM->FetchRandomBlock(ind_name, target->LastBrother);
		finder= new BxNode<key>(block, key_type, ind_name, FullNodeNum);//not null
		if(finder && finder->now_use < FullNodeNum){
			finder->key_list.push_back(target->key_list.at(0));
			finder->ptr_list.push_back(target->ptr_list.at(0));
			target->key_list.erase(target->key_list.begin());
			target->ptr_list.erase(target->ptr_list.begin());
			target->now_use--;
			finder->now_use++;
			key update= target->key_list.at(0);//update index just at the first not the first element
			delete finder;
			block= BM->FetchRandomBlock(ind_name, target->Parent);
			finder= new BxNode<key>(block, key_type, ind_name, FullNodeNum);
			temp= target;
			while(finder){
				if(*(finder->child_list.begin()) != temp->ID){
					finder->key_list.at(find(finder->child_list.begin(), finder->child_list.end(), temp->ID)-finder->child_list.begin()-1)= update;
					delete finder;
					delete temp;
					break;
				}
				delete temp;
				temp= finder;
				if(!finder->Parent){
					delete finder;
					break;
				}
				block= BM->FetchRandomBlock(ind_name, finder->Parent);
				finder= new BxNode<key>(block, key_type, ind_name, FullNodeNum);
			}
			return true;
		}
		delete finder;
	}
	if(target->NextBrother){
		block= BM->FetchRandomBlock(ind_name, target->NextBrother);
		finder= new BxNode<key>(block, key_type, ind_name, FullNodeNum);//right brother
		if(finder->now_use < FullNodeNum){//send to right brother
			finder->key_list.insert(finder->key_list.begin(), target->key_list.at(target->now_use-1));
			finder->ptr_list.insert(finder->ptr_list.begin(), target->ptr_list.at(target->now_use-1));
			target->key_list.pop_back();
			target->ptr_list.pop_back();
			target->now_use--;
			finder->now_use++;
			key update= *(finder->key_list.begin());
			delete temp;
			temp= finder;
			block= BM->FetchRandomBlock(ind_name, finder->Parent);
			finder= new BxNode<key>(block, key_type, ind_name, FullNodeNum);
			while(finder){
				if(*(finder->child_list.begin()) != temp->ID){
					finder->key_list.at(find(finder->child_list.begin(), finder->child_list.end(), temp->ID)-finder->child_list.begin()-1)= update;
					delete finder;
					delete temp;
					break;
				}
				delete temp;
				temp= finder;
				if(!finder->Parent){
					delete finder;
					break;
				}
				block= BM->FetchRandomBlock(ind_name, finder->Parent);
				finder= new BxNode<key>(block, key_type, ind_name, FullNodeNum);
			}
			return true;
		}
		delete finder;
	}
	temp= target;
	new_node= target->Split(GetNewID());//self split
	block= BM->FetchRandomBlock(ind_name, target->Parent);
	finder= new BxNode<key>(block, key_type, ind_name, FullNodeNum);
	Node_num++;
	typename vector<key>::iterator p= finder->key_list.begin();
	vector<int>::iterator q= finder->child_list.begin();
	while(finder){
		for(; *q != temp->ID; p++, q++);
		if(!new_node->type){//internal
		    finder->key_list.insert(p, new_node->key_list.at(0));
			new_node->key_list.erase(new_node->key_list.begin());
			new_node->now_use--;
		}
		else//leaf
			finder->key_list.insert(p, new_node->key_list.at(0));
		finder->child_list.insert(++q, new_node->ID);
		finder->now_use++;
		if(finder->now_use <= FullNodeNum){
			delete temp;
			delete finder;
			delete new_node;
			return true;
		}
		else if(!finder->Parent){
			delete temp;
			delete new_node;
			return AfterInsert(finder);
		}
		delete temp;
		temp= finder;
		delete new_node;
		new_node= finder->Split(GetNewID());
		block= BM->FetchRandomBlock(ind_name, finder->Parent);
		finder= new BxNode<key>(block, key_type, ind_name, FullNodeNum);
		Node_num++;
        p= finder->key_list.begin();
        q= finder->child_list.begin();
	}
}

template <typename key>
bool BxTree::AfterDelete(BxNode<key>* target){
	BxNode<key>* finder= nullptr;
	BxNode<key>* del_node= nullptr;
	BxNode<key>* temp= target;

	if(!temp->Parent){//root
		delete target;
		return  true;
	}
	else if(temp->now_use >= FullNodeNum/2){//update index
		key update= temp->key_list.at(0);
		block= BM->FetchRandomBlock(ind_name, temp->Parent);
		finder= new BxNode<key>(block, key_type, ind_name, FullNodeNum);
		while(true){
			if(*(finder->child_list.begin()) != temp->ID){
				finder->key_list.at(find(finder->child_list.begin(), finder->child_list.end(), temp->ID)-finder->child_list.begin()-1)= update;
				delete finder;
				delete temp;
				break;
			}
			delete temp;
			temp= finder;
			if(!finder->Parent){
				delete finder;
				break;
			}
			block= BM->FetchRandomBlock(ind_name, finder->Parent);
			finder= new BxNode<key>(block, key_type, ind_name, FullNodeNum);
		}
		return true;
	}
	else if(target->ID != 1){
		block= BM->FetchRandomBlock(ind_name, target->LastBrother);
		finder= new BxNode<key>(block, key_type, ind_name, FullNodeNum);//not null
		if(finder->now_use > FullNodeNum/2){//borrow the left element
			target->key_list.insert(target->key_list.begin(), finder->key_list.at(finder->now_use-1));
			target->ptr_list.insert(target->ptr_list.begin(), finder->ptr_list.at(finder->now_use-1));
			finder->key_list.pop_back();
			finder->ptr_list.pop_back();
			finder->now_use--;
			target->now_use++;
			key update= target->key_list.at(0);
			delete finder;
			block= BM->FetchRandomBlock(ind_name, target->Parent);
			finder= new BxNode<key>(block, key_type, ind_name, FullNodeNum);
			while(true){
				if(*(finder->child_list.begin()) != temp->ID){
					finder->key_list.at(find(finder->child_list.begin(), finder->child_list.end(), temp->ID)-finder->child_list.begin()-1)= update;
					delete finder;
					delete temp;
					break;
				}
				delete temp;
				temp= finder;
				if(!finder->Parent){
					delete finder;
					break;
				}
				block= BM->FetchRandomBlock(ind_name, finder->Parent);
				finder= new BxNode<key>(block, key_type, ind_name, FullNodeNum);
			}
			return true;
		}
		else
			del_node= finder;
	}
	if(target->NextBrother){//right brother
		block= BM->FetchRandomBlock(ind_name, target->NextBrother);
		finder= new BxNode<key>(block, key_type, ind_name, FullNodeNum);
		if(finder->now_use > FullNodeNum/2){//borrow the right element
			target->key_list.push_back(finder->key_list.at(0));
			target->ptr_list.push_back(finder->ptr_list.at(0));
			finder->key_list.erase(finder->key_list.begin());
			finder->ptr_list.erase(finder->ptr_list.begin());
			finder->now_use--;
			target->now_use++;
			key update= finder->key_list.at(0);
			delete temp;
			temp= finder;
			block= BM->FetchRandomBlock(ind_name, finder->Parent);
			finder= new BxNode<key>(block, key_type, ind_name, FullNodeNum);
			while(true){
				if(*(finder->child_list.begin()) != temp->ID){
					finder->key_list.at(find(finder->child_list.begin(), finder->child_list.end(), temp->ID)-finder->child_list.begin()-1)= update;
					delete finder;
					delete temp;
					break;
				}
				delete temp;
				temp= finder;
				if(!finder->Parent){
					delete finder;
					break;
				}
				block= BM->FetchRandomBlock(ind_name, finder->Parent);
				finder= new BxNode<key>(block, key_type, ind_name, FullNodeNum);
			}
			return true;
		}
	}
	if(del_node && del_node->now_use+target->now_use <= FullNodeNum){//union left
		typename vector<key>::iterator p= target->key_list.begin();
		typename vector<InfoBlock<key>* >::iterator q= target->ptr_list.begin();
		for(; p != target->key_list.end(); p++, q++){
			del_node->key_list.push_back(*p);
			del_node->ptr_list.push_back(*q);
			del_node->now_use++;
		}
		for(int i=0; i<target->now_use; i++)
		    target->ptr_list.pop_back();
		del_node->NextBrother= target->NextBrother;
		if(target->NextBrother){
			block= BM->FetchRandomBlock(ind_name, target->NextBrother);
			temp= new BxNode<key>(block, key_type, ind_name, FullNodeNum);
			temp->LastBrother= del_node->ID;
			delete temp;
		}
		block= BM->FetchRandomBlock(ind_name, target->Parent);
		finder= new BxNode<key>(block, key_type, ind_name, FullNodeNum);
		finder->key_list.erase(finder->key_list.begin()+ (find(finder->child_list.begin(), finder->child_list.end(), target->ID)-finder->child_list.begin())-1);
		finder->child_list.erase(find(finder->child_list.begin(), finder->child_list.end(), target->ID));
		int temp_ID= target->ID;
		target->now_use= 0;
		delete target;
		EraseID(temp_ID);
		Node_num--;
		finder->now_use--;
		delete del_node;
		del_node= finder;
	}
	else if(finder &&finder->now_use+target->now_use <= FullNodeNum){//union right
		typename vector<key>::iterator p= finder->key_list.begin();
		typename vector<InfoBlock<key>* >::iterator q= finder->ptr_list.begin();
		for(; p != finder->key_list.end(); p++, q++){
			target->key_list.push_back(*p);
			target->ptr_list.push_back(*q);
			target->now_use++;
		}
        for(int i=0; i<finder->now_use; i++)
            finder->ptr_list.pop_back();
		target->NextBrother= finder->NextBrother;
		if(finder->NextBrother){
			block= BM->FetchRandomBlock(ind_name, finder->NextBrother);
			temp= new BxNode<key>(block, key_type, ind_name, FullNodeNum);
			temp->LastBrother= target->ID;
			delete temp;
		}
		if(del_node)
			delete del_node;
		delete target;
		del_node= finder;
		block= BM->FetchRandomBlock(ind_name, finder->Parent);
		finder= new BxNode<key>(block, key_type, ind_name, FullNodeNum);
		finder->key_list.erase(finder->key_list.begin()+ (find(finder->child_list.begin(), finder->child_list.end(), del_node->ID)-finder->child_list.begin())-1);
		finder->child_list.erase(find(finder->child_list.begin(), finder->child_list.end(), del_node->ID)); 
		int temp_ID= del_node->ID;
		del_node->now_use= 0;
		delete del_node;
		EraseID(temp_ID);
		Node_num--;
		finder->now_use--;
		del_node= finder;
	}
	while(del_node){//internal adjust
		if(!del_node->Parent){//root
			if(!del_node->now_use){//empty root
				block= BM->FetchRandomBlock(ind_name, del_node->child_list.at(0));
				temp= new BxNode<key>(block, key_type, ind_name, FullNodeNum);
				temp->Parent= 0;
				delete del_node;
				EraseID(Root);
				Root= temp->ID;
				delete temp;
				Node_num--;
				level--;
			}
			else
				delete del_node;
			return true;
		}
		else if(del_node->now_use >= FullNodeNum/2){
			delete del_node;
			return true;
		}
		else{
			block= BM->FetchRandomBlock(ind_name, del_node->Parent);
			finder= new BxNode<key>(block, key_type, ind_name, FullNodeNum);
			if(*(finder->child_list.begin()) != del_node->ID){//if have left node
				temp= finder;
				block= BM->FetchRandomBlock(ind_name, *(find(finder->child_list.begin(), finder->child_list.end(), del_node->ID)-1));
				finder= new BxNode<key>(block, key_type, ind_name, FullNodeNum);
				if(finder->now_use > FullNodeNum/2){//borrow left element
					del_node->child_list.insert(del_node->child_list.begin(), finder->child_list.at(static_cast<unsigned long long int>(finder->now_use)));
					block= BM->FetchRandomBlock(ind_name, del_node->child_list.at(0));
					target= new BxNode<key>(block, key_type, ind_name, FullNodeNum);
					target->Parent= del_node->ID;//update the parent
					delete target;
					finder->child_list.pop_back();
					del_node->key_list.insert(del_node->key_list.begin(), temp->key_list.at(find(temp->child_list.begin(), temp->child_list.end(), finder->ID)-temp->child_list.begin()));
					temp->key_list.at(find(temp->child_list.begin(), temp->child_list.end(), finder->ID)-temp->child_list.begin())= finder->key_list.at(finder->now_use-1);
					finder->key_list.pop_back();
					finder->now_use--;
					del_node->now_use++;
					delete del_node;
					del_node= temp;
					delete finder;
					continue;
				}
				else if(finder->now_use+del_node->now_use < FullNodeNum){//union the left
					typename vector<key>::iterator p= del_node->key_list.begin();
					vector<int>::iterator q= del_node->child_list.begin();
					finder->key_list.push_back(temp->key_list.at(find(temp->child_list.begin(), temp->child_list.end(), finder->ID)-temp->child_list.begin()));
					temp->key_list.erase(temp->key_list.begin()+ (find(temp->child_list.begin(), temp->child_list.end(), finder->ID)-temp->child_list.begin()));
					temp->child_list.erase(find(temp->child_list.begin(), temp->child_list.end(), del_node->ID));
					temp->now_use--;
					finder->now_use++;
					for(; p != del_node->key_list.end(); p++, q++){
						finder->key_list.push_back(*p);
						block= BM->FetchRandomBlock(ind_name, *q);
						target= new BxNode<key>(block, key_type, ind_name, FullNodeNum);
						target->Parent= finder->ID;
						delete target;
						finder->child_list.push_back(*q);
						finder->now_use++;
					}
                    block= BM->FetchRandomBlock(ind_name, *q);
                    target= new BxNode<key>(block, key_type, ind_name, FullNodeNum);
                    target->Parent= finder->ID;
                    delete target;
                    finder->child_list.push_back(*q);
                    finder->NextBrother= del_node->NextBrother;
                    if(del_node->NextBrother){
						block= BM->FetchRandomBlock(ind_name, del_node->NextBrother);
						target= new BxNode<key>(block, key_type, ind_name, FullNodeNum);
						target->LastBrother= finder->ID;
						delete target;
                    }
					int temp_ID= del_node->ID;
					delete del_node;
					EraseID(temp_ID);
					Node_num--;
					del_node= temp;
					delete finder;
					continue;
				}
				finder= temp;
			}
			if(*(finder->child_list.end()-1) != del_node->ID){//if have right node
				temp= finder;
				block= BM->FetchRandomBlock(ind_name, *(find(finder->child_list.begin(), finder->child_list.end(), del_node->ID)+1));
				finder= new BxNode<key>(block, key_type, ind_name, FullNodeNum);
				if(finder->now_use > FullNodeNum/2){//borrow right element
                    block= BM->FetchRandomBlock(ind_name, finder->child_list.at(0));
                    target= new BxNode<key>(block, key_type, ind_name, FullNodeNum);
                    target->Parent= del_node->ID;//update the parent
                    delete target;
					del_node->child_list.push_back(finder->child_list.at(0));
					finder->child_list.erase(finder->child_list.begin());
					del_node->key_list.push_back(temp->key_list.at(find(temp->child_list.begin(), temp->child_list.end(), del_node->ID)-temp->child_list.begin()));
					temp->key_list.at(find(temp->child_list.begin(), temp->child_list.end(), del_node->ID)-temp->child_list.begin())= finder->key_list.at(0);
					finder->key_list.erase(finder->key_list.begin());
					finder->now_use--;
					del_node->now_use++;
					delete del_node;
					del_node= temp;
					delete finder;
					continue;
				}
				else if(finder->now_use+del_node->now_use < FullNodeNum){//union the right
					typename vector<key>::iterator p= finder->key_list.begin();
					vector<int>::iterator q= finder->child_list.begin();
					del_node->key_list.push_back(temp->key_list.at(find(temp->child_list.begin(), temp->child_list.end(), del_node->ID)-temp->child_list.begin()));
					temp->key_list.erase(temp->key_list.begin()+ (find(temp->child_list.begin(), temp->child_list.end(), del_node->ID)-temp->child_list.begin()));
					temp->child_list.erase(find(temp->child_list.begin(), temp->child_list.end(), finder->ID));
					temp->now_use--;
					del_node->now_use++;
					for(; p != finder->key_list.end(); p++, q++){
						del_node->key_list.push_back(*p);
						block= BM->FetchRandomBlock(ind_name, *q);
						target= new BxNode<key>(block, key_type, ind_name, FullNodeNum);
						target->Parent= del_node->ID;
						delete target;
						del_node->child_list.push_back(*q);
						del_node->now_use++;
					}
                    block= BM->FetchRandomBlock(ind_name, *q);
                    target= new BxNode<key>(block, key_type, ind_name, FullNodeNum);
                    target->Parent= del_node->ID;
                    delete target;
					del_node->child_list.push_back(*q);
					del_node->NextBrother= finder->NextBrother;
					if(finder->NextBrother){
						block= BM->FetchRandomBlock(ind_name, finder->NextBrother);
						target= new BxNode<key>(block, key_type, ind_name, FullNodeNum);
						target->LastBrother= del_node->ID;
						delete target;
					}
					int temp_ID= finder->ID;
					delete finder;
					EraseID(temp_ID);
					Node_num--;
					delete del_node;
					del_node= temp;
					continue;
				}
			}
		}
	}
}

template <typename key>
void BxTree::Print(){
    vector<int> queue;
    BxNode<key>* temp;
    queue.push_back(Root);
    while(queue.size()){
    	block= BM->FetchRandomBlock(ind_name, queue.at(0));
        temp= new BxNode<key>(block, key_type, ind_name, FullNodeNum);
        for(int i=0; i<temp->now_use; i++)
            cout<<temp->key_list.at(i)<<"|";
        cout<<endl;
        if(!temp->type){
            for(int j=0; j<=temp->now_use; j++)
                queue.push_back(temp->child_list.at(static_cast<unsigned long long int>(j)));
        }
        delete temp;
        queue.erase(queue.begin());
    }
}

#endif