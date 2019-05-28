#include <cmath>
#include <vector>

#include "kmeans.h"

// Gives random number in range [0..max_value]
unsigned int UniformRandom(unsigned int max_value) {
    unsigned int rnd = ((static_cast<unsigned int>(rand()) % 32768) << 17) |
                       ((static_cast<unsigned int>(rand()) % 32768) << 2) |
                       rand() % 4;
    return ((max_value + 1 == 0) ? rnd : rnd % (max_value + 1));
}

double Distance(const Point &point1, const Point &point2) {
    double distance_sqr = 0;
    size_t dimensions = point1.size();
    for (size_t i = 0; i < dimensions; ++i) {
        distance_sqr += (point1[i] - point2[i]) * (point1[i] - point2[i]);
    }
    return sqrt(distance_sqr);
}

size_t FindNearestCentroid(const std::vector<Point> &centroids, const Point &point) {
    double min_distance = Distance(point, centroids[0]);
    size_t centroid_index = 0;
    for (size_t i = 1; i < centroids.size(); ++i) {
        double distance = Distance(point, centroids[i]);
        if (distance < min_distance) {
            min_distance = distance;
            centroid_index = i;
        }
    }
    return centroid_index;
}

// Calculates new centroid position as mean of positions of 3 random centroids
Point GetRandomPosition(const std::vector<Point> &centroids) {
    size_t K = centroids.size();
    int c1 = rand() % K;
    int c2 = rand() % K;
    int c3 = rand() % K;
    size_t dimensions = centroids[0].size();
    Point new_position(dimensions);
    for (size_t d = 0; d < dimensions; ++d) {
        new_position[d] = (centroids[c1][d] + centroids[c2][d] + centroids[c3][d]) / 3;
    }
    return new_position;
}

std::vector<size_t> KMeans(const std::vector<Point> &data, size_t K) {
    size_t data_size = data.size();
    size_t dimensions = data[0].size();
    std::vector<size_t> clusters(data_size);

    // Initialize centroids randomly at data points
    std::vector<Point> centroids(K);
#pragma omp parallel for
    for (size_t i = 0; i < K; ++i) {
        centroids[i] = data[UniformRandom(data_size - 1)];
    }

    bool converged;
    while (true) {
        converged = true;
#pragma omp parallel for
        for (size_t i = 0; i < data_size; ++i) {
            size_t nearest_cluster = FindNearestCentroid(centroids, data[i]);
            if (clusters[i] != nearest_cluster) {
                clusters[i] = nearest_cluster;
                converged = false;
            }
        }
        if (converged) {
            break;
        }

        std::vector<size_t> clusters_sizes(K);
        centroids.assign(K, Point(dimensions));
#pragma omp parallel
        {
            std::vector<size_t> cluster_sizes_local(K);
            std::vector<Point> centroids_local(K);
            centroids_local.assign(K, Point(dimensions));
#pragma omp for nowait
            for (size_t i = 0; i < data_size; ++i) {
                for (size_t d = 0; d < dimensions; ++d)
                    centroids_local[clusters[i]][d] += data[i][d];
                ++cluster_sizes_local[clusters[i]];
            }
#pragma omp critical
            {
                for (int i = 0; i < K; ++i) {
                    for (int d = 0; d < dimensions; ++d)
                        centroids[i][d] += centroids_local[i][d];
                    clusters_sizes[i] += cluster_sizes_local[i];
                }
            }
        }

#pragma omp parallel for
        for (size_t i = 0; i < K; ++i) {
            if (clusters_sizes[i] != 0) {
                for (size_t d = 0; d < dimensions; ++d) {
                    centroids[i][d] /= clusters_sizes[i];
                }
            } else {
                centroids[i] = GetRandomPosition(centroids);
            }
        }
    }

    return clusters;
}