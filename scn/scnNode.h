/*
 * Sentinel Chicken Networks Artifical Neural Network Library (scnANNlib) 
 * scnNode.h  --  Header file for node unit class
 *
 * Copyright (C) 2001  Timothy D. Morgan
 *
 * This file is a part of scnANNlib
 *
 * scnANNlib is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * scnANNlib is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with scnANNlib; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * For more information on scnANNlib, visit http://www.sentinelchicken.org
 * For questions about this license, write to scnannlib@sentinelchicken.net
 * For questions on how to use this library, please read the documentation
 * before bothering the development team.
 * 
 * $Id: scnNode.h,v 1.3 2001/09/23 17:52:43 tmorgan Exp $
 */

#ifndef SCNNODE_H
#define SCNNODE_H

#include <iostream>
#include <math.h>
#include <stdlib.h>
#include <time.h>

// Prototype this here since layer and node classes are 
// interdependent
class scnLayer;

double sigmoid(double x);

class scnNode
{
 public:
  scnNode(int numChildNodes, scnLayer* childLayer,
	  double rangeHigh, double rangeLow); 
  ~scnNode();

  double getCurrentActivation();
  double getCurrentDelta();
  double getChildWeight(int i);
  
  void calculateNewActivation(const double* input = NULL);
  void updateDelta(double pseudoError);
  void backPropagate(double learningRate,
		     double momentumRate,
		     const double* lastInput = NULL);

 private:
  int numInputs;
  double currentActivation;
  double currentDelta;
  double* weightVector;
  double* oldWeightDiffs;
  scnLayer* inputLayer;
};

#include "scnLayer.h"

#endif
