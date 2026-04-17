#ifndef UTIL_H
#define UTIL_H

using namespace std;
#include <iostream>
#include "data.h"
#include <vector>
#include "disjoint_set.h"
#include "modified_lsh.h"
#include "random.h"

#include <Eigen/Dense>

Dataset preprossesData(InputFile& data, float ratio, int minPts);
size_t groupByDisjointSet(DisjointSet* ds, ModifiedBucket* bucket, int dimension, vector<ClusterPoint>* points);
int hashReduceRow(const Eigen::RowVectorXi& row);

#define NOISE -1

#endif // UTIL_H