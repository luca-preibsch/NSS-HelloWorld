CXX = clang++
CXXFLAGS = -std=c++11 # -Wall

LIBDIR = /Users/luca/Dev/FAU/FAU_Proj/nss/dist/Debug/lib
LNKFLAGS = -L$(LIBDIR) -lnss3 -lnspr4 -lssl3 # list all used libraries (-l<libraryname> without lib prefix and file ending>)

.PHONY: all clean run-server run-client

all: server client

clean:
	rm -f src/client/*.o client src/server/*.o server

run-server: server
	export DYLD_LIBRARY_PATH=$(LIBDIR):$$DYLD_LIBRARY_PATH && ./server

run-client: client
	export DYLD_LIBRARY_PATH=$(LIBDIR):$$DYLD_LIBRARY_PATH && ./client

server: src/server/server.o
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LNKFLAGS)

client: src/client/client.o
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LNKFLAGS)

src/server/%.o: src/server/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

src/client/%.o: src/client/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@
