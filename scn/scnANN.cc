/*
 * Sentinel Chicken Networks Artifical Neural Network Library (scnANNlib)
 * scnANN.cc -- Main code file for the network class
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
 * $Id: scnANN.cc,v 1.3 2001/09/23 17:52:42 tmorgan Exp $
 */


#include <iostream>
#include <stdlib.h>
#include "scnANN.h"

#ifdef _WIN32
#define srandom srand
#endif // _WIN32

scnANN::scnANN(int inputSize,
	       int outputSize,
	       int numHiddenLayers,
	       const int hiddenLayerSizes[],  // From top downward
	       double randomRangeHigh,
	       double randomRangeLow,
	       double learningCoefficient,
	       double momentumCoefficient)
{
  srandom(time(NULL));
  numInputs = inputSize;
  numOutputs = outputSize;
  numLayers = numHiddenLayers + 1;
  layers = new scnLayer*[numLayers];
  lastInput = new double[numInputs];
  learningRate = learningCoefficient;
  momentumRate = momentumCoefficient;

  scnLayer* lastLayerMade = NULL;
  scnLayer* curLayer;
  for(int i = (numHiddenLayers - 1); i >= 0; i--)
  {
    if ((i == 0) && (i == (numHiddenLayers - 1)))
    {
      curLayer = new scnLayer(hiddenLayerSizes[i],
			      numInputs, lastLayerMade,
			      randomRangeHigh,
			      randomRangeLow,
			      true);
    }
    else if (i == (numHiddenLayers - 1))
    {
      curLayer = new scnLayer(hiddenLayerSizes[i],
			      numInputs, lastLayerMade,
			      randomRangeHigh,
			      randomRangeLow,
			      true);
    }
    else
    {    
      curLayer = new scnLayer(hiddenLayerSizes[i],
			      hiddenLayerSizes[i+1], lastLayerMade,
			      randomRangeHigh,
			      randomRangeLow);
    }
    layers[i+1] = curLayer;
    lastLayerMade = curLayer;
  }

  if(numHiddenLayers == 0)
  {
    layers[0] = new scnLayer(outputSize,
			     numInputs, NULL,
			     randomRangeHigh,
			     randomRangeLow);
  }
  else 
  {
    layers[0] = new scnLayer(outputSize,
			     hiddenLayerSizes[0], lastLayerMade,
			     randomRangeHigh,
			     randomRangeLow);
  }
}


scnANN::~scnANN()
{
  // Get rid of temporary array
  delete lastInput;

  // Deallocate layers
  for(int i = numLayers-1; i >=0 ; i--)
    delete (layers[i]);
  delete layers;
}


int scnANN::getNumInputs()
{ return numInputs; }


int scnANN::getNumOutputs()
{ return numOutputs; }


double scnANN::getLearningCoefficient()
{ return learningRate; }


double scnANN::getMomentumCoefficient()
{ return momentumRate; }


void scnANN::processInput(const double* input)
{
  //TODO: Find a way to eliminate this loop without mem leak
  int i;
  for(i = 0; i < numInputs; i++)
    lastInput[i] = input[i];
  layers[numLayers - 1]->calculateActivations(input);
  for(i = numLayers - 2; i >= 0; i--)
    layers[i]->calculateActivations();
}


void scnANN::backPropagate(const double* idealDiffs)
{
  layers[0]->backPropagate(idealDiffs, learningRate, momentumRate); 

  for(int i = 1; i < numLayers - 1; i++)
    layers[i]->backPropagate(layers[i-1]->getPseudoError(), 
			     learningRate, momentumRate);

  layers[numLayers-1]->backPropagate(idealDiffs, learningRate, 
				     momentumRate, lastInput);
}


double scnANN::getOutput(int i)
{
  return layers[0]->getActivation(i);
}


void scnANN::trainNetwork(const double* input,
			  const double* idealOutput)
{
  processInput(input);
  double* diffs = new double[numOutputs];
  for(int i = 0; i < numOutputs; i++)
    diffs[i] = idealOutput[i] - getOutput(i);

  backPropagate(diffs);
  delete diffs;
}


double scnANN::testNetwork(const double* input,
			   const double* idealOutput)
{
  processInput(input);
  double errorSum = 0.0;
  for (int i = 0; i < numOutputs; i++)
  {
    errorSum
      += ((idealOutput[i] - getOutput(i))
	  * (idealOutput[i] - getOutput(i)) / 2);
  }

  return (errorSum / numOutputs);
}


void scnANN::trainNetworkMultiple(int setSize,
				  const double** inputs,
				  const double** idealOutputs)
{
  for(int i = 0; i < setSize; i++)
    trainNetwork(inputs[i], idealOutputs[i]);
}


double scnANN::testNetworkMultiple(int setSize,
				   const double** inputs,
				   const double** idealOutputs)
{
  double errorSum = 0.0;
  for(int i = 0; i < setSize; i++)
    errorSum += testNetwork(inputs[i], idealOutputs[i]);

  return (errorSum / setSize);
}
