#pragma once

#include <atomic>
#include <string>
#include <iostream>

#include "fork.h"

using Clock = std::chrono::high_resolution_clock;

class BasePhilosopher {
public:
    BasePhilosopher(const unsigned id, const std::shared_ptr<Fork> fork_left, const std::shared_ptr<Fork> fork_right, const unsigned think_delay, const unsigned eat_delay, const bool debug_flag)
        : id(id), id_(std::to_string(id)), fork_left(fork_left), fork_right(fork_right), think_delay(think_delay), eat_delay(eat_delay), eat_count(0), wait_time(0), stop_flag(false), debug_flag(debug_flag)
    {
    }

    virtual void run() = 0;
    virtual ~BasePhilosopher() = default;

    void stop() {
        stop_flag = true;
    }

    void print_stats() const {
        std::cout << "[" << id << "] " << eat_count << " " << wait_time << "\n";
    }

protected:
    void think() {
        if (debug_flag)
            std::cout << "[" + id_ + "] thinking\n";

        sleep(think_delay);

        if (debug_flag)
            std::cout << "[" + id_ + "] hungry\n";

        wait_start = Clock::now();
    }

    void eat() {
        wait_time += std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - wait_start).count();

        if (debug_flag)
            std::cout << "[" + id_ + "] eating\n";

        sleep(eat_delay);
        eat_count++;
    }

    static void sleep(const int max_delay) {
        std::this_thread::sleep_for(std::chrono::milliseconds(rand() % (max_delay + 1)));
    }

    unsigned id;
    std::string id_;
    std::shared_ptr<Fork> fork_left;
    std::shared_ptr<Fork> fork_right;
    unsigned think_delay;
    unsigned eat_delay;
    unsigned eat_count;
    unsigned wait_time;
    Clock::time_point wait_start;
    std::atomic<bool> stop_flag;
    bool debug_flag;
};