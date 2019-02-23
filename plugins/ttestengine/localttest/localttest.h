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
* \file localttest.h
*
* \brief SICAK t-test computation engine plugin, local cpu
*
*
* \author Petr Socha
* \version 1.1
*/

#ifndef LOCALTTEST_H
#define LOCALTTEST_H 

#include <QObject>
#include <QtPlugin>
#include "ttestengine.h"
#include "exceptions.hpp"
#include "ompttest.hpp"

/**
* \class LocalTTest
* \ingroup TTestEngine
*
* \brief t-test context computation SICAK TTestEngine plugin
*
*/
class LocalTTest : public QObject, TTestEngine {
    
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "cz.cvut.fit.Sicak.TTestInterface/1.1" FILE "localttest.json")
    Q_INTERFACES(TTestEngine)
        
public:
    
    LocalTTest();
    virtual ~LocalTTest() override;
    
    virtual QString getPluginName() override;
    virtual QString getPluginInfo() override;
    
    virtual void init(int platform, int device, size_t noOfTracesRandom, size_t noOfTracesConst, size_t samplesPerTrace, const char * param) override;
    virtual void deInit() override;
    
    virtual QString queryDevices() override;
        
    virtual Moments2DContext<double> createContext(const PowerTraces<int16_t> & randTraces, const PowerTraces<int16_t> & constTraces) override;    
    virtual void mergeContexts(Moments2DContext<double> & firstAndOut, const Moments2DContext<double> & second) override;
    virtual Matrix<double> finalizeContext(const Moments2DContext<double> & context) override;
            
};

#endif /* LOCALTTEST_H */
 
