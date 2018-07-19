//
// Created by SwJ on 2018/6/19.
//

#ifndef API_API_H
#define API_API_H

#include <string>
#include <utility>
#include <vector>
#include <iostream>
#include <ctime>
#include "MyClass.h"

#define FLOAT (-1)
#define INT 0

#define EQUAL 0
#define UNEQUAL (-1)
#define GREATER 2
#define GREATER_E 3
#define LESS 4
#define LESS_E 5

class API{
public:
    void CreateTable(const std::string &table_name, std::vector<Attribute*> *attributes);
    void DropTable(const std::string &table_name);
    void CreateIndex(const std::string &index_name, const std::string &table_name, const std::string &attribute_name);
    void DropIndex(const std::string &index_name);
    void SelectRecord(const std::string &table_name, std::vector<Condition*> *conditions);
    void InsertRecord(const std::string &table_name, std::vector<std::string> *values);
    void DeleteRecord(const std::string &table_name, Condition *condition);
    //void PrintRecord(const std::string &records);
private:
    IndexManager* im= MiniSQL::GetIM();
    RecordManager* rm= MiniSQL::GetRM();
    CatalogManager* cm= MiniSQL::GetCM();
    
	bool CheckDuplicate(bool isIndex, int type,const std::string &value, const std::string &index, const std::string &table, const std::string &attribute);
//    template <typename key>
//	void InsertbyType(const std::string &index,int type, const std::string &value, InfoBlock<key> *ifb);
};

#endif //API_API_H
