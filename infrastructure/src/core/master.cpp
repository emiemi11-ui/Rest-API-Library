#include "core/master.hpp"
#include "core/workerprocess.hpp"
#include "core/worker.hpp"

#include <unistd.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <cstring>
#include <iostream>
#include <thread>

// Signal handler global pentru graceful shutdown
static volatile sig_atomic_t graceful_shutdown_requested = 0;

static void signal_handler(int signum) {
    if (signum == SIGTERM || signum == SIGINT) {
        graceful_shutdown_requested = 1;
    }
}

MasterProcess::MasterProcess(int port, int num_workers)
    : port_(port),
      num_workers_(num_workers),
      server_fd_(-1),
      epoll_fd_(-1),
      running_(false),
      shutdown_requested_(false),
      job_queue_(nullptr),
      worker_status_shm_(nullptr),
      global_stats_(nullptr) {

    if (num_workers_ > MAX_WORKERS) {
        num_workers_ = MAX_WORKERS;
    }
}

MasterProcess::~MasterProcess() {
    stop();
    cleanup();
}

void MasterProcess::setRouter(const Router& r) {
    router_ = r;
}

void MasterProcess::set_shutdown_timeout(std::chrono::seconds timeout) {
    shutdown_timeout_ = timeout;
}

void MasterProcess::setup_signals() {
    struct sigaction sa;
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGINT, &sa, NULL);

    // Ignore SIGPIPE (broken pipe când client se deconectează)
    signal(SIGPIPE, SIG_IGN);

    // Prevent zombie processes
    sa.sa_handler = SIG_IGN;
    sa.sa_flags = SA_NOCLDWAIT;  // Auto-reap children
    // Note: We'll use WNOHANG in waitpid instead for monitoring

    std::cout << "[Master] Signal handlers configured\n";
}

void MasterProcess::setup_epoll() {
    epoll_fd_ = epoll_create1(0);
    if (epoll_fd_ == -1) {
        perror("epoll_create1");
        throw std::runtime_error("Failed to create epoll instance");
    }

    struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLET;  // Edge-triggered mode
    ev.data.fd = server_fd_;

    if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, server_fd_, &ev) == -1) {
        perror("epoll_ctl");
        close(epoll_fd_);
        throw std::runtime_error("Failed to add server_fd to epoll");
    }

    std::cout << "[Master] epoll configured for non-blocking I/O\n";
}

void MasterProcess::start() {
    // 1. Creează socket TCP
    server_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd_ < 0) {
        perror("socket");
        return;
    }

    // Set socket options
    int opt = 1;
    if (setsockopt(server_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
    }

    // Set non-blocking mode pentru accept
    int flags = fcntl(server_fd_, F_GETFL, 0);
    fcntl(server_fd_, F_SETFL, flags | O_NONBLOCK);

    // 2. Bind & Listen
    struct sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port_);

    if (bind(server_fd_, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(server_fd_);
        return;
    }

    if (listen(server_fd_, 128) < 0) {
        perror("listen");
        close(server_fd_);
        return;
    }

    std::cout << "[Master] Socket listening on port " << port_ << "\n";

    // 3. Setup signal handlers
    setup_signals();

    // 4. Creează SharedQueue pentru job distribution
    try {
        job_queue_ = new SharedQueue<int>("/rest_api_jobs", 1024, true);
        std::cout << "[Master] SharedQueue created for IPC\n";
    } catch (const std::exception& e) {
        std::cerr << "[Master] Failed to create SharedQueue: " << e.what() << "\n";
        close(server_fd_);
        return;
    }

    // 5. Creează SharedMemory pentru worker statistics
    try {
        size_t stats_size = sizeof(GlobalStats);
        worker_status_shm_ = new SharedMemory("/rest_api_stats", stats_size, true);
        global_stats_ = reinterpret_cast<GlobalStats*>(worker_status_shm_->get_ptr());

        // Inițializare statistici
        global_stats_->total_requests = 0;
        global_stats_->total_errors = 0;
        global_stats_->active_connections = 0;

        for (int i = 0; i < MAX_WORKERS; i++) {
            global_stats_->workers[i].pid = 0;
            global_stats_->workers[i].requests_handled = 0;
            global_stats_->workers[i].requests_failed = 0;
            global_stats_->workers[i].status = 0;
            std::memset(global_stats_->workers[i].last_error, 0, 256);
        }

        std::cout << "[Master] SharedMemory created for statistics\n";
    } catch (const std::exception& e) {
        std::cerr << "[Master] Failed to create SharedMemory: " << e.what() << "\n";
        delete job_queue_;
        close(server_fd_);
        return;
    }

    // 6. Setup epoll
    setup_epoll();

    // 7. Fork workers
    create_workers();

    // 8. Start accept loop
    running_ = true;
    std::cout << "[Master] Starting on port " << port_
              << " with " << num_workers_ << " worker processes\n";
    std::cout << "[Master] All workers ready. Accepting connections...\n";

    accept_loop_epoll();
}

