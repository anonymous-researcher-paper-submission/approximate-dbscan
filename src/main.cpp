#include "main.h"

#include <limits.h>
#include <unistd.h>

vector<vector<float>> readCoordinates(const string& inputFilePath) {
    ifstream inputFile(inputFilePath);
    if (!inputFile.is_open()) {
        cerr << "Could not open the file!" << endl;
        exit(1);
    }

    vector<vector<float>> coordinates;
    string line;
    int dimension = 0;
    while (getline(inputFile, line)) {
        if (dimension == 0) {
            stringstream tempStream(line);
            float tempValue;
            while (tempStream >> tempValue) {
                ++dimension;
            }
        }
        vector<float> point;
        point.reserve(dimension);
        stringstream ss(line);
        float value;
        while (ss >> value) {
            point.push_back(value);
        }
        coordinates.push_back(point);
    }

    inputFile.close();
    return coordinates;
}

InputFile readInputFile(const string& inputFilePath) {
    InputFile output;
    vector<vector<float>> coordinates = readCoordinates(inputFilePath);
    output.dimension = coordinates[0].size();
    output.size = coordinates.size();
    output.data = vector<Point>(output.size);
    for (unsigned long i = 0; i < output.size; ++i) {
        Point point;
        point.coordinates = coordinates[i];
        point.index = i;
        output.data[i] = point;
    }

    return output;
}

ParameterFile readParameterFile(const string& parameterFilePath) {
    ifstream parameterFile(parameterFilePath);
    if (!parameterFile.is_open()) {
        cerr << "Could not open the file!" << endl;
        exit(1);
    }

    json parameterObject;
    parameterFile >> parameterObject;

    auto output = parameterObject.get<ParameterFile>();

    if (parameterObject.contains("rho")) {
        output.rho = parameterObject["rho"].get<double>();
    } else {
        output.rho = 0.0;
    }

    // fix domain of rho
    if (output.rho < 0) {
        output.rho = 0.0;
    }

    if (parameterObject.contains("successRate")) {
        output.successRate = parameterObject["successRate"].get<double>();
        cout << "success rate: " << output.successRate << endl;
    } else {
        output.successRate = 0.0;
    }

    // fix domain of success rate
    if (output.successRate >= 1) {
        cout << "not possible to achieve success rate of 1; using default success rate of 1/n" << endl;
        output.successRate = 0.0;
    } else if (output.successRate <= 0) {
        cout << "not possible to achieve success rate of 0; using default success rate of 1/n" << endl;
        output.successRate = 0.0;
    }

    return output;
}

double getSuccessRate(ParameterFile parameter, int size) {
    if (parameter.successRate > 0) {
        return parameter.successRate;
    } else {
        return 1 - 1 / (double)size;
    }
}

void outputToFile(Dataset& dataset, string outputFilePath) {
    ofstream outputFile(outputFilePath);
    vector<int> outputPoints = vector<int>(dataset.size);
    for (ClusterPoint& point : dataset.clusterPoints) {
        outputPoints[point.originalIndex] = point.label;
    }
    if (!outputFile.is_open()) {
        cerr << "Could not open the file!" << endl;
        exit(1);
    }

    for (auto label:outputPoints) {
        outputFile << label << " ";
    }

    outputFile.close();
}

