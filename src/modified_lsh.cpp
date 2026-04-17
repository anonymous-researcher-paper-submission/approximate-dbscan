#include "modified_lsh.h"

ModifiedLSH::ModifiedLSH(int k, int w, int dimensions, vector<ClusterPoint>* points,
                         vector<ClusterPoint*>* corePoints, DisjointSet* ds, float rho, MatrixXfRowMajor& coreMatrix, MatrixXfRowMajor* dataEigen) {
    this->k = k;
    this->w = w;
    this->dimensions = dimensions;
    auto dis = (1 + rho) * COMPARISON_DISTANCE;
    this->comparisonDistance = dis * dis;
    this->points = points;
    this->ds = ds;
    this->corePoints = corePoints;
    this->dataEigen = dataEigen;
    this->initializeHashFunctions();
    this->initializeHashBuckets();
    this->hashPointsIntoBuckets(coreMatrix);
}

ModifiedLSH::~ModifiedLSH() {
    for (auto it = modifiedBucket.begin(); it != modifiedBucket.end(); ++it) {
        ModifiedBucket* modifiedBucket = it->second;
        if (modifiedBucket != nullptr && modifiedBucket->indices != nullptr) {
            delete modifiedBucket->indices;
            modifiedBucket->indicesVec.clear();
        }
        delete modifiedBucket;
    }
}

void ModifiedLSH::initializeHashBuckets() {
    this->modifiedBucket = unordered_map<int, ModifiedBucket*>();
}

void ModifiedLSH::hashPointsIntoBuckets(MatrixXfRowMajor& coreMatrix) {
    auto hashIndices = coreMatrix * this->hashMatrix.a;
    Eigen::MatrixXi hashes = ((hashIndices.rowwise() + hashMatrix.b).array() / w).floor().cast<int>();
    for (int i = 0; i < hashes.rows(); ++i) {
        Eigen::RowVectorXi row = hashes.row(i);
        int hashV = hashReduceRow(row);
        if (modifiedBucket.find(hashV) == modifiedBucket.end()) {
            ModifiedBucket* modBucket = new ModifiedBucket();
            modBucket->isFisrtVisit = true;
            modBucket->indices = new DoublyLinkedList<BucketGroup*>();
            modifiedBucket[hashV] = modBucket;
        }
        modifiedBucket[hashV]->indicesVec.push_back(corePoints->at(i)->index);
    }
}

void ModifiedLSH::groupConcatenation(BucketGroup* left, BucketGroup* right) {
    left->sentinel->prev->next = right->sentinel->next;
    right->sentinel->next->prev = left->sentinel->prev;
    left->sentinel->prev = right->sentinel->prev;
    // reset null pointer
    left->sentinel->prev->next = left->sentinel;

    // prep to remove right
    right->sentinel->next = right->sentinel;
    right->sentinel->prev = right->sentinel;
}

size_t ModifiedLSH::groupPoints(ModifiedBucket* bucket) {
    size_t monoColored = groupByDisjointSet(ds, bucket, dimensions, points);
    return monoColored;
}

void ModifiedLSH::connectCorePoints() {
    auto& currentTable = this->modifiedBucket;
    // each element is a bucket
    for (auto& element: currentTable) {
        ModifiedBucket* bucket = element.second;
        if (bucket->indicesVec.size() <= 1) {
            continue;
        }
        auto start = chrono::high_resolution_clock::now();
        // check if bucket only contain one group, in which case skip
        size_t groupSize = groupPoints(bucket);
        bool monoColored = groupSize <= 1;
        auto end = chrono::high_resolution_clock::now();

        size_t size = groupSize;
        HashBucketType vec = bucket->indicesVec;
        
        ccGroupTime += end - start;
        if (monoColored) {
            continue;
        }

        Node<BucketGroup*>* currentGroup = bucket->indices->sentinel->next;
        while (currentGroup != bucket->indices->sentinel) {
            BucketGroup* groupVecList = currentGroup->data;
            Node<GroupElement>* elementPtr = groupVecList->sentinel->next;
            // traverse all elements in this outer group
            while (elementPtr != groupVecList->sentinel) {
                GroupElement element = elementPtr->data;
                for (int i = 0; i < element.size; i++) {
                    int offsetI = i + element.offset;
                    int index = vec[offsetI];
                    ClusterPoint& p = (*points)[index];

                    Node<BucketGroup*>* otherGroup = currentGroup->next;
                    while (otherGroup != bucket->indices->sentinel) {
                        Node<BucketGroup*>* nextGroup = otherGroup->next;
                        BucketGroup* innerVecList = otherGroup->data;
                        Node<GroupElement>* innerPtr = innerVecList->sentinel->next;
                        // traverse all elements in this inner group
                        while (innerPtr != innerVecList->sentinel) {
                            GroupElement innerElement = innerPtr->data;
                            bool shouldBreak = false;
                            for (int j=0; j < innerElement.size; j++) {
                                int offsetJ = j + innerElement.offset;
                                int otherIndex = vec[offsetJ];
                                ClusterPoint& q = (*points)[otherIndex];
                                if ((this->dataEigen->row(index) - this->dataEigen->row(otherIndex)).squaredNorm() <= comparisonDistance) {
                                    ds->unionSets(index, otherIndex);
                                    // concate
                                    groupConcatenation(groupVecList, innerVecList);
                                    // remove
                                    bucket->indices->remove(otherGroup);
                                    shouldBreak = true;
                                    break;
                                }
                            }
                            if (shouldBreak) {
                                break;
                            }
                            innerPtr = innerPtr->next;
                        }

                        otherGroup = nextGroup;
                    }
                }
                elementPtr = elementPtr->next;
            }
            currentGroup = currentGroup->next;
        }
    }
}
