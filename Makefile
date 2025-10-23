NAME := yancc

CXX :=g++
CXXFLAGS := -Wall -Wextra -Isrc/include
LDFLAGS := -lb15fdrv

TARGET := build/bin/$(NAME)
SRC := $(wildcard src/*.cpp)
OBJ := $(SRC:src/%.cpp=build/%.o)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(OBJ) -o $(TARGET) $(LDFLAGS)

build/%.o: src/%.cpp
	@mkdir -p build/bin
	$(CXX) $(CXXFLAGS) $< -c -o $@

clean:
	rm -f $(OBJ) $(TARGET)
	rmdir -p build/bin

run: $(TARGET)
	$(TARGET)