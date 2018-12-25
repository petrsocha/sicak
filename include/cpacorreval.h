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
* \file cpacorreval.h
*
* \brief Correlation matrix evaluation plugin interface for use e.g. in correv
*
*
* \author Petr Socha
* \version 1.0
*/

#ifndef CPACORREVAL_H
#define CPACORREVAL_H

#include <QString>
#include "types_basic.hpp"

/**
* \class CpaCorrEval
* \ingroup SicakInterface
*
* \brief CPA correlation matrix evaluation QT plugin interface
*
*/
class CpaCorrEval {        
    
public:

    virtual ~CpaCorrEval() {}

    /// Plugin name
    virtual QString getPluginName() = 0;
    /// Plugin info
    virtual QString getPluginInfo() = 0;
    
    /// Initialize the plugin
    virtual void init(const char * param) = 0;
    /// Deinitialize the plugin
    virtual void deInit() = 0;
    
    /// Evaluate the correlation matrix, save the results in sample and keyCandidate
    virtual void evaluateCorrelations(MatrixType<double> & correlationMatrix, size_t & sample, size_t & keyCandidate) = 0;    
    
};        

#define CpaCorrEval_iid "cz.cvut.fit.Sicak.CpaCorrEvalInterface/1.0"

Q_DECLARE_INTERFACE(CpaCorrEval, CpaCorrEval_iid)


#endif /* CPACORREVAL_H */
