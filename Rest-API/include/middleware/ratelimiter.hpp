#ifndef RATELIMITER_HPP
#define RATELIMITER_HPP

#include <string>
#include <map>
#include <deque>
#include <chrono>
#include <mutex>

// Forward declarations
namespace http {
    class Request;
    class Response;
}

namespace middleware {

/**
 * @brief Rate limiting middleware to prevent API abuse
 *
 * Implements token bucket algorithm for rate limiting.
 * Tracks requests per IP address.
 */
class RateLimiter {
public:
    /**
     * @brief Constructor
     * @param max_requests Maximum requests allowed in time window
     * @param window_seconds Time window in seconds
     */
    RateLimiter(int max_requests = 100, int window_seconds = 60);

    /**
     * @brief Check if request should be allowed
     * @param request HTTP request
     * @param response HTTP response (used to send 429 if rate limited)
     * @return true if request is allowed, false if rate limited
     */
    bool checkLimit(const http::Request& request, http::Response& response);

    /**
     * @brief Reset rate limit for specific IP
     * @param ip IP address
     */
    void resetLimit(const std::string& ip);

    /**
     * @brief Get remaining requests for IP
     * @param ip IP address
     * @return Number of remaining requests
     */
    int getRemainingRequests(const std::string& ip);

    /**
     * @brief Cleanup old entries (call periodically)
     */
    void cleanup();

private:
    int max_requests_;
    int window_seconds_;

    struct ClientInfo {
        std::deque<std::chrono::steady_clock::time_point> requests;
        std::chrono::steady_clock::time_point last_cleanup;
    };

    std::map<std::string, ClientInfo> clients_;
    std::mutex mutex_;

    std::string extractClientIp(const http::Request& request);
    void removeOldRequests(ClientInfo& client);
};

} // namespace middleware

#endif // RATELIMITER_HPP
