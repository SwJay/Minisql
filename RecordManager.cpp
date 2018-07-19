#include "RecordManager.h"

string RecordManager::SelectRecords(string table, vector<Condition*> *conditions) {
    string result;
    Block *block[3];
    int blockoffset = 0;
    int fileoffset = 0;
    int length = cm->Getlength(table);
    string taildata;
    for (int i = 0; i < length; i++) {
        taildata += "-";
    }
    string fuck = "-";
    block[0] = bm->FetchBlockHead(table);
    while (block[0]) {
        if (BLOCK_SIZE - blockoffset < length) {
            block[0] = bm->FetchNextBlock(block[0]);
            fileoffset++;
            blockoffset = 0;
        }
        else if (block[0]->ReadBlock(blockoffset, length) == taildata || block[0]->ReadBlock(blockoffset + 4, 1) == fuck) {
            blockoffset+=length;
        }
        else {
            int flag = 1;
            int order;
            for (vector<Condition*>::const_iterator it = conditions->begin(); it < conditions->end(); it++) {
                order = cm->AttributeOrder(table, (*it)->attribute_name);
                if ((*it)->type == 0) {
                    int rvalue = atoi(((*it)->value).c_str());
                    int v = block[0]->ReadBlock<int>(blockoffset);
                    if ((*it)->operate == 0) {
                        if (v != rvalue) {
                            flag = 0;
                            break;
                        }
                    }
                    else if ((*it)->operate == -1) {
                        if (v == rvalue) {
                            flag = 0;
                            break;
                        }
                    }
                    else if ((*it)->operate == 2) {
                        if (v <= rvalue) {
                            flag = 0;
                            break;
                        }
                    }
                    else if ((*it)->operate == 3) {
                        if (v < rvalue) {
                            flag = 0;
                            break;
                        }
                    }
                    else if ((*it)->operate == 4) {
                        if (v >= rvalue) {
                            flag = 0;
                            break;
                        }
                    }
                    else if ((*it)->operate == 5) {
                        if (v > rvalue) {
                            flag = 0;
                            break;
                        }
                    }
                }
                else if ((*it)->type == -1) {
                    float rvalue = atof(((*it)->value).c_str());
                    float v = block[0]->ReadBlock<float>(blockoffset);
                    if ((*it)->operate == 0) {
                        if (v != rvalue) {
                            flag = 0;
                            break;
                        }
                    }
                    else if ((*it)->operate == -1) {
                        if (v == rvalue) {
                            flag = 0;
                            break;
                        }
                    }
                    else if ((*it)->operate == 2) {
                        if (v <= rvalue) {
                            flag = 0;
                            break;
                        }
                    }
                    else if ((*it)->operate == 3) {
                        if (v < rvalue) {
                            flag = 0;
                            break;
                        }
                    }
                    else if ((*it)->operate == 4) {
                        if (v >= rvalue) {
                            flag = 0;
                            break;
                        }
                    }
                    else if ((*it)->operate == 5) {
                        if (v > rvalue) {
                            flag = 0;
                            break;
                        }
                    }
                }
                else {
                    string rvalue = (*it)->value;
                    string v = block[0]->ReadBlock(blockoffset+order,cm->AttributeType(table,(*it)->attribute_name));
                    if ((*it)->operate == 0) {
                        if (v != rvalue) {
                            flag = 0;
                            break;
                        }
                    }
                    else if ((*it)->operate == -1) {
                        if (v == rvalue) {
                            flag = 0;
                            break;
                        }
                    }
                    else if ((*it)->operate == 2) {
                        if (v <= rvalue) {
                            flag = 0;
                            break;
                        }
                    }
                    else if ((*it)->operate == 3) {
                        if (v < rvalue) {
                            flag = 0;
                            break;
                        }
                    }
                    else if ((*it)->operate == 4) {
                        if (v >= rvalue) {
                            flag = 0;
                            break;
                        }
                    }
                    else if ((*it)->operate == 5) {
                        if (v > rvalue) {
                            flag = 0;
                            break;
                        }
                    }
                }
            }
            if (flag == 1) {
                string data = cm->Tabletype(table);
                string::size_type pos1, pos2;
                pos1 = 0;
                pos2 = data.find('-');
				int init = blockoffset;
                while (string::npos != pos2) {
                    string temp = data.substr(pos1, pos2 - pos1);
                    if (temp == "int") {
                        result += (to_string(block[0]->ReadBlock<int>(blockoffset)) + "\t"+" | ");
                        blockoffset += 4;
                    }
                    else if (temp == "float") {
                        result += (to_string(block[0]->ReadBlock<float>(blockoffset)) +"\t"+ " | ");
                        blockoffset += 4;
                    }
                    else {
                        int l = atoi(temp.c_str());
                        result += (block[0]->ReadBlock(blockoffset, l)) + "\t"+" | ";
                        blockoffset += l;
                    }
                    pos1 = pos2 + 1;
                    pos2 = data.find('-', pos1);
                }
                result += "\n";
				blockoffset = init + length;

            }
            else {
                blockoffset += length;
            }

        }
    }
    for (vector<Condition*>::const_iterator it = conditions->begin(); it < conditions->end(); it++) {
        delete (*it);
    }
    return result;
}

