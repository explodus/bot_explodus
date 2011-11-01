#ifndef BOT_H_
#define BOT_H_

#include "State.h"
#include <queue>
#include <vector>
#include <list>
#include <limits>
#include <iostream>
#include <stdio.h>
#include <cstdlib>
#include <cmath>
#include <iterator>
#ifdef _WIN64
# include <memory>
#elif _WIN32
# include <memory>
#else
# include <tr1/memory>
#endif 

namespace calc
{
  struct Path
  {
    Location* start;
    Location* dest;

    long long cost;
		unsigned int turn_counter;

    typedef std::deque<Location*> t_vertex;
		typedef std::vector<Location*> t_vvertex;
    t_vertex nodes;

    static int astar_break;

    Path() 
      : start( 0 )
      , dest( 0 )
      , cost(std::numeric_limits<unsigned int>::max())
			, turn_counter(0)
    {}

    Path( 
        Location* s
      , Location* d
      , State &state )
      : start( s )
      , dest( d )
      , cost(std::numeric_limits<unsigned int>::max())
			, turn_counter(0)
    {
			state.bug << "before astar" << std::endl;

      if (start && dest && astar(state))
      {
        cost = 0;
        for (t_vertex::const_iterator 
            itb(nodes.begin())
          , ite(nodes.end())
          ; itb != ite
          ; ++itb)
          cost += (*itb)->weightcosts();
				state.bug << "path costs: "<< cost << std::endl;
      }
			state.bug << "after astar" << std::endl;
    }

    Path( const Path & p) 
      : start(p.start)
      , dest(p.dest)
      , cost(p.cost)
			, turn_counter(p.turn_counter)
			, nodes(p.nodes)
    {
    }

    Path& operator=(const Path & p)
    {
      start = p.start;
      dest = p.dest;
      cost = p.cost;
			turn_counter = p.turn_counter;
			nodes = p.nodes;
      return *this;
    }

    bool operator>( const Path & rhs ) const
    { return cost > rhs.cost; }
    bool operator<( const Path & rhs ) const
    { return cost < rhs.cost; }

    inline bool operator==( const Location *l ) const 
    { return start == l; }

    bool astar(State &state);

		inline void fill_nodes( t_vertex &olist ) 
		{
			Location *v(olist.front());
			while(v->prev)
			{
				nodes.push_front(v);
				v = v->prev;
			}
		}
	};

  typedef std::vector<Path> t_order;

	inline std::ostream& operator<<(std::ostream &os, const calc::Path &p)
	{
		if (p.start)
			os << *p.start << "/";
		if (p.dest)
			os << *p.dest;
		for(calc::Path::t_vertex::const_iterator 
			  itb(p.nodes.begin())
			, ite(p.nodes.end())
			; itb!=ite
			; ++itb)
			if (*itb)
				os << "->" << **itb ;

		return os;
	}

}

/*
This struct represents your bot in the game of Ants
*/
struct Bot
{
  State state;

  calc::t_order orders;

  Bot();

  void playGame();    //plays a single game of Ants

  void makeMoves();   //makes moves for a single turn
  void endTurn();     //indicates to the engine that it has made its moves

  Location * closest_food(const Location &loc);
  Location * closest_hill(const Location &loc);
  Location * closest_enemy(const Location &loc);
  Location * closest_ant(const Location &loc);

};

#endif //BOT_H_
