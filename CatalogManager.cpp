#include "CatalogManager.h"

string CatalogManager::getArrtribute(string tablename) {
	string data;
	string firstline;
	for (vector<TableMetadata>::const_iterator it = Metadata->begin(); it < Metadata->end(); it++) {
		string::size_type newpos;
		newpos = ((*it).firstine).find("-");
		if (((*it).firstine).substr(0, newpos) == tablename) {
			firstline = (*it).firstine;
		}
	}
	string::size_type pos1, pos2;
	pos1 = 0;
	pos2 = firstline.find('-');
	int i = 0;
	while (string::npos != pos2) {
	    i++;
	    if ((i % 4) == 2 && i != 1) {
	        data += (firstline.substr(pos1, pos2 - pos1) + '-');
	    }
	    pos1 = pos2 + 1;
	    pos2 = firstline.find('-', pos1);
	}
	return data + firstline.substr(pos1, pos2 - pos1);
}
bool CatalogManager::IsUnique(const std::string &table, Condition *condition) {
	string result;
	string fuck = "-";
	Block *block[3];
	int length = Getlength(table);
	int blockoffset = 0;
	int fileoffset = 0;
	string taildata;
	for (int j = 0; j < length; j++) {
		taildata += '-';
	}
	string attribute = condition->attribute_name;
	int order = AttributeOrder(table, attribute);
	if (condition->type == -1) {
		float v = atof((condition->value).c_str());
		block[0] = bm->FetchBlockHead(table);
		while (block[0]) {
			if (BLOCK_SIZE - blockoffset < length) {
				fileoffset++;
				blockoffset = 0;
				block[0] = bm->FetchNextBlock(block[0]);
			}
			else if (block[0]->ReadBlock(blockoffset, length) == taildata || block[0]->ReadBlock(blockoffset + 4, 1) == fuck) {
				blockoffset += length;
			}
			else {
				if (block[0]->ReadBlock<float>(blockoffset + order) == v) {
					return false;
				}
				blockoffset += length;
			}
		}
	}
	else if (condition->type == 0) {
		int v = atoi((condition->value).c_str());
		block[0] = bm->FetchBlockHead(table);
		while (block[0]) {
			if (BLOCK_SIZE - blockoffset < length) {
				fileoffset++;
				blockoffset = 0;
				block[0] = bm->FetchNextBlock(block[0]);
			}
			else if (block[0]->ReadBlock(blockoffset, length) == taildata || block[0]->ReadBlock(blockoffset + 4, 1) == fuck) {
				blockoffset += length;
			}
			else {
				if (block[0]->ReadBlock<int>(blockoffset + order) == v) {
					return false;
				}
				blockoffset += length;
			}
		}
	}
	else {
		string v = condition->value;
		int l = condition->type;
		block[0] = bm->FetchBlockHead(table);
		while (block[0]) {
			if (BLOCK_SIZE - blockoffset < length) {
				fileoffset++;
				blockoffset = 0;
				block[0] = bm->FetchNextBlock(block[0]);
			}
			else if (block[0]->ReadBlock(blockoffset, length) == taildata || block[0]->ReadBlock(blockoffset + 4, 1) == fuck) {
				blockoffset += length;
			}
			else {
				if (block[0]->ReadBlock(blockoffset + order, l) == v) {
					return false;
				}
				blockoffset += length;
			}
		}
	}
	return true;
}


bool CatalogManager::IsUnique(string tablename, string indexname) {
	if (indexname == "") {
		return false;
	}
		string::size_type pos1, pos2;
		string firstline;
		for (vector<TableMetadata>::const_iterator it = Metadata->begin(); it < Metadata->end(); it++) {
			string::size_type newpos;
			newpos = ((*it).firstine).find("-");
			if (((*it).firstine).substr(0, newpos) == tablename) {
				firstline = (*it).firstine;
			}
		}
		pos1 = 0;
		pos2 = firstline.find('-');
		while (string::npos != pos2) {
			if (firstline.substr(pos1, pos2 - pos1) == indexname) {
				int time = 3;
				while (time) {
					pos1 = pos2 + 1;
					pos2 = firstline.find('-', pos1);
					time--;
				}
				return firstline.substr(pos1, pos2 - pos1) == "1";
			}
			pos1 = pos2 + 1;
			pos2 = firstline.find('-', pos1);
		}
		return false;
}