string RecordManager::AllRecords(string table) {
    string fuck = "-";
    string result;
    int length;
    length = cm->Getlength(table);
    Block *block[2];
    int i = 0;
    string taildata;
    for (int j = 0; j < length; j++) {
        taildata += '-';
    }
    block[0]=bm->FetchRandomBlock(table,i);
    int blockoffset = 0;
    while (block[0]) {
        if (BLOCK_SIZE - blockoffset <= length) {
            block[0] = bm->FetchNextBlock(block[0]);
            blockoffset = 0;
        }
        else if (block[0]->ReadBlock(blockoffset, length) == taildata|| block[0]->ReadBlock(blockoffset+4,1)==fuck) {
            blockoffset += length;
        }
        else {
            string data = cm->Tabletype(table);
            string::size_type pos1, pos2;
            pos1 = 0;
            pos2 = data.find('-');
			int init = blockoffset;
            while (string::npos != pos2) {
                string temp = data.substr(pos1, pos2 - pos1);
                if (temp == "int") {
                    result += (to_string(block[0]->ReadBlock<int>(blockoffset))+"\t"+" | ");
                    blockoffset += 4;
                }
                else if (temp == "float") {
                    result += (to_string(block[0]->ReadBlock<float>(blockoffset)) +"\t"+" | ");
                    blockoffset += 4;
                }
                else {
                    int l = atoi(temp.c_str());
                    result += (block[0]->ReadBlock(blockoffset, l))+ "\t" +" | ";
                    blockoffset += l;
                }
                pos1 = pos2 + 1;
                pos2 = data.find('-', pos1);
            }
            result += "\n";
			blockoffset = init + length;
        }
    }
    return result;
}

