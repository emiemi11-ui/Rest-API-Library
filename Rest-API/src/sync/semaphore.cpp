#include "sync/semaphore.hpp"
#include <fcntl.h>
#include <sys/stat.h>
#include <stdexcept>
#include <iostream>

Semaphore::Semaphore(const std::string& name, unsigned int value)
    : sem(nullptr), name(name), is_named(true)
{
    sem = sem_open(name.c_str(), O_CREAT, 0644, value);
    if (sem == SEM_FAILED)
        throw std::runtime_error("Failed to create named semaphore: " + name);
    std::cout << "[Semaphore] Created named: " << name << " (value=" << value << ")\n";
}

Semaphore::Semaphore(unsigned int value)
    : sem(new sem_t), is_named(false)
{
    if (sem_init(sem, 0, value) != 0) {
        delete sem;
        throw std::runtime_error("Failed to init unnamed semaphore");
    }
    std::cout << "[Semaphore] Created unnamed (value=" << value << ")\n";
}

Semaphore::~Semaphore() {
    if (is_named) {
        sem_close(sem);
        sem_unlink(name.c_str());
        std::cout << "[Semaphore] Destroyed named: " << name << "\n";
    } else {
        sem_destroy(sem);
        delete sem;
        std::cout << "[Semaphore] Destroyed unnamed\n";
    }
}

void Semaphore::wait() {
    if (sem_wait(sem) != 0)
        throw std::runtime_error("Semaphore wait failed");
}

void Semaphore::post() {
    if (sem_post(sem) != 0)
        throw std::runtime_error("Semaphore post failed");
}

bool Semaphore::try_wait() {
    return sem_trywait(sem) == 0;
}

int Semaphore::get_value() const {
    int value;
    sem_getvalue(sem, &value);
    return value;
}
