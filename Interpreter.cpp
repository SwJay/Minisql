//
// Created by SwJ on 2018/6/22.
//

#include "Interpreter.h"
#include "CatalogManager.h"
#include "MyClass.h"
#include <utility>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>

using namespace std;

bool Interpreter::interpret(string query) {
    vector<Word>::iterator x;
    if(lexer(std::move(query))){
        if(tokens.empty());
        else if(tokens[0].token == "create"){
            if(tokens.size() >= 2){
                if(tokens[1].token == "table"){
                    if(tokens.size() >= 3)
                        CreateTable();
                    else
                        cout << "[ERROR] Syntax error: table name is missing!" << endl;
                }
                else if(tokens[1].token == "index"){
                    if(tokens.size() == 8)
                        CreateIndex();
                    else
                        cout << "[ERROR] Syntax error: wrong count of words when create index!" << endl;
                }
                else
                    cout << "[ERROR] Syntax error: can't create '" + tokens[1].token + "'!" << endl;
            }
            else
                cout << "[ERROR] Syntax error: can't resolve single 'create'!" << endl;
        }
        else if(tokens[0].token == "drop"){
            if(tokens.size() == 3){
                if(tokens[1].token == "table")
                    DropTable();
                else if(tokens[1].token == "index")
                    DropIndex();
                else
                    cout << "[ERROR] Syntax error: can't drop type '" + tokens[1].token + "'!" << endl;
            }
            else
                cout << "[ERROR] Syntax error: wrong count of words when drop table!" << endl;
        }
        else if(tokens[0].token == "select"){
            if( tokens.size() >= 4 && tokens[1].token == "*"&& tokens[2].token == "from")
                Select();
            else
                cout << "[ERROR] Syntax error: please start with 'select * from ' !" << endl;
        }
        else if(tokens[0].token == "insert"){
            if(tokens[1].token == "into")
                Insert();
            else
                cout << "[ERROR] Syntax error: can't interpret 'insert " + tokens[1].token + "'!" << endl;
        }
        else if(tokens[0].token == "delete"){
            if(tokens[1].token == "from")
                Delete();
            else
                cout << "[ERROR] Syntax error: can't interpret 'delete " + tokens[1].token + "'!" << endl;
        }
        else if(tokens[0].token == "quit"){
            if( tokens.size() == 1)
                return false;
            else
                cout << "[ERROR] Syntax error: leave 'quit' alone without '"+ tokens[1].token + "'!" << endl;
        }
        else if(tokens[0].token == "execfile")
            Execfile();
        else
            cout << "[ERROR] Syntax error: no such operation '"+ tokens[0].token + "'!" << endl;
    }
    tokens.clear();
    return true;
}

bool Interpreter::lexer(string query) {
    int cursor = 0, length = 0;
    TOKEN state = STANDBY;
    Word word;
    char c;
    while(cursor + length < query.size()){
        c = query[cursor + length];
        switch(state){
            case STANDBY:
                if(c >= 'a' && c <= 'z' || c == '_')
                    state = WORD;
                else if((c >= '0' && c <= '9') || c == '+' || c == '-' || c == '.')
                    state = NUM;
                else if(c == '=' || c == '<' || c == '>')
                    state = OPERATOR;
                else if(c == '(' || c == ')'|| c == '*' )
                    state = DOMAIN;
                else if(c == '\''){
                    state = STRING;
                    cursor++;
                }
                else if(c == ',' || c == ' ' || c == '\n' || c == '\t'){
                    cursor++;
                    length = -1;
                }
                else{
                    cout << "[ERROR] Unknown character appeared!" << endl;
                    tokens.clear();
                    return false;
                }
                break;
            case WORD:
                if((c >= 'a' && c <= 'z')|| (c >= '0' && c <= '9') || c == '_');
                else{
                    fill(query.substr(cursor, length),state);
                    cursor += length;
                    length = -1;
                    state = STANDBY;
                }
                break;
            case NUM:
                if((c >= '0' && c <= '9') || c == '+' || c == '-' || c == '.');
                else{
                    fill(query.substr(cursor, length),state);
                    cursor += length;
                    length = -1;
                    state = STANDBY;
                }
                break;
            case OPERATOR:
                if(c == '=' || c == '<' || c == '>');
                else{
                    fill(query.substr(cursor, length),state);
                    cursor += length;
                    length = -1;
                    state = STANDBY;
                }
                break;
            case DOMAIN:
                fill(query.substr(cursor,length),state);
                cursor++;
                length = -1;
                state = STANDBY;
                break;
            case STRING:
                if(c == '\''){
                    fill(query.substr(cursor,length),state);
                    cursor += length + 1;
                    length = -1;
                    state = STANDBY;
                }
                else if(c == '\n' || c == 0){
                    cout << "[ERROR] syntax error: '\''is missing when reading a string!" << endl;
                    tokens.clear();
                    return false;
                }
                else ;
                break;
        }
        length++;
    }
    if(state == OPERATOR){
        cout << "[ERROR] syntax error: right side of （in）equation is missing!" << endl;
        tokens.clear();
        return false;
    }
    else if(state == STRING){
        cout << "[ERROR] syntax error: syntax error: '\''is missing when reading a string!" << endl;
        tokens.clear();
        return false;
    }
    else if(length!=0)
        fill(query.substr(cursor),state);
    return true;
}

