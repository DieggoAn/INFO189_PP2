CXX = g++
CXXFLAGS = -std=c++17

backend: Backend/llamarBuscador.cpp
	$(CXX) $(CXXFLAGS) -o Backend/backend Backend/llamarBuscador.cpp

searcher: Frontend/frontend.cpp
	$(CXX) $(CXXFLAGS) -o Frontend/searcher Frontend/frontend.cpp

memcache: Cache/cache.cpp
	$(CXX) $(CXXFLAGS) -o Cache/memcache Cache/cache.cpp


all: backend searcher memcache


.PHONY: all clean