vector<std::string>* CatalogManager:: IsTableIndexed(std::string table) {
	vector<string>* v;
	v = new vector<string>;
	for (vector<TableMetadata>::const_iterator it = Metadata->begin(); it < Metadata->end(); it++) {
		string::size_type newpos;
		newpos = ((*it).firstine).find("-");
		if (((*it).firstine).substr(0, newpos) == table) {
			for (vector<string>::const_iterator ct = ((*it).indexdata).begin(); ct < ((*it).indexdata).end(); ct++) {
				newpos = (*ct).find("-");
				v->push_back((*ct).substr(newpos + 1, (*ct).size() - newpos - 1));
				
			}
		}
	}
	return v;
}

string CatalogManager::GivePrimary(string tablename) {
		string::size_type pos1, pos2, lastpos1, lastpos2;
		string firstline;
		for (vector<TableMetadata>::const_iterator it = Metadata->begin(); it < Metadata->end(); it++) {
			string::size_type newpos;
			newpos = ((*it).firstine).find("-");
			if (((*it).firstine).substr(0, newpos) == tablename) {
				firstline = (*it).firstine;
			}
		}
		pos1 = 0;
		int i = 0;
		pos2 = firstline.find('-');
		while (string::npos != pos2) {
			//cout << i << endl;
			i++;
			if ((i % 4) == 3 && i != 1) {
				if (firstline.substr(pos1, pos2 - pos1) == "1") {
					return firstline.substr(lastpos1, lastpos2 - lastpos1);
				}
			}
			lastpos1 = pos1;
			lastpos2 = pos2;
			pos1 = pos2 + 1;
			pos2 = firstline.find('-', pos1);
		}
		return "ERROR";
}

string CatalogManager::GiveAttribute(std::string table, std::string index) {
	string data="";
	for (vector<TableMetadata>::const_iterator it = Metadata->begin(); it < Metadata->end(); it++) {
		string::size_type newpos;
		newpos = ((*it).firstine).find("-");
		if (((*it).firstine).substr(0, newpos) == table) {
			for (vector<string>::const_iterator ct = ((*it).indexdata).begin(); ct < ((*it).indexdata).end(); ct++) {
				newpos = (*ct).find("-");
				if ((*ct).substr(newpos + 1, (*ct).size() - newpos - 1) == index) {
					return (*ct).substr(0, newpos);
				}
			}
		}
	}
	return data;
}

string CatalogManager::GiveIndex(std::string table, std::string attribute) {
	string data = "";
	for (vector<TableMetadata>::const_iterator it = Metadata->begin(); it < Metadata->end(); it++) {
		string::size_type newpos;
		newpos = ((*it).firstine).find("-");
		if (((*it).firstine).substr(0, newpos) == table) {
			for (vector<string>::const_iterator ct = ((*it).indexdata).begin(); ct < ((*it).indexdata).end(); ct++) {
				newpos = (*ct).find("-");
				if ((*ct).substr(0, newpos) == attribute) {
					return (*ct).substr(newpos + 1, (*ct).size() - newpos - 1);
				}
			}
		}
	}
	return data;
}


vector<string>* CatalogManager::GiveUnique(string table) {
	string data;
	for (vector<TableMetadata>::const_iterator it = Metadata->begin(); it < Metadata->end(); it++) {
		string::size_type newpos;
		newpos = ((*it).firstine).find("-");
		if (((*it).firstine).substr(0, newpos) == table) {
			data = (*it).firstine;
		}
	}
	vector<string> * v;
	v = new vector<string>;
	string::size_type pos1[100], pos2[100];
	pos1[1] = 0;
	pos2[1] = data.find('-');
	int i = 0;
	while (string::npos != pos2[i]) {
		i++;
		if ((i % 4) == 1 && (data.substr(pos1[i], pos2[i] - pos1[i]) == "1" || data.substr(pos1[i], data.size() - 1 - pos1[i]) == "1")) {
			v->push_back(data.substr(pos1[i - 3], pos2[i - 3] - pos1[i - 3]));
		}
		pos1[i + 1] = pos2[i] + 1;
		pos2[i + 1] = data.find('-', pos1[i + 1]);

	}
	return v;
}

int CatalogManager::AttributeType(string table, string attribute) {
	string data;
	for (vector<TableMetadata>::const_iterator it = Metadata->begin(); it < Metadata->end(); it++) {
		string::size_type newpos;
		newpos = ((*it).firstine).find("-");
		if (((*it).firstine).substr(0, newpos) == table) {
			data = (*it).firstine;
		}
	}
	string::size_type pos1, pos2;
	pos1 = 0;
	pos2 = data.find('-');
	int i = 0;
	while (string::npos != pos2) {
		i++;
		//cout << data.substr(pos1, pos2 - pos1) << endl;
		if ((i % 4) == 2 && data.substr(pos1, pos2 - pos1) == attribute) {
			int time = 2;
			while (time) {
				i++;
				pos1 = pos2 + 1;
				pos2 = data.find('-', pos1);
				time--;
			}
			if (data.substr(pos1, pos2 - pos1) == "int") {
				return 0;
			}
			else if (data.substr(pos1, pos2 - pos1) == "float") {
				return -1;
			}
			else
				return atoi((data.substr(pos1, pos2 - pos1)).c_str());
		}
		pos1 = pos2 + 1;
		pos2 = data.find('-', pos1);

	}

}

