#ifndef SQUARE_H_
#define SQUARE_H_

#include <vector>
#include "Location.h"

/*
struct for representing a square in the grid.
*/
struct Square
{
  Location loc;

  inline Square() 
  { reset(); }

  //resets the information for the square except water information
  inline void reset()
  { loc.reset(); }
};

#endif //SQUARE_H_
