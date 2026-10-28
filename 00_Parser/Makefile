CXX = g++
CXXFLAGS = -Wall

SRC = settings.cpp main.cpp

all: debug release

debug:
	$(CXX) $(SRC) $(CXXFLAGS) -g -o test2

release:
	$(CXX) $(SRC) $(CXXFLAGS) -Ofast -DNDEBUG -o test2_release

clean:
	rm test2 test2_release