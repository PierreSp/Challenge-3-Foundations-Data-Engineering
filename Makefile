
CPPFLAGS = -std=c++11 -pthread
OPTFLAGS = -O3
DEBUGFLAGS = -g

all: my.cpp
	g++ $(CPPFLAGS) $(OPTFLAGS) my.cpp
debug: my.cpp
	g++ $(CPPFLAGS) $(DEBUGFLAGS) my.cpp
profile: my.cpp
	g++ $(CPPFLAGS) -pg my.cpp
