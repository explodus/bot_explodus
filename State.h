#ifndef STATE_H_
#define STATE_H_

#include <iostream>
#include <stdio.h>
#include <cstdlib>
#include <cmath>
#include <string>
#include <vector>
#include <queue>
#include <stack>
#include <list>

#include "Timer.h"
#include "Bug.h"
#include "Square.h"
#include "Location.h"

/*
    constants
*/
enum TDIRECTIONS
{
    e_north = 0
  , e_east = 1
  , e_south = 2 
  , e_west = 3
  , TDIRECTIONS_SIZE = 4
};
const char CDIRECTIONS[4] = {'N', 'E', 'S', 'W'};
//{N, E, S, W}
const int DIRECTIONS[4][2] = { {-1, 0}, {0, 1}, {1, 0}, {0, -1} };      

struct moves
{
	Location * from;
	Location * to;
	TDIRECTIONS d;
	int ant;
};
typedef std::list<moves> t_moves;

/*
    struct to store current state information
*/
struct State
{
    /*
        Variables
    */
    int rows, cols, turn, turns, noPlayers;
    double attackradius, spawnradius, viewradius;
    double loadtime, turntime;
    std::vector<double> scores;
    bool gameover;
		t_moves _moves;

    std::vector<std::vector<Square> > grid;
    std::vector<Location*> 
        myAnts
      , enemyAnts
      , myHills
      , enemyHills
      , food;

    Timer timer;
    Bug bug;

    /*
        Functions
    */
    State();
    ~State();

    void setup();
    void reset();

    void makeMoves(Location &loc, Location &dest, TDIRECTIONS direction);
		void endMoves();

    double distance(const Location &loc1, const Location &loc2);
		/// returns the new location from moving in a 
		/// given direction with the edges wrapped
		//inline Location* State::getLocation(const Location &loc, int direction)
		//{
		//	return &grid
		//		[((loc.row + DIRECTIONS[direction][0] + rows) % rows)]
		//	[((loc.col + DIRECTIONS[direction][1] + cols) % cols)].loc;
		//}
		inline Location* getLocation(const Location &startLoc, int direction)
		{ return startLoc.around[direction]; }

    TDIRECTIONS get_direction(const Location &start, const Location &dest);

    void updateVisionInformation();
};

std::ostream& operator<<(std::ostream &os, const State &state);
std::istream& operator>>(std::istream &is, State &state);

#endif //STATE_H_
