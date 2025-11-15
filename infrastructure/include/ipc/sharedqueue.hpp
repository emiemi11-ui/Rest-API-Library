#pragma once
#include "ipc/sharedmemory.hpp"
#include "sync/semaphore.hpp"
#include <cstring>
#include <stdexcept>
#include <iostream>

template <typename T>
class SharedQueue {
private:
    struct QueueHeader {
        int head;
        int tail;
        int size;
        int capacity;
    };

    SharedMemory* shm;
    Semaphore* sem;
    QueueHeader* header;
    T* elements;

public:
    SharedQueue(const std::string& name, int capacity, bool creator);
    ~SharedQueue();

    void enqueue(const T& element);
    T dequeue();

    bool is_empty() const { return header->size == 0; }
    bool is_full() const { return header->size >= header->capacity; }
};

// === Implementare SharedQueue ===

template <typename T>
SharedQueue<T>::SharedQueue(const std::string& name, int capacity, bool creator) {
    size_t size_needed = sizeof(QueueHeader) + capacity * sizeof(T);

    shm = new SharedMemory(name, size_needed, creator);

    void* base_ptr = shm->get_ptr();
    header = reinterpret_cast<QueueHeader*>(base_ptr);
    elements = reinterpret_cast<T*>((char*)base_ptr + sizeof(QueueHeader));

    if (creator) {
        header->head = 0;
        header->tail = 0;
        header->size = 0;
        header->capacity = capacity;
    }

    sem = new Semaphore(name + "_sem", 1);
    std::cout << "[SharedQueue] Creata cu capacitate " << capacity << "\n";
}

template <typename T>
SharedQueue<T>::~SharedQueue() {
    delete sem;
    delete shm;
}

template <typename T>
void SharedQueue<T>::enqueue(const T& element) {
    sem->wait();

    if (header->size >= header->capacity) {
        sem->post();
        throw std::runtime_error("Coada e plina!");
    }

    elements[header->tail] = element;
    header->tail = (header->tail + 1) % header->capacity;
    header->size++;

    sem->post();
}

template <typename T>
T SharedQueue<T>::dequeue() {
    sem->wait();

    if (header->size == 0) {
        sem->post();
        throw std::runtime_error("Coada e goala!");
    }

    T element = elements[header->head];
    header->head = (header->head + 1) % header->capacity;
    header->size--;

    sem->post();
    return element;
}

