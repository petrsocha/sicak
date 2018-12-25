/*
*  SICAK - SIde-Channel Analysis toolKit
*  Copyright (C) 2018 Petr Socha, FIT, CTU in Prague
*
*  This program is free software: you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation, either version 3 of the License, or
*  (at your option) any later version.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

/**
* \file localcpa.cpp
*
* \brief SICAK CPA computation engine plugin, local cpu
*
*
* \author Petr Socha
* \version 1.0
*/

#include "localcpa.h"

LocalCPA::LocalCPA() {
    
}

LocalCPA::~LocalCPA() {
    
}

QString LocalCPA::getPluginName() {
    return "First Order Univariate CPA";
}

QString LocalCPA::getPluginInfo() {
    return "Computes first order univariate correlation power analysis from power traces and power predictions";
}

void LocalCPA::init(int platform, int device, size_t noOfTraces, size_t samplesPerTrace, size_t noOfCandidates, const char * param) {
    Q_UNUSED(platform);
    Q_UNUSED(device);
    Q_UNUSED(noOfTraces);
    Q_UNUSED(samplesPerTrace);
    Q_UNUSED(noOfCandidates);
    Q_UNUSED(param);
    return;
}

void LocalCPA::deInit() {
    return;
}

QString LocalCPA::queryDevices() {
    return "    * Platform ID: '0', name: 'localcpu'\n        * Device ID: '0', name: 'localcpu'\n";
}
    
void LocalCPA::setConstTraces(bool constTraces){
    Q_UNUSED(constTraces);
    return;
}
    
UnivariateContext<double> LocalCPA::createContext(const PowerTraces<int16_t> & powerTraces, const PowerPredictions<uint8_t> & powerPredictions) {
    
    // Create an empty context    
    UnivariateContext<double> context(powerTraces.samplesPerTrace(), powerPredictions.noOfCandidates(), 1, 2, 1);
    context.reset();
    // Compute context (covariance, variances and means)
    UniFoCpaAddTraces(context, powerTraces, powerPredictions);
    return context;
    
}

void LocalCPA::mergeContexts(UnivariateContext<double> & firstAndOut, const UnivariateContext<double> & second) {
    
    UniFoCpaMergeContexts(firstAndOut, second);
    
}

Matrix<double> LocalCPA::finalizeContext(const UnivariateContext<double> & context) {
 
    Matrix<double> correlations;    
    UniFoCpaComputeCorrelationMatrix(context, correlations);
    return correlations;
    
}
