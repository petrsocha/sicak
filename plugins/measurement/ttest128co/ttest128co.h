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
* \file ttest128co.h
*
* \brief SICAK CPA measurement scenario plugin: sends key command (0x01), followed by the cipher key, followed by N times {encrypt command (0x02) followed by 16 bytes of either random or constant data}. Makes use of multiple captures per oscilloscope run (e.g. Picoscope's rapid block mode).
*
*
* \author Petr Socha
* \version 1.0
*/

#ifndef TTEST128CO_H
#define TTEST128CO_H 

#include <QObject>
#include <QtPlugin>
#include <QString>
#include "measurement.h"
#include "exceptions.hpp"


/**
* \class TTest128CO
* \ingroup Measurement
*
* \brief CPA Measurement scenario plugin: sends key command (0x01), followed by the cipher key, followed by N times {encrypt command (0x02) followed by 16 bytes of either random or constant data}. Makes use of multiple captures per oscilloscope run (e.g. Picoscope's rapid block mode).
*
*/
class TTest128CO : public QObject, Measurement {
    
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "cz.cvut.fit.Sicak.MeasurementInterface/1.0" FILE "ttest128co.json")
    Q_INTERFACES(Measurement)
                
public:
    
    TTest128CO();
    virtual ~TTest128CO() override;
    
    virtual QString getPluginName() override;
    virtual QString getPluginInfo() override;
    
    virtual void init(const char * param) override;
    virtual void deInit() override;
    
    virtual void run(const char * measurementId, size_t measurements, Oscilloscope * oscilloscope, CharDevice * charDevice) override;
    
protected:
    int m_channel;    
    
};

#endif /* TTEST128CO_H */
 
