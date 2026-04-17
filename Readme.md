The source code is for the paper "Approximate High-Dimensional DBSCAN with Linear Space". 

## Project Set up

To build the project, please make sure to install eigen3 and nlohmann/json packages. Then include the path to these packages in MakeFile. Then to build the project simply run make. 

Before building, make sure to set BUILD_TYPE = DEPLOY or BUILD_TYPE = RELEASE, for benchmarking. 

Please create folder "target/", for output executive file, or change TARGET to a desired output location. 

A pre-built binary version is included in target/main for Linux with all dependencies included. Included test parameter.json file and data.input file for reader. 

## Running the program

Assuming the project is built in target/main, run: 

```
./target/main "parameter.json" "data.input"
```

For path to parameter.json file and path to data.input data input file. 

Optionally, append "_suffix" if suffix is needed. 

## Parameter File

A typical parameter.json file looks like:
```
{"minPts": 40, "epsilon": 5.778, "rho": 1}
```

Fields: 

| Field | Description |
| -------- | -------- |
| minPts    | a positive integer minPts in paper   |
| epsilon    | a positive real value $\epsilon$ in paper   |
| rho (Optional)   | Not inputting the value and the result will be c = 1, exact DBSCAN. rho = c - 1 for approximation factor c in c-approximate DBSCAN, e.g. for c = 2, take rho = 1 |
| successRate (Optional)    | by default this is $1 - 1/n$, if entered, use inputed value |

## Input File

No header, only input coordinates. Each row contains d floating number in string. n rows for input of size n. An example input file of n = 3, and d = 2 is shown. 

```
16.4 -12.1
15.3 10.53
13.4 10.56
```
