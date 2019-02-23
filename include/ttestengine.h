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
* \file ttestengine.h
*
* \brief t-test computation plugin interface for use e.g. in stan
*
*
* \author Petr Socha
* \version 1.1
*/

#ifndef TTESTENGINE_H
#define TTESTENGINE_H

#include <QString>
#include "types_power.hpp"
#include "types_stat.hpp"

/**
* \class TTestEngine
* \ingroup SicakInterface
*
* \brief t-test computation engine QT plugin interface
*
*/
class TTestEngine {        
    
public:

    virtual ~TTestEngine() {}

    /// Plugin name
    virtual QString getPluginName() = 0;
    /// Plugin info
    virtual QString getPluginInfo() = 0;
    
    /// Initialize the CPA computation engine with specified parameters
    virtual void init(int platform, int device, size_t noOfTracesRandom, size_t noOfTracesConst, size_t samplesPerTrace, const char * param) = 0;
    /// Deinitialize the plugin
    virtual void deInit() = 0;
    
    /// Query available devices
    virtual QString queryDevices() = 0;
        
    /// Create a t-test computation context based on given random and constant power traces
    virtual Moments2DContext<double> createContext(const PowerTraces<int16_t> & randTraces, const PowerTraces<int16_t> & constTraces) = 0;    
    /// Merge the two t-test contexts, stores the result in the first of the contexts
    virtual void mergeContexts(Moments2DContext<double> & firstAndOut, const Moments2DContext<double> & second) = 0;
    /// Compute t-values (stored in first row) and degrees of freedom (second row) based on the given context
    virtual Matrix<double> finalizeContext(const Moments2DContext<double> & context) = 0;
};        

#define TTestEngine_iid "cz.cvut.fit.Sicak.TTestInterface/1.1"

Q_DECLARE_INTERFACE(TTestEngine, TTestEngine_iid)


#endif /* TTESTENGINE_H */
