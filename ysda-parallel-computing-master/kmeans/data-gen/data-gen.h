#pragma once

#include <vector>
#include <iostream>

using Point = std::vector<double>;

std::vector<Point> GetPoints(size_t dimensions, size_t points, size_t clusters);