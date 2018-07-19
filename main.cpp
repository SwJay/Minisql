#include <iostream>
#include "Interpreter.h"
#include "MyClass.h"
#include <string>
//#include "IndexManager.h"

using namespace std;

//int main(){
//    IndexManager test;
//    test.Print<string>("test");
//}
int main() {
	Interpreter it;
	MiniSQL mini_sql;
	string query;
	cout << "Welcome to the MiniSQL monitor.  Commands end with ;" << endl;
	while (true) {
		cout << "minisql> ";
		getline(cin, query, ';');
		if (!it.interpret(query))
			break;
	}
	return 0;
}