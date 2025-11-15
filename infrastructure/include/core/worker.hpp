#pragma once
#include <string>

class Router;  // forward declaration

namespace Worker {
    void handle_client(int client_fd, Router* router);
    std::string read_request(int fd);
    void send_response(int fd, const std::string& response);
    void initialize();
}
