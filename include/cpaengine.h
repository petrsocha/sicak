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
* \file cpaengine.h
*
* \brief CPA computation plugin interface for use e.g. in stan
*
*
* \author Petr Socha
* \version 1.1
*/

#ifndef CPAENGINE_H
#define CPAENGINE_H

#include <QString>
#include "types_power.hpp"
#include "types_stat.hpp"

/**
* \class CpaEngine
* \ingroup SicakInterface
*
* \brief CPA computation engine QT plugin interface
*
*/
class CpaEngine {        
    
public:

    virtual ~CpaEngine() {}

    /// Plugin name
    virtual QString getPluginName() = 0;
    /// Plugin info
    virtual QString getPluginInfo() = 0;
    
    /// Initialize the CPA computation engine with specified parameters
    virtual void init(int platform, int device, size_t noOfTraces, size_t samplesPerTrace, size_t noOfCandidates, const char * param) = 0;
    /// Deinitialize the CPA engine
    virtual void deInit() = 0;
    
    /// Query for devices available
    virtual QString queryDevices() = 0;
        
    /// When constTraces is set, the engine assumes the PowerTraces object is same in every call to createContext function and does not change between calls
    virtual void setConstTraces(bool constTraces = false) = 0;
    
    /// Create a CPA computation context based on given power traces and power predictions
    virtual Moments2DContext<double> createContext(const PowerTraces<int16_t> & powerTraces, const PowerPredictions<uint8_t> & powerPredictions) = 0;    
    /// Merge the two CPA contexts, stores the result in the first of the contexts
    virtual void mergeContexts(Moments2DContext<double> & firstAndOut, const Moments2DContext<double> & second) = 0;
    /// Compute correlation matrix based on given context
    virtual Matrix<double> finalizeContext(const Moments2DContext<double> & context) = 0;
    
};        

#define CpaEngine_iid "cz.cvut.fit.Sicak.CpaEngineInterface/1.1"

Q_DECLARE_INTERFACE(CpaEngine, CpaEngine_iid)


#endif /* CPAENGINE_H */
