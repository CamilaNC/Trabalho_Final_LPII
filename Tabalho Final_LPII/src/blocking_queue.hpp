#pragma once

#include <condition_variable>
#include <mutex>
#include <queue>

template<typename T>
class BlockingQueue {
public:
    BlockingQueue() = default;
    ~BlockingQueue() = default;
    void push(T item) {
        {
            std::lock_guard<std::mutex> lk(m_);
            q_.push(std::move(item));
        }
        cv_.notify_one();
    }

    bool pop(T &out) {
        std::unique_lock<std::mutex> lk(m_);
        cv_.wait(lk, [&]() { return !q_.empty() || shutdown_; });
        if (q_.empty()) return false;
        out = std::move(q_.front());
        q_.pop();
        return true;
    }

    void notify_shutdown() {
        {
            std::lock_guard<std::mutex> lk(m_);
            shutdown_ = true;
        }
        cv_.notify_all();
    }

    bool empty() const {
        std::lock_guard<std::mutex> lk(m_);
        return q_.empty();
    }

private:
    mutable std::mutex m_;
    std::condition_variable cv_;
    std::queue<T> q_;
    bool shutdown_ = false;
};
