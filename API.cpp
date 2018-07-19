//
// Created by SwJ on 2018/6/19.
//

#include "API.h"
#include "CatalogManager.h"
#include "RecordManager.h"
#include <utility>

using namespace std;

void API::CreateTable(const string &table, vector<Attribute*> *attributes) {
    vector<Attribute*>::iterator a;
    string index_name;
    if(cm->IsTable(table))
        cout << "[ERROR] This table already exits!" << endl;
    else{
        for(a = attributes->begin(); a < attributes->end(); a++)
            if((*a)->primary)
                break;
        index_name = table + "^.^" + (*a)->attribute_name;
        cm->CreateTable(table,attributes);
        rm->CreateTable(table,attributes);
        im->Create(index_name, table, (*a)->attribute_name, (*a)->type);
        cout << "Create table '" + table + "' successfully." << endl;
    }
}

void API::DropTable(const string &table) {
    vector<string> *index(cm->IsTableIndexed(table));
    if(cm->IsTable(table)) { // table exits
        if(index->size()==1) { // no other indexs
            im->Drop(index->back());
            cm->DropIndex(index->back());
            rm->DropTable(table);
            cm->DropTable(table);
            cout << "Drop table '" + table + "' successfully."<< endl;
        }
        else
            cout << "[ERROR] User-defined indexes on this table remain, delete those indexes before dropping table." << endl;
    }
    else
        cout << "[ERROR] No corresponding table exits!" <<  endl;
}

void API::CreateIndex(const string &index, const string &table, const string &attribute) {
    if(cm->IsIndex(index)||cm->IsIndex(table, attribute))
        cout << "[ERROR] This index already exits!" << endl;
    else if(cm->IsUnique(table, attribute)){
            im->Create(index, table, attribute, cm->AttributeType(table, attribute));
            cout << "Create index '" + index + "' successfully." << endl;
        }
    else
         cout << "[ERROR] A unique attribute is a must for index!" << endl;
}

void API::DropIndex(const string &index) {
    if(cm->IsIndex(index)){
        im->Drop(index);
        cm->DropIndex(index);
        cout << "Drop index '" + index + "' successfully." << endl;
    }
    else
        cout << "[ERROR] No corresponding index exits!" << endl;
}

void API::SelectRecord(const string &table, vector<Condition*> *conditions) {
    vector<Condition*>::iterator c1, c2;
    string index_name, value;
    bool flag = false;
    int type, op;
    if(conditions == nullptr){ // 无条件
        cout << rm->AllRecords(table) << endl;
        return;
    }
    for(c1 = conditions->begin(); c1 < conditions->end(); c1++){ // 找索引
        index_name = cm->GiveIndex(table, (*c1)->attribute_name);
        if(cm->IsIndex(index_name)){ //
            if((*c1)->operate == EQUAL) {
                type = (*c1)->type;
                value = (*c1)->value;
                switch(type){
                    case FLOAT: {
                        auto *results = new vector<InfoBlock<float> *>;
                        im->Find<float>(EQUAL, index_name, (float) atof(value.c_str()), (float) atof(value.c_str()), results);
                        conditions->erase(c1);
                        if(results->empty())
                            cout << "No eligible records!" << endl;
                        else
                            cout << rm->SelectRecords(table, results, conditions) << endl;
                        return;
                    }
                    case INT: {
                        auto *results = new vector<InfoBlock<int> *>;
                        im->Find<int>(EQUAL, index_name, atoi(value.c_str()), atoi(value.c_str()), results);
                        conditions->erase(c1);
                        if(results->empty())
                            cout << "No eligible records!" << endl;
                        else
                            cout << rm->SelectRecords(table, results, conditions) << endl;
                        return;
                    }
                    default: {
                        auto *results = new vector<InfoBlock<string> *>;
                        im->Find<string>(EQUAL, index_name, value, value, results);
                        conditions->erase(c1);
                        if(results->empty())
                            cout << "No eligible records!" << endl;
                        else
                            cout << rm->SelectRecords(table, results, conditions) << endl;
                        return;
                    }
                }
                break;
            }
            else if((*c1)->operate > EQUAL && !flag) { // 偏序
                c2 = c1;
                flag = true;
            }
        }
    }
    if(c1 == conditions->end() && flag) { // 未找到等于，找到偏序
        type = (*c2)->type;
        value = (*c2)->value;
        op = (*c2)->operate;
        index_name = cm->GiveIndex(table, (*c2)->attribute_name);
        switch(type){
            case FLOAT: {
                auto *results = new vector<InfoBlock<float> *>;
                im->Find<float>(op, index_name, (float) atof(value.c_str()), (float) atof(value.c_str()), results);
                conditions->erase(c2);
                if(results->empty())
                    cout << "No eligible records!" << endl;
                else
                    cout << rm->SelectRecords(table, results, conditions) << endl;
                return;
            }
            case INT: {
                auto *results = new vector<InfoBlock<int> *>;
                im->Find<int>(op, index_name, atoi(value.c_str()), atoi(value.c_str()), results);
                conditions->erase(c2);
                if(results->empty())
                    cout << "No eligible records!" << endl;
                else
                    cout << rm->SelectRecords(table, results, conditions) << endl;
                return;
            }
            default: {
                auto *results = new vector<InfoBlock<string> *>;
                im->Find<string>(op, index_name, value, value, results);
                conditions->erase(c2);
                if(results->empty())
                    cout << "No eligible records!" << endl;
                else
                    cout << rm->SelectRecords(table, results, conditions) << endl;
                return;
            }
        }
    }
    else
        cout << rm->SelectRecords(table, conditions) << endl;
}

