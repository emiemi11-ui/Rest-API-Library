#pragma once
#include "data/idatabase.hpp"
#include <vector>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <functional>
#include <chrono>
#include <atomic>

// Connection Pool pentru DATABASE (SCALABILITATE!)
class ConnectionPool {
private:
    struct PooledConnection {
        std::unique_ptr<IDatabase> conn;
        std::chrono::steady_clock::time_point last_used;
        bool in_use;
        uint32_t connection_id;
    };

    std::vector<PooledConnection> pool_;
    mutable std::mutex pool_mutex_;
    mutable std::condition_variable pool_cv_;

    size_t min_size_;
    size_t max_size_;
    size_t current_size_;
    std::chrono::seconds idle_timeout_;

    // Connection factory
    std::function<std::unique_ptr<IDatabase>()> factory_;

    // Metrics (pentru PERFORMANȚĂ!)
    std::atomic<uint64_t> total_acquired_{0};
    std::atomic<uint64_t> total_created_{0};
    std::atomic<uint64_t> total_destroyed_{0};
    std::atomic<uint64_t> wait_count_{0};
    std::atomic<uint64_t> wait_time_ms_{0};

public:
    ConnectionPool(size_t min, size_t max,
                   std::function<std::unique_ptr<IDatabase>()> factory,
                   std::chrono::seconds idle_timeout = std::chrono::seconds(300));

    ~ConnectionPool();

    // RAII wrapper pentru connection
    class PooledConnectionGuard {
        ConnectionPool* pool_;
        PooledConnection* conn_;
        std::chrono::steady_clock::time_point acquire_time_;
    public:
        PooledConnectionGuard(ConnectionPool* p, PooledConnection* c);
        ~PooledConnectionGuard();

        // Delete copy
        PooledConnectionGuard(const PooledConnectionGuard&) = delete;
        PooledConnectionGuard& operator=(const PooledConnectionGuard&) = delete;

        // Allow move
        PooledConnectionGuard(PooledConnectionGuard&& other) noexcept;
        PooledConnectionGuard& operator=(PooledConnectionGuard&& other) noexcept;

        IDatabase& operator*() { return *conn_->conn; }
        IDatabase* operator->() { return conn_->conn.get(); }
        bool isValid() const { return conn_ != nullptr; }
    };

    // Acquire connection (blocking cu timeout)
    PooledConnectionGuard acquire(std::chrono::milliseconds timeout = std::chrono::milliseconds(5000));

    // Release connection (internal, called by guard destructor)
    void release(PooledConnection* conn);

    // Maintenance
    void prune_idle_connections();

    // Metrics
    struct Stats {
        size_t active_connections;
        size_t idle_connections;
        size_t total_connections;
        uint64_t total_acquired;
        uint64_t total_created;
        uint64_t total_destroyed;
        uint64_t wait_count;
        uint64_t avg_wait_time_ms;
    };
    Stats get_stats() const;

    void print_stats() const;
};