int  CatalogManager::AttributeOrder(string table, string attribute) {
	int order = 0;
	string data;
	for (vector<TableMetadata>::const_iterator it = Metadata->begin(); it < Metadata->end(); it++) {
		string::size_type newpos;
		newpos = ((*it).firstine).find("-");
		if (((*it).firstine).substr(0, newpos) == table) {
			data = (*it).firstine;
		}
	}
	string::size_type pos1, pos2;
	pos1 = 0;
	pos2 = data.find('-');
	int i = 0;
	while (string::npos != pos2) {
		i++;
		if ((i % 4) == 2 && data.substr(pos1, pos2 - pos1) == attribute) {
			return order;
		}
		else if ((i % 4) == 2 && data.substr(pos1, pos2 - pos1) != attribute) {
			int time = 2;
			while (time) {
				i++;
				pos1 = pos2 + 1;
				pos2 = data.find('-', pos1);
				time--;
			}
			if (data.substr(pos1, pos2 - pos1) == "int" || data.substr(pos1, pos2 - pos1) == "float") {
				order += 4;
			}
			else {
				order += atoi((data.substr(pos1, pos2 - pos1)).c_str());
			}
		}
		pos1 = pos2 + 1;
		pos2 = data.find('-', pos1);

	}

}

int  CatalogManager::Attributerank(string table, string attribute) {
	int order = 0;
	string data;
	for (vector<TableMetadata>::const_iterator it = Metadata->begin(); it < Metadata->end(); it++) {
		string::size_type newpos;
		newpos = ((*it).firstine).find("-");
		if (((*it).firstine).substr(0, newpos) == table) {
			data = (*it).firstine;
		}
	}
	string::size_type pos1, pos2;
	pos1 = 0;
	pos2 = data.find('-');
	int i = 0;
	while (string::npos != pos2) {
		i++;
		if ((i % 4) == 2 && data.substr(pos1, pos2 - pos1) == attribute) {
			return order;
		}
		else if ((i % 4) == 2 && data.substr(pos1, pos2 - pos1) != attribute) {
			order++;
		}
		pos1 = pos2 + 1;
		pos2 = data.find('-', pos1);

	}
}


int  CatalogManager::Getlength(string table) {
	string filename = "Metadata " + table + ".txt";
	int order = 0;
	string data;
	for (vector<TableMetadata>::const_iterator it = Metadata->begin(); it < Metadata->end(); it++) {
		string::size_type newpos;
		newpos = ((*it).firstine).find("-");
		if (((*it).firstine).substr(0, newpos) == table) {
			data = (*it).firstine;
		}
	}
	string::size_type pos1, pos2;
	pos1 = 0;
	pos2 = data.find('-');
	int i = 0;
	while (string::npos != pos2) {
		i++;
		if ((i % 4) == 2) {
			int time = 2;
			while (time) {
				i++;
				pos1 = pos2 + 1;
				pos2 = data.find('-', pos1);
				time--;
			}
			if (data.substr(pos1, pos2 - pos1) == "int" || data.substr(pos1, pos2 - pos1) == "float") {
				order += 4;
			}
			else {
				order += atoi((data.substr(pos1, pos2 - pos1)).c_str());
			}
		}
		pos1 = pos2 + 1;
		pos2 = data.find('-', pos1);

	}
	if (order >= 9) {
		return order;
	}
	else {
		return 9;
	}

}


