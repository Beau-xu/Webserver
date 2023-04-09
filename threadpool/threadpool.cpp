#include "threadpool.h"

#include <cassert>
#include <thread>

ThreadPool::ThreadPool(int threadCount = 8) : isClosed_(false) {
    assert(threadCount > 0);
    for (int i = 0; i < threadCount; ++i) {
        std::thread([this] {  // 捕获this以访问私有成员
            std::unique_lock<std::mutex> locker(mtx_);
            while (true) {
                if (isClosed_) break;
                if (tasks_.empty()) condVar_.wait(locker);
                auto task = std::move(tasks_.front());
                tasks_.pop();
                locker.unlock();
                task();
                locker.lock();
            }
        }).detach();
    }
}

ThreadPool::~ThreadPool() {
    {
        std::lock_guard<std::mutex> locker(mtx_);
        isClosed_ = true;
    }
    condVar_.notify_all();
}
