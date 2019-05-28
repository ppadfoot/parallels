#include <fstream>

#include "data-gen.h"

void WritePoint(Point point, std::ofstream& out) {
    for (size_t i = 0; i < point.size() - 1; ++i)
        out << point[i] << " ";
    out << point[point.size() - 1] << std::endl;
}

int main(int argc , char** argv) {
    if (argc != 5) {
        std::cerr << "Usage: " << argv[0] << " dimensions number_of_points number_of_clusters output_file" << std::endl;
        return 1;
    }
    size_t dimensions = atoi(argv[1]);
    size_t number_of_points = atoi(argv[2]);
    size_t number_of_clusters = atoi(argv[3]); // 0..32767

    std::string output_file = argv[4];
    std::ofstream output(output_file);
    if (!output.is_open()) {
        std::cerr << "Error: output file could not be opened" << std::endl;
        return 1;
    }

    output << number_of_points << " " << dimensions << std::endl;

    auto points = GetPoints(dimensions, number_of_points, number_of_clusters);
    for (size_t i = 0; i < number_of_points; ++i)
        WritePoint(points[i], output);

    output.close();
    return 0;
}