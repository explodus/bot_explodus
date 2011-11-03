#ifndef LOCATION_H_
#define LOCATION_H_

#include <iostream>
#include <stdio.h>
#include <cstdlib>
#include <cmath>
#include <queue>
#include <vector>

/*
struct for representing locations in the grid.
*/
struct Location;

typedef std::deque<Location*> t_location_deque;
typedef std::vector<Location*> t_location_vector;

struct Location
{
  bool isVisible, isWater, isHill, isFood, isDead;
  int ant, hillPlayer;

  int row, col;
  double dist_food;
  double dist_hill;
  double dist_enemy;
  double dist_ant;

  long long cost;
  long long weight;

	Location *prev;

	t_location_vector around;
  
  inline Location() 
    : row(0)
    , col(0)
    , isWater(0)
    , cost(1)
    , weight(10)
		, prev(0)
		, around(4)
  {
    reset();
  }

  inline Location(int r, int c) 
    : row(r)
    , col(c)
    , isWater(0)
    , cost(1)
    , weight(10) 
		, prev(0)
		, around(4)
  {
    reset();
  }

  inline Location(const Location& l) 
    : isVisible(l.isVisible)
    , isWater(l.isWater)
    , isHill(l.isHill)
    , isFood(l.isFood)
    , isDead(l.isDead)
    , ant(l.ant)
    , hillPlayer(l.hillPlayer)
    , row(l.row)
    , col(l.col)
    , dist_food(l.dist_food)
    , dist_hill(l.dist_hill)
    , cost(l.cost)
    , weight(l.weight) 
		, prev(l.prev)
		, around(l.around)
  {
  }

  inline void reset()
  {
    isVisible = isHill = isFood = isDead = 0;
    ant = hillPlayer = -1;
		cost = 1;
    weight = isWater? 99: 10;
		prev = 0;
  }

  inline Location& operator=( const Location &l ) 
  { 
    isVisible = l.isVisible;
    isWater = l.isWater;
    isHill = l.isHill;
    isFood = l.isFood;
    isDead = l.isDead;
    ant = l.ant;
    hillPlayer = l.hillPlayer;
    row = l.row;
    col = l.col;
    dist_food = l.dist_food;
    dist_hill = l.dist_hill;
    row = l.row;
    col = l.col;
    isWater = l.isWater;
    cost = l.cost;
    weight = l.weight;
		prev = l.prev;
		around = l.around;
    return *this;
  }

  inline long long weightcosts() const
  { return cost+weight; }

	inline bool operator==( const Location &l ) const 
	{ return (row == l.row && col == l.col); }

	inline bool operator>( const Location & rhs ) const
	{ return weightcosts() > rhs.weightcosts(); }
	inline bool operator<( const Location & rhs ) const
	{ return weightcosts() < rhs.weightcosts(); }
};

inline std::ostream& operator<<(std::ostream &os, const Location &l)
{
  os << '(' << l.row << ',' << l.col << ")";
  return os;
}

inline int manhattan_method(const Location & l1, const Location & l2)
{ return 10 * (abs(l1.row - l2.row) + abs(l1.col - l2.col)); }

#endif //LOCATION_H_
