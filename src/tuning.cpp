#include "tuning.h"

double computeFunctionP(double w, double c) {
    if (c == 0) {
        return 1;
    }
    double x = w / c;
    return 1 - erfc(x / M_SQRT2) - M_2_SQRTPI / M_SQRT2 / x * (1 - exp(-pow(x, 2) / 2));
}

double computeLfromKPFloat(int k, double successProbability, double w) {
    return log(1 - successProbability) / log(1 - pow(computeFunctionP(w, 1), k));
}

long computeLfromKP(int k, double successProbability, double w) {
    return ceil(computeLfromKPFloat(k, successProbability, w));
}

#define ESTIMATE_ITERATION 20
#define ESTIMATE_SUBSET 1000
#define DEFAULT_K 20
#define DEFAULT_K_APPROX 2
#define FC_CONSTANT_FACTOR (1.0 / 4.0);
#define CC_CONSTANT_FACTOR (1.0 / 2.0);

void calculateHashingTime(int k, ClusterPoint query, vector<ClusterPoint>* arr, float w, int d,
                          double* hash_time, double* comp_time, bool fc) {
    MatrixXfRowMajor A(arr->size(), d);
    for (int i = 0; i < arr->size(); i++) {
        for (int j = 0; j < d; j++) {
            A(i, j) = arr->at(i).coordinates[j];
        }
    }
    LSH lsh = LSH(k, w, d, arr, 0, &A);

    for (int i = 0; i < ESTIMATE_ITERATION; i++) {
        lsh.estimateHashTime(query, hash_time);
    }
    for (int i = 0; i < ESTIMATE_ITERATION; i++) {
        if (fc) lsh.estimateQueryTime(query, comp_time);
        else lsh.estimateQueryTimeModified(query, comp_time);
    }
    *hash_time = *hash_time / ESTIMATE_ITERATION;
    *comp_time = *comp_time / ESTIMATE_ITERATION;

    cout << "Hash time: " << *hash_time << ", Comparison time: " << *comp_time << endl;
}

int getKValue(int n, int w, float factor) {
    float approx_factor = DEFAULT_K_APPROX;
    if (factor > 0.0) {
        approx_factor = (1 + factor);
    }
    float p_2 = computeFunctionP(w, approx_factor);
    auto k = ceil(log((float)n) / log(1 / p_2));
    return max(k, 1.0f);
}

int getKValue(int n, int w, float factor, int minPts) {
    float approx_factor = DEFAULT_K_APPROX;
    if (factor > 0.0) {
        approx_factor = (1 + factor);
        // return 1;
    }
    float p_2 = computeFunctionP(w, approx_factor);
    auto k = ceil(log((float)n / minPts) / log(1 / p_2));
    return max(k, 1.0f);
}

float getExpectCollision(float dist, float w, int k) {
    float p = computeFunctionP(w, dist);
    return pow(p, k);
}

