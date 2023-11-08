CXX = g++
CXXFLAGS = -std=c++17

invertedIndex: programas/invertedIndex.cpp
	$(CXX) $(CXXFLAGS) -o ejecutables/invertedIndex programas/invertedIndex.cpp

backend: programas/llamarBuscador.cpp
	$(CXX) $(CXXFLAGS) -o ejecutables/backend programas/llamarBuscador.cpp

searcher: programas/frontend.cpp
	$(CXX) $(CXXFLAGS) -o ejecutables/searcher programas/frontend.cpp

memcache: programas/cache.cpp
	$(CXX) $(CXXFLAGS) -o ejecutables/memcache programas/cache.cpp


all: invertedIndex backend searcher memcache

clean:
	rm -f ejecutables/*

.PHONY: all clean
