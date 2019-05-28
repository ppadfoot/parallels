#pragma once

#include <iostream>
#include <vector>

using Point = std::vector<double>;

std::vector<size_t> KMeans(const std::vector<Point> &data, size_t K);