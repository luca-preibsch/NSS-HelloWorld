CXX = clang++
CXXFLAGS = -std=c++11 -Wall

LIBDIR = ../dist/Debug/lib

.PHONY: all clean run-server run-client

all: server client

clean:
	rm -f src/client/*.o client src/server/*.o server

run-server: server
	export LD_LIBRARY_PATH=$(LIBDIR):$$LD_LIBRARY_PATH && ./server

run-client: client
	export LD_LIBRARY_PATH=$(LIBDIR):$$LD_LIBRARY_PATH && ./client

server: src/server/server.o
	$(CXX) $(CXXFLAGS) $^ -o $@

client: src/client/client.o
	$(CXX) $(CXXFLAGS) $^ -o $@

src/server/%.o: src/server/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

src/client/%.o: src/client/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@
