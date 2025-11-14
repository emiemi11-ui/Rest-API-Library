/**
 * Production-Ready Load Test Client
 * Tests REST API servers on local or remote networks with realistic scenarios
 * Provides comprehensive metrics, real-time progress, and detailed reporting
 */

#include <iostream>
#include <vector>
#include <thread>
#include <atomic>
#include <chrono>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <algorithm>
#include <random>
#include <mutex>
#include <map>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <cmath>
#include <climits>

// ANSI color codes
#define COLOR_RESET   "\033[0m"
#define COLOR_RED     "\033[31m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_YELLOW  "\033[33m"
#define COLOR_BLUE    "\033[34m"
#define COLOR_MAGENTA "\033[35m"
#define COLOR_CYAN    "\033[36m"
#define COLOR_BOLD    "\033[1m"

// Configuration structure
struct LoadTestConfig {
    std::string server_ip = "127.0.0.1";
    int server_port = 8080;
    int num_threads = 10;
    int total_requests = 1000;
    int duration_seconds = 0;
    int think_time_ms = 100;
    bool save_csv = false;
    std::string csv_filename = "load_test_results.csv";
};

// Metrics structure
struct RequestMetrics {
    int status_code = 0;
    long long latency_ms = 0;
    std::string endpoint;
    bool success = false;
    std::string error_message;
};

// Global metrics
std::atomic<int> total_requests_sent{0};
std::atomic<int> total_requests_completed{0};
std::atomic<bool> test_running{true};
std::mutex metrics_mutex;
std::vector<RequestMetrics> all_metrics;
std::map<int, int> status_code_distribution;
std::map<std::string, int> error_distribution;

// Test scenarios (e-commerce workflow)
std::vector<std::string> test_scenarios = {
    "GET /api/products HTTP/1.1\r\nHost: {host}\r\n\r\n",
    "GET /api/products/search?q=laptop HTTP/1.1\r\nHost: {host}\r\n\r\n",
    "GET /api/products/1 HTTP/1.1\r\nHost: {host}\r\n\r\n",
    "GET /api/products/2 HTTP/1.1\r\nHost: {host}\r\n\r\n",
    "GET /api/products/5 HTTP/1.1\r\nHost: {host}\r\n\r\n",
    "POST /api/orders HTTP/1.1\r\nHost: {host}\r\nContent-Type: application/json\r\nContent-Length: 67\r\n\r\n{\"product_id\":1,\"quantity\":2,\"customer\":\"test@example.com\"}",
    "GET /api/orders HTTP/1.1\r\nHost: {host}\r\n\r\n",
    "GET /health HTTP/1.1\r\nHost: {host}\r\n\r\n"
};

// Helper function to replace placeholder
std::string replace_host(const std::string& request, const std::string& host) {
    std::string result = request;
    size_t pos = result.find("{host}");
    while (pos != std::string::npos) {
        result.replace(pos, 6, host);
        pos = result.find("{host}", pos + host.length());
    }
    return result;
}

// Parse HTTP status code
int parse_status_code(const std::string& response) {
    if (response.size() < 12) return 0;
    size_t pos = response.find("HTTP/1.1 ");
    if (pos == std::string::npos) return 0;
    pos += 9;
    if (pos + 3 > response.size()) return 0;
    try {
        return std::stoi(response.substr(pos, 3));
    } catch (...) {
        return 0;
    }
}

// Extract endpoint from request
std::string extract_endpoint(const std::string& request) {
    size_t start = request.find(" ") + 1;
    size_t end = request.find(" ", start);
    if (start == std::string::npos || end == std::string::npos) return "unknown";
    return request.substr(start, end - start);
}

