/*
*  SICAK - SIde-Channel Analysis toolKit
*  Copyright (C) 2018-2019 Petr Socha, FIT, CTU in Prague
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
* \file localttest.cpp
*
* \brief SICAK t-test computation engine plugin, local cpu
*
*
* \author Petr Socha
* \version 1.1
*/

#include "localttest.h"

LocalTTest::LocalTTest() {
    
}

LocalTTest::~LocalTTest() {
    
}

QString LocalTTest::getPluginName() {
    return "First Order Non-Specific Univariate Welch's t-test";
}

QString LocalTTest::getPluginInfo() {
    return "Computes first order univariate Welch's t-test from random data power traces and constant data power traces";
}

void LocalTTest::init(int platform, int device, size_t noOfTracesRandom, size_t noOfTracesConst, size_t samplesPerTrace, const char * param) {
    Q_UNUSED(platform);
    Q_UNUSED(device);
    Q_UNUSED(noOfTracesRandom);
    Q_UNUSED(noOfTracesConst);
    Q_UNUSED(samplesPerTrace);
    Q_UNUSED(param);
    return;
}

void LocalTTest::deInit() {
    return;
}

QString LocalTTest::queryDevices() {
    return "    * Platform ID: '0', name: 'localcpu'\n        * Device ID: '0', name: 'localcpu'\n";
}    
    
Moments2DContext<double> LocalTTest::createContext(const PowerTraces<int16_t> & randTraces, const PowerTraces<int16_t> & constTraces) {
    
    // Create an empty context    
    Moments2DContext<double> context(randTraces.samplesPerTrace(), constTraces.samplesPerTrace(), 1, 1, 2, 2, 0);
    context.reset();
    // Compute context (covariance, variances and means)
    UniFoTTestAddTraces(context, randTraces, constTraces);
    return context;
    
}

void LocalTTest::mergeContexts(Moments2DContext<double> & firstAndOut, const Moments2DContext<double> & second) {
    
    UniFoTTestMergeContexts(firstAndOut, second);
    
}

Matrix<double> LocalTTest::finalizeContext(const Moments2DContext<double> & context) {
 
    Matrix<double> correlations;
    UniFoTTestComputeTValsDegs(context, correlations);
    return correlations;
    
}