void Interpreter::fill(std::string query, TOKEN type) {
    Word word;
    word.token = std::move(query);
    word.type = type;
    tokens.push_back(word);
}

void Interpreter::CreateTable() {
    string table = tokens[2].token;
    string att_name;
    int type, i = 4;
    bool unique = false;
    bool primary = false;
    auto * att_v = new vector<Attribute*>;
    vector<Attribute*>::iterator a;
    if(tokens[3].token != "("){
        cout << "[ERROR] syntax error: '(' is missing when create table!" << endl;
        return ;
    }
    while( tokens[i].token != "primary" && i < tokens.size()){
        att_name = tokens[i++].token;
        if(!GetType(i,type))
            return;
        if(tokens[++i].token == "unique"){
            unique = true;
            i++;
        }
        Attribute *att  = new Attribute(att_name, type, unique, false);
        (*att_v).push_back(att);
        unique = false;
    }
    if(tokens[i++].token == "primary"){
        if(tokens[i++].token == "key") {
            if (tokens[i++].token == "(") {
                att_name = tokens[i].token;
                for (a = (*att_v).begin(); a < (*att_v).end(); a++)
                    if ((*a)->attribute_name == att_name) {
                        (*a)->primary = true;
                        break;
                    }
                if (a == (*att_v).end()) {
                    cout << "[ERROR] syntax error: '" + att_name + "' is not an attribute!" << endl;
                    return;
                }
                if(tokens[i+1].token == ")" && tokens[i+2].token == ")"){
                    api->CreateTable(table, att_v);
                    return ;
                }
                else{
                    cout << "[ERROR] syntax error: ')' is missing when create table!" << endl;
                    return;
                }
            }
            else{
                cout << "[ERROR] syntax error: ')' is missing when create table!" << endl;
                return;
            }
        }
        else{
            cout << "[ERROR] syntax error: you should declare 'primary key'!" << endl;
            return;
        }
    }
    else{
        cout << "[ERROR] syntax error: you haven't announce primary key!" << endl;
        return;
    }
}

void Interpreter::DropTable() {
    api->DropTable(tokens[2].token);
}

void Interpreter::CreateIndex() {
    if(tokens[3].token == "on" && tokens[5].token == "(" && tokens[7].token == ")")
        api->CreateIndex(tokens[2].token, tokens[4].token, tokens[6].token);
    else
        cout << "[ERROR] syntax error: fail to follow instructions like \"create index 'index_name' on 'table_name' ('attribute_name') !" << endl;
}

void Interpreter::DropIndex() {
    string index = tokens[2].token;
    api->DropIndex(tokens[2].token);
}

