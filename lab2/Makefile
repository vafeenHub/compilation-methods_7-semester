TARGET = main.exe
CXX = g++
CXXFLAGS = -Wall -std=c++17
SRC = main.cpp

all: clean stash build run unstash

stash:
	git stash -u

unstash:
	git stash pop

build:
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

run:
	./$(TARGET)

clean:
	rm -f $(TARGET)
