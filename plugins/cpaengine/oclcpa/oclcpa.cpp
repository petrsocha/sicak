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
* \file oclcpa.cpp
*
* \brief SICAK CPA computation engine plugin, OpenCL
*
*
* \author Petr Socha
* \version 1.1
*/

#include "oclcpa.h"

OclCPA::OclCPA(): m_handle(nullptr), m_constTraces(false), m_tracesLoaded(false) {
    
}

OclCPA::~OclCPA() {
    
}

QString OclCPA::getPluginName() {
    return "OpenCL accelerated First Order Univariate CPA";
}

QString OclCPA::getPluginInfo() {
    return "Uses GPU to perform first order univariate correlation power analysis from power traces and power predictions";
}

void OclCPA::init(int platform, int device, size_t noOfTraces, size_t samplesPerTrace, size_t noOfCandidates, const char * param) {
    if(noOfTraces*samplesPerTrace == 0 || noOfTraces*noOfCandidates == 0) throw RuntimeException("Invalid computation parameters (sizes).");
    m_handle = new OclCpaEngine<double, int16_t, uint8_t>(platform, device, samplesPerTrace, noOfCandidates, noOfTraces);
    Q_UNUSED(param);
}

void OclCPA::deInit() {
    if(m_handle)
        delete m_handle;
    m_handle = nullptr;
}

QString OclCPA::queryDevices() {
    return (OclEngine<double>::queryDevices()).c_str();
}
    
void OclCPA::setConstTraces(bool constTraces){
    m_constTraces = constTraces;
    m_tracesLoaded = false;
}    
    
Moments2DContext<double> OclCPA::createContext(const PowerTraces<int16_t> & powerTraces, const PowerPredictions<uint8_t> & powerPredictions) {
    
    if(m_handle == nullptr)
        throw RuntimeException("The Ocl engine needs to be properly initialized first");        
    
    // Load power traces to the GPU
    if(!m_constTraces || !m_tracesLoaded){
        m_handle->loadTracesToDevice(powerTraces);
        m_tracesLoaded = true;
    }
    
    // Load power predictions
    m_handle->loadPredictionsToDevice(powerPredictions);
    
    // If program is not built yet, build it now
    m_handle->buildProgram();
        
    Moments2DContext<double> context;
    
    // Launch the computation
    m_handle->compute(context, 1000); 
    
    return context;
        
}

void OclCPA::mergeContexts(Moments2DContext<double> & firstAndOut, const Moments2DContext<double> & second) {
    // Not a time-demanding operation, use OpenMP version
    UniFoCpaMergeContexts(firstAndOut, second);
}

Matrix<double> OclCPA::finalizeContext(const Moments2DContext<double> & context) {
    Matrix<double> correlations;
    // Not a time-demanding operation, use OpenMP version
    UniFoCpaComputeCorrelationMatrix(context, correlations);
    return correlations;
}