void outputStatistics(string outputStatisticsPath, double totalTime, double connectCore,
                      double findCore, double setParameter, string outputFilePath, int noCore,
                      int inputSize, ParameterFile parameter, long memoryUsage) {
    ofstream outputFile(outputStatisticsPath);
    if (!outputFile.is_open()) {
        cerr << "Could not open the file!" << endl;
        exit(1);
    }

    RunStatistics statistics;
    statistics.totalTime = totalTime;
    statistics.connectCore = connectCore;
    statistics.findCore = findCore;
    statistics.setParameter = setParameter;
    statistics.outputFile = outputFilePath;
    statistics.noCore = noCore;
    statistics.inputSize = inputSize;
    statistics.minPts = parameter.minPts;
    statistics.epsilon = parameter.epsilon;
    statistics.k = parameter.k;
    statistics.L = parameter.L;
    statistics.k_cc = parameter.k_cc;
    statistics.L_cc = parameter.L_cc;
    statistics.rho = parameter.rho;
    statistics.successRate = getSuccessRate(parameter, inputSize);
    statistics.memoryUsage = memoryUsage;
    FindCoreBreakDown findCoreBreakdown;
    findCoreBreakdown.hashTime = hashTime.count();
    findCoreBreakdown.comparisonTime = queryTime.count();
    findCoreBreakdown.buildLSH = bsElapsed.count();
    findCoreBreakdown.queryTotal = queryElapsed.count();
    statistics.findCoreBreakdown = findCoreBreakdown;
    ConnectCoreBreakDown connectCoreBreakdown;
    connectCoreBreakdown.hashTime = ccHashTime.count();
    connectCoreBreakdown.queryTime = ccQueryTime.count();
    connectCoreBreakdown.groupTime = ccGroupTime.count();
    connectCoreBreakdown.buildLSH = ccbsTime.count();
    connectCoreBreakdown.queryTotal = ccQueryTotal.count();
    statistics.connectCoreBreakdown = connectCoreBreakdown;

    json statisticsJson = statistics;
    outputFile << statisticsJson.dump(4);
    outputFile.close();
}

void setParameter(ParameterFile& parameter, Dataset& dataset) {
    parameter.w = 4;
    estimateBestParameters(parameter, dataset);
}

string getOutputPath(string parameterPath, string dataPath, string suffix) {
    string outputPath = "output_";

    size_t lastindex_slash = parameterPath.find_last_of("/");
    int low = 0;
    if (lastindex_slash != string::npos) {
        low = lastindex_slash + 1;
    }
    size_t lastindex = parameterPath.find_last_of(".");
    string rawname = parameterPath.substr(low, lastindex - low);
    outputPath += rawname;
    outputPath += "_";

    size_t lastindex_data_slash = dataPath.find_last_of("/");
    int low_data = 0;
    if (lastindex_data_slash != string::npos) {
        low_data = lastindex_data_slash + 1;
    }
    size_t lastindex_data = dataPath.find_last_of(".");
    string rawname_data = dataPath.substr(low_data, lastindex_data - low_data);
    outputPath += rawname_data;

    outputPath += suffix;

    outputPath += ".output";

    cout << "Output: " << outputPath << endl;
    return outputPath;
}

string getStatisticsPath(string parameterPath, string dataPath, string suffix) {
    string statisticsPath = "statistics_";
    size_t lastindex = parameterPath.find_last_of(".");
    size_t lastindex_slash = parameterPath.find_last_of("/");
    int low = 0;
    if (lastindex_slash != string::npos) {
        low = lastindex_slash + 1;
    }
    string rawname = parameterPath.substr(low, lastindex - low);
    statisticsPath += rawname;
    statisticsPath += "_";

    size_t lastindex_data = dataPath.find_last_of(".");
    size_t lastindex_data_slash = dataPath.find_last_of("/");
    int low_data = 0;
    if (lastindex_data_slash != string::npos) {
        low_data = lastindex_data_slash + 1;
    }
    string rawname_data = dataPath.substr(low_data, lastindex_data - low_data);
    statisticsPath += rawname_data;

    statisticsPath += suffix;

    statisticsPath += ".json";

    cout << "stat: " << statisticsPath << endl;
    return statisticsPath;
}

long getPeakRSS() {
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);
    return usage.ru_maxrss;
}

