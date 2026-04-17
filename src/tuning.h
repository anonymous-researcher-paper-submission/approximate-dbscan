#include <math.h>
#include "data.h"
#include "random.h"
#include <unordered_set>
#include <vector>
#include "lsh.h"
#include "disjoint_set.h"
using namespace std;

#define DEFAULT_SUCCESS_PROBABILITY 0.9
#define TUNING_ITERATIONS 3

double computeFunctionP(double w, double c);
long computeLfromKP(int k, double successProbability, double w);
double computeLfromKPFloat(int k, double successProbability, double w);
void estimateBestParameters(ParameterFile& parameter, Dataset& dataset);
void estimateBestParametersConnectCore(ParameterFile& parameter, Dataset& dataset, vector<ClusterPoint *>& corePoints, DisjointSet& ds, MatrixXfRowMajor& coreMatrix);
