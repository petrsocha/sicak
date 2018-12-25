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
* \file blockprocess.h
*
* \brief Block processing plugin interface for use e.g. in prep
*
*
* \author Petr Socha
* \version 1.0
*/

#ifndef BLOCKPROCESS_H
#define BLOCKPROCESS_H

#include <QString>
#include "types_power.hpp"

/**
* \class BlockProcess
* \ingroup SicakInterface
*
* \brief Block data processing QT plugin interface
*
*/
class BlockProcess {        
    
public:

    virtual ~BlockProcess() {}
    
    /// Plugin name
    virtual QString getPluginName() = 0;
    /// Plugin info
    virtual QString getPluginInfo() = 0;
    
    /// Initialize the plugin
    virtual void init(const char * param) = 0;
    /// Deinitialize the plugin
    virtual void deInit() = 0;
    
    /// Process data and create/save related output files
    virtual void processBlockData(MatrixType<uint8_t> & data, const char * id) = 0;    
    
};        

#define BlockProcess_iid "cz.cvut.fit.Sicak.BlockProcessInterface/1.0"

Q_DECLARE_INTERFACE(BlockProcess, BlockProcess_iid)


#endif /* BLOCKPROCESS_H */