bool RecordManager::InsertRecords(string tablename, vector<string>* lol) {
    string record;
    int k = 0;
    Block* block[3];
    string filename = tablename;
    int length = cm->Getlength(tablename);
    int finalfileoffset, finalblockoffset;
    string data = cm->Tabletype(tablename);
    //cout << data << endl;
    block[0] = bm->FetchRandomBlock(filename, 0);
    block[0]->SetPin(true);
    //cout << block[0]->ReadBlock(0, length) << endl;
    for (vector<string>::const_iterator it = lol->begin(); it < lol->end(); it++) {
        record += ((*it) + "-");
    }
    //cout << "record"<<record << endl;
    string firstline;
    firstline = block[0]->ReadBlock(0, length);
    //cout << firstline << endl;
    //int fileoffset = block[0]->ReadBlock<int>(0);
    string taildata;
    for (int i = 0; i<length; i++) {
        taildata += "-";
    }
    if (firstline.substr(0, static_cast<unsigned long long int>(length)) == taildata) {
        block[0]->WriteBlock(0, 0);
        block[0]->WriteBlock(4, (char*)"-", 1);
        block[0]->WriteBlock(5, 2*length);
        finalfileoffset = 0;
        finalblockoffset = length;
        //cout << block[0]->ReadBlock<int>(0) << block[0]->ReadBlock(4, 1) << block[0]->ReadBlock<int>(5) << endl;
        int blocktail = length;
        string::size_type datapos1, datapos2, recpos1, recpos2;
        datapos1 = 0;
        recpos1 = 0;
        string tempdata;
        datapos2 = data.find('-');
        recpos2 = record.find('-');
        //cout << string::npos << "           "<<datapos2<<endl;
        while (string::npos != datapos2) {
            tempdata = record.substr(recpos1, recpos2 - recpos1);
            //cout <<tempdata << endl;
            //cout << "data"<<data.substr(datapos1, datapos2 - datapos1) << endl;
            if (data.substr(datapos1, datapos2 - datapos1) == "int") {
                //cout << "?";
                block[0]->WriteBlock(blocktail, atoi(tempdata.c_str()));
                blocktail += 4;
            }
            else if (data.substr(datapos1, datapos2 - datapos1) == "float") {
                //cout << "!";
                block[0]->WriteBlock(blocktail, atof(tempdata.c_str()));
                blocktail += 4;
            }
            else {
                //cout << "@";
                int l = atoi((data.substr(datapos1, datapos2 - datapos1)).c_str());
                auto * p = (char*)tempdata.c_str();
                block[0]->WriteBlock(blocktail, p, l);
                blocktail += l;
            }
            datapos1 = datapos2 + 1;
            datapos2 = data.find('-', datapos1);
            recpos1 = recpos2 + 1;
            recpos2 = record.find('-', recpos1);
        }
        tempdata = record.substr(recpos1, recpos2 - recpos1);
        if (data.substr(datapos1, data.size() - 1 - datapos1) == "int") {
            block[0]->WriteBlock(blocktail, atoi(tempdata.c_str()));
            blocktail += 4;
        }
        else if (data.substr(datapos1, data.size() - 1 - datapos1) == "float") {
            block[0]->WriteBlock(blocktail, atof(tempdata.c_str()));
            blocktail += 4;
        }
        else {
            int l = atoi((data.substr(datapos1, datapos2 - datapos1)).c_str());
            auto * p = (char*)tempdata.c_str();
            block[0]->WriteBlock(blocktail, p, l);
            blocktail += l;
        }
    }
    else {
        int fileoffset = block[0]->ReadBlock<int>(0);
        int blockoffset = block[0]->ReadBlock<int>(5);
        //cout << block[0]->ReadBlock(0, 1) << endl;
        //cout << fileoffset << "             "<<blockoffset << endl;
        int init = blockoffset;
        block[1] = bm->FetchRandomBlock(filename, fileoffset);
        //
        if (BLOCK_SIZE - blockoffset < length) {
            blockoffset = 0;
            block[2]= bm->FetchNextBlock(block[1]);
            if(!block[2])
                block[1] = bm->AppendBlock(block[1]);
            else
                block[1] = block[2];
            block[0]->WriteBlock(0, fileoffset + 1);
            block[0]->WriteBlock(4, (char *)"-", 1);
            block[0]->WriteBlock(5, length);
            finalfileoffset = fileoffset + 1;
            finalblockoffset = 0;
        }
        else if (block[1]->ReadBlock(blockoffset, length) == taildata) {
            block[0]->WriteBlock(0, fileoffset);
            block[0]->WriteBlock(4, (char *)"-", 1);
            block[0]->WriteBlock(5, blockoffset + length);
            finalfileoffset = fileoffset;
            finalblockoffset = blockoffset;
        }
        else {
            int newfileoffset = block[1]->ReadBlock<int>(init);
            int newblockoffset = block[1]->ReadBlock<int>(init + 5);
            block[0]->WriteBlock(0, newfileoffset);
            block[0]->WriteBlock(4, (char *)"-", 1);
            block[0]->WriteBlock(5, newblockoffset);
            finalfileoffset = fileoffset;
            finalblockoffset = blockoffset;
            //cout << "v"<<finalfileoffset << finalblockoffset << endl;
            //cout << block[0]->ReadBlock<int>(5) << endl;
        }
        string::size_type datapos1, datapos2, recpos1, recpos2;
        datapos1 = 0;
        recpos1 = 0;
        string tempdata;
        datapos2 = data.find('-');
        recpos2 = record.find('-');
        while (string::npos != datapos2) {
            tempdata = record.substr(recpos1, recpos2 - recpos1);
            if (data.substr(datapos1, datapos2 - datapos1) == "int") {
                block[1]->WriteBlock(blockoffset, atoi(tempdata.c_str()));
                blockoffset += 4;
            }
            else if (data.substr(datapos1, datapos2 - datapos1) == "float") {
                block[1]->WriteBlock(blockoffset, atof(tempdata.c_str()));
                blockoffset += 4;
            }
            else {
                int l = atoi((data.substr(datapos1, datapos2 - datapos1)).c_str());
                auto * p = (char*)tempdata.c_str();
                block[1]->WriteBlock(blockoffset, p, l);
                blockoffset += l;
            }
            datapos1 = datapos2 + 1;
            datapos2 = data.find('-', datapos1);
            recpos1 = recpos2 + 1;
            recpos2 = record.find('-', recpos1);
        }
        tempdata = record.substr(recpos1, recpos2 - recpos1);
        if (data.substr(datapos1, data.size() - 1 - datapos1) == "int") {
            block[1]->WriteBlock(blockoffset, atoi(tempdata.c_str()));
            blockoffset += 4;
        }
        else if (data.substr(datapos1, data.size() - 1 - datapos1) == "float") {
            block[1]->WriteBlock(blockoffset, atof(tempdata.c_str()));
            blockoffset += 4;
        }
        else {
            int l = atoi((data.substr(datapos1, datapos2 - datapos1)).c_str());
            auto * p = (char*)tempdata.c_str();
            block[1]->WriteBlock(blockoffset, p, l);
            blockoffset += l;
        }
        //cout << blockoffset << endl;
        //cout <<block[1]->ReadBlock<int>(0) << endl;
        //cout << block[1]->ReadBlock(0+4, 1) << endl;
        //cout << block[1]->ReadBlock<int>(0 + 5) << endl;
    }
    //我需要告诉你，INDEXNAME,VALUE,两个offset？
    //cout <<"final" <<finalfileoffset << "  " << finalblockoffset << endl;
    vector<string> *vfinal = cm->IsTableIndexed(tablename);
    block[0]->SetPin(false);
    block[0] = bm->FetchRandomBlock(tablename, finalfileoffset);
    block[0]->SetPin(true);
    for (vector<string > ::const_iterator it = vfinal->begin(); it < vfinal->end(); it++) {
        string aaa = *it;
        string finalattribute = cm->GiveAttribute(tablename, aaa);
        //cout << finalattribute << endl;
        int finaltype = cm->AttributeType(tablename,finalattribute);
        int finalorder = cm->AttributeOrder(tablename, finalattribute);
        string whatfuck = (*it);
        //cout << cm->GiveIndex(tablename, finalattribute) << endl;
        string finalindex = cm->GiveIndex(tablename, finalattribute);
        if (finaltype== 0){
            int finalvalue = block[0]->ReadBlock<int>(finalblockoffset + finalorder);
            InfoBlock<int> * v;
            v = new InfoBlock<int>(finalvalue, finalfileoffset, finalblockoffset);
//            cout << finalvalue << endl;
//            cout << finalfileoffset << endl;
//            cout << finalblockoffset << endl;
            im->Insert<int>(finalindex, v);
        }
        else if (finaltype == -1) {
            float finalvalue = block[0]->ReadBlock<float>(finalblockoffset + finalorder);
            InfoBlock<float> * v;
            v = new InfoBlock<float>(finalvalue, finalfileoffset, finalblockoffset);
            //cout << finalvalue << endl;
            //cout << finalfileoffset << endl;
            //cout << finalblockoffset << endl;
            im->Insert<float>(finalindex, v);
        }
        else {
            string finalvalue = block[0]->ReadBlock(finalblockoffset + finalorder,finaltype);
            InfoBlock<string> * v;
            v = new InfoBlock<string>(finalvalue, finalfileoffset, finalblockoffset);
            //cout << finalvalue << endl;
            //cout << finalfileoffset << endl;
            //cout << finalblockoffset << endl;
            im->Insert<string>(finalindex, v);
        }
    }
    block[0]->SetPin(false);
    return true;
}

