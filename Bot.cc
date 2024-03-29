#include "Bot.h"
#include <limits>
#include <algorithm>
#include <vector>
#include <map>
#include <set>
#include <functional>
#include <ios>

#include <assert.h>

#include "scn/scnANN.h"
#include "pathfind/tiling.h"
#include "pathfind/astar.h"

using namespace std;

int calc::Path::astar_obreak = 100;
int calc::Path::astar_cbreak = 256;

static const int s_numberRuns = 100;
static const int s_columns = 50;
static const int s_rows = 50;
static const float s_obstaclePercentage = 0.2;
static const long long int s_nodesLimit = 100000000L; 

const int NUM_INPUTS = 72;
const int NUM_OUTPUTS = 1;
const int NUM_HIDDEN_LAYERS = 1;
const int LAYER_SIZES[NUM_HIDDEN_LAYERS] = {32};
const double HIGH_RANDOM_LIMIT = 0.1;
const double LOW_RANDOM_LIMIT = -0.1;
const double LEARNING_RATE = 0.5;
const double MOMENTUM_COEFFICIENT = 0.0; 

double input[NUM_INPUTS];
double idealOutput[NUM_OUTPUTS]; 

scnANN Network(
	  NUM_INPUTS
	, NUM_OUTPUTS
	, NUM_HIDDEN_LAYERS
	, LAYER_SIZES
	, HIGH_RANDOM_LIMIT
	, LOW_RANDOM_LIMIT
	, LEARNING_RATE
	, MOMENTUM_COEFFICIENT);

namespace 
{
  class find_by_loc
  {
    const Location * _loc;
		bool _destination;
  public:
    find_by_loc(
			  const Location * l
			, bool destination = false) 
			: _loc(l)
			, _destination(destination)
		{}
    inline bool operator()(const calc::Path& t_) const
    { 
			if (_destination && t_.dest)
				return t_.dest==_loc; 
			else
				return t_.start==_loc && t_.turn_visited == false; 
		}
		inline bool operator()(const Location& t_) const
		{ return t_==*_loc; }
		inline bool operator()(const Location* t_) const
		{ return *t_==*_loc; }
  };

  template<typename Type, typename Compare = std::less<Type> >
  struct pless : public std::binary_function<Type *, Type *, bool> 
  {
    bool operator()(const Type *x, const Type *y) const
    { return Compare()(*x, *y); }
  };

	bool readOnePair(istream& in, double* input, double* idealOutput)
	{
		bool retVal = !in.eof();
		if(retVal)
		{
			for(int j = 0; j < NUM_INPUTS; j++)
				in >> input[j];
			for(int j = 0; j < NUM_OUTPUTS; j++)
				in >> idealOutput[j];
		}
		return retVal;
	} 

	inline bool sort_food(const Location * lhs, const Location * rhs)
	{ return lhs->dist_food < rhs->dist_food; }
	inline bool sort_hill(const Location * lhs, const Location * rhs)
	{ return lhs->dist_hill < rhs->dist_hill; }
	inline bool sort_enemy(const Location * lhs, const Location * rhs)
	{ return lhs->dist_enemy < rhs->dist_enemy; }
	inline bool sort_ant(const Location * lhs, const Location * rhs)
	{ return lhs->dist_ant < rhs->dist_ant; }
}

