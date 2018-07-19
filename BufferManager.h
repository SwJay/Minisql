//
// Created by SwJ on 2018/6/14.
//

#ifndef BUFFERMANAGER_BUFFERMANAGER_H
#define BUFFERMANAGER_BUFFERMANAGER_H

#include  <string>
#include <iostream>
#include <cstring>

#define BUFFER_SIZE 2000
#define BLOCK_SIZE 4096

class Err : public std::exception{
    public:
    std::string error_str;
    explicit Err(const std::string &str):error_str(str){
        std::cout << str << std::endl;
    }
};

class Block{
private:
    int block_offset;
    bool pin;
    bool reference;
    bool dirty;
    char *address;
    Block *prev_block;
    Block *next_block;
    std::string file_name;
public:
    friend class BufferManager;
    Block();
    ~Block();
    template <class T>
        const T& ReadBlock(int offset);
    const std::string ReadBlock(int offset, int length);
    template <class T>
        void WriteBlock(int offset, T t);
    void WriteBlock(int offset, char *t, int length);
    bool SetPin(bool t){pin = t;} // set pin value
};

class BufferManager{
private:
    Block *block_pool[BUFFER_SIZE];
    int buffer_position;
    Block *Clock(); // error when all pinned
    void InitBlock(Block *block, const std::string &filename, int offset);
    Block *Disk2MemorySingle(const std::string &filename, int offset); // error when no corresponding file exits
    bool Memory2DiskSingle(Block *block);
public:
    BufferManager();
    ~BufferManager();
    Block* FetchBlockHead(std::string filename); // fetch the header block of the named file
    Block* FetchNextBlock(Block *block); //fetch the next block
    Block* FetchRandomBlock(const std::string &filename, int offset); //fetch block randomly
    Block* AppendBlock(Block *tail); // append a new block to this tail
    Block* NewFile(const std::string &filename); // creates a new filename, returning the header block
    bool DeleteFile(const std::string &filename); // delete whole file
};

template <class T>
const T& Block::ReadBlock(int offset){
    if(offset + sizeof(T) > BLOCK_SIZE)
        throw Err("Reading out of range!");
    char *c = address + offset;
    return *(T *)c;
}

template <class T>
void Block::WriteBlock(int offset, T t){
    if(offset + sizeof(T) >= BLOCK_SIZE)
        throw Err("Writing out of range!");
    dirty = true;
    memcpy(address + offset, &t, sizeof(T));
}

#endif //BUFFERMANAGER_BUFFERMANAGER_H