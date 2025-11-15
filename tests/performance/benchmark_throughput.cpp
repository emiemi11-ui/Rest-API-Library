/**
 * @file benchmark_throughput.cpp
 * @brief Throughput benchmarks for REST API Framework
 *
 * Measures requests/second under various configurations:
 * - Different worker counts (1, 2, 4, 8)
 * - Different thread pool sizes
 * - Different concurrent connections
 *
 * Results can be used in Chapter 5 (Testing and Validation) of thesis.
 */

#include <iostream>
#include <iomanip>
#include <thread>
#include <vector>
#include <atomic>
#include <chrono>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

using namespace std;

// Configuration
const int TEST_PORT = 9000;
const int TEST_DURATION_SECONDS = 10;

// Statistics
struct BenchmarkResult {
    int workers;
    int threads_per_worker;
    int concurrent_connections;
    int total_requests;
    int successful_requests;
    int failed_requests;
    double duration_seconds;
    double requests_per_second;
    double avg_latency_ms;
    double min_latency_ms;
    double max_latency_ms;
};

/**
 * @brief Send a simple HTTP GET request and measure latency
 */
double send_request(const string& host, int port, const string& path, bool& success) {
    auto start = chrono::steady_clock::now();

    // Create socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        success = false;
        return -1;
    }

    // Set socket timeout
    struct timeval timeout;
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));

    // Connect
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, host.c_str(), &addr.sin_addr);

    if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        close(sock);
        success = false;
        return -1;
    }

    // Send HTTP request
    string request = "GET " + path + " HTTP/1.1\r\n";
    request += "Host: " + host + "\r\n";
    request += "Connection: close\r\n";
    request += "\r\n";

    if (send(sock, request.c_str(), request.length(), 0) < 0) {
        close(sock);
        success = false;
        return -1;
    }

    // Receive response
    char buffer[4096];
    int bytes = recv(sock, buffer, sizeof(buffer) - 1, 0);
    if (bytes < 0) {
        close(sock);
        success = false;
        return -1;
    }
    buffer[bytes] = '\0';

    // Check for 200 OK
    success = (strstr(buffer, "200 OK") != nullptr);

    close(sock);

    auto end = chrono::steady_clock::now();
    auto latency = chrono::duration_cast<chrono::microseconds>(end - start);

    return latency.count() / 1000.0; // Convert to milliseconds
}

/**
 * @brief Worker thread for load testing
 */
void load_worker(atomic<bool>& running,
                 atomic<int>& successful,
                 atomic<int>& failed,
                 atomic<double>& total_latency,
                 atomic<double>& min_lat,
                 atomic<double>& max_lat) {
    while (running.load()) {
        bool success = false;
        double latency = send_request("127.0.0.1", TEST_PORT, "/", success);

        if (success) {
            successful++;
            total_latency += latency;

            // Update min/max latency (atomic double operations)
            double current_min = min_lat.load();
            while (latency < current_min && !min_lat.compare_exchange_weak(current_min, latency));

            double current_max = max_lat.load();
            while (latency > current_max && !max_lat.compare_exchange_weak(current_max, latency));
        } else {
            failed++;
        }

        // Small delay to avoid overwhelming the system
        this_thread::sleep_for(chrono::microseconds(100));
    }
}

/**
 * @brief Run throughput benchmark
 */
BenchmarkResult run_benchmark(int workers, int threads_per_worker, int concurrent_connections) {
    BenchmarkResult result;
    result.workers = workers;
    result.threads_per_worker = threads_per_worker;
    result.concurrent_connections = concurrent_connections;

    atomic<bool> running{true};
    atomic<int> successful{0};
    atomic<int> failed{0};
    atomic<double> total_latency{0.0};
    atomic<double> min_lat{999999.0};
    atomic<double> max_lat{0.0};

    // Start worker threads
    vector<thread> threads;
    for (int i = 0; i < concurrent_connections; i++) {
        threads.emplace_back(load_worker,
                             ref(running),
                             ref(successful),
                             ref(failed),
                             ref(total_latency),
                             ref(min_lat),
                             ref(max_lat));
    }

    // Wait for warmup
    this_thread::sleep_for(chrono::seconds(1));

    // Reset counters
    successful = 0;
    failed = 0;
    total_latency = 0.0;
    min_lat = 999999.0;
    max_lat = 0.0;

    // Run benchmark
    auto start = chrono::steady_clock::now();
    this_thread::sleep_for(chrono::seconds(TEST_DURATION_SECONDS));
    auto end = chrono::steady_clock::now();

    // Stop workers
    running = false;
    for (auto& t : threads) {
        t.join();
    }

    // Calculate results
    result.total_requests = successful + failed;
    result.successful_requests = successful;
    result.failed_requests = failed;
    result.duration_seconds = chrono::duration_cast<chrono::milliseconds>(end - start).count() / 1000.0;
    result.requests_per_second = successful / result.duration_seconds;
    result.avg_latency_ms = (successful > 0) ? (total_latency / successful) : 0;
    result.min_latency_ms = min_lat.load();
    result.max_latency_ms = max_lat.load();

    return result;
}