//	inline long long DFS(
//		  State & state
//		, long long start_cost
//		, Location & node
//		, Location & dest
//		, const long long & cost_limit
//		, t_location_vector & path_so_far
//		, t_location_deque & solution)
//	{
//#ifdef DEBUG
//		state.bug 
//			<< "DFS( " 
//			<< start_cost 
//			<< ", " 
//			<< node 
//			<< ", " 
//			<< dest 
//			<< ", " 
//			<< cost_limit 
//			<< ", " 
//			<< path_so_far.size() 
//			<< " )" 
//			<< std::endl;
//#endif // DEBUG
//
//		long long minimum_cost = 
//			  start_cost 
//			+ node.weightcosts() 
//			+ state.manhattan_method(dest, node);
//
//		if (minimum_cost > cost_limit)
//			return minimum_cost;
//
//		if (node == dest)
//		{
//			Location *v(path_so_far.back());
//			while(v->prev)
//			{
//				solution.push_front(v);
//				v = v->prev;
//			}
//			return cost_limit;
//		}
//
//		long long next_cost_limit = minimum_cost;
//
//	  for(int d(e_north); d<TDIRECTIONS_SIZE; ++d)
//	  { // expand vertex
//		  Location &loc(*state.getLocation(node, static_cast<TDIRECTIONS>(d)));
//
//			if (loc.isWater || loc.ant != -1)
//				continue;
//
//			long long new_start_cost = 
//				  start_cost 
//				+ loc.weightcosts()
//				+ state.manhattan_method(dest, loc);
//			
//			loc.prev = &node;
//			path_so_far.push_back(&loc);
//			t_location_vector::size_type e = path_so_far.size()-1;
//
//			long long new_cost_limit = DFS(
//				  state
//				, new_start_cost
//				, loc
//				, dest
//				, cost_limit
//				, path_so_far
//				, solution);
//
//			if (solution.size())
//				return new_cost_limit;
//
//			next_cost_limit = std::min(next_cost_limit, new_cost_limit);
//
//			if (path_so_far.size() > e)
//				path_so_far.erase(path_so_far.begin()+e);
//		}
//
//		return next_cost_limit;
//	}
//}
//
//bool calc::Path::astar( State &state )
//{
//	long long cost_limit(50);
//	t_location_vector path_so_far;
//	path_so_far.reserve(100);
//	path_so_far.push_back(start);
//
//	cost_limit = DFS(state, 0, *start, *dest, cost_limit, path_so_far, nodes);
//	if (nodes.size())
//		return true;
//	else
//		return false;
//}

bool calc::Path::astar( State &state )
{
	t_location_deque olist; // open
	t_location_vector clist; // closed
	clist.reserve(state.rows*state.cols);

  cost = 0;
	long long step = 0;

  try
  {
	  olist.push_back(start);

	  do 
	  {
      if (state.timer.getTime() > state.turntime - 100)
      {
        if (olist.empty())
          return false;
        else
          return true;
      }

		  Location *v = olist.front();

//#ifdef DEBUG
//			state.bug << "olist.front() " << *v << "\n";
//			state.bug << "step: " << step << "\n";
//#endif // DEBUG

			if (*v == *dest)
			  break;			
			if (olist.size() > static_cast<t_location_deque::size_type>(astar_obreak))
			  break;			
			if (clist.size() > static_cast<t_location_deque::size_type>(astar_cbreak))
			  break;

      olist.pop_front();

      cost -= v->weightcosts();

		  for(int d(e_north); d<TDIRECTIONS_SIZE; ++d)
		  { // expand vertex
			  Location *loc(state.getLocation(*v, static_cast<TDIRECTIONS>(d)));

			  t_location_vector::iterator vi(std::find_if(
            clist.begin()
          , clist.end()
          , find_by_loc(loc)));
			  if (vi != clist.end())
				  continue;

				if (loc->isWater || loc->ant != -1 /*|| !loc->isVisible*/)
				{
					clist.push_back(loc);
					continue;
				}

				Location *successor(loc);

				int manhatten = state.manhattan_method(*dest, *successor);
				////manhatten *= (1.0 + (1.0/static_cast<double>(astar_obreak)));
				long euclid = 1; //static_cast<long>(state.distance(*dest, *successor));
				long long new_weight(successor->weightcosts() + ( manhatten * euclid));
				//long long new_weight(state.getHeuristic(*dest, *successor));

			  t_location_deque::iterator vd(std::find_if(
            olist.begin()
          , olist.end()
          , find_by_loc(successor)));
			  if (vd != olist.end() && new_weight >= v->weightcosts())
				  continue;
				if (vd != olist.end())
					olist.erase(vd);

				successor->prev = v;
			  successor->cost = new_weight;
				olist.push_back(successor);

				cost += successor->weightcosts();

//#ifdef DEBUG
//				state.bug << "olist.push_back( " << *successor << ")\n";
//				state.bug << "successor( " 
//					<< successor->weightcosts() 
//					<< ", " 
//					<< state.manhattan_method(*dest, *successor)
//					<< ", " 
//					<< state.distance(*dest, *successor)
//					<< ", " 
//					<< d 
//					<< ")\n";
//#endif // DEBUG
		  }
		
		  std::sort(olist.begin(), olist.end(), pless<Location>());
		
		  clist.push_back(v);

			++step;

		} while (!olist.empty());

#ifdef DEBUG
		state.bug << "path olist size: " << olist.size() << endl;
		state.bug << "path clist size: " << clist.size() << endl;
#endif // DEBUG

		if (olist.empty())
		{
			return false;

		}

		fill_nodes(olist);

    state.bug << "path: " << *this << endl;
  }
  catch (std::exception &e)
  {
    cost = std::numeric_limits<unsigned int>::max();
    state.bug << "path: " << *this << endl;
    state.bug << "exception: " << e.what() << endl;
    return false;
  }
  catch (...)
  {
    state.bug << "path: " << *this << endl;
    state.bug << "exception: unknown" << endl;
    cost = std::numeric_limits<unsigned int>::max();
    return false;
  }

  return true;
}

