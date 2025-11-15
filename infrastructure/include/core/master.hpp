#pragma once

#include <vector>
#include <atomic>
#include <chrono>
#include <sys/epoll.h>
#include <sys/types.h>
#include <csignal>

#include "ipc/sharedqueue.hpp"
#include "ipc/sharedmemory.hpp"
#include "http/router.hpp"

#define MAX_EVENTS 64
#define MAX_WORKERS 32

// Structură pentru statistici workers în shared memory
struct WorkerStats {
    pid_t pid;
    std::atomic<uint64_t> requests_handled;
    std::atomic<uint64_t> requests_failed;
    std::atomic<int> status;  // 0=dead, 1=idle, 2=busy
    char last_error[256];
};

// Structură pentru statistici globale în shared memory
struct GlobalStats {
    std::atomic<uint64_t> total_requests;
    std::atomic<uint64_t> total_errors;
    std::atomic<int> active_connections;
    WorkerStats workers[MAX_WORKERS];
};

// Informații despre fiecare worker process
struct WorkerInfo {
    pid_t pid;
    int status;  // 0=dead, 1=alive, 2=busy
    uint64_t requests_handled;
    std::chrono::steady_clock::time_point last_health_check;
};

class MasterProcess {
private:
    int port_;
    int num_workers_;
    int server_fd_;
    int epoll_fd_;

    std::atomic<bool> running_;
    std::atomic<bool> shutdown_requested_;

    std::vector<WorkerInfo> workers_;
    SharedQueue<int>* job_queue_;           // Coadă de file descriptors
    SharedMemory* worker_status_shm_;       // Status workers în shared memory
    GlobalStats* global_stats_;

    Router router_;

    // Timeout pentru graceful shutdown
    std::chrono::seconds shutdown_timeout_{30};

    // Metode private
    void create_workers();
    void setup_signals();
    void setup_epoll();
    void accept_loop_epoll();
    void distribute_connection(int client_fd);
    void monitor_workers();
    void handle_worker_death(pid_t pid, int worker_index);
    void cleanup();

public:
    MasterProcess(int port, int num_workers);
    ~MasterProcess();

    void start();
    void stop();
    void graceful_shutdown();

    void setRouter(const Router& r);
    void set_shutdown_timeout(std::chrono::seconds timeout);
};
