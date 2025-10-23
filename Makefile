CXX :=g++
CXXFLAGS := -Wall -Wextra
LDFLAGS := -lb15fdrv

TARGET := yancc
SRC := main.cpp
OBJ := $(SRC:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(OBJ) -o $(TARGET) $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $< -c -o $@
	

clean:
	rm -f $(OBJ) $(TARGET)