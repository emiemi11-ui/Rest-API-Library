#include "core/workerprocess.hpp"
#include "core/worker.hpp"
#include "core/master.hpp"  // Pentru GlobalStats

#include <iostream>
#include <unistd.h>
#include <csignal>
#include <cstring>

// Signal handler global pentru workers
static volatile sig_atomic_t worker_shutdown_requested = 0;

static void worker_signal_handler(int signum) {
    if (signum == SIGTERM || signum == SIGINT) {
        worker_shutdown_requested = 1;
    }
}

WorkerProcess::WorkerProcess(int id, Router* r, SharedQueue<int>* queue, SharedMemory* shm)
    : worker_id_(id),
      pid_(getpid()),
      thread_pool_(),
      router_(r),
      job_queue_(queue),
      worker_status_shm_(shm),
      global_stats_(nullptr) {

    // Map shared memory pentru statistici
    if (worker_status_shm_) {
        global_stats_ = reinterpret_cast<GlobalStats*>(worker_status_shm_->get_ptr());
    }
}

WorkerProcess::~WorkerProcess() {
    stop();
}

void WorkerProcess::setup_signals() {
    struct sigaction sa;
    sa.sa_handler = worker_signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGINT, &sa, NULL);

    // Ignore SIGPIPE
    signal(SIGPIPE, SIG_IGN);

    // std::cout << "[Worker " << worker_id_ << "] Signal handlers configured\n";
}

void WorkerProcess::start() {
    running_ = true;

    // Setup signal handlers
    setup_signals();

    // Inițializează ThreadPool cu 8 threads per worker
    thread_pool_.init(8);

    std::cout << "[Worker " << worker_id_ << "] Started with ThreadPool (8 threads)\n";

    // Update status
    if (global_stats_) {
        global_stats_->workers[worker_id_].status = 1;  // idle
    }

    // Start work loop
    work_loop();

    // Cleanup când ieșim din loop
    thread_pool_.stop();

    if (global_stats_) {
        global_stats_->workers[worker_id_].status = 0;  // dead
    }

    std::cout << "[Worker " << worker_id_ << "] Stopped\n";
}

void WorkerProcess::work_loop() {
    while (running_ && !worker_shutdown_requested) {
        try {
            // DEQUEUE file descriptor din SharedQueue (IPC!)
            // Dacă coada e goală, asta va arunca excepție

            // Check dacă avem ceva în coadă
            if (job_queue_->is_empty()) {
                // Așteptăm puțin
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                continue;
            }

            int client_fd = job_queue_->dequeue();

            // std::cout << "[Worker " << worker_id_ << "] Got FD=" << client_fd << "\n";

            // Update status - busy
            if (global_stats_) {
                global_stats_->workers[worker_id_].status = 2;  // busy
            }

            // Procesare în ThreadPool
            thread_pool_.enqueue([this, client_fd]() {
                process_request(client_fd);
            });

            // Update status - idle
            if (global_stats_) {
                global_stats_->workers[worker_id_].status = 1;  // idle
                global_stats_->workers[worker_id_].requests_handled++;
            }

        } catch (const std::runtime_error& e) {
            // Coada goală sau altă eroare
            if (!running_ || worker_shutdown_requested) {
                break;  // Shutdown normal
            }

            // Altfel, așteptăm puțin și reîncercăm
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        } catch (const std::exception& e) {
            std::cerr << "[Worker " << worker_id_ << "] Error: " << e.what() << "\n";

            if (global_stats_) {
                global_stats_->workers[worker_id_].requests_failed++;
                strncpy(global_stats_->workers[worker_id_].last_error,
                       e.what(), 255);
            }
        }
    }

    std::cout << "[Worker " << worker_id_ << "] Shutdown signal received, exiting work loop\n";
}

void WorkerProcess::process_request(int client_fd) {
    try {
        // Folosește Worker::handle_client pentru procesarea efectivă
        Worker::handle_client(client_fd, router_);

        // Decrement active connections
        if (global_stats_) {
            global_stats_->active_connections--;
        }
    } catch (const std::exception& e) {
        std::cerr << "[Worker " << worker_id_ << "] Failed to process request: "
                 << e.what() << "\n";

        if (global_stats_) {
            global_stats_->workers[worker_id_].requests_failed++;
            global_stats_->total_errors++;
        }

        close(client_fd);
    }
}

void WorkerProcess::stop() {
    if (!running_) return;
    running_ = false;
}
