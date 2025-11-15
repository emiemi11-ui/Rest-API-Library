
#pragma once
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <functional>
#include <vector>
#include <atomic>

class ThreadPool {
public:
    ThreadPool();
    explicit ThreadPool(int n);
    ~ThreadPool();
    void init(int n);
    void enqueue(std::function<void()> f);
    void stop();
private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex m;
    std::condition_variable cv;
    std::atomic<bool> stopping{false};
    void worker_loop();
};
