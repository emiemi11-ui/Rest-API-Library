#include "core/server.hpp"
#include <iostream>

Server::Server(int port, int num_workers)
    : port(port), num_workers(num_workers), master(nullptr) {
    // Creează MasterProcess
    master = new MasterProcess(port, num_workers);
}

Server::~Server() {
    stop();
    if (master) {
        delete master;
        master = nullptr;
    }
}

void Server::setRouter(const Router& r) {
    router_ = r;
    if (master) {
        master->setRouter(router_);
    }
}

void Server::start() {
    if (!master) {
        std::cerr << "[Server] MasterProcess not initialized!\n";
        return;
    }

    std::cout << "[Server] Starting with REAL multi-processing architecture\n";
    master->start();  // Master fork-uiește workers și rulează accept loop
}

void Server::stop() {
    if (master) {
        master->stop();
    }
}

void Server::request_shutdown() {
    if (master) {
        master->graceful_shutdown();
    }
}

void Server::set_shutdown_timeout(std::chrono::seconds timeout) {
    if (master) {
        master->set_shutdown_timeout(timeout);
    }
}
