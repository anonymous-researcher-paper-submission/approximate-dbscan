#include "dbscan.h"

// benchmark
void findCoresNormal(Dataset& dataset, ParameterFile parameter) {
    for (ClusterPoint& point : dataset.clusterPoints) {
        for (ClusterPoint& neighbor : dataset.clusterPoints) {
            if (l2_norm(point.coordinates, neighbor.coordinates) <= 1) {
                point.neighbors.push_back(neighbor.index);
            }
        }
        if (point.neighbors.size() >= parameter.minPts) {
            point.isCore = true;
        }
    }
}

float logBaseDefault(float number) {
    return log(number) / log(1 / (1 - DEFAULT_SUCCESS_PROBABILITY));
}

int getFindCoresIterationsFloat(Dataset& dataset, ParameterFile& parameter) {
    int size = dataset.size;
    if (parameter.successRate > 0) {
        return (logBaseDefault(2) + logBaseDefault(size) + logBaseDefault(parameter.minPts) +
                    logBaseDefault(1 / (1 - parameter.successRate)));
    } else {
        return (logBaseDefault(2) + 2 * logBaseDefault(size) +
                    logBaseDefault(parameter.minPts));
    }
}

int getFindCoresIterations(Dataset& dataset, ParameterFile& parameter) {
    return ceil(getFindCoresIterationsFloat(dataset, parameter));
}

void findCores(Dataset& dataset, ParameterFile& parameter, vector<ClusterPoint*>& corePoints) {
    cout << "find cores" << endl;
    double iterations = getFindCoresIterationsFloat(dataset, parameter);
    double L = computeLfromKPFloat(parameter.k, DEFAULT_SUCCESS_PROBABILITY, parameter.w);

    cout << "iterations: " << ceil(L * iterations) << endl;
    parameter.L = (long) ceil(L * iterations);

    if (parameter.minPts <= 1) {
        cout << "all points are core" << endl;
        for (auto& p : dataset.clusterPoints) {
            p.isCore = true;
            corePoints.push_back(&p);
        }
        return;
    }
    
    for (int i = 0; i < ceil(L * iterations); i++) {
        // all points are core, no need to search anymore
        if (corePoints.size() == dataset.clusterPoints.size()) {
            cout << "all points are core" << endl;
            return;
        }
        auto bsStart = chrono::high_resolution_clock::now();
        LSH lsh = LSH(parameter.k, parameter.w, dataset.dimension,
            &dataset.clusterPoints, parameter.rho, dataset.dataEigen);
        auto bsEnd = chrono::high_resolution_clock::now();
        bsElapsed += bsEnd - bsStart;
        auto queryStart = chrono::high_resolution_clock::now();
        lsh.findCorePoints(corePoints, parameter.minPts - 1);
        auto queryEnd = chrono::high_resolution_clock::now();
        queryElapsed += queryEnd - queryStart;
        
    }
    cout << "Time to build structure: " << bsElapsed.count() << "s" << endl;
    cout << "Time to query: " << queryElapsed.count() << "s" << endl;
}

double getConnectCoreIterationsFloat(int size, ParameterFile& parameter) {
    if (parameter.successRate > 0) {
        return (logBaseDefault(2) + 1 * logBaseDefault(size) + logBaseDefault(1 / (1 - parameter.successRate)));
    } else {
        return (logBaseDefault(2) + 2 * logBaseDefault(size));
    }
}

int getConnectCoreIterations(int size, ParameterFile& parameter) {
    return ceil(getConnectCoreIterationsFloat(size, parameter));
}

void connectCorePoints(DisjointSet& ds, Dataset& dataset, vector<ClusterPoint*>& corePoints,
                       ParameterFile& parameter, MatrixXfRowMajor& coreMatrix) {
    cout << "connect core points" << endl;

    if (corePoints.size() <= 1) {
        return;
    }

    // heuristics to handle when minPts is very small
    // add some edges before finding parameters to have potentially a smaller k, L
    for (int i = 0; i < max(13 - parameter.minPts, 2); i++) {
        auto bsStart = chrono::high_resolution_clock::now();
        ModifiedLSH lsh = ModifiedLSH(parameter.k, parameter.w, dataset.dimension,
                                        &dataset.clusterPoints, &corePoints, &ds, parameter.rho, coreMatrix, dataset.dataEigen);
        auto bsEnd = chrono::high_resolution_clock::now();
        ccbsTime += bsEnd - bsStart;
        
        auto start = chrono::high_resolution_clock::now();
        lsh.connectCorePoints();
        auto end = chrono::high_resolution_clock::now();
        ccQueryTotal += end - start;
    }

    auto spStart = chrono::high_resolution_clock::now();
    estimateBestParametersConnectCore(parameter, dataset, corePoints, ds, coreMatrix);
    auto spEnd = chrono::high_resolution_clock::now();
    spTime = spEnd - spStart;
    cout << "Time to connect find parameters for connect cores: " << spTime.count() << "s" << endl;

    int param_k = parameter.k_cc;
    double iterations = getConnectCoreIterationsFloat(corePoints.size(), parameter);

    double L = computeLfromKPFloat(param_k, DEFAULT_SUCCESS_PROBABILITY, parameter.w);
    cout << "iterations: " << ceil(iterations * L) << endl;
    parameter.L_cc = (long) ceil(L * iterations);
    for (int i = 0; i < ceil(iterations * L); i++) {
        auto bsStart = chrono::high_resolution_clock::now();
        ModifiedLSH lsh = ModifiedLSH(param_k, parameter.w, dataset.dimension,
                                        &dataset.clusterPoints, &corePoints, &ds, parameter.rho, coreMatrix, dataset.dataEigen);
        auto bsEnd = chrono::high_resolution_clock::now();
        ccbsTime += bsEnd - bsStart;
        
        auto start = chrono::high_resolution_clock::now();
        lsh.connectCorePoints();
        auto end = chrono::high_resolution_clock::now();
        ccQueryTotal += end - start;
    }
}

void labelPoints(vector<ClusterPoint*>& corePoints, Dataset& dataset, DisjointSet& ds) {
    int current = 0;
    // then identify core or non-core
    vector<ClusterPoint*> nonCorePoints;

    for (ClusterPoint& point : dataset.clusterPoints) {
        if (!point.isCore) {
            nonCorePoints.push_back(&point);
            continue;
        }
        int* label = (&dataset.clusterPoints[ds.find(point.index)].label);

        if (*label == NOISE) {
            (*label) = current++;
        }
        point.label = (*label);
    }
    
    // attach b to neighbors
    for (ClusterPoint* b : nonCorePoints) {
        for (int index : b->neighbors) {
            if (dataset.clusterPoints[index].isCore) {
                b->label = dataset.clusterPoints[index].label;
            }
        }
    }
}