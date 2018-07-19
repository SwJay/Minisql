#include<IndexManager.h>
#include "BufferManager.h"

IndexManager::~IndexManager(){
    ind_ptr= ind_list.begin();
    for(; ind_ptr != ind_list.end(); ind_ptr++)
        delete *ind_ptr;
    cout<<"destructor"<<endl;
}

BxTree* IndexManager::GetBxTree(const string &ind_name){
    ind_ptr= ind_list.begin();
    for(; ind_ptr != ind_list.end(); ind_ptr++)
        if((*ind_ptr)->GetIndexName() == ind_name)
            return *(ind_ptr);
    auto * born= new BxTree(ind_name);
    ind_list.push_back(born);
    return born;
}

int IndexManager::Create(const string &ind_name, const string &table_name, const string &attr_name, int key_type){
    int size_per_key;
    switch(key_type){
        case(-1): size_per_key= sizeof(float);break;
        case(0): size_per_key= sizeof(int);break;
        default: size_per_key= key_type;break;
    }
    //int FullNodeNum= 4;
    int FullNodeNum= (BLOCK_SIZE-6*sizeof(int)-1)/(size_per_key+2*sizeof(int));
    auto * born= new BxTree(ind_name, table_name, attr_name, FullNodeNum, key_type);
    ind_list.push_back(born);
    return born->GetKeyNum();
}

void IndexManager::Drop(const string &ind_name){
    BxTree* temp= GetBxTree(ind_name);
    temp->Drop();
    delete temp;
    ind_list.erase(find(ind_list.begin(), ind_list.end(), temp));
}