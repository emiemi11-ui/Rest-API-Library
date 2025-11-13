#ifndef METRICS_HPP
#define METRICS_HPP

#include <string>
#include <map>
#include <atomic>
#include <mutex>
#include <chrono>
#include <vector>

namespace monitoring {

/**
 * @brief Metrics collector for monitoring server performance
 *
 * Collects and exposes various metrics about server operation.
 */
class Metrics {
public:
    /**
     * @brief Get singleton instance
     */
    static Metrics& getInstance();

    /**
     * @brief Record request
     * @param method HTTP method
     * @param path Request path
     * @param status_code Response status code
     * @param duration_ms Request duration in milliseconds
     */
    void recordRequest(const std::string& method, const std::string& path,
                      int status_code, double duration_ms);

    /**
     * @brief Increment counter
     * @param name Counter name
     * @param value Value to add (default: 1)
     */
    void incrementCounter(const std::string& name, int value = 1);

    /**
     * @brief Set gauge value
     * @param name Gauge name
     * @param value Current value
     */
    void setGauge(const std::string& name, double value);

    /**
     * @brief Record histogram value
     * @param name Histogram name
     * @param value Value to record
     */
    void recordHistogram(const std::string& name, double value);

    /**
     * @brief Get metrics as JSON
     * @return JSON string with all metrics
     */
    std::string toJson() const;

    /**
     * @brief Get metrics as Prometheus format
     * @return Prometheus-formatted metrics
     */
    std::string toPrometheus() const;

    /**
     * @brief Reset all metrics
     */
    void reset();

    // Specific metrics getters
    uint64_t getTotalRequests() const { return total_requests_; }
    uint64_t getSuccessfulRequests() const { return successful_requests_; }
    uint64_t getFailedRequests() const { return failed_requests_; }
    double getAverageResponseTime() const;
    double getRequestsPerSecond() const;

private:
    Metrics();
    ~Metrics() = default;
    Metrics(const Metrics&) = delete;
    Metrics& operator=(const Metrics&) = delete;

    // Request metrics
    std::atomic<uint64_t> total_requests_{0};
    std::atomic<uint64_t> successful_requests_{0};  // 2xx status codes
    std::atomic<uint64_t> failed_requests_{0};      // 4xx, 5xx status codes

    // Timing metrics
    std::atomic<double> total_response_time_{0.0};
    std::chrono::steady_clock::time_point start_time_;

    // Status code distribution
    mutable std::mutex mutex_;
    std::map<int, uint64_t> status_code_counts_;
    std::map<std::string, uint64_t> endpoint_counts_;

    // Custom counters and gauges
    std::map<std::string, std::atomic<int>> counters_;
    std::map<std::string, double> gauges_;

    // Histograms (simplified - stores recent values)
    struct Histogram {
        std::vector<double> values;
        static const size_t MAX_VALUES = 1000;

        void add(double value) {
            if (values.size() >= MAX_VALUES) {
                values.erase(values.begin());
            }
            values.push_back(value);
        }

        double getPercentile(double percentile) const;
        double getAverage() const;
    };

    std::map<std::string, Histogram> histograms_;
};

} // namespace monitoring

#endif // METRICS_HPP
