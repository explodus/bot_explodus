cmake_minimum_required (VERSION 2.6) 
project (explodus) 

add_executable (explodus Bot.cc MyBot.cc State.cc) 

# Link the executable to the Hello library. 
#target_link_libraries (explodus ) 