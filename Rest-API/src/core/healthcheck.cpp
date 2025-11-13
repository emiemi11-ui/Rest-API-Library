#include "core/healthcheck.hpp"
#include <iostream>

void HealthCheck::register_check(const std::string& name, CheckFunction check) {
    checks_[name] = std::move(check);
    std::cout << "[HealthCheck] Registered: " << name << "\n";
}

std::vector<HealthCheckResult> HealthCheck::run_all() {
    std::vector<HealthCheckResult> results;

    for (const auto& [name, check] : checks_) {
        try {
            auto start = std::chrono::steady_clock::now();
            auto result = check();
            auto end = std::chrono::steady_clock::now();

            result.response_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
            result.timestamp = std::chrono::system_clock::now();

            results.push_back(result);
        } catch (const std::exception& e) {
            HealthCheckResult result;
            result.component = name;
            result.status = HealthStatus::UNHEALTHY;
            result.message = std::string("Exception: ") + e.what();
            result.timestamp = std::chrono::system_clock::now();
            results.push_back(result);
        }
    }

    std::lock_guard<std::mutex> lock(results_mutex_);
    last_results_ = results;

    return results;
}

HealthCheckResult HealthCheck::run_check(const std::string& name) {
    auto it = checks_.find(name);
    if (it == checks_.end()) {
        HealthCheckResult result;
        result.component = name;
        result.status = HealthStatus::UNHEALTHY;
        result.message = "Check not found";
        return result;
    }

    try {
        auto start = std::chrono::steady_clock::now();
        auto result = it->second();
        auto end = std::chrono::steady_clock::now();

        result.response_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        result.timestamp = std::chrono::system_clock::now();

        return result;
    } catch (const std::exception& e) {
        HealthCheckResult result;
        result.component = name;
        result.status = HealthStatus::UNHEALTHY;
        result.message = std::string("Exception: ") + e.what();
        return result;
    }
}

HealthStatus HealthCheck::get_overall_status() {
    std::lock_guard<std::mutex> lock(results_mutex_);

    if (last_results_.empty()) {
        return HealthStatus::UNHEALTHY;
    }

    bool has_unhealthy = false;
    bool has_degraded = false;

    for (const auto& result : last_results_) {
        if (result.status == HealthStatus::UNHEALTHY) {
            has_unhealthy = true;
        } else if (result.status == HealthStatus::DEGRADED) {
            has_degraded = true;
        }
    }

    if (has_unhealthy) return HealthStatus::UNHEALTHY;
    if (has_degraded) return HealthStatus::DEGRADED;
    return HealthStatus::HEALTHY;
}

void HealthCheck::start_periodic_checks(std::chrono::seconds interval) {
    if (running_.exchange(true)) {
        return; // Already running
    }

    check_thread_ = std::thread([this, interval]() {
        while (running_) {
            auto results = run_all();

            // Log results
            for (const auto& r : results) {
                const char* status_str = r.status == HealthStatus::HEALTHY ? "HEALTHY" :
                                       r.status == HealthStatus::DEGRADED ? "DEGRADED" : "UNHEALTHY";
                std::cout << "[HealthCheck] " << r.component << ": " << status_str
                         << " (" << r.response_time.count() << "ms) - " << r.message << "\n";
            }

            std::this_thread::sleep_for(interval);
        }
    });
}

void HealthCheck::stop_periodic_checks() {
    if (running_.exchange(false)) {
        if (check_thread_.joinable()) {
            check_thread_.join();
        }
    }
}
