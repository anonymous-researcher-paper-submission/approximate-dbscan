#ifndef RANDOM_H
#define RANDOM_H
#include <iostream>
#include <random>
#include <chrono>
#include <algorithm>
#include "data.h"

using namespace std;

void seedRandomEngine();
float randomNormal();
float randomUniform(float max);
void shuffleArray(vector<Point>& array);

#endif // RANDOM_H