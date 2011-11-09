cmake_minimum_required (VERSION 2.6) 
project (explodus) 

set(explodus_header
	Bot.h 
	Bug.h 
	Location.h 
	Square.h 
	State.h 
	Timer.h 
	scn/scnANN.h 
	scn/scnLayer.h 
	scn/scnNode.h
	pathfind/astar.h
	pathfind/fringesearch.h
	pathfind/tiling.h
	pathfind/graph.h
	pathfind/idastar.h
	pathfind/marker.h
	pathfind/search.h
	pathfind/environment.h
	pathfind/statistics.h
	pathfind/util.h
	pathfind/searchutils.h
	pathfind/error.h
	pathfind/pathfind.h
	pathfind/version.h
)

set(explodus_source
	Bot.cc 
	MyBot.cc 
	State.cc 
	scn/scnLayer.cc 
	scn/scnNode.cc 
	scn/scnANN.cc 
	pathfind/idastar.cpp
	pathfind/tiling.cpp
	pathfind/statistics.cpp
	pathfind/astar.cpp
	pathfind/searchutils.cpp
	pathfind/fringesearch.cpp
	pathfind/util.cpp
	pathfind/environment.cpp
	pathfind/search.cpp
	pathfind/error.cpp
	pathfind/version.cpp
)

add_executable (explodus ${explodus_source} ${explodus_header}) 

# Link the executable to the Hello library. 
# target_link_libraries (explodus) 