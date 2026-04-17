#include <iostream>
#include <vector>
#include <string>
#include <nlohmann/json.hpp>
#include <fstream>
#include <unordered_set>
#include <sys/resource.h>

using namespace std;
using json = nlohmann::json;

#include "geometry.h"
#include "util.h"
#include "data.h"
#include "lsh.h"
#include "dbscan.h"
#include "random.h"
#include "disjoint_set.h"
#include "linked_list.h"
#include "tuning.h"
#include <csignal>

#include <Eigen/Dense>
using namespace Eigen::indexing;

#ifndef MAIN_H
#define MAIN_H

#endif // MAIN_H