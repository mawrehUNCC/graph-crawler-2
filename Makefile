CXX = g++
CXXFLAGS = -std=c++17 -Wall -O2
LIBS = -lcurl
INCLUDES = -I./include/rapidjson/include  # Point to the correct location for RapidJSON

SRC = parallel_bfs.cpp
OUT = parallel_bfs

# Rule to build the final executable
all: $(OUT)

$(OUT): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(OUT) $(LIBS) $(INCLUDES)

# Clean up generated files
clean:
	rm -f $(OUT)
