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
#include <xutility>

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

const int COST_ONE = 100;
const int COST_SQRT2 = 142; 

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
	t_location_vector
		  myAnts
		, enemyAnts
		, myHills
		, enemyHills
		, food;

	Location * seenHill;

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

	inline int getHeuristic(const Location & start, const Location & target) const
	{
		int colStart = start.col % cols;
		int colTarget = target.col % cols;
		int rowStart = start.row / rows;
		int rowTarget = target.row / rows;
		int diffCol = abs(colTarget - colStart);
		int diffRow = abs(rowTarget - rowStart);
		// Vancouver distance
		// See P.Yap: Grid-based Path-Finding (LNAI 2338 pp.44-55)
		{
			int correction = 0;
			if (diffCol % 2 != 0)
			{
				if (rowTarget < rowStart)
					correction = colTarget % 2;
				else if (rowTarget > rowStart)
					correction = colStart % 2;
			}
			// Note: formula in paper is wrong, corrected below.  
			int dist = std::max(0, diffRow - diffCol / 2 - correction) + diffCol;
			return dist * COST_ONE;
		}
		return 0;
	} 
	/// returns the euclidean distance between two locations 
	/// with the edges wrapped
	inline double distance(const Location &loc1, const Location &loc2) const
	{ 
		int d1 = abs(loc1.row-loc2.row),
			d2 = abs(loc1.col-loc2.col),
			dr = std::min(d1, rows-d1),
			dc = std::min(d2, cols-d2);
		return sqrt(static_cast<double>(dr*dr + dc*dc));
	}

	inline int manhattan_method(const Location & l1, const Location & l2) const
	{ return 2 * (abs(l1.row - l2.row) + abs(l1.col - l2.col)); }
	//inline int manhattan_method(const Location & l, const Location & dest)
	//{ 
	//	static int D(2)
	//		, D2(static_cast<int>(sqrt(2.0)*static_cast<double>(D)));
	//	int h_d = std::min(std::abs(l.col-dest.col), std::abs(l.row-dest.row));
	//	int h_s = (std::abs(l.col-dest.col) + std::abs(l.row-dest.row));
	//	return (D2 * h_d) + (D * (h_s - 2*h_d));
	//}

	/// returns the distance between two locations 
	/// with the edges wrapped
	//inline int manhattan_method(const Location & loc1, const Location & loc2)
	//{ 
	//	int d1 = abs(loc1.row-loc2.row),
	//		d2 = abs(loc1.col-loc2.col),
	//		dr = std::min(d1, rows-d1),
	//		dc = std::min(d2, cols-d2);
	//	return static_cast<int>(sqrt(static_cast<double>(dr*dr + dc*dc))*10.0);
	//}

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
