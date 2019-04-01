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
* \file hottest.cpp
*
* \brief SICAK Higher-Order t-test computation engine plugin, local cpu
*
*
* \author Petr Socha
* \version 1.2
*/

#include "hottest.h"

HOTTest::HOTTest(): m_order(1) {
    
}

HOTTest::~HOTTest() {
    
}

QString HOTTest::getPluginName() {
    return "Higher-Order Non-Specific Univariate Welch's t-test, use --param=\"order=N\"";
}

QString HOTTest::getPluginInfo() {
    return "Computes arbitrary-order univariate Welch's t-test from random data power traces and constant data power traces. Use --param=\"order=N\" to set the order of the attack, default is N=1.";
}

void HOTTest::init(int platform, int device, size_t noOfTracesRandom, size_t noOfTracesConst, size_t samplesPerTrace, const char * param) {
    Q_UNUSED(platform);
    Q_UNUSED(device);
    Q_UNUSED(noOfTracesRandom);
    Q_UNUSED(noOfTracesConst);
    Q_UNUSED(samplesPerTrace);
    
    QStringList params = QString(param).split(";");
    QString paramVal;
    
    int order = 0;
    
    for (int i = 0; i < params.size(); ++i){ // iterate thru all parameters
        
         if(params.at(i).startsWith("order=")){
             
             paramVal = params.at(i);
             paramVal.remove(0,6);
             
             order = paramVal.toInt();
             if(order <= 0) throw RuntimeException("Invalid order param");                          
             
         }
    
    }
    
    if(!order){
        // using default order
        order = 1;
    }
    
    m_order = order;
    
    return;
}

void HOTTest::deInit() {
    return;
}

QString HOTTest::queryDevices() {
    return "    * Platform ID: '0', name: 'localcpu'\n        * Device ID: '0', name: 'localcpu'\n";
}    
    
Moments2DContext<double> HOTTest::createContext(const PowerTraces<int16_t> & randTraces, const PowerTraces<int16_t> & constTraces) {
    
    // Create an empty context    
    Moments2DContext<double> context(randTraces.samplesPerTrace(), constTraces.samplesPerTrace(), 1, 1, 2 * m_order, 2 * m_order, 0);    
    context.reset();
    // Compute context (covariance, variances and means)
    UniHoTTestAddTraces(context, randTraces, constTraces, m_order);
    return context;
    
}

void HOTTest::mergeContexts(Moments2DContext<double> & firstAndOut, const Moments2DContext<double> & second) {
    
    UniHoTTestMergeContexts(firstAndOut, second);
    
}

Matrix<double> HOTTest::finalizeContext(const Moments2DContext<double> & context) {
 
    Matrix<double> tValsDegs;
    UniHoTTestComputeTValsDegs(context, tValsDegs, m_order);
    return tValsDegs;
    
}