calc::Path::Path( Location* s, Location* d, State &state, bool exact ) 
	: start( s )
	, dest( d )
	, cost(std::numeric_limits<unsigned int>::max())
	, turn_counter(0)
	, searchFood(false)
	, searchHill(false)
	, searchUnseen(false)
{
	state.bug << "before astar" << std::endl;

	//exact = false;

	if (exact)
	{
		int 
			  startid(state.tiling->getNodeId(start->row, start->col))
			, destid(state.tiling->getNodeId(dest->row, dest->col))
			, top(start->row)
			, left(start->col)
			, width((abs(start->row - dest->row)+10))
			, height((abs(start->col - dest->col)+10));

#		ifdef DEBUG
			state.bug 
				<< "id_start: " 
				<< startid 
				<< " id_dest: " 
				<< destid
				<< std::endl;
			state.bug 
				<< " start_row: " 
				<< top
				<< " start_col: " 
				<< left
				<< " width: " 
				<< width
				<< " height: " 
				<< height
				<< std::endl;
#		endif // DEBUG

		PathFind::Tiling *t(state.tiling.get());
		PathFind::AStar<> fr;
		fr.setNodesLimit((state.rows*state.cols)); 
		if (
				 start 
			&& dest 
			&& fr.findPath(*t, startid, destid)
			&& fr.getPath().size())
		{
			if (dest->isFood)
				searchFood = true;
			else
				searchFood = false;

			if (dest->isHill)
				searchHill = true;
			else
				searchHill = false;

			if (!searchFood && !searchHill)
				searchUnseen = true;
			else
				searchUnseen = false;

			nodes.clear(); unsigned cnt(0);
			for (std::vector<int>::const_reverse_iterator
					itb(fr.getPath().rbegin())
				, ite(fr.getPath().rend())
				; itb != ite
				; ++itb, ++cnt)
			{
				PathFind::TilingNodeInfo & info = state.tiling->getNodeInfo(*itb);
				nodes.push_back(&state.grid[info.getRow()][info.getColumn()].loc);

				if (searchUnseen && cnt > 4)
				{
					dest = nodes.back();
					break;
				}
			}

			if (nodes.size() && *nodes.front() == *start)
				nodes.pop_front();

#			ifdef DEBUG
				state.tiling->printFormatted(state.bug.file, fr.getPath());
				state.bug << "tiling: " << fr.getPath().size() << std::endl;
				state.bug << "path: " << *this << std::endl;
				state.bug << "path_size: " << nodes.size() << std::endl;
#			endif // DEBUG
		}
	}
	else
	{
		if (start && dest && astar(state))
		{
			if (dest->isFood)
				searchFood = true;
			else
				searchFood = false;

			if (dest->isHill)
				searchHill = true;
			else
				searchHill = false;

			if (!searchFood && !searchHill)
				searchUnseen = true;
			else
				searchUnseen = false;
		}
	}
	state.bug << "after astar" << std::endl;
}

//constructor
Bot::Bot()
{
	ann_out.open("ann.txt");

#ifdef TEST_ANN
	std::ifstream fin_train("train.txt");
	for (long long i(0); readOnePair(fin_train, input, idealOutput); ++i)
		Network.trainNetwork(input, idealOutput);
	fin_train.close(); 
#endif // TEST_ANN
}

//plays a single game of Ants.
void Bot::playGame()
{
  //reads the game parameters and sets up
  cin >> state;
  state.setup();
  endTurn();

  //continues making moves while the game is not over
  while(cin >> state)
  {
    state.updateVisionInformation();
    makeMoves();
    endTurn();
  }
}

