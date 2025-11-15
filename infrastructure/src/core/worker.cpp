#include "core/worker.hpp"
#include "http/request.hpp"
#include "http/response.hpp"
#include "http/router.hpp"
#include <unistd.h>
#include <sys/socket.h>
#include <vector>
#include <iostream>
#include <sstream>

namespace Worker {

// Helper simplu pentru parsarea primei linii din request
static HttpRequest parse_simple_request(const std::string& raw) {
    HttpRequest req;
    req.raw = raw;  // Salvează raw request complet

    // Găsește prima linie (până la \r\n)
    size_t end_of_first_line = raw.find("\r\n");
    if (end_of_first_line == std::string::npos) {
        return req;  // Request invalid
    }

    std::string first_line = raw.substr(0, end_of_first_line);

    // Parsează: "GET /path HTTP/1.1"
    std::istringstream iss(first_line);
    std::string method, target, version;
    if (!(iss >> method >> target >> version)) {
        return req;  // Parsare eșuată
    }

    req.method = method;
    req.target = target;

    // Extrage path (fără query string)
    size_t qpos = target.find('?');
    if (qpos == std::string::npos) {
        req.path = target;
    } else {
        req.path = target.substr(0, qpos);
    }

    // Extrage body-ul (după \r\n\r\n)
    size_t body_start = raw.find("\r\n\r\n");
    if (body_start != std::string::npos) {
        req.body = raw.substr(body_start + 4);
    }

    return req;
}

void initialize() {
    // Nu mai este nevoie - toate componentele sunt create în main.cpp
    // Această funcție este păstrată pentru compatibilitate
}

std::string read_request(int fd){
    std::vector<char> buf(8192);
    ssize_t n = ::recv(fd, buf.data(), buf.size()-1, 0);
    if (n <= 0) return std::string();
    buf[n]=0;
    return std::string(buf.data());
}

void send_response(int fd, const std::string& response){
    ::send(fd, response.data(), response.size(), 0);
}

void handle_client(int client_fd, Router* router){
    if (!router) {
        std::cerr << "[Worker] EROARE: Router este nullptr!\n";
        ::close(client_fd);
        return;
    }

    // Citește cererea
    std::string raw = read_request(client_fd);
    if (raw.empty()){
        ::close(client_fd);
        return;
    }

    std::cout << "\n[Worker] ========== CERERE NOUĂ ==========\n";

    // Parsează cererea
    HttpRequest req = parse_simple_request(raw);
    std::cout << "[Worker] " << req.method << " " << req.path << "\n";

    // Procesează prin router
    std::string response = router->handle(req);

    // Trimite răspunsul
    send_response(client_fd, response);

    std::cout << "[Worker] Răspuns trimis\n";
    std::cout << "[Worker] =====================================\n\n";

    // Închide conexiunea
    ::shutdown(client_fd, SHUT_RDWR);
    ::close(client_fd);
}
}
