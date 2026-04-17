#include "lsh.h"

LSH::LSH(int k, int w, int dimensions, vector<ClusterPoint>* points, float rho, MatrixXfRowMajor* dataEigen) {
    this->k = k;
    this->w = w;
    auto dis = (1 + rho) * COMPARISON_DISTANCE;
    this->comparisonDistance = dis * dis;
    this->dimensions = dimensions;
    this->points = points;
    this->dataEigen = dataEigen;
    this->initializeHashFunctions();
    this->initializeHashBuckets();
    this->hashPointsIntoBuckets();
}

void LSH::initializeHashFunctions() {
    Eigen::MatrixXf a = Eigen::MatrixXf::Zero(dimensions, k);
    Eigen::RowVectorXf b = Eigen::RowVectorXf::Zero(k);
    hashMatrix = LSHHashMatrix{a, b};
    for (int j = 0; j < k; j++) {
        for (int d = 0; d < dimensions; d++) {
            hashMatrix.a(d, j) = randomNormal();
        }
        hashMatrix.b(j) = randomUniform(w);
    }
}

void LSH::initializeHashBuckets() {
    unordered_map<int, vector<int>> bucket;
    hashBucket = bucket;
}

void LSH::hashPointsIntoBuckets() {
    auto hashIndices = *dataEigen * this->hashMatrix.a;
    Eigen::MatrixXi hashes = ((hashIndices.rowwise() + hashMatrix.b).array() / w).floor().cast<int>();
    for (int i = 0; i < hashes.rows(); ++i) {
        Eigen::RowVectorXi row = hashes.row(i);
        int hashV = hashReduceRow(row);
        hashBucket[hashV].push_back(i);
    }
}

void LSH::estimateHashTime(ClusterPoint& point, double* T_g) {
    auto start_hash = std::chrono::high_resolution_clock::now();
    long long hash = 0;
    auto hashIndices = *dataEigen * this->hashMatrix.a;
    Eigen::MatrixXi hashes = ((hashIndices.rowwise() + hashMatrix.b).array() / w).floor().cast<int>();
    for (int i = 0; i < hashes.rows(); ++i) {
        Eigen::RowVectorXi row = hashes.row(i);
        int hashV = hashReduceRow(row);
        hash += hashV;
    }
    // trick the compiler to avoid Dead Code Elimination using o3
    asm volatile("" : : "g"(hash) : "memory");
    auto end_hash = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> hash_duration = end_hash - start_hash;
    auto hash_time = hash_duration.count();
    hash_time /= points->size();
    hash_time /= k;
    *T_g += hash_time;
}

