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
)

set(explodus_source
	Bot.cc 
	MyBot.cc 
	State.cc 
	scn/scnLayer.cc 
	scn/scnNode.cc 
	scn/scnANN.cc 
)

add_executable (explodus ${explodus_source} ${explodus_header}) 

# Link the executable to the Hello library. 
# target_link_libraries (explodus) 