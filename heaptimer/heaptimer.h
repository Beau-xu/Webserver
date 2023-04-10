#ifndef HEAPTIMER_H
#define HEAPTIMER_H

#include <chrono>
#include <functional>
#include <unordered_map>
#include <vector>

#include "../log/log.h"

typedef std::function<void()> TimeoutCallBack;
typedef std::chrono::high_resolution_clock Clock;
typedef std::chrono::milliseconds MS;
typedef Clock::time_point TimeStamp;

struct TimerNode {
    int id;
    TimeStamp expires;
    TimeoutCallBack cb;
    bool operator<(const TimerNode& t) { return expires < t.expires; }
};

class HeapTimer {
   public:
    HeapTimer() { heap_.reserve(64); }
    ~HeapTimer() { clear(); }
    void adjust(int id, int newExpires);
    void add(int id, int timeOut, const TimeoutCallBack& cb);
    void doWork(int id);
    void clear();
    void tick();
    int getNextTick();

   private:
    void swapNode_(size_t i, size_t j);
    void siftup_(size_t i);
    bool siftdown_(size_t index, size_t n);
    void del_(size_t i);
    std::vector<TimerNode> heap_;
    std::unordered_map<int, size_t> ref_;  // Node id 与 heap index 的对应关系
};

#endif  // HEAPTIMER_H
