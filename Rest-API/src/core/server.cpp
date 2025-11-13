
#include "core/server.hpp"
#include "core/worker.hpp"
#include <unistd.h>
#include <arpa/inet.h>
#include <cstring>
#include <iostream>

Server::Server(int port, int num_workers)
: port(port), num_workers(num_workers), running(false), server_fd(-1), pool() {}

Server::~Server(){ stop(); }

void Server::setRouter(const Router& r) {
    router_ = r;   // copie simplă; dacă vrei, poți face std::move când îl primești by value
}
void Server::start(){
    server_fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) { perror("socket"); return; }
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))<0){ perror("setsockopt"); }

    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);
    if (bind(server_fd, (sockaddr*)&addr, sizeof(addr)) < 0) { perror("bind"); ::close(server_fd); return; }
    if (listen(server_fd, 128) < 0) { perror("listen"); ::close(server_fd); return; }

    pool.init(num_workers);
    running = true;
    std::cout << "Server listening on port " << port << " with " << num_workers << " workers...\n";
    accept_loop();
}

void Server::stop(){
    if (!running) return;
    running = false;
    if (server_fd >= 0) { ::shutdown(server_fd, SHUT_RDWR); ::close(server_fd); server_fd = -1; }
    pool.stop();
    std::cout << "Server stopped.\n";
}

void Server::accept_loop(){
    while (running){
        sockaddr_in caddr{}; socklen_t clen=sizeof(caddr);
        int cfd = ::accept(server_fd, (sockaddr*)&caddr, &clen);
        if (cfd < 0){
            if (errno==EINTR) continue;
            if (!running) break;
            perror("accept");
            continue;
        }
        // Pasează router-ul la Worker pentru a procesa cererea
        pool.enqueue([cfd, this](){ Worker::handle_client(cfd, &router_); });
    }
}
