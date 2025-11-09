TARGET = main.exe
CXX = g++
CXXFLAGS = -Wall -std=c++17
SRC = main.cpp

all: clean 
	git stash
	$(TARGET)
	./$(TARGET)
	git stash pop

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

clean:
	rm -f $(TARGET)
