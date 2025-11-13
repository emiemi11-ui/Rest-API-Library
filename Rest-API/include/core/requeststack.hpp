#pragma once
#include <vector>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <chrono>
#include "http/request.hpp"
#include "ipc/priorityqueue.hpp"

// STACK pentru procesare cereri (cerință profesoară: argumentare STACK)
class RequestStack {
private:
    struct RequestEntry {
        HttpRequest request;
        int client_fd;
        MessageFlags priority;  // Prioritate
        uint64_t timestamp;     // Timestamp pentru aging
    };

    std::vector<RequestEntry> stack_;  // STACK (LIFO)
    mutable std::mutex mutex_;
    mutable std::condition_variable cv_;
    size_t max_size_;

    // Metrics
    std::atomic<uint64_t> total_pushed_{0};
    std::atomic<uint64_t> total_popped_{0};
    std::atomic<uint64_t> total_rejected_{0};

public:
    RequestStack(size_t max_size = 1000) : max_size_(max_size) {}

    // PUSH (Thread-safe)
    bool push(const HttpRequest& req, int client_fd, MessageFlags priority);

    // POP (Thread-safe, blocking)
    RequestEntry pop();

    // TRY POP (non-blocking)
    bool try_pop(RequestEntry& out, std::chrono::milliseconds timeout);

    // Stats
    size_t size() const;
    bool is_empty() const;
    bool is_full() const;

    struct Stats {
        size_t current_size;
        uint64_t total_pushed;
        uint64_t total_popped;
        uint64_t total_rejected;
    };
    Stats get_stats() const;
};
