cmake_minimum_required (VERSION 2.6) 
project (explodus) 

add_executable (explodus Bot.cc MyBot.cc State.cc Bot.h Bug.h Location.h Square.h State.h Timer.h) 

# Link the executable to the Hello library. 
target_link_libraries (explodus) 