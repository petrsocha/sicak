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
* \file hottest.h
*
* \brief SICAK Higher-Order t-test computation engine plugin, local cpu
*
*
* \author Petr Socha
* \version 1.2
*/

#ifndef HOTTEST_H
#define HOTTEST_H 

#include <QObject>
#include <QtPlugin>
#include "ttestengine.h"
#include "exceptions.hpp"
#include "ompttest.hpp"

/**
* \class HOTTest
* \ingroup TTestEngine
*
* \brief t-test context computation SICAK TTestEngine plugin
*
*/
class HOTTest : public QObject, TTestEngine {
    
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "cz.cvut.fit.Sicak.TTestInterface/1.1" FILE "hottest.json")
    Q_INTERFACES(TTestEngine)
        
public:
    
    HOTTest();
    virtual ~HOTTest() override;
    
    virtual QString getPluginName() override;
    virtual QString getPluginInfo() override;
    
    virtual void init(int platform, int device, size_t noOfTracesRandom, size_t noOfTracesConst, size_t samplesPerTrace, const char * param) override;
    virtual void deInit() override;
    
    virtual QString queryDevices() override;
        
    virtual Moments2DContext<double> createContext(const PowerTraces<int16_t> & randTraces, const PowerTraces<int16_t> & constTraces) override;    
    virtual void mergeContexts(Moments2DContext<double> & firstAndOut, const Moments2DContext<double> & second) override;
    virtual Matrix<double> finalizeContext(const Moments2DContext<double> & context) override;

protected:
    size_t m_order;
    
};

#endif /* HOTTEST_H */
 
