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
* \file measurement.h
*
* \brief Measurement scenario plugin interface for use e.g. in meas
*
*
* \author Petr Socha
* \version 1.0
*/

#ifndef MEASUREMENT_H
#define MEASUREMENT_H

#include <QString>
#include "oscilloscope.h"
#include "chardevice.h"

/**
* \class Measurement
* \ingroup SicakInterface
*
* \brief Measurement scenario QT plugin interface
*
*/
class Measurement {        
    
public:

    virtual ~Measurement() {}
    
    /// Plugin name
    virtual QString getPluginName() = 0;
    /// Plugin info
    virtual QString getPluginInfo() = 0;

    /// Initialize the plugin
    virtual void init(const char * param) = 0;
    /// Deinitialize the plugin
    virtual void deInit() = 0;
    
    /// Run the measurement scenario, given either initialized and configured oscilloscope/charDevice pointers or nullptr-s, and creates/saves related files
    virtual void run(const char * measurementId, size_t measurements, Oscilloscope * oscilloscope, CharDevice * charDevice) = 0;
    
};        

#define Measurement_iid "cz.cvut.fit.Sicak.MeasurementInterface/1.0"

Q_DECLARE_INTERFACE(Measurement, Measurement_iid)


#endif /* MEASUREMENT_H */
