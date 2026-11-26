CXX      ?= c++
CXXFLAGS ?= -std=c++17 -O3 -Xpreprocessor -fopenmp -I$(CONDA_PREFIX)/include -Wall -Wextra
LDFLAGS  ?= $(CONDA_PREFIX)/lib/libomp.dylib -Wl,-rpath,$(CONDA_PREFIX)/lib
SRC      := main.cpp csv_reader.cpp nearest_furthest.cpp rng.cpp

nearest_furthest: $(SRC) $(wildcard *.h)
	$(CXX) $(CXXFLAGS) $(SRC) -o $@ $(LDFLAGS)

clean:
	rm -f nearest_furthest nearest.csv furthest.csv