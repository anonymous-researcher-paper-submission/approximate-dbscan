#include "util.h"

#include <vector>

Dataset preprossesData(InputFile& data, float ratio, int minPts) {
    Dataset dataset;

    // randomly permutate the array such that the input order does not affect order in hash buckets
    shuffleArray(data.data);

    for (int i = 0; i < data.data.size(); ++i) {
        auto& point = data.data[i];

        ClusterPoint clusterPoint;
        clusterPoint.coordinates = point.coordinates;
        for (float& coordinate : clusterPoint.coordinates) {
            coordinate = coordinate / ratio;
        }
        clusterPoint.index = i;
        clusterPoint.originalIndex = point.index;
        clusterPoint.label = NOISE;
        clusterPoint.isCore = false;
        clusterPoint.neighbors = vector<int>();
        clusterPoint.neighbors.reserve(minPts);
        dataset.clusterPoints.push_back(clusterPoint);
    }
    dataset.dimension = data.dimension;
    dataset.size = data.size;
    return dataset;
}

size_t groupHash(DisjointSet* ds, ModifiedBucket* bucket, int dimension, vector<ClusterPoint>* points) {
    unordered_map<int, vector<int>> groups;

    for (auto index : bucket->indicesVec) {
        int key = ds->find(index);
        groups[key].push_back(index);
    }
    if (groups.size() <= 1) {
        return groups.size();
    }

    vector<int> copy(bucket->indicesVec.size());

    int counter = 0;
    for (const auto& group : groups) {
        bool isFisrt = true;
        BucketGroup* bGroup = new DoublyLinkedList<GroupElement>();
        int offset = counter;
        for (int i = 0; i < group.second.size(); i++) {
            int element = group.second[i];
            copy[counter] = element;
            counter++;
        }
        GroupElement e;
        e.offset = offset;
        e.size = group.second.size();
        bGroup->append(e);
        bucket->indices->append(bGroup);
    }

    bucket->indicesVec.clear();
    bucket->indicesVec = copy;
    return groups.size();
}

size_t groupByDisjointSet(DisjointSet* ds, ModifiedBucket* bucket, int dimension, vector<ClusterPoint>* points) {
    if (ds == nullptr) {
        std::cerr << "Disjoint set or points is not initialized!" << std::endl;
        return true;
    }
    return groupHash(ds, bucket, dimension, points);
}

int hashReduceRow(const Eigen::RowVectorXi& row) {
    std::hash<int> hasher;
    int seed = 0;
    for (int i = 0; i < row.size(); i++) {
        seed ^= hasher(row(i)) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }
    return seed;
}
