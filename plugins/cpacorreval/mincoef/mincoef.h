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
* \file mincoef.h
*
* \brief SICAK CPA correlation matrix evaluation plugin: minimum coefficient
*
*
* \author Petr Socha
* \version 1.0
*/

#ifndef MINCOEF_H
#define MINCOEF_H 

#include <QObject>
#include <QtPlugin>
#include "cpacorreval.h"
#include "exceptions.hpp"

/**
* \class MinCoef
* \ingroup CpaCorrEval
*
* \brief CPA correlation matrix evaluation SICAK CpaCorrEval plugin, looking for minimum value of coefficient
*
*/
class MinCoef : public QObject, CpaCorrEval {
    
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "cz.cvut.fit.Sicak.CpaCorrEvalInterface/1.0" FILE "mincoef.json")
    Q_INTERFACES(CpaCorrEval)
                
public:
    
    MinCoef();
    virtual ~MinCoef() override;
    
    virtual QString getPluginName() override;
    virtual QString getPluginInfo() override;
    
    virtual void init(const char * param) override;
    virtual void deInit() override;
    
    virtual void evaluateCorrelations(MatrixType<double> & correlationMatrix, size_t & sample, size_t & keyCandidate) override;
    
    
};

#endif /* MINCOEF_H */
 