void Interpreter::Select() {
    int i = 5;
    string table = tokens[3].token;
    string attr, value;
    int op;
    auto *con_v = new vector<Condition*>;
    if(cm->IsTable(table)){ // table exits
        if(tokens.size() == 4) // no condition
            api->SelectRecord(table, nullptr);
        else if(tokens[4].token == "where"){
            if(tokens.size()%4){
                cout << "[ERROR] syntax error: please give conditions in 'attribute' 'operator' 'value', and join them with 'and'!" << endl;
                return;
            }
            while(i < tokens.size()){
                attr = tokens[i].token;
                if(cm->checkattributewhetherin(table,attr)){
                    if(tokens[++i].type == OPERATOR){
                        if(GetOperator(tokens[i++].token, op)){
                            if(TypeMatch(tokens[i], cm->AttributeType(table,attr))){
                                value= tokens[i++].token;
                                if(tokens.size() == i || tokens[i].token == "and"){
                                    Condition *c = new Condition(attr, op, value, cm->AttributeType(table,attr));
                                    (*con_v).push_back(c);
                                    i++;
                                }
                                else{
                                    cout << "[ERROR] syntax error: please use 'and' to connect multi conditions!" << endl;
                                    return;
                                }
                            }
                            else{
                                cout << "[ERROR] syntax error: type of the value doesn't match!" << endl;
                                return;
                            }
                        }
                        else{
                            cout << "[ERROR] syntax error: an operator is supposed to be '>', '<', '=', '>=', '<=', '<>' !" << endl;
                            return;
                        }
                    }
                    else{
                        cout << "[ERROR] syntax error: a required operator is missing in condition(s)!" << endl;
                        return;
                    }
                }
                else{
                    cout << "[ERROR] No corresponding attribute '" << attr << "' exits on table '" + table + "'!" << endl;
                    return;
                }
            }
            api->SelectRecord(table,con_v);
        }
        else
            cout << "[ERROR] syntax error: please follow 'where' to select conditionally!" << endl;
    }
    else {
        cout << "[ERROR] No corresponding table '" << table << "' exits!" << endl;
        return;
    }
}

void Interpreter::Insert() {
    string table;
    auto *value_v = new vector<string>;
    int i = 3;
    if(tokens.size() >= 7){
        table = tokens[2].token;
        if(cm->IsTable(table)){
            if(tokens[i++].token == "values"){
                if(tokens[i++].token == "("){
                    vector<int> *int_v = cm->GiveTypes(table);
                    if(tokens.size() - 6 == (*int_v).size()){
                        while (i < tokens.size() - 1){
                            if(tokens[i].type == STRING){
                                if((*int_v)[i-5] > 0){
                                    if(tokens[i].token.size() < (*int_v)[i-5] + 1)
                                        (*value_v).push_back(tokens[i++].token);
                                    else{
                                        cout << "[ERROR] length of value '" + tokens[i].token + "' is out of range!" << endl;
                                        return;
                                    }
                                }
                                else{
                                    cout << "[ERROR] type of value '" + tokens[i].token + "' is supposed to be string!" << endl;
                                    return;
                                }
                            }
                            else if(tokens[i].type == NUM){
                                if((*int_v)[i-5] <= 0)
                                    (*value_v).push_back(tokens[i++].token);
                                else{
                                    cout << "[ERROR] type of value '" + tokens[i].token + "' is supposed to be int/float!" << endl;
                                    return;
                                }
                            } else{
                                cout << "[ERROR] invalid type of value!" << endl;
                                return;
                            }
                        }
                    }
                    else {
                        cout << "[ERROR] number of values don't match the number of attributes!" << endl;
                        return;
                    }
                }
                else {
                    cout << "[ERROR] syntax error: '(' is missing when insert values!" << endl;
                    return;
                }
            }
            else {
                cout << "[ERROR] syntax error: key word 'values' is missing!" << endl;
                return;
            }
        }
        else {
            cout << "[ERROR] No corresponding table '" + table + "' exits!" << endl;
            return;
        }
    }
    else {
        cout << "[ERROR] syntax error: too few arguments!" << endl;
        return;
    }
    api->InsertRecord(table,value_v);
}