void estimateBestParameters(ParameterFile& parameter, Dataset& dataset) {
    double hash_time = 0;
    double comp_time = 0;

    int setParameterBudget = dataset.size;
    int sampleSize = dataset.size;

    ClusterPoint query = dataset.clusterPoints[rand() % sampleSize];

    std::vector<ClusterPoint> subArray;
    for (int i = 0; i < ESTIMATE_SUBSET && i < dataset.clusterPoints.size(); ++i) {
        subArray.push_back(dataset.clusterPoints[i]);
    }

    calculateHashingTime(DEFAULT_K, query, &subArray, parameter.w, dataset.dimension, &hash_time,
                         &comp_time, true);

    int bestK = 0;
    int bestL = 0;
    double best_time = std::numeric_limits<double>::max();

    setParameterBudget = min((int) floor(sqrt(dataset.size)), dataset.size);
    int D = dataset.dimension;

    Eigen::MatrixXf A(setParameterBudget, D);
    Eigen::MatrixXf B(setParameterBudget, D);
    for (int i = 0; i < setParameterBudget; i++) {
        int random_ind = rand() % sampleSize;
        ClusterPoint query = dataset.clusterPoints[random_ind];
        for (int j = 0; j < D; j++) {
            A(i, j) = dataset.dataEigen->row(random_ind)[j];
        }
        int random_curr = rand() % sampleSize;
        ClusterPoint current = dataset.clusterPoints[random_curr];
        for (int j = 0; j < D; j++) {
            B(i, j) = dataset.dataEigen->row(random_curr)[j];
        }
    }
    Eigen::MatrixXf dot = A * B.transpose();
    Eigen::VectorXf a_norms = A.rowwise().squaredNorm();
    Eigen::VectorXf b_norms = B.rowwise().squaredNorm();

    Eigen::MatrixXf dist_sq = -2 * dot;
    dist_sq.colwise() += a_norms;
    dist_sq.rowwise() += b_norms.transpose();
    Eigen::MatrixXf dist = dist_sq.cwiseMax(0).cwiseSqrt();

    for (int k = 1; k <= getKValue(dataset.size, parameter.w, parameter.rho, parameter.minPts); k += 1) {
        double expected_collisions = 0;
        long L = computeLfromKP(k, DEFAULT_SUCCESS_PROBABILITY, parameter.w);

        cout << "k, L: " << k << " " << L << endl;
        for (int i = 0; i < setParameterBudget; i++) {
            for (int j = 0; j < setParameterBudget; j++) {
                // if (i == j) continue;
                float dist_ij = dist(i, j);
                expected_collisions += getExpectCollision(dist_ij, parameter.w, k);
            }
        }
        double T_G = k * hash_time * L;
        double T_C = (dataset.size / ((double) setParameterBudget * setParameterBudget)) * expected_collisions * comp_time * L * FC_CONSTANT_FACTOR;

        cout << "Estimated time for hashing: " << T_G
             << " Estimated time for comparison T_C: " << T_C << endl;
        double total = T_C + T_G;

        if (total <= best_time) {
            bestK = k;
            bestL = L;
            best_time = total;
        }
    }

    cout << "Best k: " << bestK << endl;

    parameter.k = bestK;
    parameter.L = bestL;
}

void estimateBestParametersConnectCore(ParameterFile& parameter, Dataset& dataset, vector<ClusterPoint *>& corePoints, DisjointSet& ds, MatrixXfRowMajor& coreMatrix) {
    double hash_time = 0;
    double comp_time = 0;

    long size = corePoints.size();

    if (size <= 1) {
        parameter.k_cc = 1;
        parameter.L_cc = 1;
        return;
    }

    int setParameterBudget = size;

    ClusterPoint* query = corePoints[rand() % size];

    std::vector<ClusterPoint> subArray;
    for (int i = 0; i < ESTIMATE_SUBSET && i < size; ++i) {
        subArray.push_back(*corePoints[i]);
    }

    calculateHashingTime(DEFAULT_K, *query, &subArray, parameter.w, dataset.dimension, &hash_time,
                         &comp_time, false);

    int bestK = 0;
    int bestL = 0;
    double best_time = std::numeric_limits<double>::max();

    setParameterBudget = size;
    int D = dataset.dimension;
    int max_k = min(getKValue(size, parameter.w, parameter.rho), getKValue(dataset.size, parameter.w, parameter.rho, parameter.minPts));

    vector<float> espected_colls = vector<float>(max_k, 0.0);

    for (int i = 0; i < setParameterBudget; i++) {
        int random_ind = rand() % size;
        ClusterPoint query = *corePoints[random_ind];
        int random_curr = rand() % size;
        ClusterPoint current = *corePoints[random_curr];
        if (random_ind == random_curr) continue;
        if(ds.find(query.index) == ds.find(current.index)) {
            continue;
        };
        float dist = (coreMatrix.row(random_ind) - coreMatrix.row(random_curr)).norm();

        for (int k = 1; k <= max_k; k += 1) {
            espected_colls[k - 1] += getExpectCollision(dist, parameter.w, k);
        }
    }

    for (int k = 1; k <= max_k; k += 1) {
        float expected_collisions = espected_colls[k - 1];
        long L = computeLfromKP(k, DEFAULT_SUCCESS_PROBABILITY, parameter.w);

        cout << "k, L: " << k << " " << L << endl;
        float T_G = k * hash_time * L;
        float T_C = (size / (double) setParameterBudget) * expected_collisions * comp_time * L * CC_CONSTANT_FACTOR;

        cout << "Estimated time for hashing: " << T_G
             << " Estimated time for comparison T_C: " << T_C << endl;
        float total = T_C + T_G;

        if (total <= best_time) {
            bestK = k;
            bestL = L;
            best_time = total;
        }
    }

    cout << "Best k: " << bestK << endl;

    parameter.k_cc = bestK;
    parameter.L_cc = bestL;
}
