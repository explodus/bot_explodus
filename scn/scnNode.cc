/*
 * Sentinel Chicken Networks Artifical Neural Network Library (scnANNlib)
 * scnNode.cc -- Main code file for the node class
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
 * $Id: scnNode.cc,v 1.3 2001/09/23 17:52:43 tmorgan Exp $
 */


#include "scnNode.h"
#include <iostream>

#ifdef _WIN32
#define random rand
#define _USE_MATH_DEFINES
#endif // _WIN32

#include <math.h>

using namespace std;


double sigmoid(double x)
{
  return 1/(1+pow(M_E,0-x));
}


scnNode::scnNode(int numChildNodes, scnLayer* childLayer,
		 double rangeHigh, double rangeLow)
{
  numInputs = numChildNodes;
  inputLayer = childLayer;
  weightVector = new double[numInputs];
  oldWeightDiffs = new double[numInputs];
  
  // Randomize weight vector
  for(int i = 0; i < numInputs; i++)
  {
    weightVector[i] 
      = ((double)random() / 2147483647.0) 
      * (rangeHigh - rangeLow) + rangeLow;
  }
}


scnNode::~scnNode()
{
  delete weightVector;
  delete oldWeightDiffs;
}


double scnNode::getCurrentActivation()
{
  return currentActivation;
}


double scnNode::getCurrentDelta()
{
  return currentDelta;
}


double scnNode::getChildWeight(int i)
{
  return weightVector[i];
}


void scnNode::calculateNewActivation(const double* input /*= NULL*/)
{
  double tempSum = 0;
  if(input != NULL)
    for(int i = 0; i < numInputs; i++)
      tempSum += weightVector[i]*input[i];
  else if (inputLayer != NULL)
    for(int i = 0; i < numInputs; i++)
      tempSum += weightVector[i]*(inputLayer->getActivation(i));
  else
    {
      cerr << "Fatal error in scnNode::calculateNewActivation()"
	   << ": No below layer and no input." << endl;
      exit(2);
    }

  currentActivation = sigmoid(tempSum);
}


void scnNode::updateDelta(double pseudoError)
{
  currentDelta =
    currentActivation * (1-currentActivation) * pseudoError;
}


void scnNode::backPropagate(double learningRate,
			    double momentumRate,
			    const double* lastInput /*= NULL*/)
{
  double curWeightDiff;
  if(inputLayer != NULL)
    for(int i = 0; i < numInputs; i++)
    {
      curWeightDiff 
	= (learningRate * currentDelta * (inputLayer->getActivation(i))) 
	+ (momentumRate * oldWeightDiffs[i]);
      oldWeightDiffs[i] = curWeightDiff;
      weightVector[i] += curWeightDiff;
    }
  else if(lastInput != NULL)
    for(int i = 0; i < numInputs; i++)
    {
      curWeightDiff
        = (learningRate * currentDelta * lastInput[i])
        + (momentumRate * oldWeightDiffs[i]);
      oldWeightDiffs[i] = curWeightDiff;
      weightVector[i] += curWeightDiff;
    }
  else
  {
    cerr << "Fatal error in scnNode::backPropagate()"
	 << ": No below layer and no input." << endl;
    exit(2);
  }
}
