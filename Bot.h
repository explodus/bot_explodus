#ifndef BOT_H_
#define BOT_H_

#include "State.h"
#include <queue>
#include <vector>
#include <list>
#include <limits>
#ifdef _WIN64
# include <memory>
#else
# include <tr1/memory>
#endif 

namespace calc
{
  class Vertex
  {
	public:
		Location *loc;
		std::tr1::shared_ptr<Vertex> prev;

    inline Vertex(Location *l = 0) 
      : loc(l), prev()
    {
		}

    inline Vertex(const Vertex &v) 
      : loc(v.loc), prev(v.prev)
    { }

		inline unsigned int weightcosts() const
		{ return (loc!=0) ? loc->weightcosts() : 2; }

    inline bool operator>( const Vertex & rhs ) const
    { return weightcosts() > rhs.weightcosts(); }
    inline bool operator<( const Vertex & rhs ) const
    { return weightcosts() < rhs.weightcosts(); }
    inline bool operator==( const Vertex& rhs ) const 
    { return (*loc == *rhs.loc); }
		inline bool operator==( const Location *l ) const 
		{ return *loc == *l; }

  };

  struct Path
  {
    Vertex start;
    Vertex dest;

    unsigned int cost;
		unsigned int turn_counter;

    typedef std::deque<Vertex> t_vertex;
    t_vertex nodes;

    static int astar_break;

    Path() 
      : start( 0 )
      , dest( 0 )
      , cost(std::numeric_limits<unsigned int>::max())
			, turn_counter(0)
    {}

    Path( 
        const Vertex & s
      , const Vertex & d
      , State &state )
      : start( s )
      , dest( d )
      , cost(std::numeric_limits<unsigned int>::max())
			, turn_counter(0)
    {
      if (start.loc && dest.loc && astar(state))
      {
        cost = 0;
        for (t_vertex::const_iterator 
            itb(nodes.begin())
          , ite(nodes.end())
          ; itb != ite
          ; ++itb)
          cost += itb->weightcosts();
      }
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
  };

  typedef std::vector<Path> t_order;
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
