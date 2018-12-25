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
* \file localcpa.h
*
* \brief SICAK CPA computation engine plugin, local cpu
*
*
* \author Petr Socha
* \version 1.0
*/

#ifndef LOCALCPA_H
#define LOCALCPA_H 

#include <QObject>
#include <QtPlugin>
#include "cpaengine.h"
#include "exceptions.hpp"
#include "ompcpa.hpp"

/**
* \class LocalCPA
* \ingroup CpaEngine
*
* \brief CPA context computation SICAK CpaEngine plugin
*
*/
class LocalCPA : public QObject, CpaEngine {
    
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "cz.cvut.fit.Sicak.CpaEngineInterface/1.0" FILE "localcpa.json")
    Q_INTERFACES(CpaEngine)
        
public:
    
    LocalCPA();
    virtual ~LocalCPA() override;
    
    virtual QString getPluginName() override;
    virtual QString getPluginInfo() override;
    
    virtual void init(int platform, int device, size_t noOfTraces, size_t samplesPerTrace, size_t noOfCandidates, const char * param) override;
    virtual void deInit() override;
    
    virtual QString queryDevices() override;
        
    virtual void setConstTraces(bool constTraces = false) override;
    
    virtual UnivariateContext<double> createContext(const PowerTraces<int16_t> & powerTraces, const PowerPredictions<uint8_t> & powerPredictions) override;    
    virtual void mergeContexts(UnivariateContext<double> & firstAndOut, const UnivariateContext<double> & second) override;
    virtual Matrix<double> finalizeContext(const UnivariateContext<double> & context) override;
            
};

#endif /* LOCALCPA_H */
 
