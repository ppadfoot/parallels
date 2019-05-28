#include <ctime>
#include <fstream>
#include <omp.h>

#include "kmeans.h"

void ReadPoints(std::vector<Point> *data, std::ifstream& input) {
    size_t data_size;
    size_t dimensions;
    input >> data_size >> dimensions;
    data->assign(data_size, Point(dimensions));
    for (size_t i = 0; i < data_size; ++i) {
        for (size_t d = 0; d < dimensions; ++d) {
            double coord;
            input >> coord;
            (*data)[i][d] = coord;
        }
    }
}

void WriteOutput(const std::vector<size_t>& clusters, std::ofstream& output) {
    for (size_t i = 0; i < clusters.size(); ++i) {
        output << clusters[i] << std::endl;
    }
}

int main(int argc , char** argv) {
    double start = omp_get_wtime();
    int num_threads = omp_get_num_threads();

    if (argc != 4) {
        std::printf("Usage: %s number_of_clusters input_file output_file\n", argv[0]);
        return 1;
    }
    size_t K = atoi(argv[1]);

    char *input_file = argv[2];
    std::ifstream input;
    input.open(input_file, std::ifstream::in);
    if (!input) {
        std::cerr << "Error: input file could not be opened" << std::endl;
        return 1;
    }

    std::vector<Point> data;
    ReadPoints(&data, input);
    input.close();

    char *output_file = argv[3];
    std::ofstream output;
    output.open(output_file, std::ifstream::out);
    if (!output) {
        std::cerr << "Error: output file could not be opened" << std::endl;
        return 1;
    }

    srand(123); // for reproducible results

    std::vector<size_t> clusters = KMeans(data, K);

    WriteOutput(clusters, output);
    output.close();

    double end = omp_get_wtime();
    std::cout << "n_threads=" << num_threads << ", Runtime=" << end - start << std::endl;

    return 0;
}