void API::InsertRecord(const string &table, vector<string> *values) {
	string attribute = cm->GivePrimary(table);
	string index = cm->GiveIndex(table, attribute);
	string value;
	int type;
	bool isIndex;
	vector<string> *unique = cm->GiveUnique(table);//, *index_name;
    vector<string>::iterator x;
	//InfoBlock *ifb;

	value = (*values)[cm->Attributerank(table, attribute)];
	type = cm->AttributeType(table, attribute);
	if (CheckDuplicate(true, type, value, index, table, attribute)) { // primary key
		for (x = unique->begin(); x < unique->end(); x++) { // unique if
			attribute = *x;
			type = cm->AttributeType(table, attribute);
			value = (*values)[cm->Attributerank(table, attribute)];
			isIndex = cm->IsIndex(table, attribute);
			if (isIndex)
				index = cm->GiveIndex(table, attribute);
			if (CheckDuplicate(isIndex, type, value, index, table, attribute))
				continue;
			else {
				cout << "[ERROR] Can't insert! Duplicate value on key '" + attribute + "'!" << endl;
				return;
			}
		}

		if (rm->InsertRecords(table, values))
			;//cout << "Insert record in table'" + table + "' successfully." << endl;
		/*        index_name = cm.IsTableIndexed(table);
		for(x = index_name->begin(); x < index_name->end(); x++){
		type = cm.IndexType(table, *x);
		value = (*values)[cm.AttributeOrder(table, cm.GiveAttribute(table,*x))];
		InsertbyType(*x, type, value, ifb);
		}
		*/
	}
	else {
		cout << "[ERROR] Can't insert! Duplicate value on primary key!" << endl;
		return;
	}
}

void API::DeleteRecord(const string &table, Condition* condition) {
	string attr, index, value;
	int type;
	if (condition == nullptr)
		rm->DeleteRecord(table, condition);

	else {
		attr = condition->attribute_name;
		if (cm->IsIndex(table, attr)) {
			index = cm->GiveIndex(table, attr);
			value = condition->value;
			type = condition->type;
			switch (type) {
			case FLOAT: {
                auto *inf_v = new vector<InfoBlock<float> *>;
                im->Delete<float>(condition->operate, index, (float) atof(value.c_str()), (float) atof(value.c_str()), inf_v);
                rm->DeleteRecord<float>(table, inf_v);
                break;
            }
			case INT: {
                auto *inf_v = new vector<InfoBlock<int> *>;
                im->Delete<int>(condition->operate, index, atoi(value.c_str()), atoi(value.c_str()), inf_v);
                rm->DeleteRecord<int>(table, inf_v);
                break;
            }
			default: {
                auto *inf_v = new vector<InfoBlock<string> *>;
                im->Delete<string>(condition->operate, index, value, value, inf_v);
                rm->DeleteRecord<string>(table, inf_v);
                break;
            }
			}
		}
		else {
			rm->DeleteRecord(table, condition);
		}
	}
	cout << "Delete record in table'" + table + "' successfully." << endl;

}


//-----------------------------------------------------------------------------------

bool API::CheckDuplicate(bool isIndex, int type, const std::string &value, const std::string &index, const std::string &table, const string &attribute){
    if(isIndex) // index
        switch(type){ // 丑陋的接口
            case INT: // int
                return !im->Insert<int>(index, atoi(value.c_str()));
            case FLOAT: // float
                return !im->Insert<float>(index, (float)atof(value.c_str()));
            default:
                return !im->Insert<string>(index, value);
        }
    else{
        Condition *condition = new Condition(attribute, 0, value, type);
        return cm->IsUnique(table, condition);
    }
}

//void API::InsertbyType(const std::string &index,int type, const std::string &value, InfoBlock *ifb){
//    switch(type){
//        case EQUAL: // int
//            InfoBlock<int> *fi = new InfoBlock<int>(atoi(value.c_str()), ifb->file_off, ifb->block_off);
//            im->Insert<int>(index, fi);
//            break;
//        case FLOAT: // int
//            InfoBlock<float> *ff = new InfoBlock<float>((float)atof(value.c_str()), ifb->file_off, ifb->block_off);
//            im->Insert<float>(index, ff);
//            break;
//        default: // int
//            InfoBlock<string> *fs = new InfoBlock<string>(value, ifb->file_off, ifb->block_off);
//            im->Insert<string>(index, fs);
//            break;
//    }
//
//}