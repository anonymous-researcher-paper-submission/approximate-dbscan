#ifndef DATA_H
#define DATA_H

#include <nlohmann/json.hpp>
#include <vector>

#include <Eigen/Dense>

using namespace std;

struct Point {
    std::vector<float> coordinates;
    unsigned long index;
};

struct InputFile {
    int dimension;
    int size;
    std::vector<Point> data;
};

struct ParameterFile {
    int minPts;
    double epsilon;
    double rho;
    double w;
    int k;
    long L;
    int k_cc;
    long L_cc;
    double successRate;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Point, coordinates, index)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(InputFile, dimension, size, data)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ParameterFile, minPts, epsilon)

struct ClusterPoint {
    std::vector<float> coordinates;
    std::vector<int> neighbors;
    int index;
    int originalIndex;
    int label;
    bool isCore;
};

typedef Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> MatrixXfRowMajor;

struct Dataset {
    std::vector<ClusterPoint> clusterPoints;
    MatrixXfRowMajor* dataEigen;
    int dimension;
    int size;
};

struct OutputPoint {
    int index;
    int label;
    bool isCore;
};

struct FindCoreBreakDown {
    float hashTime;
    float comparisonTime;
    float queryTotal;
    float buildLSH;
};

struct ConnectCoreBreakDown {
    float hashTime;
    float queryTime;
    float groupTime;
    float queryTotal;
    float buildLSH;
};

struct RunStatistics {
    float totalTime;
    float connectCore;
    float findCore;
    float setParameter;
    string outputFile;
    int noCore;
    int inputSize;
    int minPts;
    double epsilon;
    int k;
    long L;
    int k_cc;
    long L_cc;
    float rho;
    float successRate;
    long memoryUsage;
    FindCoreBreakDown findCoreBreakdown;
    ConnectCoreBreakDown connectCoreBreakdown;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(OutputPoint, index, isCore, label)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(FindCoreBreakDown, queryTotal, buildLSH)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ConnectCoreBreakDown, queryTime, groupTime, queryTotal, buildLSH)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(RunStatistics, totalTime, connectCore, findCore, setParameter,
                                   outputFile, noCore, inputSize, minPts, epsilon, k, L, k_cc, L_cc,
                                   rho, successRate, memoryUsage, findCoreBreakdown, connectCoreBreakdown)

#endif  // DATA_H