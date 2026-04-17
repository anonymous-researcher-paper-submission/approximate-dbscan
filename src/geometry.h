#include <vector>
#include <cmath>
#include <stdexcept>

#ifndef GEOMETRY_H
#define GEOMETRY_H

float l2_norm_squared(const std::vector<float>& a, const std::vector<float>& b);
float l2_norm(const std::vector<float>& a, const std::vector<float>& b);
float distance(const std::vector<float>& a, const std::vector<float>& b);
float distance_squared(const std::vector<float>& vec1, const std::vector<float>& vec2);

#endif // GEOMETRY_H
