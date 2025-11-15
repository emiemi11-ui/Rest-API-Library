
#include "core/threadpool.hpp"

ThreadPool::ThreadPool() {}
ThreadPool::ThreadPool(int n){ init(n); }
ThreadPool::~ThreadPool(){ stop(); }

void ThreadPool::init(int n){
    if (!workers.empty()) return;
    stopping=false;
    for (int i=0;i<n;i++){
        workers.emplace_back([this](){ this->worker_loop(); });
    }
}
void ThreadPool::enqueue(std::function<void()> f){
    {
        std::lock_guard<std::mutex> lk(m);
        tasks.push(std::move(f));
    }
    cv.notify_one();
}
void ThreadPool::stop(){
    stopping = true;
    cv.notify_all();
    for (auto& t: workers){
        if (t.joinable()) t.join();
    }
    workers.clear();
}
void ThreadPool::worker_loop(){
    while(true){
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lk(m);
            cv.wait(lk, [this](){ return stopping || !tasks.empty(); });
            if (stopping && tasks.empty()) return;
            task = std::move(tasks.front());
            tasks.pop();
        }
        task();
    }
}