void MasterProcess::create_workers() {
    workers_.resize(num_workers_);

    for (int i = 0; i < num_workers_; i++) {
        std::cout << "[Master] Forking worker " << i << "...\n";

        pid_t pid = fork();

        if (pid < 0) {
            perror("fork");
            std::cerr << "[Master] Failed to fork worker " << i << "\n";
            continue;
        }

        if (pid == 0) {
            // ===== PROCES COPIL (WORKER) =====

            // Închide epoll și server socket în worker (nu le folosește)
            if (epoll_fd_ >= 0) close(epoll_fd_);
            if (server_fd_ >= 0) close(server_fd_);

            // Worker nu e creator de SharedQueue/SharedMemory, le deschide
            // (sunt deja create de Master)

            std::cout << "[Worker " << i << "] PID=" << getpid()
                      << " started (parent PID=" << getppid() << ")\n";

            // Creează WorkerProcess și rulează-l
            WorkerProcess worker(i, &router_, job_queue_, worker_status_shm_);

            // Update global stats cu PID worker
            global_stats_->workers[i].pid = getpid();
            global_stats_->workers[i].status = 1;  // idle

            worker.start();

            // Când worker.start() se termină, ieșim din proces copil
            std::cout << "[Worker " << i << "] PID=" << getpid() << " exiting\n";
            exit(0);
        } else {
            // ===== PROCES PĂRINTE (MASTER) =====
            workers_[i].pid = pid;
            workers_[i].status = 1;  // alive
            workers_[i].requests_handled = 0;
            workers_[i].last_health_check = std::chrono::steady_clock::now();

            global_stats_->workers[i].pid = pid;
            global_stats_->workers[i].status = 1;

            std::cout << "[Master] Worker " << i << " forked with PID=" << pid << "\n";
        }
    }

    std::cout << "[Master] All " << num_workers_ << " workers forked successfully\n";
}

void MasterProcess::accept_loop_epoll() {
    struct epoll_event events[MAX_EVENTS];

    while (running_ && !graceful_shutdown_requested) {
        // epoll_wait cu timeout 1s pentru a putea verifica signals
        int n = epoll_wait(epoll_fd_, events, MAX_EVENTS, 1000);

        if (n < 0) {
            if (errno == EINTR) {
                continue;  // Signal întrerupt
            }
            perror("epoll_wait");
            break;
        }

        // Process events
        for (int i = 0; i < n; i++) {
            if (events[i].data.fd == server_fd_) {
                // Acceptă toate conexiunile disponibile (edge-triggered)
                while (true) {
                    struct sockaddr_in client_addr;
                    socklen_t client_len = sizeof(client_addr);

                    int client_fd = accept(server_fd_,
                                          (struct sockaddr*)&client_addr,
                                          &client_len);

                    if (client_fd < 0) {
                        if (errno == EAGAIN || errno == EWOULDBLOCK) {
                            // Nu mai sunt conexiuni disponibile
                            break;
                        } else if (errno == EINTR) {
                            continue;
                        } else {
                            perror("accept");
                            break;
                        }
                    }

                    // Distribuie conexiunea către workers prin SharedQueue
                    distribute_connection(client_fd);
                }
            }
        }

        // Periodic: monitorizează workers
        static int monitor_counter = 0;
        if (++monitor_counter >= 10) {  // La fiecare ~10s
            monitor_workers();
            monitor_counter = 0;
        }
    }

    // Dacă am ieșit din loop, e shutdown
    if (graceful_shutdown_requested) {
        graceful_shutdown();
    }
}

void MasterProcess::distribute_connection(int client_fd) {
    try {
        // Pune file descriptor în SharedQueue pentru workers
        job_queue_->enqueue(client_fd);

        global_stats_->total_requests++;
        global_stats_->active_connections++;

        // std::cout << "[Master] Connection FD=" << client_fd << " enqueued\n";
    } catch (const std::exception& e) {
        std::cerr << "[Master] Failed to enqueue connection: " << e.what() << "\n";
        close(client_fd);  // Închide conexiunea dacă nu poate fi distribuită
        global_stats_->total_errors++;
    }
}

void MasterProcess::monitor_workers() {
    // std::cout << "\n=== [Master] Worker Health Check ===\n";

    for (int i = 0; i < num_workers_; i++) {
        if (workers_[i].status == 0) {
            continue;  // Worker mort
        }

        int status;
        pid_t result = waitpid(workers_[i].pid, &status, WNOHANG);

        if (result > 0) {
            // Worker a murit!
            if (WIFEXITED(status)) {
                std::cerr << "[Master] Worker " << i << " (PID " << workers_[i].pid
                         << ") exited with status " << WEXITSTATUS(status) << "\n";
            } else if (WIFSIGNALED(status)) {
                std::cerr << "[Master] Worker " << i << " (PID " << workers_[i].pid
                         << ") killed by signal " << WTERMSIG(status) << "\n";
            }

            handle_worker_death(workers_[i].pid, i);
        }
    }

    // Print statistici
    /*
    std::cout << "Total requests: " << global_stats_->total_requests << "\n";
    std::cout << "Active connections: " << global_stats_->active_connections << "\n";
    for (int i = 0; i < num_workers_; i++) {
        if (workers_[i].status == 1) {
            std::cout << "  Worker " << i << " (PID " << workers_[i].pid
                     << "): " << global_stats_->workers[i].requests_handled
                     << " requests handled\n";
        }
    }
    */
}

