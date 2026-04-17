#include <cmath>
#include <iostream>
#include <random>
#include <unordered_map>
#include <vector>
using namespace std;
#include <unordered_set>

#include "data.h"
#include "disjoint_set.h"
#include "geometry.h"
#include "linked_list.h"
#include "lsh.h"
#include "random.h"
#include "util.h"

// lsh.h
#ifndef MODIFIED_LSH_H
#define MODIFIED_LSH_H

struct ModifiedBucket {
    DoublyLinkedList<BucketGroup*>* indices;
    vector<int> indicesVec;
    bool isFisrtVisit;
};

class ModifiedLSH : public LSH {
   public:
    ModifiedLSH(int k, int w, int dimensions, vector<ClusterPoint>* points,
                vector<ClusterPoint*>* corePoints, DisjointSet* ds, float rho, MatrixXfRowMajor& coreMatrix, MatrixXfRowMajor* dataEigen);
    ~ModifiedLSH();
    void modifiedQuery(ClusterPoint& point);
    void connectCorePoints();

   private:
    unordered_map<int, ModifiedBucket*> modifiedBucket;
    vector<ClusterPoint*>* corePoints;
    DisjointSet* ds;
    size_t groupPoints(ModifiedBucket* bucket);
    void groupConcatenation(BucketGroup* left, BucketGroup* right);

   protected:
    void initializeHashBuckets() override;
    void hashPointsIntoBuckets(MatrixXfRowMajor& coreMatrix);
};

#endif  // MODIFIED_LSH_H