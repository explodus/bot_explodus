#ifndef BOT_H_
#define BOT_H_

#include "State.h"
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
		bool turn_visited;

    t_location_deque nodes;

		static int astar_obreak;
		static int astar_cbreak;

		bool searchFood, searchHill, searchUnseen;

    Path() 
      : start( 0 )
      , dest( 0 )
      , cost(std::numeric_limits<unsigned int>::max())
			, turn_counter(0)
			, turn_visited(false)
			, searchFood(false)
			, searchHill(false)
			, searchUnseen(false)
    {}

    Path( 
        Location* s
      , Location* d
      , State &state
			, bool exact = false );

    Path( const Path & p) 
      : start(p.start)
      , dest(p.dest)
      , cost(p.cost)
			, turn_counter(p.turn_counter)
			, turn_visited(p.turn_visited)
			, nodes(p.nodes)
			, searchFood(p.searchFood)
			, searchHill(p.searchHill)
			, searchUnseen(p.searchUnseen)
    {
    }

    Path& operator=(const Path & p)
    {
      start = p.start;
      dest = p.dest;
      cost = p.cost;
			turn_counter = p.turn_counter;
			turn_visited = p.turn_visited;
			nodes = p.nodes;
			searchFood = p.searchFood;
			searchHill = p.searchHill;
			searchUnseen = p.searchUnseen;
      return *this;
    }

    bool operator>( const Path & rhs ) const
    { return cost > rhs.cost; }
    bool operator<( const Path & rhs ) const
    { return cost < rhs.cost; }

    inline bool operator==( const Location *l ) const 
    { return start == l; }

    bool astar(State &state);

		inline void fill_nodes( t_location_deque &olist ) 
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
		for(t_location_deque::const_iterator 
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
	Ann ann_out;

  calc::t_order orders;

  Bot();

  void playGame();    //plays a single game of Ants

  void makeMoves();   //makes moves for a single turn

	int makeMoves(Location* loc, calc::t_order::iterator& o);

	void sort_all();

	void sort_one( t_location_vector &to_sort_vector, Location & top_left );

	bool preMakeMoves(calc::t_order::iterator &o, Location * loc);
	void postMakeMoves(calc::Path & p, Location * ant);

	void endTurn();     //indicates to the engine that it has made its moves

  Location * closest_food(const Location &loc);
  Location * closest_hill(const Location &loc);
  Location * closest_enemy(const Location &loc);
	Location * closest_ant(const Location &loc);
	Location * closest_ant_for_reverse(const Location &loc);

	inline void fill_suround( t_location_vector &suround, const Location & loc )
	{
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
	}

};

#endif //BOT_H_
