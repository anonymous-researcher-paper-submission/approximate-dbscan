CC = g++

# BUILD_TYPE = RELEASE
# BUILD_TYPE = DEBUG
BUILD_TYPE = DEPLOY
# BUILD_TYPE = PROFILE
# BUILD_TYPE = INSTRUCTIONS

# Run DEPLOY or RELEASE for benchmarking, other modes for debug only

ifeq ($(BUILD_TYPE), DEBUG)
    CFLAGS = -g -O0 -Wall
else ifeq ($(BUILD_TYPE), DEPLOY)
    CFLAGS = -O3 -DNDEBUG -static -march=native -DEIGEN_NO_DEBUG
else ifeq ($(BUILD_TYPE), PROFILE)
    CFLAGS = -O3 -march=native -g
else ifeq ($(BUILD_TYPE), INSTRUCTIONS)
    CFLAGS = -O3 -march=native -pg
else
    CFLAGS = -O3 -DNDEBUG -march=native -DEIGEN_NO_DEBUG
endif

SRC = $(wildcard src/*.cpp)
TARGET = target/main

# Note to reader, please install eigen3 (for math) and nlohmann/json (for parameter file and statistics); 
# linked to path pointed by VCPKG_ROOT
# Path to vcpkg
VCPKG_ROOT = /home/user/vcpkg
VCPKG_TRIPLET = x64-linux

VCPKG_INCLUDE = $(VCPKG_ROOT)/installed/$(VCPKG_TRIPLET)/include
VCPKG_LIB = $(VCPKG_ROOT)/installed/$(VCPKG_TRIPLET)/lib

CFLAGS += -I$(VCPKG_INCLUDE)
CFLAGS += -I$(VCPKG_INCLUDE)/eigen3
LDFLAGS += -L$(VCPKG_LIB)

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)