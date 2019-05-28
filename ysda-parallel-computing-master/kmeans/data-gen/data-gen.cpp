#include <cmath>
#include <vector>

#include "data-gen.h"

struct ClusterParams {
    Point mean;
    double var;
};

// Pretty good uniform distribution on [0..1]
double RandUniform01() {
    int rand1 = rand();
    double div = RAND_MAX;
    while (rand1 == RAND_MAX) {
        rand1 = rand();
    }
    return ((double)rand1 + rand() / div) / div;
}

// Box-Muller transform
double RandNormal(double mean, double sigma) {
    double x, y, r;
    do {  
        x = 2 * RandUniform01() - 1;
        y = 2 * RandUniform01() - 1;
        r = x * x + y * y;
    } while (r == 0.0 || r > 1.0);
    return sigma * x * sqrt(-2 * log(r) / r) + mean;
}

Point RandomPointGauss(ClusterParams params) {
    size_t dimensions = params.mean.size();
    Point coord(dimensions);
    for (size_t i = 0; i < dimensions; ++i) {
        coord[i] = RandNormal(params.mean[i], params.var);
    }
    return coord;
}

Point RandomPointUniform(size_t dimensions, double space_size) {
    Point coord(dimensions);
    for (size_t i = 0; i < dimensions; ++i) {
        coord[i] = RandUniform01() * space_size;
    }
    return coord;
}

std::vector<Point> GetPoints(size_t dimensions, size_t points, size_t clusters) {
    // advanced params
    double space_size = 100;
    double cluster_size = 5;
    int random_point_pct = 20;

    srand((unsigned) time(nullptr));

    std::vector<ClusterParams> cluster_params(clusters);
    for (size_t i = 0; i < clusters; ++i) {
        cluster_params[i].mean = RandomPointUniform(dimensions, space_size);
        cluster_params[i].var = cluster_size / 2 + RandUniform01() * cluster_size;
    }

    std::vector<Point> points_arr(points);
    for (size_t i = 0; i < points; ++i) {
        bool in_cluster = (rand() % 100) >= random_point_pct;
        points_arr[i] = in_cluster
                        ? RandomPointGauss(cluster_params[rand() % clusters])
                        : RandomPointUniform(dimensions, space_size);
    }

    return points_arr;
}