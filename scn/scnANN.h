/*
 * Sentinel Chicken Networks Artifical Neural Network Library (scnANNlib)
 * scnANN.h -- Header file for Artificial Neural Network class
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
 * $Id: scnANN.h,v 1.8 2001/09/23 17:59:50 tmorgan Exp $
 */

#ifndef SCNANN_H
#define SCNANN_H
#include "scnLayer.h"

/**
 * The class for creating network objects.  
 * With just a few start up parameters, entire networks can be initialized as
 * class instances and then trained using a simple API.
 *
 */
class scnANN
{
 public:
  
  /** 
   * scnANN constructor. 
   * Creates an instance of an scnANN object, fully initialized and allocated.
   *
   * @param inputSize the number of inputs the network takes 
   *                  (sometimes referred to as the input layer size)
   * @param outputSize the number of activations of the output layer
   * @param numHiddenLayers the number of hidden layers
   * @param hiddenLayerSizes the sizes of hidden layers starting with those 
   *                         nearest the output layer
   * @param randomRangeHigh randomized initial weights will not be above this 
   *                        number
   * @param randomRangeLow randomized initial weights will not be below this 
   *                       number
   * @param learningCoefficient the rate at which the network learns
   * @param momentumCoefficient how much the network smoothes erratic weight 
   *                            adjustments
   * @see ~scnANN()
   */
  scnANN(int inputSize,
	int outputSize,
	int numHiddenLayers,
	const int hiddenLayerSizes[],   // From top downward
	double randomRangeHigh,
	double randomRangeLow,
	double learningCoefficient, 
	double momentumCoefficient);

  
  /**
   * scnANN destructor. Deallocates all layers and nodes in network.
   *
   * @see scnANN()
   */
  ~scnANN();


  /**
   * Inspector for number of network inputs.
   *
   * @return Number of inputs the network requires.
   * @see scnANN()
   *
   */
  int getNumInputs();

  
  /**
   * Inspector for number of network outputs.
   *
   * @return Number of outputs network will produce.
   * @see scnANN()
   *
   */
  int getNumOutputs();


  /**
   * Inspector for learning coefficient.
   *
   * @return Learning coefficient.
   * @see scnANN()
   *
   */
  double getLearningCoefficient();

  
  /**
   * Inspector for momentum coefficient.
   *
   * @return Momentum coefficient
   * @see scnANN()
   *
   */
  double getMomentumCoefficient();


  /**
   * Generates a new set of activation outputs from hidden layer calculations.
   * Output can be retrieved through function getOutput().
   *
   * @param input a pointer to an array of inputs
   * @see getOutput
   *
   */
  void processInput(const double* input);

  
  /**
   * Teaches network new tricks.
   * Adjusts network weights according to differences between ideal outputs 
   * and previous network output (from processInput()).
   *  
   * @param idealDiffs the resulting array of differences from subtracting 
   *                   previous network outputs from ideal outputs element wise.
   * @see processInput()
   * @see trainNetwork()
   *
   */
  void backPropagate(const double* idealDiffs);


  /**
   * Allows access to one activation in the array of output activations.  
   * Output activations are created by processInput().
   *
   * @param i the activation desired
   * @return the ith activation
   * @see processInput()
   * 
   */
  double getOutput(int i);

  
  /**
   * Trains network on one input/idealOutput training set.
   * Calculates the differences between the previous network outputs and given 
   * ideal outputs (element wise) and stores them in an array.  
   * It then calls backPropagate on this resulting array input.
   * 
   * @param input
   * @param idealOutput 
   * @see processInput()
   * @see backPropagate()
   * @see trainNetworkMultiple()
   *
   */
  void trainNetwork(const double* input, 
		    const double* idealOutput);


  /**
   * Tests the network's accuracy on one set of data.
   * First runs a processInput() on the given input, and then calculates an 
   * error measure based upon differences between the resulting output and 
   * the idealOutput.
   *
   * @param input 
   * @param idealOutput 
   * @return
   * @see processInput()
   * @see testNetworkMultiple()
   *
   */
  double testNetwork(const double* input,
		     const double* idealOutput);


  /**
   * Runs trainNetwork() many times on the arrays inputs and idealOutputs.
   * 
   * @param setSize number sets of data in arrays
   * @param inputs 
   * @param idealOutputs
   * @see trainNetwork()
   *
   */
  void trainNetworkMultiple(int setSize,
			    const double** inputs,
			    const double** idealOutputs);


  /**
   * Runs testNetwork() many times on the arrays inputs and idealOutputs.
   * 
   * @param setSize number sets of data in arrays
   * @param inputs 
   * @param idealOutputs
   * @return average error from all testNetwork() runs.
   * @see testNetwork()
   *
   */
  double testNetworkMultiple(int setSize,
			     const double** inputs,
			     const double** idealOutputs);

 
 private:
  int numInputs;
  int numOutputs;
  scnLayer** layers;
  int numLayers;
  double learningRate;
  double momentumRate;
  double* lastInput;
};

#endif
