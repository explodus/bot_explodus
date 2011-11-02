/*
 * Sentinel Chicken Networks Artifical Neural Network Library (scnANNlib)
 * scnLayer.cc -- Main code file for the layer class
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
 * $Id: scnLayer.cc,v 1.3 2001/09/23 17:52:43 tmorgan Exp $
 */


#include "scnLayer.h"

scnLayer::scnLayer(int layerSize, int inputSize,
		   scnLayer* belowLayer,
		   double rangeHigh, double rangeLow, 
		   bool isBottomLayer /*= false*/)
{
  numNodes = layerSize;
  numInputs = inputSize;
  isBottom = isBottomLayer;
  // Used for backpropagation
  nextError = new double[numInputs];

  // Create the nodes for this layer
  nodes = new scnNode*[numNodes];
  for(int i = 0; i < numNodes; i++)
    nodes[i] = new scnNode(numInputs, belowLayer, 
			   rangeHigh, rangeLow);
}


scnLayer::~scnLayer()
{
  for(int i = 0; i < numNodes; i++)
    delete nodes[i];
  delete nodes;
  delete nextError;
}


double scnLayer::getActivation(int i)
{
  return nodes[i]->getCurrentActivation();
}


int scnLayer::getSize()
{
  return numNodes;
}


double* scnLayer::getPseudoError()
{
  return nextError;
}


void scnLayer::calculateActivations(const double* belowActivations /*= NULL*/)
{
  // Calculate each node's activation.
  for(int i = 0; i < numNodes; i++)
    nodes[i]->calculateNewActivation(belowActivations);
}


void scnLayer::backPropagate(const double* aboveError, double learningRate, 
			     double momentumRate, const double* lastInput/*=NULL*/)
{
  int i, j;

  // Train each node's delta
  for(i = 0; i < numNodes; i++)
      nodes[i]->updateDelta(aboveError[i]);
  
  // Call next layer's backPropagate if this is not
  // the bottom calculation layer.
  if (!isBottom)
  {
    // First, calculate the error amounts to be passed
    // to the next layer.
    for(i = 0; i < numInputs; i++)
    {
      nextError[i] = 0.0;
      for(j = 0; j < numNodes; j++)
      {
	nextError[i]
	  += nodes[j]->getCurrentDelta() 
	  * nodes[j]->getChildWeight(i);
      }
    }
  }

  // Train each node from delta
  for(i = 0; i < numNodes; i++)
    nodes[i]->backPropagate(learningRate, momentumRate, lastInput);
}