// Send HTTP request and measure response
RequestMetrics send_request(const std::string& server_ip, int server_port,
                            const std::string& request_template, const std::string& host) {
    RequestMetrics metrics;
    std::string request = replace_host(request_template, host);
    metrics.endpoint = extract_endpoint(request);

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        metrics.error_message = "Socket creation failed";
        return metrics;
    }

    // Set timeout
    struct timeval tv;
    tv.tv_sec = 5;
    tv.tv_usec = 0;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
    setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (const char*)&tv, sizeof tv);

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);

    if (inet_pton(AF_INET, server_ip.c_str(), &server_addr.sin_addr) <= 0) {
        metrics.error_message = "Invalid address";
        close(sock);
        return metrics;
    }

    auto start = std::chrono::steady_clock::now();

    if (connect(sock, (sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        metrics.error_message = "Connection failed";
        close(sock);
        auto end = std::chrono::steady_clock::now();
        metrics.latency_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        return metrics;
    }

    if (send(sock, request.c_str(), request.size(), 0) < 0) {
        metrics.error_message = "Send failed";
        close(sock);
        auto end = std::chrono::steady_clock::now();
        metrics.latency_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        return metrics;
    }

    char buffer[4096];
    ssize_t bytes_received = recv(sock, buffer, sizeof(buffer) - 1, 0);

    auto end = std::chrono::steady_clock::now();
    metrics.latency_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    if (bytes_received < 0) {
        metrics.error_message = "Receive timeout";
        close(sock);
        return metrics;
    }

    buffer[bytes_received] = '\0';
    std::string response(buffer);

    metrics.status_code = parse_status_code(response);
    metrics.success = (metrics.status_code >= 200 && metrics.status_code < 300);

    close(sock);
    return metrics;
}

// Worker thread function (request-based)
void worker_thread_requests(const LoadTestConfig& config, int requests_per_thread) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> scenario_dist(0, test_scenarios.size() - 1);
    std::uniform_int_distribution<> think_time_dist(config.think_time_ms / 2, config.think_time_ms * 2);

    std::string host = config.server_ip + ":" + std::to_string(config.server_port);

    for (int i = 0; i < requests_per_thread && test_running.load(); i++) {
        std::string request = test_scenarios[scenario_dist(gen)];

        RequestMetrics metrics = send_request(config.server_ip, config.server_port, request, host);

        {
            std::lock_guard<std::mutex> lock(metrics_mutex);
            all_metrics.push_back(metrics);
            status_code_distribution[metrics.status_code]++;
            if (!metrics.error_message.empty()) {
                error_distribution[metrics.error_message]++;
            }
        }

        total_requests_completed++;

        // Think time between requests
        if (i < requests_per_thread - 1) {
            std::this_thread::sleep_for(std::chrono::milliseconds(think_time_dist(gen)));
        }
    }
}

// Worker thread function (duration-based)
void worker_thread_duration(const LoadTestConfig& config,
                           std::chrono::steady_clock::time_point end_time) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> scenario_dist(0, test_scenarios.size() - 1);
    std::uniform_int_distribution<> think_time_dist(config.think_time_ms / 2, config.think_time_ms * 2);

    std::string host = config.server_ip + ":" + std::to_string(config.server_port);

    while (std::chrono::steady_clock::now() < end_time && test_running.load()) {
        std::string request = test_scenarios[scenario_dist(gen)];

        RequestMetrics metrics = send_request(config.server_ip, config.server_port, request, host);

        {
            std::lock_guard<std::mutex> lock(metrics_mutex);
            all_metrics.push_back(metrics);
            status_code_distribution[metrics.status_code]++;
            if (!metrics.error_message.empty()) {
                error_distribution[metrics.error_message]++;
            }
        }

        total_requests_completed++;

        // Think time between requests
        std::this_thread::sleep_for(std::chrono::milliseconds(think_time_dist(gen)));
    }
}

