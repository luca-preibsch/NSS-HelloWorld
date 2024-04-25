CXX = clang++
CXXFLAGS = -std=c++11 -g # -Wall

# directory
LIBDIR = ../dist/Debug/lib
LNKFLAGS = -L$(LIBDIR) -lnss3 -lnspr4 -lssl3 -lplc4 # list all used libraries (-l<libraryname> without lib prefix and file ending>)

.PHONY: all clean run-server run-client

all: server client

clean:
	rm -f src/*.o client server

run-server: server
	export DYLD_LIBRARY_PATH=$(LIBDIR):$$DYLD_LIBRARY_PATH && ./server

run-client: client
	export DYLD_LIBRARY_PATH=$(LIBDIR):$$DYLD_LIBRARY_PATH && ./client

server: src/server.o
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LNKFLAGS)

client: src/client.o
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LNKFLAGS)

src/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@ -I include
