#pragma once
#include <string>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdexcept>
#include <iostream>

class SharedMemory {
private:
    std::string name;
    size_t size;
    int fd;
    void* ptr;
    bool is_creator;

public:
    SharedMemory(const std::string& name, size_t size, bool creator);
    ~SharedMemory();

    void* get_ptr() const { return ptr; }
    size_t get_size() const { return size; }
};