vector<int>* CatalogManager::GiveTypes(string tablename) {
	vector<int>* v;
	v = new vector<int>;
	string firstline;
	for (vector<TableMetadata>::const_iterator it = Metadata->begin(); it < Metadata->end(); it++) {
		string::size_type newpos;
		newpos = ((*it).firstine).find("-");
		if (((*it).firstine).substr(0, newpos) == tablename) {
			firstline = (*it).firstine;
		}
	}
		string::size_type pos1, pos2;
		pos1 = 0;
		pos2 = firstline.find("-");
		int i = 0;
		while (std::string::npos != pos2) {
			i++;
			if ((i % 4) == 0 && i != 0) {
				int temp = atoi((firstline.substr(pos1, pos2 - pos1)).c_str());
				v->push_back(temp);
			}
			pos1 = pos2 + 1;
			pos2 = firstline.find("-", pos1);
		}

	return v;
}
bool CatalogManager::DropIndex(string index) {
	string tablename = DeleteIndexFromVagina(index);
	if (tablename != "NULL") {
		int i = deleteIndex(tablename, index);
		return i != 0;
	}
	else {
		return false;
	}
}
bool CatalogManager::deleteIndex(std::string tablename, std::string indexname) {
	for (vector<TableMetadata>::iterator it = Metadata->begin(); it < Metadata->end(); it++) {
		string::size_type newpos;
		newpos = ((*it).firstine).find("-");
		if (((*it).firstine).substr(0, newpos) == tablename) {
			for (vector<string>::iterator ct = ((*it).indexdata).begin(); ct < ((*it).indexdata).end(); ct++) {
				newpos = (*ct).find("-");
				//string temp = (*ct).substr(newpos, (*ct).size() - newpos);
				if ((*ct).substr(newpos+1, (*ct).size() - newpos-1) == indexname) {
					//temp = (*ct).substr(newpos, (*ct).size() - newpos);
					(&((*it).indexdata))->erase(ct);
					return true;
				}
			}
		}
	}
	return false;
}

bool CatalogManager::IsIndex(std::string Indexname) {
    if(Indexname==""){
        return false;
    }
	for (vector<Index>::const_iterator it = index->begin(); it < index->end(); it++) {
		if ((*it).indexname == Indexname) {
			//index->erase(it);
			return true;
		}
	}
	return false;
}
bool CatalogManager::IsIndex(std::string Tablename, std::string Attribute) {
	for (vector<TableMetadata>::const_iterator it = Metadata->begin(); it < Metadata->end(); it++) {
		string::size_type npos;
		npos = ((*it).firstine).find("-");
		if (((*it).firstine).substr(0, npos) == Tablename) {
			for (vector<string>::const_iterator ct = (*it).indexdata.begin(); ct < (*it).indexdata.end(); ct++) {
				string::size_type newpos;
				newpos = (*ct).find("-");
				if ((*ct).substr(0, newpos) == Attribute){
					return true;
				}
			}
			return false;
		}
	}
	return false;
}

string CatalogManager::DeleteIndexFromVagina(std::string indexname) {
	string data = "NULL";
	for (vector<Index>::const_iterator it = index->begin(); it < index->end(); it++) {
		if ((*it).indexname == indexname) {
			data = (*it).tablename;
			index->erase(it);
			return data;
		}
	}
	return data;
}

vector<InfoBlock<string> *> * CatalogManager::CreateIndex(const string &index, const string &tablename, const string &attribute) {
	AddIndexIntoVagina(tablename, index);
	vector<InfoBlock<string>* >* v = addIndex<string>(tablename, attribute, index);
	return v;
}

bool CatalogManager::AddIndexIntoVagina(string tablename, string indexname) {
	Index in;
	in.indexname = indexname;
	in.tablename = tablename;
	index->push_back(in);
	return true;
}

bool CatalogManager::DropTable(string tablename) {
	//调用buffermanager里的删除函数
	for (vector<TableMetadata>::const_iterator it = Metadata->begin(); it < Metadata->end(); it++) {
		string table;
		string::size_type pos;
		pos = ((*it).firstine).find("-");
		table = ((*it).firstine).substr(0, pos);
		if (table == tablename) {
			Metadata->erase(it);
			return true;
		}
	}
	return false;
	
}
bool CatalogManager::CreateTable(std::string tablename, vector<Attribute*> *attributes) {
		string metadata = tablename;
		for (vector<Attribute*>::const_iterator it = attributes->begin(); it < attributes->end(); it++) {
			metadata += ('-' + (*it)->attribute_name);
			if ((*it)->primary == 1) {
				metadata += "-1";
			}
			else {
				metadata += "-0";
			}
			if ((*it)->type == -1) {
				metadata += "-float";
			}
			else if ((*it)->type == 0) {
				metadata += "-int";
			}
			else {
				metadata += ('-' + to_string((*it)->type));
			}
			if ((*it)->unique == 1) {
				metadata += "-1";
			}
			else {
				metadata += "-0";
			}
			//cout<< Metadata << endl;
		}
		TableMetadata t;
		t.firstine = (metadata);
		Metadata->push_back(t);
		return true;
	
	return false;
}
CatalogManager::~CatalogManager() {
	fstream file;
	string filename = "index.txt";
	file.open(filename, ios::in | ios::out |ios::trunc);
	if (file.is_open()) {
		for (vector<Index>::const_iterator it = index->begin(); it < index->end(); it++) {
			if ((*it).indexname != "") {
				file << (*it).indexname << "-" << (*it).tablename << "*" << endl;
			}
		}
	}
	file.close();
	filename = "table.txt";


	file.open(filename,  ios::in | ios::out|ios::trunc);
	if (file.is_open()) {
		for (vector<TableMetadata>::const_iterator it = Metadata->begin(); it < Metadata->end(); it++) {
			if ((*it).firstine != "") {
				file << "fuck" << "*" << endl;
				file << (*it).firstine << "*" << endl;
				for (vector<string>::const_iterator ct = ((*it).indexdata).begin(); ct < ((*it).indexdata).end(); ct++) {
					if ((*ct) != "") {
						file << (*ct) << "*" << endl;
					}
				}
			}
		}
	}
}
vector<TableMetadata>* CatalogManager::InitMetadata() {
	vector<TableMetadata>* v = this->Metadata;
	return v;
}
vector<Index>*  CatalogManager::InitIndex() {
	vector<Index>* v = this->index;
	return v;
}

