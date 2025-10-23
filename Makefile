CXX :=g++
CXXFLAGS := -Wall -Wextra -Isrc/include
LDFLAGS := -lb15fdrv

TARGET := build/bin/yancc
SRC := $(wildcard src/*.cpp)
OBJ := $(SRC:src/%.cpp=build/%.o)

all: $(TARGET)

$(TARGET): $(OBJ)
	mkdir -p $(dir $(TARGET))
	$(CXX) $(OBJ) -o $(TARGET) $(LDFLAGS)

build/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) $< -c -o $@
	
clean:
	rm -f $(OBJ) $(TARGET)

run: $(TARGET)
	$(TARGET)