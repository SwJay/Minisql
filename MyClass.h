#ifndef API_MYCLASS_H
#define API_MYCLASS_H

#include <string>
#include <utility>
#include <iostream>

class BufferManager;
class IndexManager;
class RecordManager;
class CatalogManager;

template <typename key>
class InfoBlock{
public:
    key value;
    int file_off;
    int block_off;
    InfoBlock(key v, int f, int b):value(v),file_off(f),block_off(b){};
};

class Attribute{
public:
    std::string attribute_name;
    int type;
    bool unique;
    bool primary;
    Attribute(std::string name, int t, bool u, bool p):attribute_name(std::move(name)),type(t),unique(u),primary(p){};
};

class Condition{
public:
    std::string attribute_name;
    int operate;
    std::string value;
    int type;
    Condition(std::string name, int o, std::string v, int t):attribute_name(std::move(name)), operate(o),value(
            std::move(v)),type(t){};
};

class MiniSQL {
private:
    static BufferManager* bm;
	static IndexManager* im;
	static CatalogManager* cm;
	static RecordManager* rm;
public:
    MiniSQL(){std::cout<<"const"<<std::endl;};
    ~MiniSQL();
	static BufferManager* GetBM() { return bm; }
	static IndexManager* GetIM() { return im; }
	static CatalogManager* GetCM() { return cm; }
	static RecordManager* GetRM() { return rm; }
};
#endif //API_MYCLASS_H
