#pragma once

#include "base_philosopher.h"

class Philosopher : public BasePhilosopher {
public:
    Philosopher(const unsigned id, const std::shared_ptr<Fork> fork_left, const std::shared_ptr<Fork> fork_right, const unsigned think_delay, const unsigned eat_delay, const bool debug_flag)
        : BasePhilosopher(id, fork_left, fork_right, think_delay, eat_delay, debug_flag)
    {
    }

    void run() override {
        while (!stop_flag) {
            think();

            while (true) {
                if (!fork_left->try_take_for(std::chrono::milliseconds(10)))
                    continue;

                if (debug_flag)
                    std::cout << "[" + id_ + "] tool left fork\n";

                if (!fork_right->try_take_for(std::chrono::milliseconds(10))) {
                    fork_left->put();

                    if (debug_flag)
                        std::cout << "[" + id_ + "] put left fork\n";

                    continue;
                }

                if (debug_flag)
                    std::cout << "[" + id_ + "] took right fork\n";

                eat();

                fork_right->put();
                if (debug_flag)
                    std::cout << "[" + id_ + "] put right fork\n";

                fork_left->put();
                if (debug_flag)
                    std::cout << "[" + id_ + "] put left fork\n";

                break;
            }
        }
    }
};