void MasterProcess::handle_worker_death(pid_t pid, int worker_index) {
    if (!running_ || shutdown_requested_) {
        return;  // Nu repornim workers în timpul shutdown
    }

    std::cout << "[Master] Restarting worker " << worker_index << "...\n";

    workers_[worker_index].status = 0;
    global_stats_->workers[worker_index].status = 0;

    // Fork un worker nou
    pid_t new_pid = fork();

    if (new_pid < 0) {
        perror("fork");
        std::cerr << "[Master] Failed to restart worker " << worker_index << "\n";
        return;
    }

    if (new_pid == 0) {
        // ===== PROCES COPIL (WORKER NOU) =====

        if (epoll_fd_ >= 0) close(epoll_fd_);
        if (server_fd_ >= 0) close(server_fd_);

        std::cout << "[Worker " << worker_index << "] PID=" << getpid()
                  << " restarted after crash\n";

        WorkerProcess worker(worker_index, &router_, job_queue_, worker_status_shm_);

        global_stats_->workers[worker_index].pid = getpid();
        global_stats_->workers[worker_index].status = 1;

        worker.start();
        exit(0);
    } else {
        // ===== MASTER =====
        workers_[worker_index].pid = new_pid;
        workers_[worker_index].status = 1;
        workers_[worker_index].requests_handled = 0;

        global_stats_->workers[worker_index].pid = new_pid;
        global_stats_->workers[worker_index].status = 1;

        std::cout << "[Master] Worker " << worker_index
                  << " restarted with new PID=" << new_pid << "\n";
    }
}

void MasterProcess::stop() {
    if (!running_) return;
    running_ = false;
    std::cout << "[Master] Stopping...\n";
}

void MasterProcess::graceful_shutdown() {
    if (shutdown_requested_.exchange(true)) {
        return;  // Already shutting down
    }

    std::cout << "\n[Master] Graceful shutdown initiated\n";

    // 1. Stop accepting new connections
    running_ = false;
    if (server_fd_ >= 0) {
        close(server_fd_);
        server_fd_ = -1;
    }
    if (epoll_fd_ >= 0) {
        close(epoll_fd_);
        epoll_fd_ = -1;
    }

    // 2. Trimite SIGTERM la toți workers
    std::cout << "[Master] Sending SIGTERM to workers...\n";
    for (int i = 0; i < num_workers_; i++) {
        if (workers_[i].status == 1) {
            std::cout << "[Master] Sending SIGTERM to worker " << i
                     << " (PID " << workers_[i].pid << ")\n";
            kill(workers_[i].pid, SIGTERM);
        }
    }

    // 3. Așteaptă workers să termine (cu timeout)
    auto start = std::chrono::steady_clock::now();
    int alive = 0;

    // Count alive workers
    for (int i = 0; i < num_workers_; i++) {
        if (workers_[i].status == 1) alive++;
    }

    std::cout << "[Master] Waiting for " << alive << " workers to terminate...\n";

    while (alive > 0) {
        for (int i = 0; i < num_workers_; i++) {
            if (workers_[i].status != 0) {
                int status;
                pid_t result = waitpid(workers_[i].pid, &status, WNOHANG);

                if (result > 0) {
                    workers_[i].status = 0;
                    alive--;
                    std::cout << "[Master] Worker " << i << " (PID " << workers_[i].pid
                             << ") terminated gracefully\n";
                }
            }
        }

        auto elapsed = std::chrono::steady_clock::now() - start;
        if (elapsed > shutdown_timeout_) {
            std::cout << "[Master] Shutdown timeout reached! Killing remaining workers...\n";
            for (int i = 0; i < num_workers_; i++) {
                if (workers_[i].status != 0) {
                    std::cout << "[Master] Sending SIGKILL to worker " << i
                             << " (PID " << workers_[i].pid << ")\n";
                    kill(workers_[i].pid, SIGKILL);
                    waitpid(workers_[i].pid, NULL, 0);  // Reap zombie
                    workers_[i].status = 0;
                }
            }
            break;
        }

        if (alive > 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }

    std::cout << "[Master] All workers terminated\n";

    // 4. Cleanup shared resources
    cleanup();

    std::cout << "[Master] Shutdown complete\n";
}

void MasterProcess::cleanup() {
    // Cleanup SharedQueue
    if (job_queue_) {
        delete job_queue_;
        job_queue_ = nullptr;
        std::cout << "[Master] SharedQueue cleanup complete\n";
    }

    // Cleanup SharedMemory
    if (worker_status_shm_) {
        delete worker_status_shm_;
        worker_status_shm_ = nullptr;
        std::cout << "[Master] SharedMemory cleanup complete\n";
    }

    // Unlink shared memory objects (master is creator)
    shm_unlink("/rest_api_jobs");
    shm_unlink("/rest_api_stats");
}
