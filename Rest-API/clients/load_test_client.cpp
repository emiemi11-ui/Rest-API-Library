// Client pentru load testing (multe requests concurente)
#include <iostream>
#include <vector>
#include <thread>
#include <atomic>
#include <chrono>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

std::atomic<int> success_count{0};
std::atomic<int> failure_count{0};
std::atomic<long long> total_latency_ms{0};

void worker_thread(const std::string& server_ip, int server_port, int num_requests) {
    for (int i = 0; i < num_requests; i++) {
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0) {
            failure_count++;
            continue;
        }

        sockaddr_in server_addr{};
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(server_port);
        inet_pton(AF_INET, server_ip.c_str(), &server_addr.sin_addr);

        auto start = std::chrono::steady_clock::now();

        if (connect(sock, (sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
            failure_count++;
            close(sock);
            continue;
        }

        std::string request = "GET /health HTTP/1.1\r\nHost: localhost\r\n\r\n";
        send(sock, request.c_str(), request.size(), 0);

        char buffer[1024];
        recv(sock, buffer, sizeof(buffer), 0);

        auto end = std::chrono::steady_clock::now();
        auto latency = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

        total_latency_ms += latency;
        success_count++;

        close(sock);
    }
}

int main(int argc, char** argv) {
    std::string server_ip = "127.0.0.1";
    int server_port = 8080;
    int num_threads = 10;
    int requests_per_thread = 100;

    if (argc > 1) num_threads = std::atoi(argv[1]);
    if (argc > 2) requests_per_thread = std::atoi(argv[2]);

    std::cout << "╔════════════════════════════════════════╗\n";
    std::cout << "║     Load Test Client                  ║\n";
    std::cout << "╠════════════════════════════════════════╣\n";
    std::cout << "║  Threads: " << num_threads << std::string(28 - std::to_string(num_threads).length(), ' ') << "║\n";
    std::cout << "║  Requests/thread: " << requests_per_thread << std::string(19 - std::to_string(requests_per_thread).length(), ' ') << "║\n";
    std::cout << "║  Total requests: " << num_threads * requests_per_thread << std::string(20 - std::to_string(num_threads * requests_per_thread).length(), ' ') << "║\n";
    std::cout << "╚════════════════════════════════════════╝\n\n";

    std::cout << "[LoadTest] Starting load test...\n";

    auto start = std::chrono::steady_clock::now();

    std::vector<std::thread> threads;
    for (int i = 0; i < num_threads; i++) {
        threads.emplace_back(worker_thread, server_ip, server_port, requests_per_thread);
    }

    for (auto& t : threads) {
        t.join();
    }

    auto end = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    int total = success_count + failure_count;
    double avg_latency = success_count.load() > 0 ? total_latency_ms.load() / (double)success_count.load() : 0.0;
    double throughput = duration > 0 ? (success_count.load() * 1000.0) / duration : 0.0;

    std::cout << "\n╔════════════════════════════════════════╗\n";
    std::cout << "║     Load Test Results                 ║\n";
    std::cout << "╠════════════════════════════════════════╣\n";
    std::cout << "║  Total requests: " << total << std::string(20 - std::to_string(total).length(), ' ') << "║\n";
    std::cout << "║  Successful: " << success_count.load() << std::string(24 - std::to_string(success_count.load()).length(), ' ') << "║\n";
    std::cout << "║  Failed: " << failure_count.load() << std::string(28 - std::to_string(failure_count.load()).length(), ' ') << "║\n";
    std::cout << "║  Duration: " << duration << " ms" << std::string(24 - std::to_string(duration).length(), ' ') << "║\n";
    std::cout << "║  Avg latency: " << avg_latency << " ms" << std::string(21 - std::to_string((int)avg_latency).length(), ' ') << "║\n";
    std::cout << "║  Throughput: " << (int)throughput << " req/sec" << std::string(17 - std::to_string((int)throughput).length(), ' ') << "║\n";
    std::cout << "╚════════════════════════════════════════╝\n";

    return 0;
}