int main(int argc, char* argv[]) {
    string parameterPath;
    string dataPath;
    string suffix = "";
    cout << "argc: " << argc << endl;
    if (argc == 3) {
        parameterPath = argv[1];
        dataPath = argv[2];
    } else if (argc == 4) {
        parameterPath = argv[1];
        dataPath = argv[2];
        suffix = argv[3];
    } else {
        cerr << "incorrect argument" << endl;
        cerr << "to use run './main path_to_parameter.json path_to_input_file.input '" << endl;
        cerr << "or optionally with suffix run './main path_to_parameter.json path_to_input_file.input 'suffix_str''" << endl;
        return 1;
    }

    cout << "Data: " << dataPath << endl;
    cout << "Parameter: " << parameterPath << endl;

    seedRandomEngine();
    InputFile input = readInputFile(dataPath);
    ParameterFile parameter = readParameterFile(parameterPath);

    if (parameter.minPts < 1) {
        cerr << "incorrect argument: minPts non-positive" << endl;
        return 1;
    }
    else if (parameter.minPts > input.size) {
        cerr << "incorrect argument: minPts larger than input size" << endl;
        return 1;
    }

    auto ppStart = chrono::high_resolution_clock::now();

    Dataset dataset = preprossesData(input, parameter.epsilon, parameter.minPts);

    int N = dataset.size;
    int D = dataset.dimension;
    MatrixXfRowMajor dataEigen(N, D);
    for (int i = 0; i < N; ++i) {
        const auto& coords = dataset.clusterPoints[i].coordinates;
        for (int j = 0; j < D; ++j) {
            dataEigen(i, j) = static_cast<float>(coords[j]);
        }
    }
    dataset.dataEigen = &dataEigen;

    auto ppEnd = chrono::high_resolution_clock::now();
    chrono::duration<double> ppElapsed = ppEnd - ppStart;
    cout << "Time to preprocess data: " << ppElapsed.count() << "s" << endl;

    auto spStart = chrono::high_resolution_clock::now();

    setParameter(parameter, dataset);
    cout << "parameters: " << parameter.k << " " << parameter.L << endl;

    auto spEnd = chrono::high_resolution_clock::now();
    chrono::duration<double> spElapsed = spEnd - spStart;
    cout << "Time to set parameters: " << spElapsed.count() << "s" << endl;

    // starts dbscan process
    auto fcStart = chrono::high_resolution_clock::now();

    vector<ClusterPoint*> corePoints;

    findCores(dataset, parameter, corePoints);

    auto fcEnd = chrono::high_resolution_clock::now();
    chrono::duration<double> fcElapsed = fcEnd - fcStart;
    cout << "Time to find cores: " << fcElapsed.count() << "s" << endl;

    cout << "Core points found: " << corePoints.size() << " / " << dataset.size << endl;

    DisjointSet ds = DisjointSet(dataset.size);

    auto ccStart = chrono::high_resolution_clock::now();

    // construct core matrix
    MatrixXfRowMajor coreMatrix = Eigen::MatrixXf::Zero(corePoints.size(), dataset.dimension);

    // connect points to their neighbors initially
    for (unsigned long i = 0; i < corePoints.size(); ++i) {
        ClusterPoint* point = corePoints[i];
        for (int d = 0; d < dataset.dimension; ++d) {
            coreMatrix(i, d) = static_cast<float>(point->coordinates[d]);
        }
        for (unsigned long neighbor : point->neighbors) {
            if (dataset.clusterPoints[neighbor].isCore) {
                ds.unionSets(point->index, neighbor);
            }
        }
    }

    connectCorePoints(ds, dataset, corePoints, parameter, coreMatrix);

    auto ccEnd = chrono::high_resolution_clock::now();
    chrono::duration<double> ccElapsed = ccEnd - ccStart;
    auto ccTime = ccElapsed.count() - spTime.count();
    cout << "Time to connect core points: " << ccTime << "s" << endl;

    labelPoints(corePoints, dataset, ds);

    auto endAlgo = chrono::high_resolution_clock::now();
    chrono::duration<double> elapsed = endAlgo - ppStart;
    cout << "Parameter: k " << parameter.k << " L " << parameter.L << " k_cc " << parameter.k_cc
         << " L_cc " << parameter.L_cc << " minPts " << parameter.minPts << " eps "
         << parameter.epsilon << endl;
    cout << "Total time: " << elapsed.count() << "s" << endl;
    cout << "Input size: " << dataset.size << endl;
    cout << "Core points: " << corePoints.size() << endl;

    string outputPath = getOutputPath(parameterPath, dataPath, suffix);
    string statsPath = getStatisticsPath(parameterPath, dataPath, suffix);

    outputToFile(dataset, outputPath);
    outputStatistics(statsPath, elapsed.count(), ccTime, fcElapsed.count(),
                     spElapsed.count() + spTime.count(), outputPath, corePoints.size(),
                     dataset.size, parameter, getPeakRSS());

    return 0;
}
