//
// Created by SwJ on 2018/6/22.
//

#ifndef INTERPRETER_INTERPRETER_H
#define INTERPRETER_INTERPRETER_H


#include "API.h"
#include "MyClass.h"
#include <string>
#include <vector>
#include <map>

enum TOKEN{
    STANDBY,
    NUM,
    WORD,
    OPERATOR,
    DOMAIN,
    STRING
};

struct Word{
    std::string token;
    TOKEN type;
};

class Interpreter{
public:
    Interpreter(){api= new API;};
    ~Interpreter(){delete api;};
    bool interpret(std::string);
private:
    API* api;
    CatalogManager *cm= MiniSQL::GetCM();
    std::vector<Word> tokens;

    bool lexer(std::string);
    void fill(std::string, TOKEN);

    bool GetType(int &i, int &type);
    bool GetOperator(std::string op, int &type);
    bool TypeMatch(Word w, int tb);

    void CreateTable();
    void DropTable();
    void CreateIndex();
    void DropIndex();
    void Select();
    void Insert();
    void Delete();
    void Execfile();
};

#endif //INTERPRETER_INTERPRETER_H
