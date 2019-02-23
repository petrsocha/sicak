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
* \file oclcpa.h
*
* \brief SICAK CPA computation engine plugin, OpenCL
*
*
* \author Petr Socha
* \version 1.1
*/

#ifndef OCLCPA_H
#define OCLCPA_H 

#include <QObject>
#include <QtPlugin>
#include "cpaengine.h"
#include "exceptions.hpp"
#include "ompcpa.hpp"
#include "oclcpaengine.hpp"

#define CL_USE_DEPRECATED_OPENCL_1_1_APIS
#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif

/**
* \class OclCPA
* \ingroup CpaEngine
*
* \brief CPA context computation SICAK CpaEngine plugin, OpenCL implementation
*
*/
class OclCPA : public QObject, CpaEngine {
    
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "cz.cvut.fit.Sicak.CpaEngineInterface/1.1" FILE "oclcpa.json")
    Q_INTERFACES(CpaEngine)
                
public:
    
    OclCPA();
    virtual ~OclCPA() override;
    
    virtual QString getPluginName() override;
    virtual QString getPluginInfo() override;
    
    virtual void init(int platform, int device, size_t noOfTraces, size_t samplesPerTrace, size_t noOfCandidates, const char * param) override;
    virtual void deInit() override;
    
    virtual QString queryDevices() override;
        
    virtual void setConstTraces(bool constTraces = false) override;
    
    /// Creates context from traces and predictions using GPU. When constTraces=true, the traces are loaded to the GPU device only the first time; predictions are loaded every time
    virtual Moments2DContext<double> createContext(const PowerTraces<int16_t> & powerTraces, const PowerPredictions<uint8_t> & powerPredictions) override;    
    virtual void mergeContexts(Moments2DContext<double> & firstAndOut, const Moments2DContext<double> & second) override;
    virtual Matrix<double> finalizeContext(const Moments2DContext<double> & context) override;
         
protected:
    
    OclCpaEngine<double, int16_t, uint8_t> * m_handle;
    bool m_constTraces;
    bool m_tracesLoaded;
};

#endif /* OCLCPA_H */
 
