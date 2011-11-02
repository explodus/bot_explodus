#include "Bot.h"
#include <limits>
#include <algorithm>
#include <vector>
#include <functional>

using namespace std;

int calc::Path::astar_break = 100;

namespace 
{
  class find_by_loc
  {
    const Location * loc;
  public:
    find_by_loc(const Location * l) : loc(l) {}
    inline bool operator()(const calc::Path& t_) const
    { return t_==loc && t_.turn_counter == 0; }
		inline bool operator()(const Location& t_) const
		{ return t_==*loc; }
		inline bool operator()(const Location* t_) const
		{ return *t_==*loc; }
  };

  template<typename Type, typename Compare = std::less<Type> >
  struct pless : public std::binary_function<Type *, Type *, bool> 
  {
    bool operator()(const Type *x, const Type *y) const
    { return Compare()(*x, *y); }
  };
}

bool calc::Path::astar( State &state )
{
	t_location_deque olist; // open
	t_location_vector clist; // closed

  cost = 0;

  try
  {
	  olist.push_back(start);

	  do 
	  {
      if (state.timer.getTime() > 450)
      {
        if (olist.empty())
          return false;
        else
          return true;
      }

		  Location *v = olist.front();

			if (*v == *dest || olist.size() > static_cast<t_location_deque::size_type>(astar_break))
			  break;

      olist.pop_front();

      cost -= v->weightcosts();

		  for(int d(e_north); d<TDIRECTIONS_SIZE; ++d)
		  { // expand vertex
			  Location *loc(state.getLocation(*v, static_cast<TDIRECTIONS>(d)));

        if (loc->isWater || loc->ant != -1/* || !loc->isVisible*/)
        {
          clist.push_back(v);
          continue;
        }

			  t_location_vector::iterator vi(std::find_if(
            clist.begin()
          , clist.end()
          , find_by_loc(loc)));
			  if (vi != clist.end())
				  continue;

			  Location *successor(loc);

        long long new_weight = 
          v->weight + (v->cost + successor->cost); 

			  t_location_deque::iterator vd(std::find_if(
            olist.begin()
          , olist.end()
          , find_by_loc(successor)));
			  if (vd != olist.end() && new_weight >= v->weightcosts())
				  continue;

				successor->prev = v;
			  successor->cost = new_weight;

			  if (vd != olist.end())
				  vd = olist.erase(vd);

			  olist.push_back(successor);
        cost += successor->weightcosts();
		  }
		
		  std::sort(olist.begin(), olist.end(), pless<Location>());
		
		  clist.push_back(v);
	  } while (!olist.empty());

    state.bug << "path olist size: " << olist.size() << endl;

    if (olist.empty())
		  return false;

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

//constructor
Bot::Bot()
{

};

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
};

//makes the bots moves for the turn
void Bot::makeMoves()
{
  state.bug << "turn " << state.turn << ":" << endl;
  //state.bug << state << endl;

	if (state.turn > 998)
		return;

	state.bug << "turn " << state.turn << ":" << endl;

  int d(0);

  try
  {
		state.bug << "ant_count " << state.myAnts.size() << endl;;

    //picks out moves for each ant
    for(std::vector<Location*>::size_type 
        ant(0), ant_count(state.myAnts.size())
      ; ant < ant_count
      ; ++ant)
    {
      Location* ant_loc(state.myAnts[ant]);

			state.bug << "ant: " << ant << endl;;

      if (state.timer.getTime() > 450)
        break;

      for (calc::t_order::iterator 
				  itb(orders.begin())
			  , ite(orders.end())
			  ; itb != ite
			  ; ++itb)
			  itb->turn_counter = 0;

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

					if (ant_count > 80 && ant_count > state.enemyAnts.size()*1.2 )
					{ // only attacking
						if (state.enemyHills.size())
						{
							calc::Path::astar_break = 96;
							ptmp_hill = calc::Path(
								  ant_loc
								, state.enemyHills[0]
								, state);
						}
						else
						{
							calc::Path::astar_break = 48;
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
					else if (ant_count > 80)
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
						calc::Path::astar_break = 100;
						if (ant_count > 30)
							ptmp_hill = calc::Path(
							  ant_loc
							, closest_hill(*ant_loc)
							, state);
						calc::Path::astar_break = 96;
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
  }
  catch (...)
  {

  }

  try
  {
    state.endMoves();
  }
  catch (...)
  {
  	
  }
  
  state.bug << "time taken: " << state.timer.getTime() << "ms" << endl << endl;
};

//finishes the turn
void Bot::endTurn()
{
  if(state.turn > 0)
    state.reset();
  state.turn++;

  cout << "go" << endl;
};

namespace 
{
  bool sort_food(const Location * lhs, const Location * rhs)
  { return lhs->dist_food < rhs->dist_food; }
  bool sort_hill(const Location * lhs, const Location * rhs)
  { return lhs->dist_hill < rhs->dist_hill; }
  bool sort_enemy(const Location * lhs, const Location * rhs)
  { return lhs->dist_enemy < rhs->dist_enemy; }
  bool sort_ant(const Location * lhs, const Location * rhs)
  { return lhs->dist_ant < rhs->dist_ant; }
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
  
  std::vector<Location*> suround;
  suround.reserve(24);

  suround.push_back(state.getLocation(loc, e_north));
  suround.push_back(state.getLocation(*suround.back(), e_east));
  suround.push_back(state.getLocation(*suround.back(), e_south));
  suround.push_back(state.getLocation(*suround.back(), e_south));
  suround.push_back(state.getLocation(*suround.back(), e_west));
  suround.push_back(state.getLocation(*suround.back(), e_west));
  suround.push_back(state.getLocation(*suround.back(), e_north));
  suround.push_back(state.getLocation(*suround.back(), e_north));
  suround.push_back(state.getLocation(*suround.back(), e_north));
  suround.push_back(state.getLocation(*suround.back(), e_east));
  suround.push_back(state.getLocation(*suround.back(), e_east));
  suround.push_back(state.getLocation(*suround.back(), e_east));
  suround.push_back(state.getLocation(*suround.back(), e_south));
  suround.push_back(state.getLocation(*suround.back(), e_south));
  suround.push_back(state.getLocation(*suround.back(), e_south));
  suround.push_back(state.getLocation(*suround.back(), e_south));
  suround.push_back(state.getLocation(*suround.back(), e_west));
  suround.push_back(state.getLocation(*suround.back(), e_west));
  suround.push_back(state.getLocation(*suround.back(), e_west));
  suround.push_back(state.getLocation(*suround.back(), e_west));
  suround.push_back(state.getLocation(*suround.back(), e_north));
  suround.push_back(state.getLocation(*suround.back(), e_north));
  suround.push_back(state.getLocation(*suround.back(), e_north));
  suround.push_back(state.getLocation(*suround.back(), e_north));

  for (std::vector<Location*>::iterator 
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