//makes the bots moves for the turn
void Bot::makeMoves()
{
  state.bug << "turn " << state.turn << ":" << endl;
  //state.bug << state << endl;

  int d(0);

  try
  {
		t_location_vector::size_type 
			  ant_count(state.myAnts.size())
			, food_count(state.food.size())
			, hill_count(state.enemyHills.size());
		state.bug << "getTime " << state.timer.getTime() << endl;
		state.bug << "turntime " << state.turntime << endl;
		state.bug << "time_diff " << long(state.timer.getTime() < state.turntime - 100) << endl;
		state.bug << "ant_count " << ant_count << endl;
		state.bug << "food_count " << food_count << endl;
		state.bug << "hill_count " << hill_count << endl;

	if (ant_count == 0)
		return;

#ifdef REVERSE_SEARCH
		calc::t_order::iterator o;
		t_location_vector::iterator l;

		static Location* lastseenHill = 0;

		if (!lastseenHill && state.seenHill)
			orders.clear();
		lastseenHill = state.seenHill;

		for (calc::t_order::iterator 
			  itb(orders.begin())
			, ite(orders.end())
			; itb != ite
			; ++itb)
			itb->turn_visited = false;

		sort_all();

		long count_food_exact(-1);
		long count_hill_exact(-1);

		{
			for (t_location_vector::iterator 
				  itb(state.food.begin())
				, ite(state.food.end())
				; itb!=ite
				; ++itb)
			{
				if (state.timer.getTime() > (state.turntime - 100))
					break;
				if (!preMakeMoves(o, *itb))
					continue;
				calc::Path::astar_obreak = 48;
				int result = makeMoves(*itb, o, ((++count_food_exact) < 3));
				if (result < 0)
					break;
				result = makeMoves(*itb, o, ((++count_food_exact) < 3));
				if (result < 0)
					break;
			}
		}

#ifdef DEBUG
		state.bug << "after food search ant_count " << state.myAnts.size() << endl;
#endif // DEBUG
		
		if (ant_count > 25)
		{
			for (t_location_vector::iterator 
					itb(state.enemyHills.begin())
				, ite(state.enemyHills.end())
				; itb!=ite
				; ++itb)
			{
				if (state.timer.getTime() > state.turntime - 100)
					break;
				if (!preMakeMoves(o, *itb))
					continue;
				calc::Path::astar_obreak = 96;
				if (makeMoves(*itb, o) < 0, ((++count_hill_exact) < 4))
					break;
				if (makeMoves(*itb, o) < 0, ((++count_hill_exact) < 4))
					break;
				if (makeMoves(*itb, o) < 0, ((++count_hill_exact) < 4))
					break;
				if (makeMoves(*itb, o) < 0, ((++count_hill_exact) < 4))
					break;
			}
		}
		
		state.bug 
			<< "after hill search ant_count " 
			<< state.myAnts.size() 
			<< endl;

		for (calc::t_order::iterator 
				itb(orders.begin())
			, ite(orders.end())
			; itb != ite
			; ++itb)
		{
			if (itb->turn_visited)
				continue;
			if (state.timer.getTime() > state.turntime - 100)
				break;
			t_location_vector::iterator l(std::find_if(
					state.myAnts.begin()
				, state.myAnts.end()
				, find_by_loc(itb->start)));
			if (l != state.myAnts.end())
				state.myAnts.erase(l);

			postMakeMoves(*itb, itb->start);
		}

		state.bug << "path moves ant_count " << state.myAnts.size() << endl;

		if (ant_count > 50)
		{
			for (t_location_vector::iterator 
					itb(state.enemyAnts.begin())
				, ite(state.enemyAnts.end())
				; itb != ite
				; ++itb)
			{
				if (state.timer.getTime() > state.turntime - 100)
					break;
				if (!preMakeMoves(o, *itb))
					continue;
				calc::Path::astar_obreak = 12;
				if (makeMoves(*itb, o) < 0)
					break;
				if (makeMoves(*itb, o) < 0)
					break;
			}
		}

		for (t_location_vector::iterator 
				itb(state.myAnts.begin())
			, ite(state.myAnts.end())
			; itb != ite
			; ++itb)
		{
			if (state.timer.getTime() > (state.turntime - 100))
				break;

			Location * ant = *itb;
			Location * loc = &state.grid[state.rows/2][state.cols/2].loc;
			if(state.seenHill)
			{
				calc::Path::astar_obreak = 24;
				loc = state.seenHill;
			}
			else
				calc::Path::astar_obreak = 12;

			state.bug << "exploring " << *ant << " / " <<  *loc << endl;

			calc::Path p(ant, loc, state);

			if (!p.nodes.size())
				continue;

			orders.push_back(p); 
			calc::t_order::iterator o = orders.end()-1;	 

			postMakeMoves(*o, ant);
		}

		state.bug << "orders " << orders.size() << endl;

		calc::t_order::iterator itb(orders.begin());
		while(itb != orders.end())
		{
			if (state.timer.getTime() > state.turntime - 100)
				break;

			if (itb->nodes.size()==0)
			{
				itb = orders.erase(itb);
				itb = orders.begin();
			}
			else if (itb->turn_visited == false)
			{
				itb = orders.erase(itb);
				itb = orders.begin();
			}
			else if (itb->turn_counter > 2)
			{
				itb = orders.erase(itb);
				itb = orders.begin();
			}
			//else if (itb->dest && itb->start && itb->dest == itb->start)
			//{
			//	itb = orders.erase(itb);
			//	itb = orders.begin();
			//}
			else if (
				   itb->dest 
				&& state.seenHill 
				&& itb->dest == state.seenHill 
				&& !itb->dest->isVisible)
				continue;
			else if (itb->searchFood 
				&& itb->dest 
				&& itb->dest->isVisible 
				&& !itb->dest->isFood)
			{
				itb = orders.erase(itb);
				itb = orders.begin();
			}
			//else if (itb->searchHill 
			//	&& itb->dest 
			//	&& itb->dest->isVisible 
			//	&& !itb->dest->isHill)
			//{
			//	itb = orders.erase(itb);
			//	itb = orders.begin();
			//}
			else
				++itb;
		}

		state.bug << "orders " << orders.size() << endl;

#else
    //picks out moves for each ant
    for(std::vector<Location*>::size_type 
        ant(0), ant_count(state.myAnts.size())
      ; ant < ant_count
      ; ++ant)
    {
      Location* ant_loc(state.myAnts[ant]);

			state.bug << "ant: " << ant << endl;;

      if (state.timer.getTime() > state.turntime - 100)
        break;

      for (calc::t_order::iterator 
				  itb(orders.begin())
			  , ite(orders.end())
			  ; itb != ite
			  ; ++itb)
			  itb->turn_visited = false;

      calc::t_order::iterator o = std::find_if(
          orders.begin()
        , orders.end()
        , find_by_loc(ant_loc));
      calc::Path* p(0);
      if (o == orders.end())
      { // recalculate
				calc::Path *ptmp(0);
				{
					calc::Path ptmp_food, ptmp_hill, ptmp_enemy, ptmp_center;

					if (ant_count > 100)
					{ // only attacking
						if (state.enemyHills.size())
						{
							calc::Path::astar_break = 72;
							ptmp_hill = calc::Path(
								  ant_loc
								, state.enemyHills[0]
								, state);
						}
						if (!ptmp_hill.dest)
						{
							calc::Path::astar_break = 12;
							ptmp_enemy = calc::Path(
								  ant_loc
								, closest_enemy(*ant_loc)
								, state);
						}

						if (!ptmp_hill.dest && !ptmp_enemy.dest)
						{
							calc::Path::astar_break = 48;
							ptmp_food = calc::Path(
								  ant_loc
								, closest_food(*ant_loc)
								, state);
							calc::Path::astar_break = 24;
							ptmp_center = calc::Path(
									ant_loc
								, &state.grid[state.rows/2][state.cols/2].loc
								, state);
						}
					}
					else if (ant_count > 50)
					{
						calc::Path::astar_break = 72;
						ptmp_hill = calc::Path(
							  ant_loc
							, closest_hill(*ant_loc)
							, state);
						calc::Path::astar_break = 72;
						ptmp_food = calc::Path(
							  ant_loc
							, closest_food(*ant_loc)
							, state);
						calc::Path::astar_break = 24;
						ptmp_enemy = calc::Path(
							  ant_loc
							, closest_enemy(*ant_loc)
							, state);
						calc::Path::astar_break = 12;
						ptmp_center = calc::Path(
							  ant_loc
							, &state.grid[state.rows/2][state.cols/2].loc
							, state);
					}
					else
					{
						//calc::Path::astar_break = 100;
						//if (ant_count > 30)
						//	ptmp_hill = calc::Path(
						//	  ant_loc
						//	, closest_hill(*ant_loc)
						//	, state);
						calc::Path::astar_break = 128;
						ptmp_food = calc::Path(
							  ant_loc
							, closest_food(*ant_loc)
							, state);
						//calc::Path::astar_break = 24;
						//ptmp_enemy = calc::Path(
						//	  ant_loc
						//	, closest_enemy(*ant_loc)
						//	, state);
						calc::Path::astar_break = 12;
						ptmp_center = calc::Path(
							  ant_loc
							, &state.grid[state.rows/2][state.cols/2].loc
							, state);					
					}

					if (ptmp_hill.dest)
						ptmp = &ptmp_hill;
					else if (ptmp_food.dest)
						ptmp = &ptmp_food;
					else if (ptmp_enemy.dest)
						ptmp = &ptmp_enemy;
					else
						ptmp = &ptmp_center;
					if (ptmp)
					{
						orders.push_back(*ptmp); 
						p = &orders.back(); 
						o = orders.end()-1;	 
					}
				}
      }
		  else
        p = &*o;

			if (p)
				state.bug 
					<< "try akt move " 
					<< p->nodes.size() 
					<< " "
					<< *p->start 
					<< " ant_loc " 
					<< *ant_loc << endl;

      if (p && p->nodes.size() && *p == ant_loc)
      {
				++p->turn_counter;
        Location *v = p->nodes.front();

        d = state.get_direction(*ant_loc, *v);

        state.bug 
          << "akt move " 
          << *v 
          << " ant_loc " 
          << *ant_loc << " direction " << d << endl;

        Location *loc = state.getLocation(*ant_loc, d);
        if(loc && !loc->isWater && loc->ant != 0 && ant_loc->ant > -1)
        {
					p->turn_visited = true;

#ifdef TRAIN_ANN
					if (p && p->dest && p->dest->isFood || p->dest->hillPlayer>0)
					{
						t_location_vector suround;
						fill_suround(suround, *ant_loc);
			
						for (t_location_vector::const_iterator 
							  isb(suround.begin())
							, ise(suround.end())
							; isb != ise
							; ++isb)
						{
							const Location * l = *isb;
							ann_out 
								<< 1.0/(static_cast<double>(l->weightcosts())-0.01)
								<< " " 
								<< 1.0/(static_cast<double>(static_cast<int>(l->isVisible) 
								+  static_cast<int>(l->isWater)
								+  static_cast<int>(l->isFood)
								+  static_cast<int>(l->isDead)
								+  static_cast<int>(l->isHill))-0.01)
								<< " " 
								<< 1.0/(static_cast<double>(l->hillPlayer)-0.01)
								<< " ";
						}
						ann_out 
							<< 1.0/(static_cast<double>(static_cast<int>(d)+1)-0.01)
							<< "\n";
					}
#endif // TRAIN_ANN
      //    if (p->nodes.size() > 4 
						//&& (!p->dest->isFood 
						//&& !(p->dest->hillPlayer > 0)))
      //    {
						//if (o != orders.end())
						//	orders.erase(o);

      //   //   calc::t_order::iterator itb(orders.begin());
      //   //   while (itb != orders.end())
      //   //   {
      //   //     if (&*itb != p)
      //   //       if (*itb->dest == *loc)
      //   //       {
      //   //         itb = orders.erase(itb);
						//			//itb = orders.begin();
      //   //         continue;
      //   //       }
      //   //     ++itb;
      //   //   }
      //    }
          
          p->nodes.pop_front();
          state.makeMoves(*ant_loc, *loc, static_cast<TDIRECTIONS>(d));
          p->start = loc;
				  continue;
        }

        if (o != orders.end())
          orders.erase(o);
      }
		  else if (o != orders.end())
			  orders.erase(o);
    }
#endif // REVERSE_SEARCH
  }
  catch (...)
  {

  }

  try
  {
//#ifdef TRAIN_ANN
//		for (t_moves::iterator 
//			  itb(state._moves.begin())
//			, ite(state._moves.end())
//			; itb != ite
//			; ++itb)
//		{
//			t_location_vector suround;
//			fill_suround(suround, *itb->from);
//
//			for (t_location_vector::const_iterator 
//				  isb(suround.begin())
//				, ise(suround.end())
//				; isb != ise
//				; ++isb)
//			{
//				const Location * l = *isb;
//				ann_out 
//					<< l->weightcosts()
//					<< " " 
//					<< static_cast<int>(l->isVisible) 
//					+  static_cast<int>(l->isWater)
//					+  static_cast<int>(l->isFood)
//					+  static_cast<int>(l->isDead)
//					+  static_cast<int>(l->isHill)
//					<< " " 
//					<< static_cast<double>(l->hillPlayer)
//					<< " ";
//			}
//			ann_out 
//				<< static_cast<int>(itb->d)+1
//				<< "\n";
//		}
//#endif // TRAIN_ANN
#ifdef TEST_ANN
		for (t_moves::iterator 
			  itb(state._moves.begin())
			, ite(state._moves.end())
			; itb != ite
			; ++itb)
		{
			t_location_vector suround;
			fill_suround(suround, *itb->from);

			double input[NUM_INPUTS];
			int i(0);
			for (t_location_vector::const_iterator 
				  isb(suround.begin())
				, ise(suround.end())
				; isb != ise
				; ++isb)
			{
				const Location * l = *isb;
				input[i++] = 1.0/(static_cast<double>(l->weightcosts())-0.01);
				input[i++] = 1.0/(static_cast<double>(static_cast<int>(l->isVisible) 
					+  static_cast<int>(l->isWater)
					+  static_cast<int>(l->isFood)
					+  static_cast<int>(l->isDead)
					+  static_cast<int>(l->isHill))-0.01);
				input[i++] = 1.0/(static_cast<double>(l->hillPlayer)-0.01);
			}

			Network.processInput(input);

			ann_out 
				<< Network.getOutput(0)
				<< " / "
				<< 1.0/(static_cast<double>(static_cast<int>(itb->d)+1) + 0.01)
				<< "\n";
		}
#endif // TEST_ANN
    state.endMoves();
  }
  catch (...)
  {
  	
  }
  
  state.bug << "time taken: " << state.timer.getTime() << "ms" << endl << endl;
};

