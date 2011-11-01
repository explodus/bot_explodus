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
		inline bool operator()(const calc::Vertex& t_) const
		{ return t_==loc; }
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
	typedef std::deque<Vertex> t_dvertex;
	typedef std::vector<Vertex> t_vvertex;
	t_dvertex olist; // open
	t_vvertex clist; // closed

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

		  Vertex v = olist.front();

		  if (v == dest || olist.size() > astar_break)
			  break;

      olist.pop_front();

      cost -= v.weightcosts();

		  for(int d(e_north); d<TDIRECTIONS_SIZE; ++d)
		  { // expand vertex
			  Location *loc(state.getLocation(*v.loc, static_cast<TDIRECTIONS>(d)));

        //if (loc->isWater /*|| !loc->isVisible*/)
        //{
        //  clist.push_back(v);
        //  continue;
        //}
        //
        if (loc->isWater || loc->ant != -1)
        {
          clist.push_back(v);
          continue;
        }

			  t_vvertex::iterator vi(std::find_if(
            clist.begin()
          , clist.end()
          , find_by_loc(loc)));
			  if (vi != clist.end())
				  continue;

			  Vertex successor(loc);

        unsigned int new_weight = 
          v.loc->weight + (v.loc->cost + successor.loc->cost); 
			  //unsigned int new_weight = 
     //       (v.loc->weightcosts() + successor.loc->weightcosts())
     //     /*+ manhattan_method(*v.loc, *successor.loc)*/;

			  t_dvertex::iterator vd(std::find_if(
            olist.begin()
          , olist.end()
          , find_by_loc(successor.loc)));
			  if (vd != olist.end() && new_weight >= v.weightcosts())
				  continue;

			  successor.prev.reset(new Vertex(v));
			  successor.loc->weight = new_weight;

			  if (vd != olist.end())
				  vd = olist.erase(vd);

			  olist.push_back(successor);
        cost += successor.weightcosts();
		  }
		
		  std::sort(olist.begin(), olist.end(), std::less<Vertex>());
		
		  clist.push_back(v);
	  } while (!olist.empty());

    state.bug << "path olist size: " << olist.size() << "\n";

    if (olist.empty())
		  return false;

	  Vertex v = olist.front();
	
	  while(v.prev)
	  {
		  nodes.push_front(v);
		  v = *v.prev;
	  }
    
    state.bug << "path: " << *this << "\n";
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

inline ostream& operator<<(ostream &os, const calc::Path &p)
{
  os << *p.start.loc << "/";
  os << *p.dest.loc << " ";
	for(calc::Path::t_vertex::const_iterator 
		  itb(p.nodes.begin())
		, ite(p.nodes.end())
		; itb!=ite
		; ++itb)
		if (itb->loc)
			os << *itb->loc << "->";

	return os;
};


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

  int d(0);

  try
  {
  /*  if (state.myAnts.size() > 50)
      calc::Path::astar_break = 75;
    else */if (state.myAnts.size() > 150)
      calc::Path::astar_break = 40;
    else
      calc::Path::astar_break = 80;

    //picks out moves for each ant
    for(std::vector<Location*>::size_type 
        ant(0), ant_count(state.myAnts.size())
      ; ant < ant_count
      ; ++ant)
    {
      Location* ant_loc(state.myAnts[ant]);

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
				if (ant_count > 200)
				{
					calc::Path ptmp_enemy(
						  calc::Vertex(ant_loc)
						, calc::Vertex(closest_enemy(*ant_loc))
						, state);
					calc::Path ptmp_center(
						  calc::Vertex(ant_loc)
						, calc::Vertex(&state.grid[state.rows/2][state.cols/2].loc)
						, state);
					if (ptmp_enemy.dest.loc)
						ptmp = &ptmp_enemy;
					else
						ptmp = &ptmp_center;
				}
				else
				{
					calc::Path ptmp_food(
							calc::Vertex(ant_loc)
						, calc::Vertex(closest_food(*ant_loc))
						, state);
					calc::Path ptmp_hill(
							calc::Vertex(ant_loc)
						, calc::Vertex(closest_hill(*ant_loc))
						, state);

					Location *loc_enemy(closest_enemy(*ant_loc));
					calc::Path ptmp_enemy(
							calc::Vertex(ant_loc)
						, calc::Vertex(loc_enemy?loc_enemy:&state.grid[state.rows/2][state.cols/2].loc)
						, state);

					if (ptmp_hill.dest.loc)
						ptmp = &ptmp_hill;
					else if (ptmp_food.dest.loc)
						ptmp = &ptmp_food;
					else if (ptmp_enemy.dest.loc)
						ptmp = &ptmp_enemy;
				}
        
				if (ptmp)
        {
          orders.push_back(*ptmp); 
          p = &orders.back(); 
          o = orders.end()-1;	 
        }
      }
		  else
        p = &*o;

      if (p && p->nodes.size() && *ant_loc== *p->start.loc)
      {
			  ++p->turn_counter;
        calc::Vertex v = p->nodes.front();

        d = state.get_direction(*ant_loc, *v.loc);

        state.bug 
          << "akt move " 
          << *v.loc 
          << " ant_loc " 
          << *ant_loc << " direction " << d << "\n";

        Location *loc = state.getLocation(*ant_loc, d);
        if(loc && !loc->isWater && loc->ant==-1 && ant_loc->ant > -1)
        {
          if (loc->hillPlayer > 0 || loc->isFood)
          {
            calc::t_order::iterator itb(orders.begin());
            while (itb != orders.end())
            {
              if (&*itb != p)
                if (*itb->dest.loc == *loc)
                {
                  itb = orders.erase(itb);
                  continue;
                }
              ++itb;
            }
          }
          
          p->nodes.pop_front();
          state.makeMoves(*ant_loc, *loc, static_cast<TDIRECTIONS>(d));
          p->start.loc = loc;
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
  Location * l(0);
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
	for (std::vector<Location>::size_type 
		  i(0)
		, cnt(state.food.size())
		; i < cnt
		; ++i)
	{
		calc::t_order::iterator o = std::find_if(
			  orders.begin()
			, orders.end()
			, find_by_loc(state.food[i]));
		calc::Path* p(0);
		if (o == orders.end())
			return state.food[i];
	}
	
  return l;
}

Location * Bot::closest_hill( const Location &loc )
{
  Location * l(0);

  if (state.enemyHills.size()==0)
    return l;

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

  sort(state.enemyHills.begin(), state.enemyHills.end(), sort_hill);
	if (state.enemyHills.size())
		l = *state.enemyHills.begin();
  return l;
}

Location * Bot::closest_enemy( const Location &loc )
{
  Location * l(0);

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