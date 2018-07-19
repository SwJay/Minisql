#ifndef _INDEX_MANAGER_
#define _INDEX_MANAGER_

#include <utility>
#include "MyClass.h"
#include "BxTree.h"

#define FLOAT (-1)
#define INT 0

#define EQUAL 0
#define UNEQUAL (-1)
#define GREATER 2
#define GREATER_E 3
#define LESS 4
#define LESS_E 5

class IndexManager{
private:
	vector<BxTree*> ind_list;
	vector<BxTree*>::iterator ind_ptr;
	BxTree* GetBxTree(const string &ind_name);
public:
	~IndexManager();
	int Create(const string &ind_name, const string &table_name, const string &attr_name, int key_type);
	template <typename key>
	bool Insert(string ind_name, key value);
	template <typename key>
	bool Insert(string ind_name, InfoBlock<key>* loc);
	template <typename key>
	vector<InfoBlock<key>* >* Delete(int f_type, string ind_name, key lvalue, key rvalue, vector<InfoBlock<key>* >* Query_list);
	template <typename key>
	vector<InfoBlock<key>* >* Find(int f_type, string ind_name, key lvalue, key rvalue, vector<InfoBlock<key>* >* Query_list);
	void Drop(const string &ind_name);
	template <typename key>
	void Print(const string& ind_name);
};

template <typename key>
bool IndexManager::Insert(string ind_name, key value){
	vector<InfoBlock<key>* >* Query_list= new vector<InfoBlock<key>* >;
	Query_list= Find<key>(EQUAL, ind_name, value, value, Query_list);
	if(Query_list->size()){
		delete Query_list;
		return true;
	}
	else{
		delete Query_list;
		return false;
	}
}

template <typename key>
bool IndexManager::Insert(string ind_name, InfoBlock<key>* loc){
	BxTree* temp= GetBxTree(ind_name);
	return temp->Insert(loc);
}

template <typename key>
vector<InfoBlock<key>* >* IndexManager::Delete(int f_type, string ind_name, key lvalue, key rvalue, vector<InfoBlock<key>* >* Query_list){
	BxTree* temp= GetBxTree(ind_name);
	Query_list= temp->Find<key>(f_type, lvalue, rvalue, Query_list);
	typename vector<InfoBlock<key>* >::iterator p= Query_list->begin();
	for(; p != Query_list->end(); p++)
		temp->Delete<key>((*p)->value);
	return Query_list;
}

template <typename key>
vector<InfoBlock<key>* >* IndexManager::Find(int f_type, string ind_name, key lvalue, key rvalue, vector<InfoBlock<key>* >* Query_list){
	BxTree* temp= GetBxTree(ind_name);
	if(f_type == UNEQUAL){
		Query_list= temp->Find<key>(LESS, lvalue, rvalue, Query_list);
		Query_list= temp->Find<key>(GREATER, lvalue, rvalue, Query_list);
		return Query_list;
	}
	Query_list= temp->Find<key>(f_type, lvalue, rvalue, Query_list);
	return Query_list;
}

template <typename key>
void IndexManager::Print(const string &ind_name){
    BxTree* temp= GetBxTree(ind_name);
    temp->Print<key>();
}

#endif