void Bot::postMakeMoves( calc::Path & p, Location * ant )
{
	if (p.nodes.size()==0 || p.turn_visited)
		return;

	++p.turn_counter;

	int d(e_north);

	Location *v(p.nodes.front());

	d = state.get_direction(*ant, *v);

	state.bug 
		<< "akt move " 
		<< *v 
		<< " ant_loc " 
		<< *ant << " direction " << d << endl;

	d = state.get_direction(*ant, *v);
	Location *loc = state.getLocation(*ant, d);
	if(loc && !loc->isWater && loc->ant != 0 && ant->ant > -1)
	{
		p.nodes.pop_front();
		state.makeMoves(*ant, *loc, static_cast<TDIRECTIONS>(d));
		p.start = loc;
		p.turn_visited = true;
		p.turn_counter = 0;
	}
}

int Bot::makeMoves(Location* loc, calc::t_order::iterator& o, bool exact)
{
	if (!loc)
		return -1;
	Location *ant(closest_ant_for_reverse(*loc));
	if (!ant)
		return -1;
	{
		calc::Path p(ant, loc, state, exact);
		if (!p.nodes.size())
			return 0;

		orders.push_back(p); 
		o = orders.end()-1;	 

		t_location_vector::iterator l(std::find_if(
			  state.myAnts.begin()
			, state.myAnts.end()
			, find_by_loc(o->start)));
		if (l != state.myAnts.end())
			state.myAnts.erase(l);
	}

	if (o->nodes.size() && o->start == ant)
		postMakeMoves(*o, ant);

	return 1;
}

