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
* \file hocpa.cpp
*
* \brief SICAK Higher-Order CPA computation engine plugin, local cpu
*
*
* \author Petr Socha
* \version 1.2
*/

#include "hocpa.h"

HOCPA::HOCPA(): m_order(1) {
    
}

HOCPA::~HOCPA() {
    
}

QString HOCPA::getPluginName() {
    return "Higher-Order Univariate CPA, use --param=\"order=N\"";
}

QString HOCPA::getPluginInfo() {
    return "Computes arbitrary-order univariate correlation power analysis from power traces and power predictions. Use --param=\"order=N\" to set the order of the attack, default is N=1.";
}

void HOCPA::init(int platform, int device, size_t noOfTraces, size_t samplesPerTrace, size_t noOfCandidates, const char * param) {
    Q_UNUSED(platform);
    Q_UNUSED(device);
    Q_UNUSED(noOfTraces);
    Q_UNUSED(samplesPerTrace);
    Q_UNUSED(noOfCandidates);
    
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

void HOCPA::deInit() {
    return;
}

QString HOCPA::queryDevices() {
    return "    * Platform ID: '0', name: 'localcpu'\n        * Device ID: '0', name: 'localcpu'\n";
}
    
void HOCPA::setConstTraces(bool constTraces){
    Q_UNUSED(constTraces);
    return;
}
    
Moments2DContext<double> HOCPA::createContext(const PowerTraces<int16_t> & powerTraces, const PowerPredictions<uint8_t> & powerPredictions) {
    
    // Create an empty context    
    Moments2DContext<double> context(powerTraces.samplesPerTrace(), powerPredictions.noOfCandidates(), 1, 1, 2 * m_order, 2, m_order);
    context.reset();
    // Compute context
    UniHoCpaAddTraces(context, powerTraces, powerPredictions, m_order);
    return context;
    
}

void HOCPA::mergeContexts(Moments2DContext<double> & firstAndOut, const Moments2DContext<double> & second) {
    
    UniHoCpaMergeContexts(firstAndOut, second);
    
}

Matrix<double> HOCPA::finalizeContext(const Moments2DContext<double> & context) {
 
    Matrix<double> correlations;        
    UniHoCpaComputeCorrelationMatrix(context, correlations, m_order);
    return correlations;
    
}
