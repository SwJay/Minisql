#include "MyClass.h"
#include "RecordManager.h"
#include "IndexManager.h"
#include "CatalogManager.h"
#include "BufferManager.h"

BufferManager* MiniSQL::bm = new BufferManager;
IndexManager* MiniSQL::im= new IndexManager;
CatalogManager* MiniSQL::cm= new CatalogManager;
RecordManager* MiniSQL::rm= new RecordManager;

MiniSQL::~MiniSQL() {
    delete im;
    delete cm;
    delete rm;
    delete bm;
}



