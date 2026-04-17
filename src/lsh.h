#include <iostream>
#include <vector>
#include <unordered_map>
#include <cmath>
#include <random>
using namespace std;
#include "data.h"
#include "random.h"
#include <unordered_set>
#include "geometry.h"
#include "timer.h"
#include "util.h"

// lsh.h
#ifndef LSH_H
#define LSH_H

#define COMPARISON_DISTANCE 1
// 4294967291 = 2^32-5
#define UH_PRIME_DEFAULT 4294967291U
// 2^32-1
#define TWO_TO_32_MINUS_1 4294967295U

struct LSHFunction {
    vector<float> a;
    float b;
};

struct LSHHashMatrix {
    Eigen::MatrixXf a;
    Eigen::RowVectorXf b;
};

typedef vector<int> HashBucketType;
typedef unordered_map<int, HashBucketType> HashTableType;

class LSH {
public:
    LSH() {};
    LSH(int k, int w, int dimensions, vector<ClusterPoint>* points, float rho, MatrixXfRowMajor* dataEigen);
    void test();
    int k;
    int w;
    int dimensions;
    float comparisonDistance;
    void estimateHashTime(ClusterPoint& point, double* T_g);
    virtual void estimateQueryTime(ClusterPoint& point, double* T_c);
    void estimateQueryTimeModified(ClusterPoint& point, double* T_c);
    void findCorePoints(vector<ClusterPoint*>& corePoints, int noNeighbors);
    
private:
    HashBucketType sortHashBucket(HashBucketType bucket, int* coreIndex);

protected:
    vector<ClusterPoint>* points;
    MatrixXfRowMajor* dataEigen;
    HashTableType hashBucket;
    LSHHashMatrix hashMatrix;
    virtual void initializeHashFunctions();
    virtual void initializeHashBuckets();
    virtual void hashPointsIntoBuckets();
    int hashLSH(ClusterPoint& point);
};

#endif // LSH_H