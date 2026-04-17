#ifndef DCSCAN_H
#define DCSCAN_H

#include <iostream>
#include <vector>
#include <unordered_map>
#include <cmath>
#include <random>
#include "data.h"
#include "geometry.h"
#include "lsh.h"
#include "disjoint_set.h"
#include "util.h"
#include "modified_lsh.h"
#include "tuning.h"
#include "timer.h"

#define DEFAULT_LOWEST_PROBABILITY 0.5

using namespace std;

void findCores(Dataset& dataset, ParameterFile& parameter, vector<ClusterPoint*>& corePoints);
void connectCorePoints(DisjointSet& ds, Dataset& dataset, vector<ClusterPoint*>& corePoints, ParameterFile& parameter, MatrixXfRowMajor& coreMatrix);
void labelPoints(vector<ClusterPoint*>& corePoints, Dataset& dataset, DisjointSet& ds);

#endif // DCSCAN_H