void Interpreter::Delete() {
    string table, attr, value;
    int op;
    if(tokens.size() == 3){
        table = tokens[2].token;
        if(cm->IsTable(table))
            api->DeleteRecord(table, nullptr);
        else{
            cout << "[ERROR] No corresponding table '" + table + "' exits!" << endl;
            return;
        }
    }
    else if(tokens.size() == 7){
        table = tokens[2].token;
        if(cm->IsTable(table)){
            if(tokens[3].token == "where"){
                attr = tokens[4].token;
                if(cm->checkattributewhetherin(table,attr)){
                    if(tokens[5].type == OPERATOR){
                        if(GetOperator(tokens[5].token, op)){
                            if(TypeMatch(tokens[6], cm->AttributeType(table,attr))){
                                value = tokens[6].token;
                                Condition* con = new Condition(attr,op,value,cm->AttributeType(table,attr));
                                api->DeleteRecord(table,con);
                            }
                            else{
                                cout << "[ERROR] syntax error: type of the value doesn't match!" << endl;
                                return;
                            }
                        }
                        else{
                            cout << "[ERROR] syntax error: an operator is supposed to be '>', '<', '=', '>=', '<=', '<>' !" << endl;
                            return;
                        }
                    }
                    else{
                        cout << "[ERROR] syntax error: a required operator is missing in condition(s)!" << endl;
                        return;
                    }
                }
                else{
                    cout << "[ERROR] No corresponding attribute '" + attr + "' exits on table '" + table + "'!" << endl;
                    return;
                }
            }
            else{
                cout << "[ERROR] key word 'where' is missing!" << endl;
                return;
            }
        }
        else{
            cout << "[ERROR] No corresponding table '" + table + "' exits!" << endl;
            return;
        }
    }
    else{
        cout << "[ERROR] syntax error: too few arguments!" << endl;
        return;
    }
}

void Interpreter::Execfile() {
    if(tokens.size() == 2){
        tokens.clear();
        string fname = "files/" + tokens[1].token + ".txt";
        string query;
        fstream fp(fname);
        if(fp){
            while(getline(fp,query,';'))
                interpret(query);
            fp.close();
        }
        else
            cout << "[ERROR] File '" + tokens[1].token + "' doesn't exit!" << endl;
    }
    else
        cout << "[ERROR] syntax error: no matching arguments for execfile!" << endl;
}

bool Interpreter::GetType(int &i, int &type) {
    if(tokens[i].token == "char"){
        if(tokens[++i].token != "("){
            cout << "[ERROR] syntax error: '(' is missed after '" + tokens[i-2].token + " char'!" << endl;
            return false;
        }
        if(tokens[++i].type == NUM)
            type = stoi(tokens[i].token);
        if(tokens[++i].token != ")"){
            cout << "[ERROR] syntax error: ')' is missed after 'char (" + tokens[i-1].token + "'!" << endl;
            return false;
        }
    }
    else if(tokens[i].token == "int")
        type = INT;
    else if(tokens[i].token == "float")
        type =  FLOAT;
    return true;
}

bool Interpreter::GetOperator(std::string op, int &type) {
    if(op == ">")
        type = GREATER;
    else if(op == ">=")
        type = GREATER_E;
    else if(op == "<")
        type = LESS;
    else if(op == "<=")
        type = LESS_E;
    else if(op == "=")
        type = EQUAL;
    else if(op == "<>")
        type = UNEQUAL;
    else
        return false;
    return true;
}

bool Interpreter::TypeMatch(Word w, int tb) {
    if(w.type == STRING){
        if(tb > 0){
            if(w.token.size() < tb + 1)
                return true;
            else{
                cout << "[ERROR] length of value '" + w.token + "' is out of range!" << endl;
                return false;
            }
        }
        else{
            cout << "[ERROR] type of value '" + w.token + "' is supposed to be string!" << endl;
            return false;
        }
    }
    else if(w.type == NUM){
        if(tb <= 0)
            return true;
        else{
            cout << "[ERROR] type of value '" + w.token + "' is supposed to be int/float!" << endl;
            return false;
        }
    } else{
        cout << "[ERROR] invalid type of value!" << endl;
        return false;
    }
}