bool Bot::preMakeMoves(calc::t_order::iterator &o, Location * loc)
{
	o = std::find_if(
		  orders.begin()
		, orders.end()
		, find_by_loc(loc, true));
	if (o != orders.end())
	{
		t_location_vector::iterator l(std::find_if(
			  state.myAnts.begin()
			, state.myAnts.end()
			, find_by_loc(o->start)));
		if (l != state.myAnts.end())
			state.myAnts.erase(l);
		return false;
	}
	return true;
}

//finishes the turn
void Bot::endTurn()
{
  if(state.turn > 0)
    state.reset();
  state.turn++;

  cout << "go" << endl;
}

Location * Bot::closest_food( const Location &loc )
{
	state.bug << "closest_food start" << std::endl;
  Location * l(0);

	if (state.food.size()==0)
		return l;

	double min_dist=std::numeric_limits<double>::max(), dist(min_dist);
  for (std::vector<Location>::size_type 
      i(0)
    , cnt(state.food.size())
    ; i < cnt
    ; ++i)
  {
    Location * ll(state.food[i]);
    ll->dist_food = state.distance(loc, *ll);
  }

  sort(state.food.begin(), state.food.end(), sort_food);

	//if (state.food.size())
	//	l = *state.food.begin();

	//if (state.myAnts.size()>100)
	//	return l;

	for (std::vector<Location*>::size_type 
		  i(0)
		, cnt(state.food.size())
		; i < cnt
		; ++i)
	{
		calc::t_order::iterator o = std::find_if(
			  orders.begin()
			, orders.end()
			, find_by_loc(state.food[i]));
		if (o == orders.end())
			return state.food[i];
	}

	state.bug << "closest_food end" << std::endl;
  return l;
}

