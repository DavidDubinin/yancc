NAME := yancc

CXX :=g++
CXXFLAGS := -Wall -Wextra -Isrc/include
LDFLAGS := -lb15fdrv

FAKE_CXXFLAGS := -std=c++11 -Wall -Isrc/include
FAKE_LDFLAGS := -pthread
FAKE_TARGET := build/bin/$(NAME)_fake15

TARGET := build/bin/$(NAME)
SRC := $(wildcard src/*.cpp)
OBJ := $(SRC:src/%.cpp=build/%.o)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(OBJ) -o $(TARGET) $(LDFLAGS)

$(FAKE_TARGET): $(OBJ)
	$(CXX) $(OBJ) -o $(FAKE_TARGET) $(FAKE_LDFLAGS)

build/%.o: src/%.cpp
	@mkdir -p build/bin
	$(CXX) $(FAKE_CXXFLAGS) $< -c -o $@

clean:
	rm -f $(OBJ) $(TARGET) $(FAKE_TARGET)
	rmdir -p build/bin

run: $(TARGET)
	$(TARGET)

fake: $(FAKE_TARGET)
	$(FAKE_TARGET)
