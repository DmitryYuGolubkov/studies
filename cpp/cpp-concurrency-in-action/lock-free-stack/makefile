CXX=/opt/gcc/gcc-4.9.4/bin/g++
CXXFLAGS="-std=c++11"

sources=$(wildcard *.cpp)
targets=$(patsubst %.cpp, %.o, $(sources))

all: $(targets)

%.o: %.cpp 
	$(CXX) $< -c $(CXXFLAGS) -o $@

