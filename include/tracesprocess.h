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
* \file tracesprocess.h
*
* \brief Traces processing plugin interface for use e.g. in prep
*
*
* \author Petr Socha
* \version 1.0
*/

#ifndef TRACESPROCESS_H
#define TRACESPROCESS_H

#include <QString>
#include "types_power.hpp"

/**
* \class TracesProcess
* \ingroup SicakInterface
*
* \brief Traces processing QT plugin interface
*
*/
class TracesProcess {        
    
public:

    virtual ~TracesProcess() {}

    /// Plugin name
    virtual QString getPluginName() = 0;
    /// Plugin info
    virtual QString getPluginInfo() = 0;
    
    /// Initialize the plugin
    virtual void init(const char * param) = 0;
    /// Deinitialize the plugin
    virtual void deInit() = 0;
    
    /// Process data and create/save related output files
    virtual void processTraces(PowerTraces<int16_t> & traces, const char * id) = 0;    
    
};        

#define TracesProcess_iid "cz.cvut.fit.Sicak.TracesProcessInterface/1.0"

Q_DECLARE_INTERFACE(TracesProcess, TracesProcess_iid)


#endif /* TRACESPROCESS_H */