void RecordManager:: DeleteRecord(string table, Condition* condition) {
    string result;
    string fuck = "-";
    int length;
    length = cm->Getlength(table);
    Block *block[2];
    int i = 0;
    string taildata;
    //vector<InfoBlock<key>*> * vv;
    //vv = new vector<InfoBlock<key>*>;
    string vfinal;
    //InfoBlock<key> * in;
    for (int j = 0; j < length; j++) {
        taildata += "-";
    }
    block[0] = bm->FetchRandomBlock(table, i);

    int fileoffset = 0;
    int blockoffset = 0;
    while (block[0]) {
        if (BLOCK_SIZE - blockoffset < length) {
            block[0] = bm->FetchNextBlock(block[0]);
            fileoffset++;
            blockoffset = 0;
        }
        else if (block[0]->ReadBlock(blockoffset, length) == taildata||block[0]->ReadBlock(blockoffset+4,1)==fuck) {
            blockoffset += length;
        }
        else {
            int order;
            int flag = 0;
            order = cm->AttributeOrder(table,condition->attribute_name);
            if (condition->type == 0) {
                int rvalue = atoi((condition->value).c_str());
                int v = block[0]->ReadBlock<int>(blockoffset+order);
                vfinal = to_string(v);
                if (condition->operate == 0) {
                    if (v == rvalue) {
                        flag = 1;
                    }
                }
                else if (condition->operate == -1) {
                    if (v != rvalue) {
                        flag = 1;
                    }
                }
                else if (condition->operate == 2) {
                    if (v > rvalue) {
                        flag = 1;
                    }
                }
                else if (condition->operate == 3) {
                    if (v >= rvalue) {
                        flag = 1;
                    }
                }
                else if (condition->operate == 4) {
                    if (v < rvalue) {
                        flag = 1;
                    }
                }
                else if (condition->operate == 5) {
                    if (v <= rvalue) {
                        flag = 1;
                    }
                }
            }
            else if (condition->type == -1) {
                float rvalue = atof((condition->value).c_str());
                float v = block[0]->ReadBlock<float>(blockoffset + order);
                vfinal = to_string(v);

                if (condition->operate == 0) {
                    if (v == rvalue) {
                        flag = 1;
                    }
                }
                else if (condition->operate == -1) {
                    if (v != rvalue) {
                        flag = 1;
                    }
                }
                else if (condition->operate == 2) {
                    if (v > rvalue) {
                        flag = 1;
                    }
                }
                else if (condition->operate == 3) {
                    if (v >= rvalue) {
                        flag = 1;
                    }
                }
                else if (condition->operate == 4) {
                    if (v < rvalue) {
                        flag = 1;
                    }
                }
                else if (condition->operate == 5) {
                    if (v <= rvalue) {
                        flag = 1;
                    }
                }
            }
            else {
                string rvalue = ((condition->value).c_str());
                string v;
                try {
                    v = block[0]->ReadBlock(blockoffset + order, cm->AttributeType(table, condition->attribute_name));
                }
                catch (exception e) {
                    break;
                }
                //vfinal = to_string(v);
                vfinal = v;
                if (condition->operate == 0) {
                    if (v == rvalue) {
                        flag = 1;
                    }
                }
                else if (condition->operate == -1) {
                    if (v != rvalue) {
                        flag = 1;
                    }
                }
                else if (condition->operate == 2) {
                    if (v > rvalue) {
                        flag = 1;
                    }
                }
                else if (condition->operate == 3) {
                    if (v >= rvalue) {
                        flag = 1;
                    }
                }
                else if (condition->operate == 4) {
                    if (v < rvalue) {
                        flag = 1;
                    }
                }
                else if (condition->operate == 5) {
                    if (v <= rvalue) {
                        flag = 1;
                    }
                }
            }
            if (flag == 0) {
                blockoffset += length;
            }
            else {
                block[1] = bm->FetchBlockHead(table);
                int oldefileoffset = block[1]->ReadBlock<int>(0);
                int oldblockoffset = block[1]->ReadBlock<int>(5);

                block[1]->WriteBlock<int>(0, fileoffset);
                block[1]->WriteBlock<char>(4, '-');
                block[1]->WriteBlock<int>(5, blockoffset);
                //cout << oldblockoffset << oldefileoffset << blockoffset << fileoffset << endl;
                //cout << block[1]->ReadBlock<int>(0) << block[1]->ReadBlock<int>(5) <<endl<< block[0]->ReadBlock<int>(0) << block[0]->ReadBlock<int>(5) << endl;
                int finalfileoffset = fileoffset;
                int finalblockoffset=blockoffset;
                vector<string> *vfuck = cm->IsTableIndexed(table);
                block[0] = bm->FetchRandomBlock(table, finalfileoffset);
                block[0]->SetPin(true);
                for (vector<string > ::const_iterator it = vfuck->begin(); it < vfuck->end(); it++) {
                    string aaa = *it;
                    string finalattribute = cm->GiveAttribute(table, aaa);
                    //cout << finalattribute << endl;
                    int finaltype = cm->AttributeType(table,finalattribute);
                    int finalorder = cm->AttributeOrder(table, finalattribute);
                    string whatfuck = (*it);
                    //cout << cm->GiveIndex(tablename, finalattribute) << endl;
                    string finalindex = cm->GiveIndex(table, finalattribute);

                    if (finaltype== 0){
                        int finalvalue = block[0]->ReadBlock<int>(finalblockoffset + finalorder);
                        InfoBlock<int> * v;
                        v = new InfoBlock<int>(finalvalue, finalfileoffset, finalblockoffset);
                        vector<InfoBlock<int>*> vv;
                        vv.push_back(v);
//            cout << finalvalue << endl;
//            cout << finalfileoffset << endl;
//            cout << finalblockoffset << endl;
                        im->Delete<int>(0, finalindex, v->value, v->value, &vv);
                        delete v;
                    }
                    else if (finaltype == -1) {
                        float finalvalue = block[0]->ReadBlock<float>(finalblockoffset + finalorder);
                        InfoBlock<float> * v;
                        v = new InfoBlock<float>(finalvalue, finalfileoffset, finalblockoffset);
                        vector<InfoBlock<float>*> vv;
                        vv.push_back(v);
                        //cout << finalvalue << endl;
                        //cout << finalfileoffset << endl;
                        //cout << finalblockoffset << endl;
                        im->Delete<float>(0, finalindex, v->value, v->value, &vv);
                        delete v;
                    }
                    else {
                        string finalvalue = block[0]->ReadBlock(finalblockoffset + finalorder,finaltype);
                        InfoBlock<string> * v;
                        v = new InfoBlock<string>(finalvalue, finalfileoffset, finalblockoffset);
                        vector<InfoBlock<string>*> vv;
                        vv.push_back(v);
                        //cout << finalvalue << endl;
                        //cout << finalfileoffset << endl;
                        //cout << finalblockoffset << endl;
                        im->Delete<string>(0, finalindex, v->value, v->value, &vv);
                        delete v;
                    }
                }
                block[0]->WriteBlock<int>(blockoffset, oldefileoffset);
                block[0]->WriteBlock<int>(blockoffset + 5, oldblockoffset);
                block[0]->WriteBlock<char>(blockoffset + 4, '-');
                block[0]->SetPin(false);
                blockoffset += length;
            }
        }
    }
}

void  RecordManager::DropTable(const string &table) {
    bm->DeleteFile(table);
}

void RecordManager::CreateTable(const string &table, vector<Attribute*>*attribute) {
    Block* block= bm->NewFile(table);
    block->WriteBlock<char>(0, '-');
}

