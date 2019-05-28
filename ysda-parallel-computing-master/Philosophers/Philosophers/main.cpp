#include <vector>
#include <algorithm>

#include "blocking_philosopher.h"
#include "philosopher.h"

template <typename TPhilosopher>
std::vector<std::shared_ptr<BasePhilosopher>> get_philosophers(const size_t n, const unsigned think_delay, const unsigned eat_delay, const bool debug_flag) {
    auto forks = std::vector<std::shared_ptr<Fork>>(n);
    for (unsigned i = 0; i < n; i++) {
        forks[i] = std::make_shared<Fork>();
    }

    auto phils = std::vector<std::shared_ptr<BasePhilosopher>>(n);
    for (unsigned i = 0; i < n; i++)
        phils[i] = std::make_shared<TPhilosopher>(i + 1, forks[(i + 1) % n], forks[i], think_delay, eat_delay, debug_flag);

    return phils;
}

int main(const int argc, char* argv[]) {
    if (argc != 6) {
        std::cout << "Usage: " << argv[0] << " phil_count duration think_delay eat_delay debug_flag\n";
        return 1;
    }

    const unsigned N = atoi(argv[1]);
    const unsigned duration = atoi(argv[2]);
    const unsigned think_delay = atoi(argv[3]);
    const unsigned eat_delay = atoi(argv[4]);
    const unsigned debug_flag = atoi(argv[5]);

    srand(static_cast<unsigned>(time(nullptr)));

    auto phils = get_philosophers<Philosopher>(N, think_delay, eat_delay, debug_flag);

    auto threads = std::vector<std::thread>(N);
    for (unsigned i = 0; i < N; i++)
        threads[i] = std::thread(&BasePhilosopher::run, phils[i]);

    std::this_thread::sleep_for(std::chrono::seconds(duration));

    for_each(phils.begin(), phils.end(), std::mem_fn(&BasePhilosopher::stop));
    for_each(threads.begin(), threads.end(), mem_fn(&std::thread::join));
    for_each(phils.begin(), phils.end(), std::mem_fn(&BasePhilosopher::print_stats));

    return 0;
}