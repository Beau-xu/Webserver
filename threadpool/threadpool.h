#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>

class ThreadPool {
   public:
    explicit ThreadPool(int threadCount = 8);
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool(ThreadPool&&) = default;
    ~ThreadPool();

    template <class F>
    void addTask(F&& task);

   private:
    // struct Pool {
    std::mutex mtx_;
    std::condition_variable condVar_;
    std::queue<std::function<void()>> tasks_;
    bool isClosed_;
    // };
    // std::shared_ptr<Pool> pool_;
};

template <class F>
void ThreadPool::addTask(F&& task) {
    {
        std::lock_guard<std::mutex> locker(pool_->mtx);
        pool_->tasks.emplace(std::forward<F>(task));
    }
    pool_->cond.notify_one();
}

#endif  // THREADPOOL_H
