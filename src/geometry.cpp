#include "geometry.h"

float l2_norm_squared(const std::vector<float>& vec1, const std::vector<float>& vec2) {
    if (vec1.size() != vec2.size()) {
        throw std::invalid_argument("Vectors must be of the same length");
    }

    float sum = 0.0;
    for (size_t i = 0; i < vec1.size(); ++i) {
        float diff = vec1[i] - vec2[i];
        sum += diff * diff;
        
    }
    return sum;
}

float l2_norm(const std::vector<float>& vec1, const std::vector<float>& vec2) {
    return std::sqrt(l2_norm_squared(vec1, vec2));
}

float distance(const std::vector<float>& vec1, const std::vector<float>& vec2) {
    return l2_norm(vec1, vec2);
}

float distance_squared(const std::vector<float>& vec1, const std::vector<float>& vec2) {
    return l2_norm_squared(vec1, vec2);
}