// Progress bar thread
void progress_thread(const LoadTestConfig& config, int total_target) {
    while (test_running.load()) {
        int completed = total_requests_completed.load();
        float progress = total_target > 0 ? (float)completed / total_target : 0.0f;
        int bar_width = 50;
        int filled = (int)(progress * bar_width);

        std::cout << "\r" << COLOR_CYAN << "[";
        for (int i = 0; i < bar_width; i++) {
            if (i < filled) std::cout << "=";
            else if (i == filled) std::cout << ">";
            else std::cout << " ";
        }
        std::cout << "] " << std::fixed << std::setprecision(1)
                  << (progress * 100.0f) << "% ("
                  << completed << "/" << total_target << ")"
                  << COLOR_RESET << std::flush;

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    std::cout << std::endl;
}

// Calculate percentile
double calculate_percentile(std::vector<long long>& latencies, double percentile) {
    if (latencies.empty()) return 0.0;
    std::sort(latencies.begin(), latencies.end());
    size_t index = (size_t)(percentile * latencies.size());
    if (index >= latencies.size()) index = latencies.size() - 1;
    return latencies[index];
}

// Print results
void print_results(const LoadTestConfig& config, long long duration_ms) {
    std::vector<long long> latencies;
    int success_count = 0;
    int failure_count = 0;
    long long total_latency = 0;
    long long min_latency = LLONG_MAX;
    long long max_latency = 0;

    for (const auto& m : all_metrics) {
        latencies.push_back(m.latency_ms);
        total_latency += m.latency_ms;

        if (m.latency_ms < min_latency) min_latency = m.latency_ms;
        if (m.latency_ms > max_latency) max_latency = m.latency_ms;

        if (m.success) success_count++;
        else failure_count++;
    }

    int total = all_metrics.size();
    double avg_latency = total > 0 ? total_latency / (double)total : 0.0;
    double throughput = duration_ms > 0 ? (total * 1000.0) / duration_ms : 0.0;
    double success_rate = total > 0 ? (success_count * 100.0) / total : 0.0;

    double p50 = calculate_percentile(latencies, 0.50);
    double p95 = calculate_percentile(latencies, 0.95);
    double p99 = calculate_percentile(latencies, 0.99);

    std::cout << "\n" << COLOR_BOLD << COLOR_GREEN;
    std::cout << "╔══════════════════════════════════════════════════════════════╗\n";
    std::cout << "║              LOAD TEST RESULTS - SUMMARY                     ║\n";
    std::cout << "╠══════════════════════════════════════════════════════════════╣" << COLOR_RESET << "\n";

    std::cout << COLOR_CYAN << "  Test Configuration:" << COLOR_RESET << "\n";
    std::cout << "    Server:          " << config.server_ip << ":" << config.server_port << "\n";
    std::cout << "    Concurrency:     " << config.num_threads << " threads\n";
    std::cout << "    Think Time:      " << config.think_time_ms << " ms\n\n";

    std::cout << COLOR_CYAN << "  Request Statistics:" << COLOR_RESET << "\n";
    std::cout << "    Total Requests:  " << total << "\n";
    std::cout << "    " << COLOR_GREEN << "Successful:      " << success_count
              << " (" << std::fixed << std::setprecision(2) << success_rate << "%)" << COLOR_RESET << "\n";
    std::cout << "    " << COLOR_RED << "Failed:          " << failure_count
              << " (" << std::fixed << std::setprecision(2) << (100.0 - success_rate) << "%)" << COLOR_RESET << "\n";
    std::cout << "    Duration:        " << duration_ms / 1000.0 << " seconds\n";
    std::cout << "    " << COLOR_YELLOW << "Throughput:      " << std::fixed << std::setprecision(2)
              << throughput << " req/sec" << COLOR_RESET << "\n\n";

    std::cout << COLOR_CYAN << "  Response Time (ms):" << COLOR_RESET << "\n";
    std::cout << "    Average:         " << std::fixed << std::setprecision(2) << avg_latency << " ms\n";
    std::cout << "    Min:             " << (min_latency == LLONG_MAX ? 0 : min_latency) << " ms\n";
    std::cout << "    Max:             " << max_latency << " ms\n";
    std::cout << "    " << COLOR_MAGENTA << "p50 (median):    " << p50 << " ms" << COLOR_RESET << "\n";
    std::cout << "    " << COLOR_MAGENTA << "p95:             " << p95 << " ms" << COLOR_RESET << "\n";
    std::cout << "    " << COLOR_MAGENTA << "p99:             " << p99 << " ms" << COLOR_RESET << "\n\n";

    std::cout << COLOR_CYAN << "  Status Code Distribution:" << COLOR_RESET << "\n";
    for (const auto& [code, count] : status_code_distribution) {
        std::string color = COLOR_RESET;
        if (code >= 200 && code < 300) color = COLOR_GREEN;
        else if (code >= 400 && code < 500) color = COLOR_YELLOW;
        else if (code >= 500) color = COLOR_RED;

        std::cout << "    " << color << (code == 0 ? "Network Error" : std::to_string(code))
                  << ": " << std::setw(6) << count
                  << " (" << std::fixed << std::setprecision(1)
                  << (count * 100.0 / total) << "%)" << COLOR_RESET << "\n";
    }

    if (!error_distribution.empty()) {
        std::cout << "\n" << COLOR_RED << "  Error Breakdown:" << COLOR_RESET << "\n";
        for (const auto& [error, count] : error_distribution) {
            std::cout << "    " << error << ": " << count << "\n";
        }
    }

    std::cout << COLOR_BOLD << COLOR_GREEN;
    std::cout << "╚══════════════════════════════════════════════════════════════╝" << COLOR_RESET << "\n";
}

// Save results to CSV
void save_to_csv(const LoadTestConfig& config, const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << COLOR_RED << "Failed to open CSV file: " << filename << COLOR_RESET << "\n";
        return;
    }

    file << "Endpoint,Status Code,Latency (ms),Success,Error\n";
    for (const auto& m : all_metrics) {
        file << m.endpoint << ","
             << m.status_code << ","
             << m.latency_ms << ","
             << (m.success ? "true" : "false") << ","
             << m.error_message << "\n";
    }

    file.close();
    std::cout << COLOR_GREEN << "\n  Results saved to: " << filename << COLOR_RESET << "\n";
}

