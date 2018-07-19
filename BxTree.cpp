#include "BxTree.h"

BxTree::BxTree(const string &ind_name):ind_name(ind_name){
    count= 0;
    block= BM->FetchBlockHead(ind_name);
    FirstEmpty= block->ReadBlock<int>(count);
    count+= sizeof(int);
    Root= block->ReadBlock<int>(count);
    count+= sizeof(int);
    level= block->ReadBlock<int>(count);
    count+= sizeof(int);
    Node_num= block->ReadBlock<int>(count);
    count+= sizeof(int);
    Key_num= block->ReadBlock<int>(count);
    count+= sizeof(int);
    FullNodeNum= block->ReadBlock<int>(count);
    count+= sizeof(int);
    key_type= block->ReadBlock<int>(count);
}

BxTree::BxTree(const string &ind_name, const string &table_name, const string &attr_name, int FullNodeNum, int key_type):ind_name(ind_name),FullNodeNum(FullNodeNum),key_type(key_type),Node_num(1),Key_num(0),Root(1),FirstEmpty(2),level(1),count(0){
    block= BM->NewFile(ind_name);
    block->WriteBlock<char>(0, '#');
    block= BM->AppendBlock(block);//the root
    if (key_type == INT){
        BxNode<int> *root = new BxNode<int>(true, 0, 0, ind_name, 1, FullNodeNum);
        root->now_use = root->LastBrother = root->NextBrother = 0;
        delete root;
    }
    else if(key_type == FLOAT){//float
        BxNode<float> *root = new BxNode<float>(true, 0, -1, ind_name, 1, FullNodeNum);
        root->now_use = root->LastBrother = root->NextBrother = 0;
        delete root;
    }
    else {//string
        BxNode<string> *root = new BxNode<string>(true, 0, key_type, ind_name, 1, FullNodeNum);
        root->now_use = root->LastBrother = root->NextBrother = 0;
        delete root;
    }
    block= BM->AppendBlock(block);
    block->WriteBlock<char>(0, '#');
    switch(key_type){
        case(INT): CreateIndex<int>(ind_name, table_name, attr_name);break;
        case(FLOAT): CreateIndex<float>(ind_name, table_name, attr_name);break;
        default: CreateIndex<string>(ind_name, table_name, attr_name);break;
    }
}

BxTree::~BxTree(){
    count= 0;
    try{
        block= BM->FetchBlockHead(ind_name);
        block->WriteBlock<int>(count, FirstEmpty);
        count+= sizeof(int);
        block->WriteBlock<int>(count, Root);
        count+= sizeof(int);
        block->WriteBlock<int>(count, level);
        count+= sizeof(int);
        block->WriteBlock<int>(count, Node_num);
        count+= sizeof(int);
        block->WriteBlock<int>(count, Key_num);
        count+= sizeof(int);
        block->WriteBlock<int>(count, FullNodeNum);
        count+= sizeof(int);
        block->WriteBlock<int>(count, key_type);
        count+= sizeof(int);
    }
    catch(Err& e){
    };
}

void BxTree::Drop(){
    BM->DeleteFile(ind_name);
}

int BxTree::GetNewID(){
    int temp= FirstEmpty;
    block= BM->FetchRandomBlock(ind_name, temp);
    char flag= block->ReadBlock<char>(0);
    if(flag == '#'){
        block= BM->AppendBlock(block);
        block->WriteBlock<char>(0, '#');
        FirstEmpty++;
    }
    else
        FirstEmpty= block->ReadBlock<int>(1);
    return temp;
}

bool BxTree::EraseID(int ID){
    block= BM->FetchRandomBlock(ind_name, ID);
    if(block){
        block->WriteBlock<int>(1, FirstEmpty);
        FirstEmpty= ID;
        return true;
    }
    return false;
}