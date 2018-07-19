//
// Created by SwJ on 2018/6/14.
//

#include "BufferManager.h"
#include <utility>
#include <iostream>
#include <fstream>
#include <exception>
#include <io.h>
#include <cstring>

using namespace std;

Block::Block(){
	block_offset = 0;
	pin = false;
	reference = false;
	dirty = false;
	address = new char[BLOCK_SIZE];
	for(int i = 0; i < BLOCK_SIZE; i++)
	    address[i] = '-';
	prev_block = next_block = nullptr;
	file_name = "";
}

Block::~Block(){
	delete address;
}

const std::string Block::ReadBlock(int offset, int length){
    std::string str;
    char *c1 = address + offset;

    if(offset + length > BLOCK_SIZE)
        throw Err("Reading out of range!");

    auto *c2 = new char[length+1];
    memcpy(c2,c1,(size_t)length);
    c2[length] = '\0';
    str = c2;
    delete c2;

    return str;
}

void Block::WriteBlock(int offset, char *t, int length){
    if(offset + length > BLOCK_SIZE)
        throw Err("Writing out of range!");
    dirty = true;
    memcpy(address + offset, t, (size_t)length);
}

//-BUFFER----------------------------------------------------------------------------

BufferManager::BufferManager(){
	for (auto &i : block_pool)
		i = new Block;
	buffer_position = 0;
}

BufferManager::~BufferManager(){
	for(auto &i : block_pool){
        Memory2DiskSingle(i);
	    delete i;
	}
}

/**
 * Clock:
 * returns a usable block in the pool according to LRU
 */

Block *BufferManager::Clock(){
	int i = 0;
	while(i++ < BUFFER_SIZE * 2 + 1){
		if(block_pool[buffer_position]->pin)
			buffer_position = (buffer_position + 1) % BUFFER_SIZE;
		else if(block_pool[buffer_position]->reference){
			block_pool[buffer_position]->reference = false;
			buffer_position = (buffer_position + 1) % BUFFER_SIZE;
		}
		else {
			Memory2DiskSingle(block_pool[buffer_position]);
			return block_pool[buffer_position];
		}
	}
	throw Err("All blocks are pinned!");
}

/**
 * InitBlock:
 * initialize attributes
 */

void BufferManager::InitBlock(Block *block, const string &filename, int offset){
	string fname = "files/" + filename + ".bin";
	block->file_name = filename;
	block->block_offset = offset;
	block->pin = false;
	block->reference = true;
	block->dirty = false;
	if(block->prev_block){
	    block->prev_block->next_block = nullptr;
	    block->prev_block = nullptr;
	}
    if(block->next_block){
        block->next_block->prev_block = nullptr;
        block->next_block = nullptr;
    }
    for(int i = 0; i < BLOCK_SIZE; i++)
        block->address[i] = '-';
}

/**
 * Disk2MemorySingle:
 * fetch data from specific offset of a file, write it in a block's data address.
 */

Block *BufferManager::Disk2MemorySingle(const string &filename, int offset){
	string fname = "files/" + filename + ".bin";
	fstream fp(fname, ios::in | ios:: out | ios::binary);
	if(!fp) // 文件不存在
        throw Err("No such a file!");

	fp.seekg(0,ios::end);
	if(fp.tellg()<offset * BLOCK_SIZE + 1) // 文件大小不够
	    return nullptr;
	else{
        Block *block = Clock();
        InitBlock(block, filename, offset);
        fp.seekg(offset * BLOCK_SIZE, ios::beg);
        fp.read(block->address, BLOCK_SIZE);
        fp.close();
        return block;
	}
}

/**
 * Memory2DiskSingle:
 * 1. If the block is undirty, ignore;
 * 2. If dirty, write back
 */

bool BufferManager::Memory2DiskSingle(Block *block){
	string fname = "files/" + block->file_name + ".bin";
	fstream fp(fname, ios::in | ios:: out | ios::binary); // 所有文件在建立（调用NewFile）时，便会在磁盘新建文件，下列throw应该永远不会发生
    if(!block->dirty) // the block is clean, no need to write back
        return true;
    else if(!fp)
        throw Err("wtf??");
	else{
		fp.seekp(block->block_offset * BLOCK_SIZE, ios::beg);
		fp.write(block->address, BLOCK_SIZE);
		fp.close();
		return true;
	}
}

/**
 * FetchBlockHead
 * fetch the header block of the named file
 */

Block* BufferManager::FetchBlockHead(string filename){
	for (auto &i : block_pool) // find in buffer
		if(i->file_name == filename && i->block_offset == 0){
			i->reference = true;
			return i;
		}
	return Disk2MemorySingle(filename, 0);
}

/**
 * FetchNextBlock:
 * fetch the next block
 */

Block* BufferManager::FetchNextBlock(Block *block){
	if(block->next_block != nullptr) // next ptr
		return block->next_block;

	else{
		for (auto &i : block_pool) { // find in buffer
			if (i->file_name == block->file_name
				&& i->block_offset == block->block_offset + 1) { // link next ptr
				block->next_block = i;
				i->prev_block = block;
				return i;
			}
		}

		block->next_block = Disk2MemorySingle(block->file_name, block->block_offset + 1);
        if(block->next_block != nullptr)
            block->next_block->prev_block = block;
		return block ->next_block;
	}
}

/**
 * FetchRandomBlock
 * fetch block randomly
 */

Block* BufferManager::FetchRandomBlock(const std::string &filename, int offset){
	for (auto &i : block_pool) // find in buffer
		if(i->file_name == filename && i->block_offset == offset){
			i->reference = true;
			return i;
		}
	return Disk2MemorySingle(filename, offset);
}

/**
 * AppendBlock
 * append a new block to the file
 */

Block* BufferManager::AppendBlock(Block *tail){
	if(FetchNextBlock(tail) != nullptr)
		throw Err("This block is not the end of the file!");
	else {
		Block *block = Clock();
		InitBlock(block, tail->file_name, tail->block_offset + 1);
		if(tail!=block){
			tail->next_block = block;
			block->prev_block = tail;
		}
		return block;
	}
}

/**
 * NewFile
 * creates a new filename, returning the header block
 */

Block* BufferManager::NewFile(const string &filename){
    string fname = "files/" + filename + ".bin";
    fstream fp(fname);
    if(fp){
        fp.close();
        throw Err("File already exits!");
    }
    else{
        fstream fp1(fname,ios::out);
        fp1.close();
        Block *block = Clock();
        InitBlock(block, filename, 0);
        return block;
    }
}

/**
 * DeleteFile:
 * delete whole file
 */

bool BufferManager::DeleteFile(const string &filename){
	auto *c = new char[260];
    string fname = "files/" + filename + ".bin";
    strcpy(c,fname.c_str());
	for( auto &i : block_pool)
		if(i->file_name == filename)
		    InitBlock(i,"",0);
	if(remove(c)) // 未删除返回EOF（-1）
	    throw Err("Unable to delete!");
}