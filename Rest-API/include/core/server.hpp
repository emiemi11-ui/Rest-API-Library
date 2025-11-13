#pragma once
#include <atomic>
#include <thread>
#include <vector>
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

private:
    int port;
    int num_workers;
    std::atomic<bool> running;
    int server_fd;
    sockaddr_in addr{};
    ThreadPool pool;

    Router router_;          // <-- păstrăm routerul aici

    void accept_loop();
};