Location * Bot::closest_hill( const Location &loc )
{
  Location * l(0);

	state.bug << "closest_hill start" << std::endl;

  if (state.enemyHills.size()==0)
    return l;

	if (state.myAnts.size()>100)
		return state.enemyHills[0];

  double min_dist=std::numeric_limits<double>::max(), dist(min_dist);
  for (std::vector<Location>::size_type 
    i(0)
    , cnt(state.enemyHills.size())
    ; i < cnt
    ; ++i)
  {
    Location * ll(state.enemyHills[i]);
    ll->dist_hill = state.distance(loc, *ll);
  }

 // sort(state.enemyHills.begin(), state.enemyHills.end(), sort_hill);
	//if (state.enemyHills.size())
	//	l = *state.enemyHills.begin();

	//if (state.myAnts.size()>50)
	//	return l;

	//for (std::vector<Location*>::size_type 
	//	  i(0)
	//	, cnt(state.enemyHills.size())
	//	; i < cnt
	//	; ++i)
	//{
	//	calc::t_order::iterator o = std::find_if(
	//		  orders.begin()
	//		, orders.end()
	//		, find_by_loc(state.enemyHills[i]));
	//	if (o == orders.end())
	//		return state.enemyHills[i];
	//}

	state.bug << "closest_hill end" << std::endl;
  return l;
}

