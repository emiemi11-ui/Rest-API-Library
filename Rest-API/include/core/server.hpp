#pragma once
#include <atomic>
#include <thread>
#include <vector>
#include <chrono>
#include <netinet/in.h>

#include "core/threadpool.hpp"
#include "http/router.hpp"   // <-- ca să avem Router

class Server {
public:
    Server(int port, int num_workers);
    ~Server();

    // pornește / oprește serverul
    void start();
    void stop();

    // setează rutele (Router) — asta lipsea
    void setRouter(const Router& r);

    // Graceful shutdown support
    void request_shutdown();
    void set_shutdown_timeout(std::chrono::seconds timeout);

private:
    int port;
    int num_workers;
    std::atomic<bool> running;
    int server_fd;
    sockaddr_in addr{};
    ThreadPool pool;

    Router router_;          // <-- păstrăm routerul aici

    // Graceful shutdown members
    std::atomic<bool> shutdown_requested_{false};
    std::atomic<int> active_connections_{0};
    std::chrono::seconds shutdown_timeout_{30};

    void accept_loop();
    void wait_for_connections_to_close();
};
