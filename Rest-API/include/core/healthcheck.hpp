#pragma once
#include <string>
#include <functional>
#include <vector>
#include <atomic>
#include <chrono>
#include <map>
#include <thread>
#include <mutex>

enum class HealthStatus {
    HEALTHY,
    DEGRADED,
    UNHEALTHY
};

struct HealthCheckResult {
    std::string component;
    HealthStatus status;
    std::string message;
    std::chrono::milliseconds response_time;
    std::chrono::system_clock::time_point timestamp;
};

// Health Check pentru componente individuale
class HealthCheck {
public:
    using CheckFunction = std::function<HealthCheckResult()>;

    void register_check(const std::string& name, CheckFunction check);

    // Run all checks
    std::vector<HealthCheckResult> run_all();

    // Run single check
    HealthCheckResult run_check(const std::string& name);

    // Overall system health
    HealthStatus get_overall_status();

    // Auto-check în background
    void start_periodic_checks(std::chrono::seconds interval);
    void stop_periodic_checks();

private:
    std::map<std::string, CheckFunction> checks_;
    std::atomic<bool> running_{false};
    std::thread check_thread_;

    std::vector<HealthCheckResult> last_results_;
    mutable std::mutex results_mutex_;
};
