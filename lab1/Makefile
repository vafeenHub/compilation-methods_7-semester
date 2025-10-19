TARGET = lexan.exe
CXX = g++
CXXFLAGS = -Wall -std=c++17
SRC = lexan.cpp

all: clean $(TARGET)
	./$(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

clean:
	rm -f $(TARGET)
