TARGET = main.exe
CXX = g++
CXXFLAGS = -Wall -std=c++17
SRC = main.cpp
HDR = main.hpp

all: clean $(TARGET)
	./$(TARGET)

$(TARGET): $(SRC) $(HDR)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

clean:
	rm -f $(TARGET)
