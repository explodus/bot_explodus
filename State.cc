#include "State.h"
#include <cmath>

using namespace std;

//constructor
State::State()
{
  gameover = 0;
  turn = 0;
  seenHill = 0;
  bug.open("debug.txt");
};

//deconstructor
State::~State()
{
  bug.close();
};

//sets the state up
void State::setup()
{
  grid = vector<vector<Square> >(rows, vector<Square>(cols, Square()));
  for(int row(0); row<rows; ++row)
    for(int col(0); col<cols; ++col)
      grid[row][col].loc = Location(row, col);

	for(int row(0); row<rows; ++row)
	{
		for(int col(0); col<cols; ++col)
		{
			if (row == 0)
				grid[row][col].loc.around[e_north] = &grid[rows-1][col].loc;
			else
				grid[row][col].loc.around[e_north] = &grid[row-1][col].loc;

			if (row == rows-1)
				grid[row][col].loc.around[e_south] = &grid[0][col].loc;
			else
				grid[row][col].loc.around[e_south] = &grid[row+1][col].loc;

			if (col == 0)
				grid[row][col].loc.around[e_west] = &grid[row][cols-1].loc;
			else
				grid[row][col].loc.around[e_west] = &grid[row][col-1].loc;

			if (col == cols-1)
				grid[row][col].loc.around[e_east] = &grid[row][0].loc;
			else
				grid[row][col].loc.around[e_east] = &grid[row][col+1].loc;
		}
	}
};

//resets all non-water squares to land and clears the bots ant vector
void State::reset()
{
  myAnts.clear();
  enemyAnts.clear();
  myHills.clear();
  enemyHills.clear();
  food.clear();

  int size = rows*cols;

  myAnts.reserve(size);
  enemyAnts.reserve(size);
  myHills.reserve(size);
  enemyHills.reserve(size);
  food.reserve(size);

  for(int row=0; row<rows; ++row)
    for(int col=0; col<cols; ++col)
      if(!grid[row][col].loc.isWater)
        grid[row][col].reset();
};

//outputs move information to the engine
void State::makeMoves(Location &loc, Location &dest, TDIRECTIONS direction)
{
  bug << "make move from " << loc << " to " << dest << endl;;
  moves m;
  m.from = &loc;
  m.to = &dest;
  m.ant = loc.ant;
  m.d = direction;
  m.to->ant = m.from->ant;
  m.from->ant = -1;
  _moves.push_back(m);
}

void State::endMoves()
{
  for (t_moves::iterator 
		  itb(_moves.begin())
		, ite(_moves.end())
		; itb != ite
		; ++itb)
    itb->from->ant = -1;

  for (t_moves::iterator 
		  itb(_moves.begin())
		, ite(_moves.end())
		; itb != ite
		; ++itb)
  {
    itb->to->ant = itb->ant;
    cout 
      << "o " 
      << itb->from->row 
      << " " 
      << itb->from->col 
      << " " 
      << CDIRECTIONS[itb->d] 
    << endl;
		bug << "make move real from " << *itb->from << " to " << *itb->to << endl;
		//++itb->from->pheromone;
		if(itb->to == seenHill)
			seenHill = 0;
  }
  _moves.clear();
}

/*
This function will update update the lastSeen value for any squares currently
visible by one of your live ants.

BE VERY CAREFUL IF YOU ARE GOING TO TRY AND MAKE THIS FUNCTION MORE EFFICIENT,
THE OBVIOUS WAY OF TRYING TO IMPROVE IT BREAKS USING THE EUCLIDEAN METRIC, FOR
A CORRECT MORE EFFICIENT IMPLEMENTATION, TAKE A LOOK AT THE GET_VISION FUNCTION
IN ANTS.PY ON THE CONTESTS GITHUB PAGE.
*/
void State::updateVisionInformation()
{
  std::queue<Location*> locQueue;
  Location *sLoc(0), *cLoc(0), *nLoc(0);

  for(int a=0; a<(int) myAnts.size(); a++)
  {
    sLoc = myAnts[a];
    locQueue.push(sLoc);

    std::vector<std::vector<int> > visited(rows, std::vector<int>(cols, 0));
    sLoc->isVisible = 1;
    visited[sLoc->row][sLoc->col] = 1;

    while(!locQueue.empty())
    {
      cLoc = locQueue.front();
      locQueue.pop();

      for(int d=0; d<TDIRECTIONS_SIZE; d++)
      {
        nLoc = getLocation(*cLoc, d);

        if(!visited[nLoc->row][nLoc->col] 
        && distance(*sLoc, *nLoc) <= viewradius)
        {
          nLoc->isVisible = 1;
          locQueue.push(nLoc);
        }
        visited[nLoc->row][nLoc->col] = 1;
      }
    }
  }
}

TDIRECTIONS State::get_direction( const Location &start, const Location &dest )
{
  TDIRECTIONS ret(e_north);

  int row1 = start.row % rows;
  int row2 = dest.row % rows;
  int col1 = start.col % cols;
  int col2 = dest.col % cols;

  if (row1 != row2)
  {
    int half_rows(rows/2);
    if (row1 < row2)
    {
      if ((row2 - row1) >= half_rows)
        return e_north;
      if ((row2 - row1) <= half_rows)
        return e_south;
    }
    if (row2 < row1)
    {
      if ((row1 - row2) >= half_rows)
        return e_south;
      if ((row1 - row2) <= half_rows)
        return e_north;
    }
  }
  if (col1 != col2)
  {
    int half_cols(cols/2);
    if (col1 < col2)
    {
      if ((col2 - col1) >= half_cols)
        return e_west;
      if ((col2 - col1) <= half_cols)
        return e_east;
    }
    if (col2 < col1)
    {
      if ((col1 - col2) >= half_cols)
        return e_east;
      if ((col1 - col2) <= half_cols)
        return e_west;
    }
  }

  return ret;
}


