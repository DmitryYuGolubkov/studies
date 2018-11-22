CXX=/opt/gcc/gcc-4.9.4/bin/g++
CXXFLAGS="-std=c++11"

targets=lock_free_stack_v7.9.o

all: $(targets)

%.o: %.cpp 
	$(CXX) $< -c $(CXXFLAGS) -o $@

