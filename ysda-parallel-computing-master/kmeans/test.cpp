#include <ctime>
#include <iostream>
#include <vector>
#include <functional>
#include <numeric>
#include <cmath>

#include "data-gen/data-gen.h"
#include "kmeans/kmeans.h"

void measure(std::function<void()> action, size_t n = 5) {
    auto measures = std::vector<double>(n);
    for (size_t i = 0; i < n; i++) {
        clock_t begin = clock();
        action();
        clock_t end = clock();
        measures[i] = double(end - begin) / CLOCKS_PER_SEC;
    }

    auto mean = std::accumulate(measures.begin(), measures.end(), 0.0) / n;

    auto squareSum = std::accumulate(measures.begin(), measures.end(), 0.0,
                                     [](double a, double b) -> double { return a + b * b; });
    auto stdev = std::sqrt(squareSum / n - mean * mean);
    std::cout << "Elapsed: " << mean << " (" << stdev << ")" << std::endl;
}

void measure_kmeans(size_t dimensions, size_t points, size_t clusters) {
    auto data = GetPoints(dimensions, points, clusters);
    measure([data, clusters]() -> void { KMeans(data, clusters); });
}

int main(int argc, char *argv[]) {
    size_t dimensions = atoi(argv[1]);
    size_t number_of_points = atoi(argv[2]);
    size_t number_of_clusters = atoi(argv[3]); // 0..32767
    measure_kmeans(dimensions, number_of_points, number_of_clusters);
}