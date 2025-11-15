#pragma once

#include <atomic>
#include <sys/types.h>
#include <csignal>

#include "core/threadpool.hpp"
#include "http/router.hpp"
#include "ipc/sharedqueue.hpp"
#include "ipc/sharedmemory.hpp"

// Forward declaration pentru GlobalStats
struct GlobalStats;

class WorkerProcess {
private:
    int worker_id_;
    pid_t pid_;
    ThreadPool thread_pool_;
    Router* router_;
    SharedQueue<int>* job_queue_;
    SharedMemory* worker_status_shm_;
    GlobalStats* global_stats_;

    std::atomic<bool> running_{false};

    void setup_signals();
    void work_loop();
    void process_request(int client_fd);

public:
    WorkerProcess(int id, Router* r, SharedQueue<int>* queue, SharedMemory* shm);
    ~WorkerProcess();

    void start();  // Rulează în proces copil (după fork)
    void stop();
};
