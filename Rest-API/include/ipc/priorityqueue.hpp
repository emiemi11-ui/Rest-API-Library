#pragma once
#include "ipc/sharedmemory.hpp"
#include "sync/semaphore.hpp"
#include <cstring>
#include <stdexcept>
#include <algorithm>
#include <cstdint>

// FLAGS pentru mesaje (conform cerință profesoară)
enum class MessageFlags : uint8_t {
    NORMAL = 0,      // Prioritate normală
    HIGH = 1,        // Prioritate înaltă
    URGENT = 2,      // Urgent (procesare imediată)
    LOW = 3          // Prioritate joasă
};

// Structura mesajului cu FLAG
template <typename T>
struct PriorityMessage {
    MessageFlags flag;           // FLAG-ul (cerință profesoară!)
    uint32_t sequence_number;    // Pentru FIFO în cadrul aceleiași priorități
    T data;                      // Payload-ul mesajului

    // Comparator pentru heap (max-heap pentru priorități)
    // Prioritate mai mare = valoare mai mică (URGENT=2 > HIGH=1 > NORMAL=0 > LOW=3)
    bool operator<(const PriorityMessage& other) const {
        if (flag != other.flag) {
            // URGENT (2) should come before HIGH (1), HIGH before NORMAL (0), etc.
            // We want higher priority values to be processed first
            if (flag == MessageFlags::URGENT) return false;
            if (other.flag == MessageFlags::URGENT) return true;
            if (flag == MessageFlags::HIGH && other.flag != MessageFlags::URGENT) return false;
            if (other.flag == MessageFlags::HIGH && flag != MessageFlags::URGENT) return true;
            if (flag == MessageFlags::NORMAL && other.flag == MessageFlags::LOW) return false;
            if (other.flag == MessageFlags::NORMAL && flag == MessageFlags::LOW) return true;
        }
        return sequence_number > other.sequence_number; // FIFO în cadrul priorității
    }
};

// Priority Queue folosind HEAP în Shared Memory
template <typename T>
class PriorityQueue {
private:
    struct QueueHeader {
        int size;
        int capacity;
        uint32_t next_sequence;  // Pentru FIFO în cadrul priorității
    };

    SharedMemory* shm;
    Semaphore* mutex;           // Protecție acces concurrent
    Semaphore* items_available; // Semnalizare elemente disponibile
    QueueHeader* header;
    PriorityMessage<T>* heap;   // Max-heap pentru priorități

    void heapify_up(int index);
    void heapify_down(int index);

public:
    PriorityQueue(const std::string& name, int capacity, bool creator);
    ~PriorityQueue();

    // ENQUEUE cu FLAG (cerință profesoară!)
    void enqueue(const T& data, MessageFlags flag);

    // DEQUEUE - returnează mesajul cu prioritatea cea mai mare
    PriorityMessage<T> dequeue();

    // Try dequeue (non-blocking)
    bool try_dequeue(PriorityMessage<T>& out);

    bool is_empty() const { return header->size == 0; }
    bool is_full() const { return header->size >= header->capacity; }
    int get_size() const { return header->size; }
};

// ===== IMPLEMENTARE =====

template <typename T>
PriorityQueue<T>::PriorityQueue(const std::string& name, int capacity, bool creator) {
    size_t size_needed = sizeof(QueueHeader) + capacity * sizeof(PriorityMessage<T>);

    shm = new SharedMemory(name, size_needed, creator);

    void* base_ptr = shm->get_ptr();
    header = reinterpret_cast<QueueHeader*>(base_ptr);
    heap = reinterpret_cast<PriorityMessage<T>*>((char*)base_ptr + sizeof(QueueHeader));

    if (creator) {
        header->size = 0;
        header->capacity = capacity;
        header->next_sequence = 0;
    }

    mutex = new Semaphore(name + "_mutex", 1);
    items_available = new Semaphore(name + "_items", 0);
}

template <typename T>
PriorityQueue<T>::~PriorityQueue() {
    delete items_available;
    delete mutex;
    delete shm;
}

template <typename T>
void PriorityQueue<T>::enqueue(const T& data, MessageFlags flag) {
    mutex->wait();

    if (header->size >= header->capacity) {
        mutex->post();
        throw std::runtime_error("PriorityQueue is full!");
    }

    // Creează mesaj cu FLAG
    PriorityMessage<T> msg;
    msg.flag = flag;
    msg.sequence_number = header->next_sequence++;
    msg.data = data;

    // Inserare în heap
    heap[header->size] = msg;
    heapify_up(header->size);
    header->size++;

    mutex->post();
    items_available->post(); // Semnalizează că avem elemente
}

template <typename T>
PriorityMessage<T> PriorityQueue<T>::dequeue() {
    items_available->wait(); // Așteaptă până avem elemente
    mutex->wait();

    if (header->size == 0) {
        mutex->post();
        throw std::runtime_error("PriorityQueue is empty!");
    }

    // Extrage root-ul (prioritatea cea mai mare)
    PriorityMessage<T> result = heap[0];

    // Mută ultimul element în root și reface heap-ul
    header->size--;
    heap[0] = heap[header->size];
    heapify_down(0);

    mutex->post();
    return result;
}

template <typename T>
bool PriorityQueue<T>::try_dequeue(PriorityMessage<T>& out) {
    if (!items_available->try_wait()) {
        return false; // Nu sunt elemente
    }

    mutex->wait();

    if (header->size == 0) {
        mutex->post();
        return false;
    }

    out = heap[0];
    header->size--;
    heap[0] = heap[header->size];
    heapify_down(0);

    mutex->post();
    return true;
}

template <typename T>
void PriorityQueue<T>::heapify_up(int index) {
    while (index > 0) {
        int parent = (index - 1) / 2;
        if (heap[parent] < heap[index]) {
            std::swap(heap[parent], heap[index]);
            index = parent;
        } else {
            break;
        }
    }
}

template <typename T>
void PriorityQueue<T>::heapify_down(int index) {
    while (true) {
        int largest = index;
        int left = 2 * index + 1;
        int right = 2 * index + 2;

        if (left < header->size && heap[largest] < heap[left]) {
            largest = left;
        }
        if (right < header->size && heap[largest] < heap[right]) {
            largest = right;
        }

        if (largest != index) {
            std::swap(heap[index], heap[largest]);
            index = largest;
        } else {
            break;
        }
    }
}
