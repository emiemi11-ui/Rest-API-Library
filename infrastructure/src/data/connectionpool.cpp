#include "data/connectionpool.hpp"
#include <iostream>
#include <algorithm>

ConnectionPool::ConnectionPool(size_t min, size_t max,
                               std::function<std::unique_ptr<IDatabase>()> factory,
                               std::chrono::seconds idle_timeout)
    : min_size_(min), max_size_(max), current_size_(0),
      idle_timeout_(idle_timeout), factory_(std::move(factory)) {

    // Pre-create minimum connections
    for (size_t i = 0; i < min_size_; i++) {
        try {
            PooledConnection pc;
            pc.conn = factory_();
            pc.in_use = false;
            pc.connection_id = i;
            pc.last_used = std::chrono::steady_clock::now();
            pool_.push_back(std::move(pc));
            current_size_++;
            total_created_++;
        } catch (const std::exception& e) {
            std::cerr << "[ConnectionPool] Failed to create connection: " << e.what() << "\n";
        }
    }

    std::cout << "[ConnectionPool] Initialized with " << current_size_ << " connections\n";
}

ConnectionPool::~ConnectionPool() {
    std::unique_lock<std::mutex> lock(pool_mutex_);
    pool_.clear();
    std::cout << "[ConnectionPool] Destroyed\n";
}

ConnectionPool::PooledConnectionGuard ConnectionPool::acquire(std::chrono::milliseconds timeout) {
    std::unique_lock<std::mutex> lock(pool_mutex_);

    auto start_wait = std::chrono::steady_clock::now();

    // Wait for available connection
    bool acquired = pool_cv_.wait_for(lock, timeout, [this] {
        // Check for idle connection
        for (auto& pc : pool_) {
            if (!pc.in_use) return true;
        }
        // Check if we can create new connection
        return current_size_ < max_size_;
    });

    if (!acquired) {
        wait_count_++;
        throw std::runtime_error("Connection pool timeout");
    }

    // Try to find idle connection
    for (auto& pc : pool_) {
        if (!pc.in_use) {
            pc.in_use = true;
            pc.last_used = std::chrono::steady_clock::now();
            total_acquired_++;

            auto wait_time = std::chrono::steady_clock::now() - start_wait;
            wait_time_ms_ += std::chrono::duration_cast<std::chrono::milliseconds>(wait_time).count();
            wait_count_++;

            return PooledConnectionGuard(this, &pc);
        }
    }

    // Create new connection if possible
    if (current_size_ < max_size_) {
        PooledConnection pc;
        pc.conn = factory_();
        pc.in_use = true;
        pc.connection_id = current_size_;
        pc.last_used = std::chrono::steady_clock::now();

        pool_.push_back(std::move(pc));
        current_size_++;
        total_created_++;
        total_acquired_++;

        return PooledConnectionGuard(this, &pool_.back());
    }

    throw std::runtime_error("Failed to acquire connection");
}

void ConnectionPool::release(PooledConnection* conn) {
    std::unique_lock<std::mutex> lock(pool_mutex_);
    conn->in_use = false;
    conn->last_used = std::chrono::steady_clock::now();
    pool_cv_.notify_one();
}

void ConnectionPool::prune_idle_connections() {
    std::unique_lock<std::mutex> lock(pool_mutex_);

    auto now = std::chrono::steady_clock::now();

    pool_.erase(
        std::remove_if(pool_.begin(), pool_.end(), [&](const PooledConnection& pc) {
            if (!pc.in_use && current_size_ > min_size_) {
                auto idle_time = std::chrono::duration_cast<std::chrono::seconds>(now - pc.last_used);
                if (idle_time > idle_timeout_) {
                    current_size_--;
                    total_destroyed_++;
                    return true;
                }
            }
            return false;
        }),
        pool_.end()
    );
}

ConnectionPool::Stats ConnectionPool::get_stats() const {
    std::unique_lock<std::mutex> lock(pool_mutex_);

    Stats s;
    s.total_connections = current_size_;
    s.active_connections = 0;
    s.idle_connections = 0;

    for (const auto& pc : pool_) {
        if (pc.in_use) s.active_connections++;
        else s.idle_connections++;
    }

    s.total_acquired = total_acquired_.load();
    s.total_created = total_created_.load();
    s.total_destroyed = total_destroyed_.load();
    s.wait_count = wait_count_.load();
    s.avg_wait_time_ms = wait_count_ > 0 ? wait_time_ms_.load() / wait_count_.load() : 0;

    return s;
}

void ConnectionPool::print_stats() const {
    auto s = get_stats();
    std::cout << "\n╔════════════════════════════════════════╗\n";
    std::cout << "║   Connection Pool Statistics          ║\n";
    std::cout << "╠════════════════════════════════════════╣\n";
    std::cout << "║  Total connections: " << s.total_connections << "               ║\n";
    std::cout << "║  Active: " << s.active_connections << "  Idle: " << s.idle_connections << "              ║\n";
    std::cout << "║  Total acquired: " << s.total_acquired << "              ║\n";
    std::cout << "║  Created: " << s.total_created << "  Destroyed: " << s.total_destroyed << "     ║\n";
    std::cout << "║  Avg wait time: " << s.avg_wait_time_ms << " ms           ║\n";
    std::cout << "╚════════════════════════════════════════╝\n";
}

// PooledConnectionGuard implementation

ConnectionPool::PooledConnectionGuard::PooledConnectionGuard(ConnectionPool* p, PooledConnection* c)
    : pool_(p), conn_(c), acquire_time_(std::chrono::steady_clock::now()) {}

ConnectionPool::PooledConnectionGuard::~PooledConnectionGuard() {
    if (conn_ && pool_) {
        pool_->release(conn_);
    }
}

ConnectionPool::PooledConnectionGuard::PooledConnectionGuard(PooledConnectionGuard&& other) noexcept
    : pool_(other.pool_), conn_(other.conn_), acquire_time_(other.acquire_time_) {
    other.pool_ = nullptr;
    other.conn_ = nullptr;
}

ConnectionPool::PooledConnectionGuard& ConnectionPool::PooledConnectionGuard::operator=(PooledConnectionGuard&& other) noexcept {
    if (this != &other) {
        if (conn_ && pool_) {
            pool_->release(conn_);
        }
        pool_ = other.pool_;
        conn_ = other.conn_;
        acquire_time_ = other.acquire_time_;
        other.pool_ = nullptr;
        other.conn_ = nullptr;
    }
    return *this;
}