// Print usage
void print_usage(const char* program_name) {
    std::cout << COLOR_BOLD << "Production-Ready Load Test Client\n" << COLOR_RESET;
    std::cout << "Usage:\n";
    std::cout << "  " << program_name << " [OPTIONS]\n\n";
    std::cout << "Options:\n";
    std::cout << "  --server <ip>        Server IP address (default: 127.0.0.1)\n";
    std::cout << "  --port <port>        Server port (default: 8080)\n";
    std::cout << "  --threads <n>        Number of concurrent threads (default: 10)\n";
    std::cout << "  --concurrency <n>    Alias for --threads\n";
    std::cout << "  --requests <n>       Total number of requests (conflicts with --duration)\n";
    std::cout << "  --duration <sec>     Test duration in seconds (conflicts with --requests)\n";
    std::cout << "  --think-time <ms>    Think time between requests in ms (default: 100)\n";
    std::cout << "  --csv <filename>     Save results to CSV file\n";
    std::cout << "  --help               Show this help message\n\n";
    std::cout << "Examples:\n";
    std::cout << "  " << program_name << " --server 192.168.1.100 --port 8080 --threads 10 --duration 60\n";
    std::cout << "  " << program_name << " --server 10.0.0.5 --port 8080 --requests 10000 --concurrency 50\n";
    std::cout << "  " << program_name << " --server localhost --port 8080 --threads 20 --duration 120 --csv results.csv\n";
}

// Parse command line arguments
bool parse_arguments(int argc, char** argv, LoadTestConfig& config) {
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];

        if (arg == "--help" || arg == "-h") {
            print_usage(argv[0]);
            return false;
        }
        else if (arg == "--server" && i + 1 < argc) {
            config.server_ip = argv[++i];
        }
        else if (arg == "--port" && i + 1 < argc) {
            config.server_port = std::atoi(argv[++i]);
        }
        else if ((arg == "--threads" || arg == "--concurrency") && i + 1 < argc) {
            config.num_threads = std::atoi(argv[++i]);
        }
        else if (arg == "--requests" && i + 1 < argc) {
            config.total_requests = std::atoi(argv[++i]);
            config.duration_seconds = 0;
        }
        else if (arg == "--duration" && i + 1 < argc) {
            config.duration_seconds = std::atoi(argv[++i]);
            config.total_requests = 0;
        }
        else if (arg == "--think-time" && i + 1 < argc) {
            config.think_time_ms = std::atoi(argv[++i]);
        }
        else if (arg == "--csv" && i + 1 < argc) {
            config.save_csv = true;
            config.csv_filename = argv[++i];
        }
        else {
            std::cerr << COLOR_RED << "Unknown argument: " << arg << COLOR_RESET << "\n";
            print_usage(argv[0]);
            return false;
        }
    }

    // Validation
    if (config.duration_seconds == 0 && config.total_requests == 0) {
        config.total_requests = 1000; // Default
    }

    if (config.num_threads <= 0 || config.num_threads > 1000) {
        std::cerr << COLOR_RED << "Invalid number of threads (1-1000)" << COLOR_RESET << "\n";
        return false;
    }

    return true;
}

