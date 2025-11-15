#pragma once
#include <atomic>
#include <thread>
#include <vector>
#include <chrono>
#include <netinet/in.h>

#include "core/master.hpp"    // <-- SCHIMBAT: MasterProcess în loc de ThreadPool
#include "http/router.hpp"

class Server {
public:
    Server(int port, int num_workers);
    ~Server();

    // pornește / oprește serverul
    void start();
    void stop();

    // setează rutele (Router)
    void setRouter(const Router& r);

    // Graceful shutdown support
    void request_shutdown();
    void set_shutdown_timeout(std::chrono::seconds timeout);

private:
    int port;
    int num_workers;
    MasterProcess* master;   // <-- SCHIMBAT: MasterProcess în loc de ThreadPool

    Router router_;
};
