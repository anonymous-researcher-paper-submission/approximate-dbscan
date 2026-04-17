#include "random.h"

default_random_engine generator;
normal_distribution<double> distribution(0, 1); 

void seedRandomEngine() {
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    generator.seed(seed);
}

float randomNormal() {
    return distribution(generator);
}

float randomUniform(float max) {
    uniform_real_distribution<double> uniform_distribution(0, max);
    return uniform_distribution(generator);
}

void shuffleArray(vector<Point>& array) {
    shuffle(array.begin(), array.end(), generator);
}
