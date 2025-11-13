#include "ipc/sharedmemory.hpp"

SharedMemory::SharedMemory(const std::string& name, size_t size, bool creator)
    : name(name), size(size), fd(-1), ptr(nullptr), is_creator(creator)
{
    // PASUL 1: Creare sau deschidere zonă
    if (creator) {
        fd = shm_open(name.c_str(), O_CREAT | O_RDWR, 0666);
    } else {
        fd = shm_open(name.c_str(), O_RDWR, 0666);
    }

    if (fd == -1)
        throw std::runtime_error("Nu pot deschide shared memory: " + name);

    // PASUL 2: Setare dimensiune doar pentru creator
    if (creator) {
        if (ftruncate(fd, size) == -1)
            throw std::runtime_error("Nu pot seta dimensiunea shared memory: " + name);
    }

    // PASUL 3: Mapare în memorie
    ptr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (ptr == MAP_FAILED) {
        close(fd);
        if (creator) shm_unlink(name.c_str());
        throw std::runtime_error("Nu pot mapa shared memory: " + name);
    }

    std::cout << "[SharedMemory] " << (creator ? "Creata" : "Deschisa")
              << ": " << name << ", " << size << " bytes\n";
}

SharedMemory::~SharedMemory() {
    if (ptr && ptr != MAP_FAILED)
        munmap(ptr, size);

    if (fd != -1)
        close(fd);

    if (is_creator) {
        shm_unlink(name.c_str());
        std::cout << "[SharedMemory] Stearsa: " << name << "\n";
    }
}
