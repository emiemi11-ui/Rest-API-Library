#pragma once
#include <semaphore.h>
#include <string>

class Semaphore {
private:
    sem_t* sem;
    std::string name;
    bool is_named;

public:
    Semaphore(const std::string& name, unsigned int value);
    
    explicit Semaphore(unsigned int value);

    ~Semaphore();

    void wait();
    void post();
    bool try_wait();
    int  get_value() const;
};
