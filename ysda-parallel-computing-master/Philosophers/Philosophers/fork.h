#pragma once

#include <mutex>

class Fork {
public:
    Fork() : m()
    {
    }

    bool try_take_for(const std::chrono::milliseconds time) {
        return m.try_lock_for(time);
    }

    void take() {
        m.lock();
    }

    void put() {
        m.unlock();
    }
private:
    std::timed_mutex m;
};