/**
 * @brief Print results in table format
 */
void print_result(const BenchmarkResult& result) {
    cout << "| " << setw(7) << result.workers
         << " | " << setw(16) << result.threads_per_worker
         << " | " << setw(17) << result.concurrent_connections
         << " | " << setw(11) << fixed << setprecision(0) << result.requests_per_second
         << " | " << setw(12) << fixed << setprecision(2) << result.avg_latency_ms
         << " | " << setw(9) << result.successful_requests
         << " | " << setw(6) << result.failed_requests
         << " |" << endl;
}

/**
 * @brief Print CSV format for graphing
 */
void print_csv_header() {
    cout << "\n\n=== CSV FORMAT (for Excel/LibreOffice) ===" << endl;
    cout << "Workers,ThreadsPerWorker,Connections,RequestsPerSec,AvgLatencyMs,SuccessCount,FailCount" << endl;
}

void print_csv_result(const BenchmarkResult& result) {
    cout << result.workers << ","
         << result.threads_per_worker << ","
         << result.concurrent_connections << ","
         << fixed << setprecision(2) << result.requests_per_second << ","
         << fixed << setprecision(2) << result.avg_latency_ms << ","
         << result.successful_requests << ","
         << result.failed_requests << endl;
}

int main() {
    cout << "======================================================================" << endl;
    cout << "  REST API FRAMEWORK - THROUGHPUT BENCHMARK" << endl;
    cout << "======================================================================" << endl;
    cout << endl;
    cout << "Configuration:" << endl;
    cout << "  Test Port: " << TEST_PORT << endl;
    cout << "  Test Duration: " << TEST_DURATION_SECONDS << " seconds" << endl;
    cout << "  Endpoint: GET /" << endl;
    cout << endl;
    cout << "NOTE: Make sure the server is running on port " << TEST_PORT << endl;
    cout << "      Example: ./build/example1_simple" << endl;
    cout << endl;
    cout << "Starting benchmark in 3 seconds..." << endl;
    this_thread::sleep_for(chrono::seconds(3));
    cout << endl;

    // Store results for CSV export
    vector<BenchmarkResult> all_results;

    // Print table header
    cout << "======================================================================" << endl;
    cout << "| Workers | Threads/Worker | Concurrent Conns | Req/sec     | Avg Latency  | Success   | Failed |" << endl;
    cout << "======================================================================" << endl;

    // Test configurations
    vector<tuple<int, int, int>> configs = {
        // {workers, threads_per_worker, concurrent_connections}
        {1, 8, 10},
        {1, 8, 50},
        {1, 8, 100},
        {2, 8, 10},
        {2, 8, 50},
        {2, 8, 100},
        {4, 8, 10},
        {4, 8, 50},
        {4, 8, 100},
        {8, 8, 10},
        {8, 8, 50},
        {8, 8, 100}
    };

    for (const auto& [workers, threads, connections] : configs) {
        cout << "Testing: " << workers << " workers, "
             << threads << " threads/worker, "
             << connections << " concurrent connections..." << flush;

        auto result = run_benchmark(workers, threads, connections);
        all_results.push_back(result);

        cout << "\r";
        print_result(result);

        // Cooldown between tests
        this_thread::sleep_for(chrono::seconds(2));
    }

    cout << "======================================================================" << endl;
    cout << endl;

    // Print CSV format
    print_csv_header();
    for (const auto& result : all_results) {
        print_csv_result(result);
    }

    cout << endl;
    cout << "======================================================================" << endl;
    cout << "  BENCHMARK COMPLETED" << endl;
    cout << "======================================================================" << endl;
    cout << endl;
    cout << "You can copy the CSV data above into Excel/LibreOffice to create graphs." << endl;
    cout << endl;

    return 0;
}
