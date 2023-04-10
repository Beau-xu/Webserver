#include "heaptimer.h"

#include <cassert>

void HeapTimer::swapNode_(size_t i, size_t j) {
    assert(i >= 0 && i < heap_.size());
    assert(j >= 0 && j < heap_.size());
    std::swap(heap_[i], heap_[j]);
    ref_[heap_[i].id] = i;
    ref_[heap_[j].id] = j;
}

void HeapTimer::siftup_(size_t child) {
    assert(child < heap_.size());
    size_t parent = (child - 1) / 2;
    while (child > 0 && heap_[parent] < heap_[child]) {
        parent = (child - 1) / 2;
        swapNode_(child, parent);
        child = parent;
    }
}

bool HeapTimer::siftdown_(size_t parent, size_t size) {
    assert(parent < heap_.size());
    assert(size >= 0 && size <= heap_.size());
    size_t i = parent;
    size_t child = i * 2 + 1;
    while (child < size) {
        if (child + 1 < size && heap_[child + 1] < heap_[child]) ++child;
        if (heap_[i] < heap_[child]) break;
        swapNode_(i, child);
        i = child;
        child = i * 2 + 1;
    }
    return i > parent;
}

void HeapTimer::add(int id, int timeout, const TimeoutCallBack& cb) {
    assert(id >= 0);
    size_t i;
    if (ref_.count(id) == 0) {
        i = heap_.size();
        ref_[id] = i;
        heap_.push_back({id, Clock::now() + MS(timeout), cb});
        siftup_(i);
    } else {
        i = ref_[id];
        heap_[i].expires = Clock::now() + MS(timeout);
        heap_[i].cb = cb;
        if (!siftdown_(i, heap_.size())) siftup_(i);
    }
}

void HeapTimer::doWork(int id) {  // 删除指定id结点，并触发回调函数
    if (heap_.empty() || ref_.count(id) == 0) return;
    size_t i = ref_[id];
    TimerNode& node = heap_[i];
    node.cb();
    del_(i);
}

void HeapTimer::del_(size_t index) {  // 删除指定位置的结点
    assert(!heap_.empty() && index >= 0 && index < heap_.size());
    /* 将要删除的结点换到队尾，然后调整堆 */
    size_t tail = heap_.size() - 1;
    assert(index <= tail);
    if (index < tail) {
        swapNode_(index, tail);
        if (!siftdown_(index, tail)) {
            siftup_(index);
        }
    }
    ref_.erase(heap_.back().id);
    heap_.pop_back();
}

void HeapTimer::adjust(int id, int timeout) {  // 调整指定id的结点
    assert(!heap_.empty() && ref_.count(id) > 0);
    heap_[ref_[id]].expires = Clock::now() + MS(timeout);
    if (!siftdown_(ref_[id], heap_.size())) siftup_(ref_[id]);
}

void HeapTimer::tick() {  // 清除超时结点
    if (heap_.empty()) return;
    while (!heap_.empty()) {
        TimerNode& node = heap_.front();
        if (std::chrono::duration_cast<MS>(node.expires - Clock::now()).count() > 0) break;
        node.cb();
        del_(0);
    }
}

void HeapTimer::clear() {
    ref_.clear();
    heap_.clear();
}

int HeapTimer::getNextTick() {
    tick();
    size_t res = 0;
    if (heap_.empty()) return -1;
    res = std::chrono::duration_cast<MS>(heap_.front().expires - Clock::now()).count();
    return res < 0 ? 0 : res;
}