int main(int argc, char** argv) {
    LoadTestConfig config;

    if (!parse_arguments(argc, argv, config)) {
        return 1;
    }

    // Print banner
    std::cout << COLOR_BOLD << COLOR_BLUE;
    std::cout << "\n╔══════════════════════════════════════════════════════════════╗\n";
    std::cout << "║          PRODUCTION LOAD TEST CLIENT v2.0                    ║\n";
    std::cout << "╠══════════════════════════════════════════════════════════════╣" << COLOR_RESET << "\n";
    std::cout << "  Target:          " << config.server_ip << ":" << config.server_port << "\n";
    std::cout << "  Concurrency:     " << config.num_threads << " threads\n";

    if (config.duration_seconds > 0) {
        std::cout << "  Duration:        " << config.duration_seconds << " seconds\n";
    } else {
        std::cout << "  Total Requests:  " << config.total_requests << "\n";
        std::cout << "  Requests/Thread: " << (config.total_requests / config.num_threads) << "\n";
    }

    std::cout << "  Think Time:      " << config.think_time_ms << " ms\n";
    std::cout << COLOR_BOLD << COLOR_BLUE;
    std::cout << "╚══════════════════════════════════════════════════════════════╝" << COLOR_RESET << "\n\n";

    std::cout << COLOR_YELLOW << "Starting load test..." << COLOR_RESET << "\n\n";

    auto start_time = std::chrono::steady_clock::now();
    std::vector<std::thread> threads;

    // Start worker threads
    if (config.duration_seconds > 0) {
        // Duration-based test
        auto end_time = start_time + std::chrono::seconds(config.duration_seconds);

        // Start progress thread (estimate)
        std::thread prog_thread([&]() {
            auto start = std::chrono::steady_clock::now();
            while (test_running.load()) {
                auto now = std::chrono::steady_clock::now();
                auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - start).count();
                float progress = (float)elapsed / config.duration_seconds;
                int bar_width = 50;
                int filled = (int)(progress * bar_width);

                std::cout << "\r" << COLOR_CYAN << "[";
                for (int i = 0; i < bar_width; i++) {
                    if (i < filled) std::cout << "=";
                    else if (i == filled) std::cout << ">";
                    else std::cout << " ";
                }
                std::cout << "] " << std::fixed << std::setprecision(1)
                          << (progress * 100.0f) << "% ("
                          << elapsed << "/" << config.duration_seconds << "s) | Requests: "
                          << total_requests_completed.load()
                          << COLOR_RESET << std::flush;

                std::this_thread::sleep_for(std::chrono::milliseconds(200));
            }
            std::cout << std::endl;
        });

        for (int i = 0; i < config.num_threads; i++) {
            threads.emplace_back(worker_thread_duration, std::ref(config), end_time);
        }

        for (auto& t : threads) {
            t.join();
        }

        test_running = false;
        prog_thread.join();
    }
    else {
        // Request-based test
        int requests_per_thread = config.total_requests / config.num_threads;
        int total_target = requests_per_thread * config.num_threads;

        std::thread prog_thread(progress_thread, std::ref(config), total_target);

        for (int i = 0; i < config.num_threads; i++) {
            threads.emplace_back(worker_thread_requests, std::ref(config), requests_per_thread);
        }

        for (auto& t : threads) {
            t.join();
        }

        test_running = false;
        prog_thread.join();
    }

    auto end_time = std::chrono::steady_clock::now();
    long long duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();

    // Print results
    print_results(config, duration_ms);

    // Save to CSV if requested
    if (config.save_csv) {
        save_to_csv(config, config.csv_filename);
    }

    return 0;
}