void LSH::estimateQueryTime(ClusterPoint& point, double* T_c) {
    auto start = std::chrono::high_resolution_clock::now();
    float total_sum = 0.0;
    for (int i = 0; i < points->size(); i++) {
        total_sum += (dataEigen->row(0) - dataEigen->row(i)).squaredNorm();
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = end - start;
    // trick the compiler to avoid Dead Code Elimination using o3
    asm volatile("" : : "g"(total_sum) : "memory");

    auto dis = duration.count();
    dis /= points->size();
    *T_c += dis;
}

void LSH::estimateQueryTimeModified(ClusterPoint& point, double* T_c) {
    // simulate address chasing in linked list
    DoublyLinkedList<int> * l = new DoublyLinkedList<int>();
    std::vector<int> data(dataEigen->rows());
    std::iota(data.begin(), data.end(), 0);

    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(data.begin(), data.end(), g);

    for (auto d : data) {
        l->append(d);
    }

    auto start = std::chrono::high_resolution_clock::now();
    float total_sum = 0.0;
    
    for (Node<int>* curr = l->sentinel->next; curr != l->sentinel; curr = curr->next) {
        total_sum += (dataEigen->row(0) - dataEigen->row(curr->data)).squaredNorm();
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = end - start;
    // trick the compiler to avoid Dead Code Elimination using o3
    asm volatile("" : : "g"(total_sum) : "memory");

    delete l;

    auto dis = duration.count();
    dis /= points->size();
    *T_c += dis;
}

HashBucketType LSH::sortHashBucket(HashBucketType bucket, int* coreIndex) {
    HashBucketType cores = vector<int>();
    HashBucketType nonCores = vector<int>();
    for (int index : bucket) {
        ClusterPoint& point = (*points)[index];
        if (point.isCore) {
            cores.push_back(index);
        } else {
            nonCores.push_back(index);
        }
    }
    *coreIndex = nonCores.size();
    for (int index : cores) {
        nonCores.push_back(index);
    }
    return nonCores;
}

#define BSEARCH_TREASHOLD 10

bool findMidLeft(ClusterPoint& sourcePoint, int targetIndex, int offset) {
    if (sourcePoint.neighbors.size() - offset <= BSEARCH_TREASHOLD) {
        return find(sourcePoint.neighbors.begin(), sourcePoint.neighbors.begin() + offset, targetIndex) != sourcePoint.neighbors.begin() + offset;
    }
    return binary_search(sourcePoint.neighbors.begin(), sourcePoint.neighbors.begin() + offset, targetIndex);
}

bool findMidRight(ClusterPoint& sourcePoint, int targetIndex, int offset) {
    return find(sourcePoint.neighbors.begin() + offset, sourcePoint.neighbors.end(), targetIndex) != sourcePoint.neighbors.end();
}

bool findNeighbor(ClusterPoint& sourcePoint, int targetIndex, int offset) {
    bool exists = findMidLeft(sourcePoint, targetIndex, offset);
    // bool exists_2 = findMidRight(sourcePoint, targetIndex, offset);
    // return exists || exists_2;
    return exists;
    // return findMidLeft(sourcePoint, targetIndex, offset);
}

void pushToNeighbors(ClusterPoint& sourcePoint, int targetIndex) {
    auto it = std::lower_bound(sourcePoint.neighbors.begin(), sourcePoint.neighbors.end(), targetIndex);
    sourcePoint.neighbors.insert(it, targetIndex);
}

void handleEndRound(std::vector<ClusterPoint>* points, HashBucketType& bucket, vector<int>& neighborSize) {
    for (int i=0; i<bucket.size(); i++) {
        int index = bucket[i];
        ClusterPoint& point = (*points)[index];
        auto neiSize = neighborSize[i];

        // if (neiSize == 0) {
        //     sort(point.neighbors.begin(), point.neighbors.end());
        //     continue;
        // }
        // else 
        if (neiSize == point.neighbors.size()) {
            continue;
        }
        // sort(point.neighbors.begin() + neiSize, point.neighbors.end());
        inplace_merge(point.neighbors.begin(), point.neighbors.begin() + neiSize, point.neighbors.end());
    }
}

void LSH::findCorePoints(vector<ClusterPoint*>& corePoints, int noNeighbors) {
    HashTableType currentTable = this->hashBucket;

    for (auto& element: currentTable) {
        HashBucketType bucket = element.second;
        if (bucket.size() <= 1) continue;

        vector<int> neighborSize(bucket.size());

        MatrixXfRowMajor bucketMat(bucket.size(), dimensions);
        for (int i = 0; i < bucket.size(); i++) {
            int index = bucket[i];
            ClusterPoint& point = (*points)[index];
            for (int j = 0; j < dimensions; j++) {
                bucketMat(i, j) = point.coordinates[j];
            }
            neighborSize[i] = point.neighbors.size();
        }

        for (int i = 0; i < bucket.size(); ++i) {
            int index = bucket[i];
            ClusterPoint& point = (*points)[index];
            if(point.isCore) continue;

            auto row = bucketMat.row(i);

            for (int j = 0; j < bucket.size(); j++) {
                auto collisionIndex = bucket[j];
                // ClusterPoint& collision = (*points)[collisionIndex];

                if (j == i) continue;
                // else if (j < i) {
                //     bool becomesCore = collision.neighbors.size() > neighborSize[j];
                //     // must have compared distance if collision is not core
                //     if (!collision.isCore) continue;
                //     // case this point findNeighborwas core before this iteration; can skip
                //     // either wise, check if collision becomes core in this bucket or in previous bucket
                //     // check last index of collision.neighbors; if it is larger, then must have compared distance
                //     else if (becomesCore && collision.neighbors[collision.neighbors.size() - 1] > index) {
                //         continue;
                //     }
                // }

                // if(findNeighbor(point, collisionIndex, point.neighbors.size())) {
                //     continue;
                // }

                float dist_2 = (bucketMat.row(j) - row).squaredNorm();

                if (dist_2 <= comparisonDistance) {
                    if(findNeighbor(point, collisionIndex, neighborSize[i])) {
                        continue;
                    }
                    point.neighbors.push_back(collisionIndex);
                    // pushToNeighbors(point, collisionIndex);

                    // if (!collision.isCore) {
                    //     if(!findNeighbor(collision, index, 0)) {
                    //         collision.neighbors.push_back(index);
                    //         if(collision.neighbors.size() >= noNeighbors) {
                    //             collision.isCore = true;
                    //             corePoints.push_back(&collision);
                    //         }
                    //     }
                    // }

                    if (point.neighbors.size() >= noNeighbors) {
                        point.isCore = true;
                        corePoints.push_back(&point);
                        break;
                    }
                }
            }
        }

        // handle bucket neighbors mergings
        handleEndRound(points, bucket, neighborSize);
    }
}