Location * Bot::closest_enemy( const Location &loc )
{
  Location * l(0);

	state.bug << "closest_enemy start" << std::endl;

	if (state.myAnts.size() < 50)
		return l;

  double min_dist=std::numeric_limits<double>::max(), dist(min_dist);
  for (std::vector<Location>::size_type 
      i(0)
    , cnt(state.enemyAnts.size())
    ; i < cnt
    ; ++i)
  {
    Location * ll(state.enemyAnts[i]);
    ll->dist_enemy = state.distance(loc, *ll);
  }

  sort(state.enemyAnts.begin(), state.enemyAnts.end(), sort_enemy);
	if (state.enemyAnts.size())
		l = *state.enemyAnts.begin();

	state.bug << "closest_enemy end" << std::endl;

	return l;
}

Location * Bot::closest_ant( const Location &loc )
{
  Location * l(0);
  
  t_location_vector suround;
	fill_suround(suround, loc);

  for (t_location_vector::iterator 
      itb(suround.begin())
    , ite(suround.end())
    ; itb!=ite
    ; ++itb)
  {
    if ((*itb)->ant > 0)
    {
      calc::t_order::iterator o = std::find_if(
          orders.begin()
        , orders.end()
        , find_by_loc(*itb));
      if (o != orders.end())
        if (o->nodes.size() > 5)
          return *itb;
    }
  }

  return l;
}

Location * Bot::closest_ant_for_reverse( const Location &loc )
{
	Location * l(0);

	if (!state.myAnts.size())
		return l;

	if (state.myAnts.size() == 1)
		return state.myAnts[0];

	double min_dist=std::numeric_limits<double>::max(), dist(min_dist);
	for (t_location_vector::iterator 
		  itb(state.myAnts.begin())
		, ite(state.myAnts.end())
		; itb!=ite
		; ++itb)
	{
		Location * ll(*itb);
		dist = state.distance(loc, *ll);
		if (min_dist > dist)
		{
			l = ll;
			min_dist = dist;
		}
	}

	return l;
}

void Bot::sort_all()
{
       Location & top_left = state.grid[0][0].loc;
       sort_one(state.food, top_left);
       sort_one(state.myAnts, top_left);
       sort_one(state.enemyAnts, top_left);
       sort_one(state.enemyHills, top_left);
}

void Bot::sort_one( t_location_vector &to_sort_vector, Location & top_left )
{
       struct s_sort_all
       {
               static bool sort(const Location * lhs, const Location * rhs)
               { return lhs->dist < rhs->dist; }
       };

       double min_dist=std::numeric_limits<double>::max(), dist(min_dist);
       for (t_location_vector::iterator
                 itb(to_sort_vector.begin())
               , ite(to_sort_vector.end())
               ; itb!=ite
               ; ++itb)
               (*itb)->dist = state.distance(top_left, **itb);
       sort(to_sort_vector.begin(), to_sort_vector.end(), s_sort_all::sort);
}