cmake_minimum_required(VERSION 3.10)
project(MiniSQL_)

set(CMAKE_CXX_STANDARD 11)

include_directories(.)

add_executable(MiniSQL_
        API.cpp
        API.h
        BufferManager.cpp
        BufferManager.h
        BxTree.h
        BxTree.cpp
        CatalogManager.h
        IndexManager.h
        IndexManager.cpp
        Interpreter.cpp
        Interpreter.h
        main.cpp
        MyClass.h
        RecordManager.h
        CatalogManager.cpp
        RecordManager.cpp
        MyClass.cpp)