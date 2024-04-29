CXX = clang++
CXXFLAGS = -std=c++11 -g # -Wall

# directory
LIBDIR = ../dist/Debug/lib

# list all used libraries (-l<libraryname> without lib prefix and file ending>)
LNKFLAGS = -L$(LIBDIR) -lnss3 -lnspr4 -lssl3 -lplc4		# for dynamic linking
# LNKFLAGS = -L$(LIBDIR) -lnss3 -lnspr4 -lssl3 -lplc4	# for static linking
# Notice: I would prefer static linking, though I was not able to find the correct static libraries so far.

.PHONY: all clean run-server run-client

all: server client

clean:
	rm -f src/*.o client server

run-server: server	# DYLD_LIBRARY_PATH only works for MacOS, has to be changed to LD_LIBRARY_PATH for Linux
	export DYLD_LIBRARY_PATH=$(LIBDIR):$$DYLD_LIBRARY_PATH && ./server		# for dynamic linking

run-client: client	# DYLD_LIBRARY_PATH only works for MacOS, has to be changed to LD_LIBRARY_PATH for Linux
	export DYLD_LIBRARY_PATH=$(LIBDIR):$$DYLD_LIBRARY_PATH && ./client		# for dynamic linking

server: src/server.o src/helpers.o
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LNKFLAGS)

client: src/client.o src/helpers.o
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LNKFLAGS)

src/%.o: src/%.cpp src/helpers.h
	$(CXX) $(CXXFLAGS) -c $< -o $@ -I include