bool CatalogManager::IsTable(string tablename) {
	for (vector<TableMetadata>::const_iterator it = Metadata->begin(); it < Metadata->end(); it++) {
		string::size_type pos;
		string data = (*it).firstine;
		pos = data.find("-");
		if (data.substr(0, pos) == tablename) {
			return true;
		}
	}
	return false;
}


CatalogManager::CatalogManager() {
	fstream file;
	string filename = "index.txt";
	string data;
	file.open(filename, ios::out | ios::in );
	if (file.is_open()) {
		index = new vector<Index>;
		Index in;
		while (!file.eof()) {
			string::size_type getllinepos;
			getline(file, data);
			getllinepos = data.find("*");
			data = data.substr(0, getllinepos);
			string::size_type pos;
			pos = data.find("-");
				in.indexname = data.substr(0, pos);
				in.tablename = data.substr(pos + 1, data.size() - 1);
				index->push_back(in);
				//getline(file, data);

		}
	}
	file.close();
	filename = "table.txt";
	file.open(filename, ios::out | ios::in );
	if (file.is_open()) {
		Metadata = new vector<TableMetadata>;
		getline(file, data);
		string::size_type getllinepos;
		getllinepos = data.find("*");
		data = data.substr(0, getllinepos-0);
		while (!file.eof()) {
			getline(file, data);
			getllinepos = data.find("*");
			data = data.substr(0, getllinepos-0);
			TableMetadata tm;
			tm.firstine = data;
			while (1) {
				getline(file, data);
				getllinepos = data.find("*");
				data = data.substr(0, getllinepos);
				if (data.substr(0,data.size()-1) == "fuck"|| data.substr(0, data.size() ) == "fuck") {
					break;
				}
				if (data != "") {
					(tm.indexdata).push_back(data);
				}
				 if (file.eof()) {
					break;
				}
				//getline(file, data);
				
			}
			Metadata->push_back(tm);
		}

	}
}
string CatalogManager::Tabletype(string tablename) {
	string data;
	string firstline;
	for (vector<TableMetadata>::const_iterator it = Metadata->begin(); it < Metadata->end(); it++) {
		string::size_type newpos;
		newpos = ((*it).firstine).find("-");
		if (((*it).firstine).substr(0, newpos) == tablename) {
			firstline = (*it).firstine;
		}
	}
		string::size_type pos1, pos2;
		pos1 = 0;
		pos2 = firstline.find('-');
		int i = 0;
		while (string::npos != pos2) {
			i++;
			if ((i % 4) == 0 && i != 0) {
				data += (firstline.substr(pos1, pos2 - pos1) + '-');
			}
			pos1 = pos2 + 1;
			pos2 = firstline.find('-', pos1);
		}
		return data;
}

bool CatalogManager::checkattributewhetherin(string tablename, string Arrtribute) {
	vector<string> v;
	string::size_type pos1, pos2;
	pos1 = 0;
	string firstline;
	for (vector<TableMetadata>::const_iterator it = Metadata->begin(); it < Metadata->end(); it++) {
		string::size_type newpos;
		newpos = ((*it).firstine).find("-");
		if (((*it).firstine).substr(0, newpos) == tablename) {
			firstline = (*it).firstine;
		}
	}
	pos2 = firstline.find('-');
	while (string::npos != pos2) {

		if (Arrtribute == firstline.substr(pos1, pos2 - pos1)) {
			return true;
		}
		pos1 = pos2 + 1;
		pos2 = firstline.find('-', pos1);
	}
	return false;
}
