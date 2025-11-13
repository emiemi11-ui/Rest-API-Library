// Client simplu care trimite requests HTTP
#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <chrono>
#include <thread>

class SimpleClient {
    int sock_;
    std::string server_ip_;
    int server_port_;

public:
    SimpleClient(const std::string& ip, int port)
        : sock_(-1), server_ip_(ip), server_port_(port) {}

    ~SimpleClient() {
        if (sock_ >= 0) close(sock_);
    }

    bool connect_to_server() {
        sock_ = socket(AF_INET, SOCK_STREAM, 0);
        if (sock_ < 0) {
            std::cerr << "[Client] Failed to create socket\n";
            return false;
        }

        sockaddr_in server_addr{};
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(server_port_);
        inet_pton(AF_INET, server_ip_.c_str(), &server_addr.sin_addr);

        if (connect(sock_, (sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
            std::cerr << "[Client] Failed to connect to server\n";
            close(sock_);
            sock_ = -1;
            return false;
        }

        std::cout << "[Client] Connected to " << server_ip_ << ":" << server_port_ << "\n";
        return true;
    }

    std::string send_request(const std::string& method, const std::string& path, const std::string& body = "") {
        std::string request = method + " " + path + " HTTP/1.1\r\n";
        request += "Host: localhost\r\n";
        request += "Content-Length: " + std::to_string(body.size()) + "\r\n";
        request += "\r\n";
        request += body;

        // Trimite request
        ssize_t sent = send(sock_, request.c_str(), request.size(), 0);
        if (sent < 0) {
            std::cerr << "[Client] Failed to send request\n";
            return "";
        }

        // Primește response
        char buffer[4096];
        ssize_t received = recv(sock_, buffer, sizeof(buffer) - 1, 0);
        if (received < 0) {
            std::cerr << "[Client] Failed to receive response\n";
            return "";
        }

        buffer[received] = '\0';
        return std::string(buffer);
    }
};

int main(int argc, char** argv) {
    std::string server_ip = "127.0.0.1";
    int server_port = 8080;

    if (argc > 1) server_ip = argv[1];
    if (argc > 2) server_port = std::atoi(argv[2]);

    std::cout << "╔════════════════════════════════════════╗\n";
    std::cout << "║     Simple REST API Client            ║\n";
    std::cout << "╚════════════════════════════════════════╝\n\n";

    SimpleClient client(server_ip, server_port);

    if (!client.connect_to_server()) {
        return 1;
    }

    // Test 1: Health check
    std::cout << "\n[Test 1] Health check:\n";
    std::string response = client.send_request("GET", "/health");
    std::cout << response << "\n";

    std::this_thread::sleep_for(std::chrono::seconds(1));

    // Test 2: Get users
    std::cout << "\n[Test 2] Get users:\n";
    response = client.send_request("GET", "/api/users");
    std::cout << response << "\n";

    std::cout << "\n[Client] Tests completed!\n";
    return 0;
}