/*
This is the output function for a state. It will add a char map
representation of the state to the output stream passed to it.

For example, you might call "cout << state << endl;"
*/
ostream& operator<<(ostream &os, const State &state)
{
  for(int row=0; row<state.rows; row++)
  {
    for(int col=0; col<state.cols; col++)
    {
      if(state.grid[row][col].loc.isWater)
        os << '%';
      else if(state.grid[row][col].loc.isFood)
        os << '*';
      else if(state.grid[row][col].loc.isHill)
        os << (char)('A' + state.grid[row][col].loc.hillPlayer);
      else if(state.grid[row][col].loc.ant >= 0)
        os << (char)('a' + state.grid[row][col].loc.ant);
      else if(state.grid[row][col].loc.isVisible)
        os << '.';
      else
        os << '?';
    }
    os << endl;
  }

  return os;
};

void costs_around( State &state, int row, int col, int new_costs );


//input function
istream& operator>>(istream &is, State &state)
{
  int row, col, player;
  string inputType, junk;

  //finds out which turn it is
  while(is >> inputType)
  {
    if(inputType == "end")
    {
      state.gameover = 1;
      break;
    }
    else if(inputType == "turn")
    {
      is >> state.turn;
      break;
    }
    else //unknown line
      getline(is, junk);
  }

  if(state.turn == 0)
  {
    //reads game parameters
    while(is >> inputType)
    {
      if(inputType == "loadtime")
        is >> state.loadtime;
      else if(inputType == "turntime")
        is >> state.turntime;
      else if(inputType == "rows")
        is >> state.rows;
      else if(inputType == "cols")
        is >> state.cols;
      else if(inputType == "turns")
        is >> state.turns;
      else if(inputType == "viewradius2")
      {
        is >> state.viewradius;
        state.viewradius = sqrt(state.viewradius);
      }
      else if(inputType == "attackradius2")
      {
        is >> state.attackradius;
        state.attackradius = sqrt(state.attackradius);
      }
      else if(inputType == "spawnradius2")
      {
        is >> state.spawnradius;
        state.spawnradius = sqrt(state.spawnradius);
      }
      else if(inputType == "ready") //end of parameter input
      {
        state.timer.start();
        break;
      }
      else    //unknown line
        getline(is, junk);
    }
  }
  else
  {
    //reads information about the current turn
    while(is >> inputType)
    {
      if(inputType == "w") //water square
      {
        is >> row >> col;
        state.grid[row][col].loc.isWater = 1;
        state.grid[row][col].loc.weight = 999;
      }
      else if(inputType == "f") //food square
      {
        is >> row >> col;
        state.grid[row][col].loc.isFood = 1;
        state.food.push_back(&state.grid[row][col].loc);
        costs_around(state, row, col, -40);
      }
      else if(inputType == "a") //live ant square
      {
        is >> row >> col >> player;
        state.grid[row][col].loc.ant = player;
        if(player == 0)
        {
          state.myAnts.push_back(&state.grid[row][col].loc);
          costs_around(state, row, col, 1);
        }
        else
        {
          state.enemyAnts.push_back(&state.grid[row][col].loc);
          costs_around(state, row, col, 15);
        }
      }
      else if(inputType == "d") //dead ant square
      {
        is >> row >> col >> state.grid[row][col].loc.ant;
        state.grid[row][col].loc.isDead = 1;
        state.grid[row][col].loc.isFood = 1;
        //state.food.push_back(&state.grid[row][col].loc);
        costs_around(state, row, col, -5);
      }
      else if(inputType == "h")
      {
        is >> row >> col >> player;
        state.grid[row][col].loc.isHill = 1;
        state.grid[row][col].loc.hillPlayer = player;
        if(player == 0)
        {
          state.myHills.push_back(&state.grid[row][col].loc);
          costs_around(state, row, col, 20);
        }
        else
        {
          state.enemyHills.push_back(&state.grid[row][col].loc);
          costs_around(state, row, col, -40);
					if(state.seenHill==0)
						state.seenHill = &state.grid[row][col].loc;
        }
      }
      else if(inputType == "players") //player information
        is >> state.noPlayers;
      else if(inputType == "scores") //score information
      {
        state.scores = vector<double>(state.noPlayers, 0.0);
        for(int p=0; p<state.noPlayers; p++)
          is >> state.scores[p];
      }
      else if(inputType == "go") //end of turn input
      {
        if(state.gameover)
          is.setstate(std::ios::failbit);
        else
          state.timer.start();
        break;
      }
      else //unknown line
        getline(is, junk);
    }
  }

  return is;
};

void costs_around( State &state, int row, int col, int new_costs )
{
	Location& l(state.grid[row][col].loc);
	if (!l.around[e_north]->isWater)
		l.around[e_north]->cost = new_costs;
	if (!l.around[e_east]->isWater)
		l.around[e_east]->cost = new_costs;
	if (!l.around[e_south]->isWater)
		l.around[e_south]->cost = new_costs;
	if (!l.around[e_west]->isWater)
		l.around[e_west]->cost = new_costs;

	//for(int d(e_north); d<TDIRECTIONS_SIZE; ++d)
	//{ // expand vertex
	//  Location *loc(state.getLocation(
	//      state.grid[row][col].loc
	//    , static_cast<TDIRECTIONS>(d)));
	//  if (!loc->isWater)
	//    loc->cost = new_costs;
	//}
}
