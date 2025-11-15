#include "core/requeststack.hpp"
#include <chrono>

bool RequestStack::push(const HttpRequest& req, int client_fd, MessageFlags priority) {
    std::unique_lock<std::mutex> lock(mutex_);

    if (stack_.size() >= max_size_) {
        total_rejected_++;
        return false; // Stack plin
    }

    RequestEntry entry;
    entry.request = req;
    entry.client_fd = client_fd;
    entry.priority = priority;
    entry.timestamp = std::chrono::steady_clock::now().time_since_epoch().count();

    stack_.push_back(entry); // PUSH pe STACK (LIFO!)
    total_pushed_++;

    cv_.notify_one(); // Notifică consumatorii
    return true;
}

RequestStack::RequestEntry RequestStack::pop() {
    std::unique_lock<std::mutex> lock(mutex_);

    // Așteaptă până avem elemente
    cv_.wait(lock, [this] { return !stack_.empty(); });

    // POP de la sfârșitul stack-ului (LIFO!)
    RequestEntry entry = stack_.back();
    stack_.pop_back();
    total_popped_++;

    return entry;
}

bool RequestStack::try_pop(RequestEntry& out, std::chrono::milliseconds timeout) {
    std::unique_lock<std::mutex> lock(mutex_);

    // Așteaptă cu timeout
    if (!cv_.wait_for(lock, timeout, [this] { return !stack_.empty(); })) {
        return false; // Timeout
    }

    out = stack_.back();
    stack_.pop_back();
    total_popped_++;

    return true;
}

size_t RequestStack::size() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return stack_.size();
}

bool RequestStack::is_empty() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return stack_.empty();
}

bool RequestStack::is_full() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return stack_.size() >= max_size_;
}

RequestStack::Stats RequestStack::get_stats() const {
    std::lock_guard<std::mutex> lock(mutex_);
    Stats s;
    s.current_size = stack_.size();
    s.total_pushed = total_pushed_.load();
    s.total_popped = total_popped_.load();
    s.total_rejected = total_rejected_.load();
    return s;
}
